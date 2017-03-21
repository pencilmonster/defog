#include "defog.h"
#include <QDebug>
using namespace cv;

const double kernRatio=0.01; //自适应核比例
const double minAtomLigth=220; //最小大气光强
const double wFactor=0.95; //w系数 用来调节
const double min_t =0.1; //最小透射率
DeFog::DeFog()
{

}

DeFog::~DeFog()
{}

Mat DeFog::minRGB(Mat src)
{
    Mat minRgb;
    if(src.empty())
        return minRgb;

    minRgb= Mat::zeros(src.rows,src.cols,CV_8UC1);
    for(int i=0;i<src.rows;i++)
        for(int j=0;j<src.cols;j++)
        {
            uchar g_minvalue =255;
            for(int c=0;c<3;c++)
            {
                if(g_minvalue>src.at<Vec3b>(i,j)[c])
                    g_minvalue=src.at<Vec3b>(i,j)[c];
            }
            minRgb.at<uchar>(i,j)=g_minvalue;
        }
    return minRgb;
}

//最小值滤波
Mat DeFog::minFilter(cv::Mat src, int ksize)
{
    Mat dst;
    //[1] --检测原始图像
    if(src.channels()!=1)
        return dst  ;  //返回空矩阵
    if(src.depth()>8)
        return dst;
    //[1]

    int r=(ksize-1)/2; //核半径

    //初始化目标图像
    dst=Mat::zeros(src.rows,src.cols,CV_8UC1);

    //[3] --最小值滤波
    for(int i=0;i<src.rows;i++)
        for(int j=0;j<src.cols;j++)
        {

            //[1] --初始化滤波核的上下左右边界
            int top=i-r;
            int bottom=i+r;
            int left=j-r;
            int right=j+r;
            //[1]

            //[2] --检查滤波核是否超出边界
            if(i-r<0)
                top=0;
            if(i+r>src.rows)
                bottom=src.rows;
            if(j-r<0)
                left=0;
            if(j+r>src.cols)
                right=src.cols;
            //[2]

            //[3] --求取模板下的最小值
            Mat ImROI=src(Range(top,bottom),Range(left,right));
            double min,max;
            minMaxLoc(ImROI,&min,&max,0,0);
            dst.at<uchar>(i,j)=min;
            //[3]
        }
    //[3]
    return dst;
}

//导向滤波
Mat DeFog::guildFilter( Mat g,  Mat p, int ksize)
{
    const double eps =1.0e-5;//regularization parameter
    //类型转换
    Mat _g;
    g.convertTo(_g,CV_64FC1);
    g=_g;

    Mat _p;
    p.convertTo(_p,CV_64FC1);
    p=_p;

    //[hei, wid] = size(I);
    int hei = g.rows;
    int wid = g.cols;

    //N = boxfilter(ones(hei, wid), r); % the size of each local patch; N=(2r+1)^2 except for boundary pixels.
    cv::Mat N;
    cv::boxFilter(cv::Mat::ones(hei, wid, g.type()), N, CV_64FC1,Size(ksize,ksize));

    //[1] --使用均值模糊求取各项系数
    Mat mean_G;
    boxFilter(g,mean_G,CV_64FC1,Size(ksize,ksize));

    Mat mean_P;
    boxFilter(p,mean_P,CV_64FC1,Size(ksize,ksize));

    Mat GP=g.mul(p);
    Mat mean_GP;
    boxFilter(GP,mean_GP,CV_64FC1,Size(ksize,ksize));

    Mat GG=g.mul(g);
    Mat mean_GG;
    boxFilter(GG,mean_GG,CV_64FC1,Size(ksize,ksize));

    Mat cov_GP;
    cov_GP=mean_GP-mean_G.mul(mean_P);

    Mat var_G;
    var_G=mean_GG-mean_G.mul(mean_G);
    //[1]

    //求系数a a=(mean(GP)-mean(G)mean(p))/(mean(GG)-mean(G)mean(G)+eps)
    Mat a=cov_GP/(var_G+eps);

    //求系数b b=mean(P)-mean(G)*a
    Mat b=mean_P-a.mul(mean_G);

    //求两个系数的均值
    Mat mean_a;
    boxFilter(a,mean_a,CV_64FC1,Size(ksize,ksize));
    //mean_a=mean_a/N;

    Mat mean_b;
    boxFilter(b,mean_b,CV_64FC1,Size(ksize,ksize));
    //mean_b=mean_b/N;

    //输出结果q
    Mat q=mean_a.mul(g)+mean_b;
   // qDebug()<<q.at<double>(100,100);

    return q;
}


//图像灰度拉伸
//src 灰度图图
//lowcut、highcut为百分比的值 如lowcut=3表示3%
//lowcut表示暗色像素的最小比例，小于该比例均为黑色
//highcut为高亮像素的最小比例，大于该比例的均为白色
Mat DeFog::grayStretch(const Mat src, double lowcut, double highcut)
{
    //[1]--统计各通道的直方图
    //参数
    const int bins = 256;
    int hist_size=bins;
    float range[]={0,255};
    const float* ranges[]={range};
    MatND desHist;
    int channels=0;
    //计算直方图
    calcHist(&src,1,&channels,Mat(),desHist,1,&hist_size,ranges,true,false);
    //[1]

    //[2] --计算上下阈值
    int pixelAmount=src.rows*src.cols; //像素总数
    float Sum=0;
    int minValue,maxValue;
    //求最小值
    for(int i=0;i<bins;i++)
    {
        Sum=Sum+desHist.at<float>(i);
        if(Sum>=pixelAmount*lowcut*0.01)
        {
            minValue=i;
            break;
        }
    }

    //求最大值
    Sum=0;
    for(int i=bins-1;i>=0;i--)
    {
        Sum=Sum+desHist.at<float>(i);
        if(Sum>=pixelAmount*highcut*0.01)
        {
            maxValue=i;
            break;
        }
    }
    //[2]

    //[3] --对各个通道进行线性拉伸
    Mat dst=src;
    //判定是否需要拉伸
    if(minValue>maxValue)
        return src;

    for(int i=0;i<src.rows;i++)
        for(int j=0;j<src.cols;j++)
        {
            if(src.at<uchar>(i,j)<minValue)
                dst.at<uchar>(i,j)=0;
            if(src.at<uchar>(i,j)>maxValue)
                dst.at<uchar>(i,j)=255;
            else
            {
                //注意这里做除法要使用double类型
                double pixelValue=((src.at<uchar>(i,j)-minValue)/
                                   (double)(maxValue-minValue))*255;
                dst.at<uchar>(i,j)=(int)pixelValue;
            }
        }
    //[3]

    return dst;
}

//暗原色原理去雾
Mat DeFog::darkChannelDefog(Mat src)
{
    //[1] --minRGB
    Mat tempImage=minRGB(src);
    //[1]

    //[2] --minFilter
    int ksize=std::max(3,std::max((int)(src.cols*kernRatio),
                                  (int)(src.rows*kernRatio))); //求取自适应核大小
    tempImage=minFilter(tempImage,ksize);
    //[2]

    //[3] --dark channel image
    m_tImage=Mat::zeros(src.rows,src.cols,CV_64FC1);
    for(int i=0;i<src.rows;i++)
        for(int j=0;j<src.cols;j++)
            m_tImage.at<double>(i,j)=((255.0-
                                       (double)tempImage.at<uchar>(i,j)*wFactor)/255)-0.005;

    //[3]

    //[4] --求取全球大气光强A(全局量)
    double A[3];Point maxLoc;
    minMaxLoc(tempImage,0,0,0,&maxLoc);
    for(int c=0;c<src.channels();c++)
        A[c]=src.at<Vec3b>(maxLoc.y,maxLoc.x)[c];
    //[4]

    //[5] --根据去雾公式求取去雾图像  J=(I-(1-t)*A)/max(t,min_t)
    m_dstImage=Mat::zeros(src.rows,src.cols,CV_64FC3);
    for(int i=0;i<src.rows;i++)
        for(int j=0;j<src.cols;j++)
            for(int c=0;c<src.channels();c++)
                m_dstImage.at<Vec3d>(i,j)[c]=(src.at<Vec3b>(i,j)[c]-
                                              (1-m_tImage.at<double>(i,j))*A[c])/
                        std::max(m_tImage.at<double>(i,j),min_t);
    m_dstImage.convertTo(m_dstImage,CV_8UC3);
    //[5]

    return m_dstImage;

}

Mat DeFog::enhanceImage(Mat src)
{
    Mat dst;
    //[6] --自动色阶（rgb三通道灰度拉伸）
    cv::Mat channels[3];
    split(src,channels);//不知道什么原因vector无法使用 只能用数组来表示
    for(int c=0;c<3;c++)
        channels[c]= grayStretch(channels[c],0.001,1); //根据实验 暗色像素的比例应该设置的较小效果会比较好
    merge(channels,3,dst);

    return dst;
}

//返回透射图
Mat DeFog::getTImage()
{
    Mat temp = Mat::zeros(m_tImage.rows,m_tImage.cols,CV_8UC1);
    for(int i=0;i<m_tImage.rows;i++)
        for( int j=0;j<m_tImage.cols;j++)
        {
            temp.at<uchar>(i,j) = (int)(m_tImage.at<double>(i,j)*255);
        }
    m_tImage.convertTo(m_tImage,CV_8UC1);
    m_tImage = temp;
    return m_tImage;
}
