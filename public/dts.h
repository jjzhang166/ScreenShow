#include <QList>
#include <QMap>
#include <QSet>
#include <QPoint>
#include <QRect>
#include <QPixmap>

#define DATA_BUFFER_LEN 450

struct Package{
    unsigned char major_id;
    unsigned int minor_id;
    unsigned int pk_num;
    unsigned long long length;
    unsigned short curr_length;
    unsigned char data[DATA_BUFFER_LEN];
}__attribute__ ((aligned(1)));

struct Data_Package{
    char full;
    QPoint pos;
    Qt::CursorShape shape;
    QRect rect;
    QPixmap pixmap;
}__attribute__ ((aligned(1)));

struct Data_Package_Without_Pixmap{
    char full;
    QPoint pos;
    Qt::CursorShape shape;
    QRect rect;
}__attribute__ ((aligned(1)));

using Frame=QList<Package>;
using SendBuffer=QList<Frame>;
using RecvBuffer=QMap<unsigned int,Frame>;
