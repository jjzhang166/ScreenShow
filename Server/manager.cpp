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


Manager::Manager(QHostAddress ip, quint16 port, QWidget *parent) : QWidget(parent),local_addr(ip),m_port(port)
{
    hide();
    int desk_w=QApplication::desktop()->width();
    int desk_h=QApplication::desktop()->height();
    if(!init_encoder(desk_w,desk_h)){
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
                    QThread::usleep(1);
                    m_sock.writeDatagram(static_cast<char*>(static_cast<void*>(&p)),sizeof(Package),QHostAddress::Broadcast,this->m_port);
                    QThread::usleep(1);
                    m_sock.writeDatagram(static_cast<char*>(static_cast<void*>(&p)),sizeof(Package),QHostAddress::Broadcast,this->m_port);
                    QThread::usleep(1);
                    m_sock.writeDatagram(static_cast<char*>(static_cast<void*>(&p)),sizeof(Package),QHostAddress::Broadcast,this->m_port);
                }
                qDebug()<<"Send a frame";
            }
            QThread::yieldCurrentThread();
        }
    }
                ).detach();




    start_cap();
}

Frame Manager::make_frame(const Data_Package &data_pkg){
    static unsigned char mj_id=0;
    if(data_pkg.bytedata.isEmpty()){
        return Frame{};
    }
    mj_id=mj_id==255?0:mj_id+1;
    QByteArray header;
    QBuffer header_buf(&header);
    header_buf.open(QIODevice::WriteOnly);
    header_buf.write(static_cast<const char*>(static_cast<const void*>(&data_pkg)),sizeof(Data_Package_Without_Pixmap));
    header_buf.close();
    QByteArray ba=header+data_pkg.bytedata;
    ba=compress(ba);
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
    header.pos=cursor().pos();
    header.shape=cursor().shape();
    header.pic_size=pixmap.size();
    header.bytedata=encode_pixmap(pixmap);
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

bool Manager::init_encoder(int width,int height){

}

QByteArray Manager::encode_pixmap(const QPixmap &pixmap){

}


void Manager::uninit_encoder(){

}
