#include "dialog.h"
#include "ui_dialog.h"
#include "defog.h"
#include <QFileDialog>
#include <QGraphicsPixmapItem>
#include <QDebug>

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);
    setWindowTitle("Haze_Removal");

    //connect
    connect(ui->openButton,SIGNAL(clicked(bool)),this,SLOT(SlotopenFile()));
    connect(ui->deFogButton,SIGNAL(clicked(bool)),this,SLOT(deFog()));
}

Dialog::~Dialog()
{
    delete ui;
}


void Dialog::paintEvent(QPaintEvent *event)
{
    QDialog::paintEvent(event);

    setItemScale(SRC_IMAGE);
    setItemScale(T_IMAGE);
    setItemScale(DEFOG_IMAGE);
    setItemScale(STRETCH_IMAGE);
}

void Dialog::SlotopenFile()
{
    fileName = QFileDialog::getOpenFileName(this, tr("Open Images"),
                                            "E:/test/",
                                            tr("Images (*.png);;\
                                               Images (*.jpg);;\
            Images (*.tif)"));

            if(fileName.isEmpty())
            return;
    //清屏
    clear();

    src=cv::imread(fileName.toStdString());
    cv::cvtColor(src,src,CV_BGR2RGB);

    double fx=(double)(ui->srcImage->width()-10)/src.cols;
    double fy=(double)(ui->srcImage->height()-10)/src.rows;
    if(fx<fy)
        fy=fx;
    if(fx>fy)
        fx=fy;
    cv::resize(src,src,cv::Size(),fx,fy);

    srcImage=Mat2QImage(src);
    showImage(srcImage,SRC_IMAGE);
}

void Dialog::deFog()
{
    if(src.empty())
        return;

    DeFog defog;
    cv::Mat deFog=defog.darkChannelDefog(src);
    deFogImage=Mat2QImage(deFog);
    showImage(deFogImage,DEFOG_IMAGE);

    cv::Mat t=defog.getTImage();
    tImage=Mat2QImage(t);
    showImage(tImage,T_IMAGE);

    cv::Mat stretch=defog.enhanceImage(deFog);
    StretchImage=Mat2QImage(stretch);
    showImage(StretchImage,STRETCH_IMAGE);
}

QImage Dialog::Mat2QImage(cv::Mat &image)
{
    QImage img;

    if(image.channels()==3)
    {
        //cvt Mat BGR 2 QImage RGB
        img =QImage((const unsigned char*)(image.data),
                    image.cols,image.rows,
                    image.cols*image.channels(),
                    QImage::Format_RGB888);
    }else if(image.channels()==1)
    {
        img =QImage((const unsigned char*)(image.data),
                    image.cols,image.rows,
                    image.cols*image.channels(),
                    QImage::Format_Grayscale8);
    }
    return img;
}

void Dialog::showImage(QImage image, int type)
{
    QGraphicsScene *scene = new QGraphicsScene(this);
    id_Scene[type] = scene;
    QGraphicsPixmapItem *item = scene->addPixmap(QPixmap::fromImage(image));
    id_Item[type] = item;

    if(type==SRC_IMAGE)
    {
        ui->srcImage->setScene(scene);
        ui->srcImage->show();
    }
    if(type==T_IMAGE)
    {
        ui->tImage->setScene(scene);
        ui->tImage->show();
    }
    if(type==DEFOG_IMAGE)
    {
        ui->deFogImage->setScene(scene);
        ui->deFogImage->show();
    }
    if(type==STRETCH_IMAGE)
    {
        ui->stretchImage->setScene(scene);
        ui->stretchImage->show();
    }
}

void Dialog::setItemScale(int type)
{
    if(!id_Item.contains(type))
        return;

    double scale;
    switch (type) {
    case SRC_IMAGE:
        scale=((double)ui->srcImage->width())/
                (double)ui->srcImage->height();
        break;
    case T_IMAGE:
        scale=((double)ui->tImage->width())/
                (double)ui->tImage->height();
        break;
        break;
    case DEFOG_IMAGE:
        scale=((double)ui->deFogImage->width())/
                (double)ui->deFogImage->height();
        break;
        break;
    case STRETCH_IMAGE:
        scale=((double)ui->stretchImage->width())/
                (double)ui->stretchImage->height();
        break;
    default:
        break;
    }

    id_Item[type]->setScale(scale);
}

void Dialog::clearScene(int type)
{
    if(!id_Scene.contains(type))
        return;
    id_Scene[type]->clear();
}

void Dialog::clear()
{
    clearScene(SRC_IMAGE);
    clearScene(T_IMAGE);
    clearScene(DEFOG_IMAGE);
    clearScene(STRETCH_IMAGE);
    id_Item.clear();
}
