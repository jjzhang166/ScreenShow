#ifndef GLOBALFUNC_H
#define GLOBALFUNC_H

#include "zlib.h"
#include <QByteArray>
#include <QPixmap>


#define MAX_BUFFER_SIZE 20
#define MAX_ZLIB_BUFFER 1024*1024*2
#define CAST_ADDR "255.255.255.255"
#define FULL_SCREEEN_COUNT 100
#define DISTENCE 10
#define QUALITY 20
#define CAP_SLEEP 20
#define IMG_FORMAT "JPEG"

QByteArray compress(const QByteArray &src);
QByteArray uncompress(const QByteArray &src);
QByteArray codec_pixmap(const QPixmap& pixmap);

#endif // GLOBALFUNC_H
