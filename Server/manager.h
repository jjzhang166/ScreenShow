#ifndef MANAGER_H
#define MANAGER_H

#include <QObject>
#include <dts.h>
#include <sock.h>
#include <QPixmap>
#include <QWidget>
#include <QTimer>
#include <QMutex>


class Manager : public QWidget
{
    Q_OBJECT
private:
    Buffer buffer;
    QMutex buffer_mutex;
    QHostAddress mcast_addr{"224.0.0.17"};
    QHostAddress local_addr;
    quint16 m_port;
    QPixmap last_pixmap;
public:
    explicit Manager(QHostAddress ip, quint16 port, QWidget *parent = 0);
    Frame make_frame(const Data_Package &data_pkg);
    Data_Package get_desktop_img(int full);
    void start_cap();
    QPixmap get_update(const QPixmap &pix1,const QPixmap &pix2,QRect &rect);
signals:

public slots:

};

#endif // MANAGER_H
