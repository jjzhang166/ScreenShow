#ifndef PICSHOW_H
#define PICSHOW_H

#include <QWidget>
#include <sock.h>
#include <dts.h>
#include <QMutex>

extern "C"{

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>

}

namespace Ui {
class PicShow;
}

class PicShow : public QWidget
{
    Q_OBJECT

private:
    Sock m_sock;
    QHostAddress local_addr;
    quint16 listen_port;
    RecvBuffer buffer;
    QMutex buffer_mutex;
    QMap<unsigned char,QSet<unsigned int>> recv_pkg;

    AVFrame *frame;
    AVPacket* pkt;
    AVCodec *codec;
    AVCodecContext *codec_context;

    bool init_decoder();
    void uninit_decoder();
    QPixmap decode_pixmap(const QByteArray &data);
public:
    explicit PicShow(QHostAddress ip, quint16 port,QWidget *parent = 0);
    ~PicShow();

private:
    Ui::PicShow *ui;

private slots:
    void on_read();
    void clear_broken(unsigned char id);
    void add_package_and_show(const Package &pkg);
    void show_frame(const Frame& frame);
    Data_Package recover_to_data_pkg(const Frame& frame);
};

#endif // PICSHOW_H
