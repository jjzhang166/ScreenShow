#ifndef GLOBALFUNC_H
#define GLOBALFUNC_H

#include "zlib.h"
#include <QByteArray>

#define MAX_BUFFER_SIZE 20
#define MAX_ZLIB_BUFFER 1024*1024*2
#define IMG_QUALITY 80
#define DEVIDE_SIZE 10
#define DISTENCE 1
#define FULL_SCREEN_TIME 100
#define CAP_SLEEP 0
#define TRANS_FORMAT "PNG"
//#define XOR_COMPRESS
#define COMPRESS_DATA
#define IMG_SIZE 60

QByteArray compress(const QByteArray &src);
QByteArray uncompress(const QByteArray &src);

#endif // GLOBALFUNC_H
