#ifndef MANAGER_H
#define MANAGER_H

#include <QObject>
#include <dts.h>
#include <sock.h>
#include <QPixmap>
#include <QWidget>
#include <QTimer>
#include <QMutex>

#define MAX_BUFFER_SIZE 20

class Manager : public QWidget
{
    Q_OBJECT
private:
    Buffer buffer;
    QMutex buffer_mutex;
    QHostAddress mcast_addr{"224.0.0.17"};
    QHostAddress local_addr;
    quint16 m_port;
public:
    explicit Manager(QHostAddress ip, quint16 port, QWidget *parent = 0);
    Frame make_frame(QPixmap pixmap);
    QPixmap get_desktop_img();
    void start_cap();
signals:

public slots:

};

#endif // MANAGER_H
