#include "sock.h"
#include <QMessageBox>
#include <QDebug>

Sock::Sock(QHostAddress addr,quint16 port,QObject *parent):QUdpSocket(parent),local_addr(addr),listen_port(port)
{
    if(!init()){
        QMessageBox::warning(0,"错误","初始化失败");
        exit(0);
    }
    connect(this,SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(on_error(QAbstractSocket::SocketError)));
}


void Sock::on_error(QAbstractSocket::SocketError){
    qDebug()<<errorString();
    reset();
    if(!init()){
        QMessageBox::warning(0,"错误","出现错误，无法恢复");
        exit(0);
    }
}


bool Sock::init(){
    if(!bind(local_addr,listen_port,QUdpSocket::ShareAddress))
        return false;
    //if(!joinMulticastGroup(multicastAddress))
    //    return false;
    //setSocketOption(QAbstractSocket::MulticastLoopbackOption, 0);
    return true;
}
