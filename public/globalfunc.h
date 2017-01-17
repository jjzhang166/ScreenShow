#ifndef GLOBALFUNC_H
#define GLOBALFUNC_H

#include "zlib.h"
#include <QByteArray>

#define MAX_BUFFER_SIZE 20
#define MAX_ZLIB_BUFFER 1024*1024*2
#define IMG_QUALITY 50
#define DEVIDE_SIZE 10
#define DISTENCE 5
#define FULL_SCREEN_TIME 100
#define CAP_SLEEP 30

QByteArray compress(const QByteArray &src);
QByteArray uncompress(const QByteArray &src);

#endif // GLOBALFUNC_H
