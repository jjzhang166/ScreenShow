#ifndef PICSHOW_H
#define PICSHOW_H

#include <QWidget>
#include <sock.h>
#include <dts.h>
#include <QMutex>

#define MAX_BUFFER_SIZE 20

namespace Ui {
class PicShow;
}

class PicShow : public QWidget
{
    Q_OBJECT

private:
    Sock m_sock;
    QHostAddress local_addr;
    quint16 listen_port;
    Buffer buffer;
    QMutex buffer_mutex;
public:
    explicit PicShow(QHostAddress ip, quint16 port,QWidget *parent = 0);
    ~PicShow();

private:
    Ui::PicShow *ui;

private slots:
    void on_read();
    void clear_broken(unsigned char id);
    void add_package_and_show(const Package &pkg);
    void show_frame(const Frame& frame);
    QPixmap recover_to_pixmap(const Frame& frame);
};

#endif // PICSHOW_H
