#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <Qmap>
#include <opencv2/opencv.hpp>

class QImage;
class QGraphicsScene;
class QGraphicsPixmapItem;

namespace cv {
class Mat;
}
namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = 0);
    ~Dialog();

public slots:
    void SlotopenFile();

protected:
    void paintEvent(QPaintEvent *event);

private:
    QImage Mat2QImage(cv::Mat& image);
    void showImage(QImage,int);
    void setItemScale(int type);
    void clearScene(int type);
    void clear();

private slots:
    void deFog();

private:
    Ui::Dialog *ui;
    QString fileName;
    QImage srcImage;
    QImage tImage;
    QImage deFogImage;
    QImage StretchImage;
    cv::Mat src;
    enum
    {
        SRC_IMAGE,
        T_IMAGE,
        DEFOG_IMAGE,
        STRETCH_IMAGE
    };

    QMap<int,QGraphicsScene*> id_Scene;
    QMap<int,QGraphicsPixmapItem*> id_Item;
};

#endif // DIALOG_H
