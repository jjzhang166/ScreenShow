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
    if(pkg.bytedata.isEmpty())
        return;
    QPixmap pixmap=decode_pixmap(pkg.bytedata,pkg.pic_size);
    if(pixmap.isNull()){
        qDebug()<<"pixmap is null";
        return;
    }
    pixmap.scaled(ui->show_lab->size());
    ui->show_lab->setPixmap(pixmap);
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
    data_pkg.bytedata=ba.right(ba.size()-sizeof(Data_Package_Without_Pixmap));
    return data_pkg;
}


QPixmap PicShow::decode_pixmap(const QByteArray &data,const QSize &pic_size){

}

bool PicShow::init_decoder(){

}


void PicShow::uninit_decoder(){

}
