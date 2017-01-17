#include "manager.h"
#include <QDesktopWidget>
#include <QApplication>
#include <QBuffer>
#include <thread>
#include <QThread>
#include <QScreen>
#include <globalfunc.h>
#include <QBitmap>
#include <cassert>
#include <QVideoFrame>


Manager::Manager(QHostAddress ip, quint16 port, QWidget *parent) : QWidget(parent),local_addr(ip),m_port(port)
{
    hide();
    if(!init_encoder()){
        qDebug()<<"encoder error";
        exit(0);
    }

    std::thread(
                [this](){
        Sock m_sock(this->local_addr);
        Frame t_frame;
        while(true){
            this->buffer_mutex.lock();
            if(!this->buffer.isEmpty()){
                t_frame=this->buffer.front();
                this->buffer.pop_front();
            }
            this->buffer_mutex.unlock();
            if(!t_frame.isEmpty()){
                for(auto &p:t_frame){
                    QThread::usleep(SEND_SLEEP);
                    m_sock.writeDatagram(static_cast<char*>(static_cast<void*>(&p)),sizeof(Package),QHostAddress::Broadcast,this->m_port);
                }
            }
            QThread::yieldCurrentThread();
        }
    }
    ).detach();




    start_cap();
}

Frame Manager::make_frame(const Data_Package &data_pkg){
    static unsigned char mj_id=0;
    if(data_pkg.isEmpty()){
        return Frame{};
    }
    mj_id=mj_id==255?0:mj_id+1;
#ifdef COMPRESS_DATA
    QByteArray ba=compress(data_pkg);
#else
    QByteArray ba=data_pkg;
#endif
    size_t block_num=(ba.size()+(DATA_BUFFER_LEN-1))/DATA_BUFFER_LEN;
    Package tmp_pkg;
    Frame ret;
    for(size_t i=0;i<block_num;++i){
        tmp_pkg.major_id=mj_id;
        tmp_pkg.minor_id=i;
        tmp_pkg.pk_num=block_num;
        tmp_pkg.length=ba.size();
        tmp_pkg.curr_length=ba.size()-DATA_BUFFER_LEN*i>DATA_BUFFER_LEN?DATA_BUFFER_LEN:ba.size()%DATA_BUFFER_LEN;
        memcpy_s(tmp_pkg.data,DATA_BUFFER_LEN,ba.data()+i*DATA_BUFFER_LEN,tmp_pkg.curr_length);
        ret.push_back(tmp_pkg);
    }
    return ret;
}

Data_Package Manager::get_desktop_img(){
    QPixmap pixmap{QApplication::primaryScreen()->grabWindow(0)};
    Data_Package header;
    header=encode_pixmap(pixmap.scaled(video_size));
    return header;
}

void Manager::start_cap(){
    while(true){
        buffer_mutex.lock();
        if(buffer.size()>MAX_BUFFER_SIZE){
            buffer_mutex.unlock();
            QThread::msleep(CAP_SLEEP);
            continue;
        }
        buffer_mutex.unlock();
        Frame t_frame{make_frame(get_desktop_img())};
        if(t_frame.isEmpty()){
            QThread::msleep(CAP_SLEEP);
            continue;
        }
        buffer_mutex.lock();
        if(buffer.size()>MAX_BUFFER_SIZE){
            buffer_mutex.unlock();
            QThread::msleep(CAP_SLEEP);
            return;
        }
        buffer.push_back(t_frame);
        buffer_mutex.unlock();
        QThread::msleep(CAP_SLEEP);
    }
}

Manager::~Manager(){
    uninit_encoder();
}

bool Manager::init_encoder(){
    avcodec_register_all();
    codec_context= NULL;
    pkt = new AVPacket;
    codec = avcodec_find_encoder(AV_CODEC_ID_H264);
    if (codec == 0)  {
        qDebug()<<"avcodec_find_encoder";
        return false;
    }

    codec_context = avcodec_alloc_context3(codec);
    if (!codec_context) {
        qDebug()<<"avcodec_alloc_context3";
        return false;
    }
    //c->bit_rate = 400000;
    video_size=QApplication::desktop()->size()/SCALE_NUM;
    codec_context->width = video_size.width();
    codec_context->height = video_size.height();
    codec_context->time_base = AVRational{1,25};
    codec_context->gop_size = 100;
    codec_context->max_b_frames = 0;
    codec_context->delay = 0;
    codec_context->pix_fmt = AV_PIX_FMT_YUV420P;
    av_opt_set(codec_context->priv_data, "preset", "superfast", 0);
    av_opt_set(codec_context->priv_data, "tune", "zerolatency", 0);
    int re = avcodec_open2(codec_context, codec, NULL);
    av_opt_set(codec_context->priv_data, "tune", "zerolatency", 0);
    if (re < 0) {
        qDebug()<<"avcodec_open2";
        return false;
    }
    av_frame = av_frame_alloc();
    if (!av_frame) {
        qDebug()<<"av_frame_alloc";
        return false;
    }
    av_frame->format = codec_context->pix_fmt;
    av_frame->width  = codec_context->width;
    av_frame->height = codec_context->height;
    int ret = av_image_alloc(av_frame->data, av_frame->linesize, codec_context->width, codec_context->height, codec_context->pix_fmt, 32);
    if (ret < 0) {
        qDebug()<<"av_image_alloc";
        return false;
    }
    pkt->data = NULL;
    pkt->size = 1000000;
    av_init_packet(pkt);
    return true;
}

QByteArray Manager::encode_pixmap(const QPixmap &pixmap){
    static quint64 pic_num=0;
    QByteArray ret_byte;
    QImage image = pixmap.toImage();
    for (int h = 0; h < codec_context->height; h++){
        for (int w = 0; w < codec_context->width; w++){
            QRgb rgb = image.pixel(w, h);
            int r = qRed(rgb);
            int g = qGreen(rgb);
            int b = qBlue(rgb);
            int dy = ((66*r + 129*g + 25*b) >> 8) + 16;
            int du = ((-38*r + -74*g + 112*b) >> 8) + 128;
            int dv = ((112*r + -94*g + -18*b) >> 8) + 128;
            uchar yy = (uchar)dy;
            uchar uu = (uchar)du;
            uchar vv = (uchar)dv;
            av_frame->data[0][h * av_frame->linesize[0] + w] = yy;
            if(h % 2 == 0 && w % 2 == 0){
                av_frame->data[1][h/2 * (av_frame->linesize[1]) + w/2] = uu;
                av_frame->data[2][h/2 * (av_frame->linesize[2]) + w/2] = vv;
            }
        }
    }

    av_frame->pts =pic_num++;
    int got_output;
    int ret = avcodec_encode_video2(codec_context, pkt, av_frame, &got_output);
    if (ret < 0){
        return QByteArray{};
    }
    if (got_output){
        int ss = pkt->size;
        ret_byte.resize(ss);
        memcpy_s(ret_byte.data(),ss,pkt->data,ss);
    }
    return ret_byte;
}


void Manager::uninit_encoder(){
    av_free_packet(pkt);
    avcodec_free_context(&codec_context);
}
