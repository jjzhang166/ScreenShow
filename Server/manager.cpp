#include "manager.h"
#include <QDesktopWidget>
#include <QApplication>
#include <QBuffer>
#include <thread>
#include <QThread>
#include <QScreen>

Manager::Manager(QHostAddress ip, quint16 port, QWidget *parent) : QWidget(parent),local_addr(ip),m_port(port)
{
    hide();
    std::thread(
                [this](){
        Sock m_sock(this->local_addr);
        Frame t_frame;
        while(true){
            this->buffer_mutex.lock();
            if(!this->buffer.isEmpty()){
                t_frame=this->buffer.front();
                this->buffer.pop_back();
            }
            this->buffer_mutex.unlock();
            if(!t_frame.isEmpty()){
                for(auto &p:t_frame){
                    m_sock.writeDatagram(static_cast<char*>(static_cast<void*>(&p)),sizeof(Package),this->mcast_addr,this->m_port);
                }
                static int i=0;
                qDebug()<<"发送一个Frame"<<i++;
            }
            QThread::yieldCurrentThread();
        }
    }

                ).detach();
    start_cap();
}

Frame Manager::make_frame(QPixmap pixmap){
    static unsigned char mj_id=0;
    mj_id=mj_id==255?0:mj_id+1;
    QByteArray ba;
    QBuffer buf(&ba);
    pixmap.save(&buf,"PNG");
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

QPixmap Manager::get_desktop_img(){
    static int i=0;
    QPixmap pixmap{QApplication::primaryScreen()->grabWindow(0)};
    //TODO: 增加鼠标图像
    return pixmap;
}

void Manager::start_cap(){
    while(true){
        Frame t_frame{make_frame(get_desktop_img())};
        buffer_mutex.lock();
        if(buffer.size()<=MAX_BUFFER_SIZE)
            buffer.push_front(t_frame);
        buffer_mutex.unlock();
    }
}