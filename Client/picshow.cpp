#include "picshow.h"
#include "ui_picshow.h"
#include <algorithm>
#include <QPainter>
#include <globalfunc.h>

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
    qDebug()<<"read "<<pkg.major_id<<pkg.minor_id<<pkg.pk_num<<pkg.curr_length;
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
    if(pkg.pk_num==1){
        will_show.push_back(pkg);
        has_show=true;
        has_find=true;
    }else{
        for(auto p=buffer.begin();p!=buffer.end();++p){
            if(p->isEmpty())
                continue;
            if(p->at(0).major_id==pkg.major_id){
                has_find=true;
                p->push_back(pkg);
                if(p->size()==pkg.pk_num){
                    qDebug()<<"Will show";
                    will_show=*p;
                    buffer.erase(p);
                    has_show=true;
                }
                break;
            }
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
    Data_Package data_pkg{recover_to_data_pkg(frame)};
    if(data_pkg.pixmap.isNull()){
        qDebug()<<"pixmap Null";
        return;
    }
    if(last_pixmap.isNull()&&data_pkg.full==0){
        qDebug()<<"last Null && not full";
        return;
    }
    if(last_pixmap.isNull()||data_pkg.full==1){
        last_pixmap=data_pkg.pixmap;
    }else{
#ifdef XOR_COMPRESS
        QImage img1=last_pixmap.copy(data_pkg.rect).toImage();
        QImage img2=data_pkg.pixmap.toImage();
        QImage img3(data_pkg.rect.size(),img1.format());
        for(int i=0;i<img1.width();++i){
            for(int j=0;j<img1.height();++j){
                img3.setPixel(i,j,img1.pixel(i,j)^img2.pixel(i,j));
            }
        }

        QPainter painter(&last_pixmap);
        painter.drawPixmap(data_pkg.rect,QPixmap::fromImage(img3));
#else
        QPainter painter(&last_pixmap);
        painter.drawPixmap(data_pkg.rect,data_pkg.pixmap);
#endif
    }
    //TODO 添加鼠标

    QPixmap tmp_pixmap=last_pixmap.scaled(ui->show_lab->size(),Qt::KeepAspectRatio,Qt::SmoothTransformation);
    ui->show_lab->setPixmap(tmp_pixmap);
    qDebug()<<"显示一个Frame";
}

Data_Package PicShow::recover_to_data_pkg(const Frame &frame){
    Frame t_frame{frame};
    std::sort(t_frame.begin(),t_frame.end(),[](const Package &a,const Package &b){return a.minor_id<b.minor_id;});
    QByteArray ba;
    for(auto &p:t_frame)
        ba.append(static_cast<const char*>(static_cast<const void*>((p.data))),p.curr_length);

    ba=uncompress(ba);

    Data_Package data_pkg;
    memmove_s(&data_pkg,sizeof(Data_Package_Without_Pixmap),ba.data(),sizeof(Data_Package_Without_Pixmap));
    data_pkg.pixmap.loadFromData(ba.right(ba.size()-sizeof(Data_Package_Without_Pixmap)));
    return data_pkg;
}
