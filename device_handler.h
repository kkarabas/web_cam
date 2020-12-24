#ifndef DEVICE_HANDLER_H
#define DEVICE_HANDLER_H

#include <linux/videodev2.h>
#include <string>
#include <QPixelFormat>
#include <QImage>

class Device_handler: public QObject
{
    Q_OBJECT

    struct buffer {
      void *start;
      size_t length;
    };

public:
    explicit Device_handler(int, int);
    ~Device_handler();

    void start_device(const char*);
    void stop_device();
    void* read_frame(unsigned long*, unsigned char*);
    void return_Buff_to_driver(unsigned char);
    u_char* getdata();

    static int frames_captured;

private: 
    int m_fd;
    int m_frame_width;
    int m_frame_height;
    struct buffer *m_buffers;
    unsigned int m_num_buffers;
    struct v4l2_format fmt;
    int m_buffers_to_DQ;

    void open_device(const char*);
    void close_device(void);
    void init_device(void);
    void uninit_device(void);
    void start_capturing(void);
    void stop_capturing(void);
    static int xioctl(int, int, void*);
    void init_mmap(void);

signals:
    void drop_frame();
};

#endif // DEVICE_HANDLER_H

