#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>


class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(int, int, QWidget *parent = nullptr);
    ~Widget();

    static int displayed;

private:
    int m_imagewidth;
    int m_imageheight;
    QImage* image;
    int brightness;

protected:
    void paintEvent(QPaintEvent *event) override;

public slots:
    void brightness_change(int);
    void frame_process(void*, unsigned long);
    //void save_frame();

};
#endif // WIDGET_H
