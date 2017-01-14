#ifndef SOCK_H
#define SOCK_H

#include <QObject>
#include <QUdpSocket>
#include <QHostAddress>

class Sock : public QUdpSocket
{
    Q_OBJECT

private:
    QHostAddress local_addr;
    bool init();
public:
    explicit Sock(QHostAddress addr,QObject *parent = 0);

signals:

public slots:
    void on_error(QAbstractSocket::SocketError);
};

#endif // SOCK_H
