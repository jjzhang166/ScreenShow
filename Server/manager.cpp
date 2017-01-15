#include "manager.h"
#include <QDesktopWidget>
#include <QApplication>
#include <QBuffer>
#include <thread>
#include <QThread>
#include <QScreen>
#include <globalfunc.h>

#define DISTENCE 5


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
                this->buffer.pop_front();
            }
            this->buffer_mutex.unlock();
            if(!t_frame.isEmpty()){
                for(auto &p:t_frame){
                    QThread::usleep(1);
                    qDebug()<<p.major_id<<p.minor_id<<p.pk_num<<p.curr_length;
                    m_sock.writeDatagram(static_cast<char*>(static_cast<void*>(&p)),sizeof(Package),this->mcast_addr,this->m_port);
                }
                qDebug()<<"发送一个Frame";
            }
            QThread::yieldCurrentThread();
        }
    }

                ).detach();
    start_cap();
}

Frame Manager::make_frame(const Data_Package &data_pkg){
    static unsigned char mj_id=0;
    mj_id=mj_id==255?0:mj_id+1;
    QByteArray ba;
    QBuffer buf(&ba);
    buf.open(QIODevice::WriteOnly);
    data_pkg.pixmap.save(&buf,"JPEG",20);
    buf.close();
    QByteArray header;
    QBuffer header_buf(&header);
    header_buf.open(QIODevice::WriteOnly);
    header_buf.write(static_cast<const char*>(static_cast<const void*>(&data_pkg)),sizeof(Data_Package_Without_Pixmap));
    header_buf.close();
    ba=header+ba;
    qDebug()<<"before "<<ba.size();
    ba=compress(ba);
    qDebug()<<"after "<<ba.size();

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

Data_Package Manager::get_desktop_img(int full){
    QPixmap pixmap{QApplication::primaryScreen()->grabWindow(0)};
    qDebug()<<"Screen "<<pixmap.size();
    Data_Package header;
    header.pos=cursor().pos();
    header.shape=cursor().shape();
    if(full||last_pixmap.size()!=pixmap.size()){
        header.full=1;
        header.pixmap=pixmap;
        header.rect=pixmap.rect();
        last_pixmap=pixmap;
        return header;
    }
    header.full=0;
    header.pixmap=get_update(last_pixmap,pixmap,header.rect);
    last_pixmap=pixmap;
    return header;
}

void Manager::start_cap(){
    static int n=-1;
    while(true){
        buffer_mutex.lock();
        if(buffer.size()>MAX_BUFFER_SIZE){
            buffer_mutex.unlock();
            QThread::yieldCurrentThread();
            continue;
        }
        buffer_mutex.unlock();
        Frame t_frame{make_frame(get_desktop_img((n=(n+1)%50)==0))};
        buffer_mutex.lock();
        if(buffer.size()>MAX_BUFFER_SIZE){
            buffer_mutex.unlock();
            QThread::yieldCurrentThread();
            return;
        }
        buffer.push_back(t_frame);
        buffer_mutex.unlock();
    }
}

QPixmap Manager::get_update(const QPixmap &pix1, const QPixmap &pix2, QRect &rect){
    if(pix1.size()!=pix2.size()){
        rect=pix2.rect();
        return pix2;
    }
    QImage img1=pix1.toImage();
    QImage img2=pix2.toImage();
    auto sz=img1.size();
    int top_point{0};
    int left_point{0};
    int bottom_point{0};
    int right_point{0};

    for(int i=DISTENCE;i<img1.height();i+=DISTENCE){
        for(int j=DISTENCE;j<sz.width();j+=DISTENCE){
            if(img1.pixel(j,i)!=img2.pixel(j,i)){
                top_point=i-DISTENCE;
                left_point=j-DISTENCE;
                goto top_over;
            }
        }
    }
    top_over:
    for(int i=DISTENCE;i<left_point;i+=DISTENCE){
        for(int j=sz.height()-DISTENCE;j>=0;j-=DISTENCE){
            if(img1.pixel(i,j)!=img2.pixel(i,j)){
                left_point=i-DISTENCE;
                bottom_point=j+DISTENCE;
                goto left_over;
            }
        }
    }
    left_over:
    for(int i=sz.height()-DISTENCE;i>=bottom_point;i-=DISTENCE){
        for(int j=sz.width()-DISTENCE;j>=0;j-=DISTENCE){
            if(img1.pixel(j,i)!=img2.pixel(j,i)){
                bottom_point=i;
                right_point=j;
                goto bottom_over;
            }
        }
    }
    bottom_over:
    for(int i=sz.width()-DISTENCE;i>=right_point;i-=DISTENCE){
        for(int j=DISTENCE;j<sz.height();j+=DISTENCE){
            if(img1.pixel(i,j)!=img2.pixel(i,j)){
                right_point=i+DISTENCE;
                top_point=top_point>j?j:top_point;
                goto right_over;
            }
        }
    }
    right_over:
    if(top_point>=bottom_point||left_point>=right_point){
        rect=QRect{0,0,0,0};
        return QPixmap{};
    }
    rect=QRect{left_point,top_point,right_point-left_point,bottom_point-top_point};
    return pix2.copy(rect);
}
