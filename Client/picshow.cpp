#include "picshow.h"
#include "ui_picshow.h"
#include <algorithm>
#include <zlib.h>

#define MAX_ZLIB_BUFFER 1024*1024*2

PicShow::PicShow(QHostAddress ip, quint16 port, QWidget *parent) :
    QWidget(parent),
    m_sock(ip,port,parent),
    local_addr(ip),
    listen_port(port),
    ui(new Ui::PicShow)
{
    ui->setupUi(this);
    connect(&m_sock,SIGNAL(readyRead()),this,SLOT(on_read()));
}

PicShow::~PicShow()
{
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
    for(auto p=buffer.begin();p!=buffer.end();++p){
        if(p->isEmpty())
            continue;
        if(p->at(0).major_id==id){
            buffer.erase(p);
            qDebug()<<"delete broken "<<(int)id;
            break;
        }
    }
    buffer_mutex.unlock();
}


void PicShow::add_package_and_show(const Package &pkg){
    buffer_mutex.lock();
    bool has_find=false;
    bool has_show=false;
    Frame will_show;
    for(auto p=buffer.begin();p!=buffer.end();++p){
        if(p->isEmpty())
            continue;
        if(p->at(0).major_id==pkg.major_id){
            has_find=true;
            p->push_back(pkg);
            if(p->size()==pkg.pk_num){
                will_show=*p;
                buffer.erase(p);
                has_show=true;
            }
            break;
        }
    }
    if(!has_find){
        Frame t_frame;
        t_frame.push_back(pkg);
        buffer.push_back(t_frame);
    }
    buffer_mutex.unlock();
    if(has_show){
        show_frame(will_show);
    }
}

void PicShow::show_frame(const Frame &frame){
    qDebug()<<"show frame";
    QPixmap pixmap{recover_to_pixmap(frame)};
    pixmap=pixmap.scaled(ui->show_lab->size(),Qt::KeepAspectRatio,Qt::SmoothTransformation);
    ui->show_lab->setPixmap(pixmap);
}

QPixmap PicShow::recover_to_pixmap(const Frame &frame){
    Frame t_frame{frame};
    std::sort(t_frame.begin(),t_frame.end(),[](const Package &a,const Package &b){return a.minor_id<b.minor_id;});
    QByteArray ba;
    for(auto &p:t_frame)
        ba.append(static_cast<const char*>(static_cast<const void*>((p.data))),p.curr_length);
    QByteArray buf;
    buf.resize(MAX_ZLIB_BUFFER);
    uLongf destlen=MAX_ZLIB_BUFFER;
    uncompress(static_cast<Bytef*>(static_cast<void*>(buf.data())),&destlen,static_cast<Bytef*>(static_cast<void*>(ba.data())),ba.size());
    buf.resize(destlen);
    QPixmap pixmap;
    pixmap.loadFromData(buf);
    return pixmap;
}
