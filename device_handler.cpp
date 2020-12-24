#include "device_handler.h"

#include "fcntl.h"
#include <iostream>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <cstring>
#include "unistd.h"
#include "stdlib.h"
#include <sys/mman.h>
#include <assert.h>
#include <qdebug.h>
#include <QElapsedTimer>

Device_handler::Device_handler(int width, int height):
    m_frame_width(width),
    m_frame_height(height),
    m_buffers_to_DQ(0)
{ 
}

int Device_handler::frames_captured = 0;

Device_handler::~Device_handler()
{

}

/************************************************************/
int Device_handler::xioctl(int fd, int request, void *arg)
{
    int r;
    do {
        r = ioctl(fd, request, arg);
    } while (-1 == r && EINTR == errno);
    return r;
}

void Device_handler::open_device(const char *path)
{
    struct stat st;
    if ( -1 == stat(path, &st) ) {
        qDebug() << "Cannot identify" << path << ":" << errno << std::strerror(errno) << '\n';
        exit(EXIT_FAILURE);
    }
    if ( !S_ISCHR(st.st_mode) ) {
        qDebug() << path << " is no device" << '\n';
        exit(EXIT_FAILURE);
    }

    if((m_fd = open(path, O_RDWR)) < 0) {
        qDebug() << "Cannot open" << path << ":" << errno << std::strerror(errno) << '\n';
        exit(EXIT_FAILURE);
    }
    qDebug() << "device is open" << '\n';
}

void Device_handler::close_device()
{
    if ( close(m_fd) == -1 ){
        qDebug() << "Cannot close device" << '\n';

     exit(EXIT_FAILURE);
    }
    qDebug() << "device is closed" << '\n';
}

/***************************************************************/

void Device_handler::init_device()
{
    struct v4l2_capability cap;
   // struct v4l2_cropcap cropcap;
   // struct v4l2_crop crop;
    unsigned int min;

    if (-1 == xioctl(m_fd, VIDIOC_QUERYCAP, &cap)) {
        if (EINVAL == errno){
            qDebug() << "This is no V4L2 device" << '\n';
            exit(EXIT_FAILURE);
        } else {
            qDebug() << "VIDIOC_QUERYCAP:" << errno << std::strerror(errno) << '\n';
        }
    }

    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
        qDebug() << "Connected device is no video capture device" << '\n';
        exit(EXIT_FAILURE);
    }

    memset(&fmt, 0, sizeof(fmt));
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width       = m_frame_width;
    fmt.fmt.pix.height      = m_frame_height;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;  //V4L2_PIX_FMT_YUYV
    //fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;

    if (-1 == xioctl(m_fd, VIDIOC_S_FMT, &fmt))
        qDebug() << "VIDIOC_S_FMT:" << errno << std::strerror(errno) << '\n';

// Buggy driver paranoia.

    min = fmt.fmt.pix.width * 2;
    if (fmt.fmt.pix.bytesperline < min)
        fmt.fmt.pix.bytesperline = min;
    min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
    if (fmt.fmt.pix.sizeimage < min)
        fmt.fmt.pix.sizeimage = min;

    init_mmap();
    qDebug() << "device init" << '\n';

    struct v4l2_streamparm streamparm;
    memset(&streamparm, 0, sizeof(streamparm));
    streamparm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (xioctl(m_fd, VIDIOC_G_PARM, &streamparm) != 0)
    {
       qDebug() << "VIDIOC_G_PARM:" << errno << std::strerror(errno) << '\n';
    }

    streamparm.parm.capture.capturemode |= V4L2_CAP_TIMEPERFRAME;
    streamparm.parm.capture.timeperframe.numerator = 1;
    streamparm.parm.capture.timeperframe.denominator = 25;
    if(xioctl(m_fd, VIDIOC_S_PARM, &streamparm) !=0) {
         qDebug() << "Failed to set frame rate ";
    }


}

/******************************************************************/

void Device_handler::init_mmap()
{
    struct v4l2_requestbuffers req;

    memset(&req, 0, sizeof(req));
    req.count = 8; // запрашиваем такое количество буферов
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    if (-1 == xioctl(m_fd, VIDIOC_REQBUFS, &req)) {
        if (EINVAL == errno) {
             qDebug() << "Device does not support memory mapping" << '\n';
             exit(EXIT_FAILURE);
        } else {
             qDebug() << "VIDIOC_REQBUFS:" << errno << std::strerror(errno) << '\n';
        }
    }

    //Получили слудующее количество буферов
    qDebug() << req.count << "buffs allocated";

    if (req.count < 2) {
        qDebug() << "Insufficient buffer memory on device" << '\n';
        exit(EXIT_FAILURE);
    }

    m_buffers = reinterpret_cast<struct buffer *>(calloc(req.count, sizeof(*m_buffers)));
    if (!m_buffers) {
        qDebug() << "Out of memory" << '\n';
        exit(EXIT_FAILURE);
    }

    m_num_buffers = req.count;

    //Записываем полученные буферы в массив m_buffers
    for (unsigned int i = 0; i < m_num_buffers; ++i) {
        struct v4l2_buffer buf;
        memset(&buf, 0, sizeof(buf));

        buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory      = V4L2_MEMORY_MMAP;
        buf.index       = i;

        if (-1 == xioctl(m_fd, VIDIOC_QUERYBUF, &buf))
            qDebug() << "VIDIOC_QUERYBUF:" << errno << std::strerror(errno) << '\n';

        m_buffers[i].length = buf.length;
        m_buffers[i].start = mmap(NULL /* start anywhere */,
                                          buf.length,
                                          PROT_READ | PROT_WRITE /* required */,
                                          MAP_SHARED /* recommended */,
                                          m_fd, buf.m.offset);

        if (MAP_FAILED == m_buffers[m_num_buffers].start)
            qDebug() << "mmap:" << errno << std::strerror(errno) << '\n';
        }
    qDebug() << "Init mmap" << '\n';
}


void Device_handler::uninit_device()
{
    unsigned int i;
    for (i = 0; i < m_num_buffers; ++i)
        if (-1 == munmap(m_buffers[i].start, m_buffers[i].length))
            qDebug() << "munmap:" << errno << std::strerror(errno) << '\n';
    free(m_buffers);
    qDebug() << "uninit device" << '\n';
}

/**********************************************************************/

void Device_handler::start_capturing()
{
    enum v4l2_buf_type type;

    for (unsigned int i = 0; i < m_num_buffers; ++i) {
        struct v4l2_buffer buf;

        memset(&buf, 0, sizeof(buf));
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;

        if (-1 == xioctl(m_fd, VIDIOC_QBUF, &buf))
            qDebug() << "VIDIOC_QBUF:" << errno << std::strerror(errno) << '\n';
        else m_buffers_to_DQ++;         //qDebug() << "VIDIOC_QBUF" << i <<": ok";
    }

    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (-1 == xioctl(m_fd, VIDIOC_STREAMON, &type))
        qDebug() << "VIDIOC_STREAMON:" << errno << std::strerror(errno) << '\n';

    qDebug() << "start capturing" << '\n';
}

void Device_handler::stop_capturing()
{
    enum v4l2_buf_type type;
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (-1 == xioctl(m_fd, VIDIOC_STREAMOFF, &type))
        qDebug() << "VIDIOC_STREAMOFF:" << errno << std::strerror(errno) << '\n';

    qDebug() << "stop capturing" << '\n';

    m_buffers_to_DQ = 0;
}

/********************************************************************/

void* Device_handler::read_frame( unsigned long* byteused, unsigned char* index )
{
    if (m_buffers_to_DQ > 0) {
        struct v4l2_buffer buf;

        memset(&buf, 0, sizeof(buf));
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;

        if (-1 == xioctl(m_fd, VIDIOC_DQBUF, &buf)) {
            switch (errno) {
                case EAGAIN:
                                return nullptr;
                case EIO:
                                /* Could ignore EIO, see spec. */
                                /* fall through */
                default:
                                qDebug() << "VIDIOC_DQBUF:" << errno << std::strerror(errno) << '\n';
             }
        }
        assert(buf.index < m_num_buffers);

        *byteused = buf.bytesused;
        *index = buf.index;

        //add_Frame_to_Queue(m_buffers[buf.index].start, buf.bytesused, buf.index);
        ++frames_captured;
        m_buffers_to_DQ--;
        return m_buffers[buf.index].start;
    } else {
        //emit drop_frame();
        return nullptr;
    }
}

void Device_handler::return_Buff_to_driver(unsigned char index)
{
    struct v4l2_buffer buf;

    memset(&buf, 0, sizeof(buf));
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = index;

    if (-1 == xioctl(m_fd, VIDIOC_QBUF, &buf))
        qDebug() << "VIDIOC_QBUF:" << errno << std::strerror(errno) << '\n';
    //else qDebug() << "VIDIOC_QBUF"<< buf.index << ": ok";
    m_buffers_to_DQ++;
}

void Device_handler::start_device(const char *dev)
{
    this->open_device(dev);
    this->init_device();
    this->start_capturing();
}

void Device_handler::stop_device()
{
    this->stop_capturing();
    this->uninit_device();
    this->close_device();
}


/*struct buffer* Device_handler::getFrame()
{
    qDebug() << "Frame# " << frame++;
    fd_set fds;
    struct timeval tv;
    int r;

    FD_ZERO(&fds);
    FD_SET(m_fd, &fds);

    tv.tv_sec = 2;
    tv.tv_usec = 0;

    r = select(m_fd + 1, &fds, NULL, NULL, &tv);
        if (-1 == r) {
            if (EINTR == errno)
                qDebug() << "select:" << errno << std::strerror(errno) << '\n';
            if (0 == r) {
                qDebug() << "select timeout" << '\n';
                exit(EXIT_FAILURE);
            }
        }
    if (read_frame())
}
*/


/*void Device_handler::print_params()
{
    struct v4l2_capability cap;
    if (xioctl(m_fd, VIDIOC_QUERYCAP, &cap) == -1)
    {
      qDebug() << "VIDIOC_QUERYCAP error " << errno << std::strerror(errno) << '\n';
      exit(EXIT_FAILURE);
    }
    qDebug() << "device driver: "   << cap.driver << '\n';
    qDebug() << "device name: "     << cap.card << '\n';
    qDebug() << "bus info:"         << cap.bus_info << '\n';
    qDebug() << "driver version: "  << ((cap.version >> 16) & 0xFF) << "."
                                     << ((cap.version >> 8) & 0xFF) << "."
                                     << (cap.version & 0xFF) << '\n';
    if (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)
        qDebug() << "The device supports the single-planar API through the Video Capture interface." << '\n';
    else
        qDebug() << "The device does not support the single-planar API through the Video Capture interface." << '\n';
    if (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE)
        qDebug() << "The device supports the multi-planar API through the Video Capture interface." << '\n';
    else
        qDebug() << "The device does not support the multi-planar API through the Video Capture interface." << '\n';
    if (cap.capabilities & V4L2_CAP_STREAMING)
        qDebug() << "The device supports the streaming I/O method." << '\n';
    else
        qDebug() << "The device does not support the streaming I/O method." << '\n';
}


void Device_handler::set_video_format(__u32 BUF_TYPE, __u32 PIX_FMT, __u32 width, __u32 height)
{
    format_m.type = BUF_TYPE;
    format_m.fmt.pix.pixelformat = PIX_FMT;   // V4L2_PIX_FMT_SPCA501   V4L2_PIX_FMT_MJPEG
    format_m.fmt.pix.width = width;
    format_m.fmt.pix.height = height;
}
*/
