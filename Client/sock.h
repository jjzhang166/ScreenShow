#ifndef SOCK_H
#define SOCK_H
#include <QUdpSocket>
#include <QHostAddress>

class Sock : public QUdpSocket
{
    Q_OBJECT

private:
    QHostAddress multicastAddress{"224.0.0.17"};
    QHostAddress local_addr;
    quint16 listen_port;

    bool init();

private slots:
    void on_error(QAbstractSocket::SocketError);

public:
    Sock(QHostAddress addr,quint16 port,QObject *parent = 0);
};

#endif // SOCK_H
