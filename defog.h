#ifndef DEFOG
#define DEFOG
#include <QObject>
#include <opencv2/opencv.hpp>

class DeFog : public QObject
{
    Q_OBJECT
public:
    explicit DeFog();
    ~DeFog();

    cv::Mat darkChannelDefog(cv::Mat);//暗原色先验去雾
   // cv::Mat guidedDefog(cv::Mat);//导向滤波去雾
    cv::Mat enhanceImage(cv::Mat);//增强
    cv::Mat getTImage();

private:
    cv::Mat minRGB(cv::Mat);
    cv::Mat minFilter(cv::Mat src,int ksize=3);
    cv::Mat guildFilter(cv::Mat g,cv::Mat p,int ksize);
    cv::Mat grayStretch(const cv::Mat src,double lowcut,double highcut);

private:
    cv::Mat m_srcImage; //原始图像
    cv::Mat m_tImage;  //透射率
    cv::Mat m_dstImage; //结果图像
};

#endif // DEFOG

