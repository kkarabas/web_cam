#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QBoxLayout>
#include <QSlider>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <vector>
#include <queue>
//#include "threadsafe_queue.h"

class Device_handler;
class Widget;

class MainWindow : public QWidget
{
    Q_OBJECT

    struct frame {
        void *buf;
        unsigned long size;
        unsigned char index;
    };

    enum framesize {
        width = 640,
        height = 480
    };

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void init_camera();
    void start_camera();
    void uninit_camera();
    void stop_camera();
    void change_drop_frame();


private:
    Device_handler *dh;
    Widget *wgt;

    std::atomic<bool> isCapturing, isInit, drop_frame;
    std::mutex m_direct_mutex, m_return_mutex;
    std::condition_variable m_data_cond;

    unsigned long frames_captured = 0;
    unsigned long frames_displayed = 0;
    std::thread m_thread_1;
    std::thread m_thread_2;
    std::queue<struct frame> m_direct_frame_queue;
    std::vector<unsigned char> m_return_frame_queue;

    //Threadsafe_queue<struct frame>* m_frame_queue;

    void frame_processing();
    void recieve_frame_from_driver();

protected:
    virtual void closeEvent(QCloseEvent *event) override;


};
#endif // MAINWINDOW_H
