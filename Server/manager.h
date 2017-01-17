#ifndef MANAGER_H
#define MANAGER_H

#include <QObject>
#include <dts.h>
#include <sock.h>
#include <QPixmap>
#include <QWidget>
#include <QTimer>
#include <QMutex>
#include <globalfunc.h>
#include <codec_api.h>



class Manager : public QWidget
{
    Q_OBJECT
private:
    SendBuffer buffer;
    QMutex buffer_mutex;
    QHostAddress mcast_addr{CAST_ADDR};
    QHostAddress local_addr;
    quint16 m_port;
    ISVCEncoder *encoder_;
    bool init_encoder(int width, int height);
    QByteArray encode_pixmap(const QPixmap &pixmap);
    void uninit_encoder();
public:
    explicit Manager(QHostAddress ip, quint16 port, QWidget *parent = 0);
    ~Manager();
    Frame make_frame(const Data_Package &data_pkg);
    Data_Package get_desktop_img();
    void start_cap();
signals:

public slots:

};

#endif // MANAGER_H
