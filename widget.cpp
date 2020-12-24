#include "widget.h"

#include <QImage>
#include <QPainter>
#include <QtDebug>

Widget::Widget(int width, int height, QWidget *parent)
    :  QWidget(parent)
     , m_imagewidth(width)
     , m_imageheight(height)
     , brightness(0)
{
    image = new QImage(m_imagewidth,m_imageheight,QImage::Format_Grayscale8);
    unsigned long size = (unsigned long)m_imagewidth * (unsigned long)m_imageheight;
    u_char *matrix = new u_char[size];

    for(int i = 0; i < m_imagewidth; i++){
        for(int j = 0; j < m_imageheight; j++){
            matrix[i * m_imageheight +j] = u_char(brightness+(float(i) * 128.0/float(m_imageheight) + float(j) * 128.0/float(m_imagewidth))); //getRandomNumber(0, 127);
           // qDebug() << "matrix[" << i * imageheight + j << "] = "  << matrix[i * imageheight +j] << '\n';
        }
    }

    for(int i=0; i < m_imagewidth; i++)
    {
       for(int j=0; j < m_imageheight; j++)
       {

       u_char  pixelval = matrix[i * m_imageheight +j];
       //qDebug() << "pixelval [" << i * imageheight +j << "]= "  << pixelval << '\n';
       QRgb color = qRgb(pixelval, pixelval, pixelval);
       image->setPixel(i,j, color);
       }
    }

    delete [] matrix;
}

int Widget::displayed = 0;

Widget::~Widget()
{
}

void Widget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    QRect rect(0, 0, m_imagewidth, m_imageheight);
    painter.drawImage(rect, *image);
}

void Widget::brightness_change(int value)
{
    brightness = value;
    unsigned long size = (unsigned long)m_imagewidth *  (unsigned long)m_imageheight;
    u_char *matrix = new u_char[size];

    for(int i = 0; i < m_imagewidth; i++){
        for(int j = 0; j < m_imageheight; j++){
            matrix[i * m_imageheight +j] = u_char(brightness+(float(i) * 128.0/float(m_imageheight) + float(j) * 128.0/float(m_imagewidth))); //getRandomNumber(0, 127);
           // qDebug() << "matrix[" << i * imageheight + j << "] = "  << matrix[i * imageheight +j] << '\n';
        }
    }

    for(int i=0; i < m_imagewidth; i++)
    {
       for(int j=0; j < m_imageheight; j++)
       {

       u_char  pixelval = matrix[i * m_imageheight +j];
       //qDebug() << "pixelval [" << i * imageheight +j << "]= "  << pixelval << '\n';
       QRgb color = qRgb(pixelval, pixelval, pixelval);
       image->setPixel(i,j, color);
       }
    }

    delete [] matrix;
    repaint();

}

void Widget::frame_process(void* pic, unsigned long ar_size)
{
    Q_UNUSED(ar_size);
    if (pic == nullptr) return;
    u_char *image_ar = (u_char*)pic;
    u_char  pixelval = *image_ar;
    for(int i=0; i < m_imageheight; i++){
       for(int j=0; j < m_imagewidth; j++){
            //qDebug() << "pixelval [" << i << "][" << j << "]= "  << pixelval << '\n';
            pixelval = *image_ar;
            QRgb color = qRgb(pixelval, pixelval, pixelval);
            image->setPixel(j,i, color);
            image_ar = image_ar + 2;
       }
    }
    repaint();
}
