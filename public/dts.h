#include <QList>
#include <QMap>
#include <QSet>
#include <QPoint>
#include <QRect>
#include <QPixmap>

#define DATA_BUFFER_LEN 1400

struct Package{
    unsigned char major_id;
    unsigned int minor_id;
    unsigned int pk_num;
    unsigned long long length;
    unsigned short curr_length;
    unsigned char data[DATA_BUFFER_LEN];
}__attribute__ ((aligned(1)));

using Data_Package=QByteArray;
using Frame=QList<Package>;
using SendBuffer=QList<Frame>;
using RecvBuffer=QMap<unsigned int,Frame>;
