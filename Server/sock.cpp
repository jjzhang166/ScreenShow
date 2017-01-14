#include "sock.h"
#include <QHostAddress>
#include <QMessageBox>
#include <QDebug>


Sock::Sock(QHostAddress addr, QObject *parent) : QUdpSocket(parent),local_addr(addr)
{
    if(!init()){
        QMessageBox::warning(0,"Warning","绑定端口失败");
        exit(1);
    }
    connect(this,SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(on_error(QAbstractSocket::SocketError)));
}

void Sock::on_error(QAbstractSocket::SocketError){
    qDebug()<<errorString();
    reset();
    if(!init()){
        qDebug()<<"无法从错误中恢复";
        exit(1);
    }
}

bool Sock::init(){
    int i{0};
    for(i=0;i<3&&!bind(local_addr,0,QUdpSocket::ShareAddress);++i);
    return i!=3;
}
