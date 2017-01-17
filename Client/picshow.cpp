#include "picshow.h"
#include "ui_picshow.h"
#include <algorithm>
#include <QPainter>
#include <globalfunc.h>
#include <QBitmap>

PicShow::PicShow(QHostAddress ip, quint16 port, QWidget *parent) :
    QWidget(parent),
    m_sock(ip,port,parent),
    local_addr(ip),
    listen_port(port),
    ui(new Ui::PicShow)
{
    ui->setupUi(this);
    if(!init_decoder()){
        qDebug()<<"init decoder error";
        exit(0);
    }
    connect(&m_sock,SIGNAL(readyRead()),this,SLOT(on_read()));
}

PicShow::~PicShow()
{
    uninit_decoder();
    delete ui;
}


void PicShow::on_read(){
    Package pkg;
    m_sock.readDatagram(static_cast<char*>(static_cast<void*>(&pkg)),sizeof(Package));
    clear_broken(static_cast<unsigned char>((pkg.major_id+MAX_BUFFER_SIZE/2)%256));
    add_package_and_show(pkg);
}


void PicShow::clear_broken(unsigned char id){
    buffer_mutex.lock();
    buffer.remove(id);
    buffer_mutex.unlock();
    recv_pkg.remove(id);
}


void PicShow::add_package_and_show(const Package &pkg){
    bool has_find=false;
    bool has_show=false;
    Frame will_show;
    if(pkg.pk_num==1){
        will_show.push_back(pkg);
        has_show=true;
        has_find=true;
    }else{
        if(recv_pkg.find(pkg.major_id)!=recv_pkg.end()){
            has_find=true;
            if(recv_pkg[pkg.major_id].find(pkg.minor_id)!=recv_pkg[pkg.major_id].end()){//重复包
                return;
            }else{
                if(recv_pkg[pkg.major_id].size()==pkg.pk_num-1){//完整一帧
                    buffer_mutex.lock();
                    will_show=buffer[pkg.major_id];
                    buffer.remove(pkg.major_id);
                    buffer_mutex.unlock();
                    recv_pkg.remove(pkg.major_id);
                    will_show.push_back(pkg);
                    has_show=true;
                }else{
                    buffer_mutex.lock();
                    buffer[pkg.major_id].push_back(pkg);
                    buffer_mutex.unlock();
                    recv_pkg[pkg.major_id].insert(pkg.minor_id);
                }
            }
        }else{
            has_find=false;
        }
    }
    if(!has_find){
        buffer_mutex.lock();
        buffer[pkg.major_id].push_back(pkg);
        buffer_mutex.unlock();
        recv_pkg[pkg.major_id].insert(pkg.minor_id);
    }

    if(has_show){
        show_frame(will_show);
    }
}

void PicShow::show_frame(const Frame &frame){
    /*
    //TODO 添加鼠标
    QPixmap tmp_pixmap=last_pixmap.scaled(ui->show_lab->size(),Qt::KeepAspectRatio,Qt::SmoothTransformation);
    ui->show_lab->setPixmap(tmp_pixmap);
    */
    Data_Package pkg=recover_to_data_pkg(frame);
    if(pkg.isEmpty())
        return;
    QPixmap pixmap=decode_pixmap(pkg);
    if(pixmap.isNull()){
        qDebug()<<"pixmap is null";
        return;
    }
    ui->show_lab->clear();

    ui->show_lab->setPixmap(pixmap.scaled(ui->show_lab->size()));
}

Data_Package PicShow::recover_to_data_pkg(const Frame &frame){
    Frame t_frame{frame};
    std::sort(t_frame.begin(),t_frame.end(),[](const Package &a,const Package &b){return a.minor_id<b.minor_id;});
    QByteArray ba;
    for(auto &p:t_frame)
        ba.append(static_cast<const char*>(static_cast<const void*>((p.data))),p.curr_length);
#ifdef COMPRESS_DATA
    ba=uncompress(ba);
#endif
    return ba;
}


QPixmap PicShow::decode_pixmap(const QByteArray &data){
    int nread = data.size();
    if(nread <= 0)
        return QPixmap{};
    pkt->size = nread;
    pkt->data = (uint8_t *)(void*)data.data();
    QPixmap ret_pixmap;
    int len, got_frame;
    len = avcodec_decode_video2(codec_context, frame, &got_frame, pkt);
    if (got_frame)  {
        QImage image(codec_context->width,codec_context->height,QImage::Format_RGB888);
        int received_frame_width = codec_context->width;
        int received_frame_height = codec_context->height;
        for(int h = 0; h < codec_context->height; h++){
            for(int w = 0; w < codec_context->width; w ++){
                int hh = h >> 1;
                int ww = w >> 1;
                int Y = frame->data[0][h * frame->linesize[0] + w];
                int U = frame->data[1][hh * (frame->linesize[1]) + ww];
                int V = frame->data[2][hh * (frame->linesize[2]) + ww];

                int C = Y - 16;
                int D = U - 128;
                int E = V - 128;

                int r = 298 * C           + 409 * E + 128;
                int g = 298 * C - 100 * D - 208 * E + 128;
                int b = 298 * C + 516 * D           + 128;

                r = qBound(0, r >> 8, 255);
                g = qBound(0, g >> 8, 255);
                b = qBound(0, b >> 8, 255);

                QRgb rgb = qRgb(r, g, b);
                image.setPixel(QPoint(w, h), rgb);
            }
        }
        ret_pixmap=QPixmap::fromImage(image);
    }
    return ret_pixmap;
}

bool PicShow::init_decoder(){ 
    avcodec_register_all();
    pkt = new AVPacket;
    av_init_packet(pkt);
    //memset(inbuf + INBUF_SIZE, 0, FF_INPUT_BUFFER_PADDING_SIZE);
    codec = avcodec_find_decoder(AV_CODEC_ID_H264);
    if (!codec){
        return false;
    }
    codec_context = avcodec_alloc_context3(codec);
    if (!codec_context) {
        return false;
    }
    av_opt_set(codec_context->priv_data, "preset", "superfast", 0);
    av_opt_set(codec_context->priv_data, "tune", "zerolatency", 0);
    codec_context->delay = 0;
    if(codec->capabilities&CODEC_CAP_TRUNCATED)
        codec_context->flags|= CODEC_FLAG_TRUNCATED;
    if (avcodec_open2(codec_context, codec, NULL) < 0) {
        return false;
    }
    frame = av_frame_alloc();
    if (!frame) {
        return false;
    }
    pkt->data=NULL;
    pkt->size = 1000000;
    av_init_packet(pkt);
    return true;
}


void PicShow::uninit_decoder(){
    av_free_packet(pkt);
    avcodec_free_context(&codec_context);
}
