#ifndef GLOBALFUNC_H
#define GLOBALFUNC_H

#include "zlib.h"
#include <QByteArray>

#define MAX_BUFFER_SIZE 100
#define MAX_ZLIB_BUFFER 1024*1024*2
QByteArray compress(const QByteArray &src);
QByteArray uncompress(const QByteArray &src);

#endif // GLOBALFUNC_H
