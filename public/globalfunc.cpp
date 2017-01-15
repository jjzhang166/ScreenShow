#include <globalfunc.h>

#define MAX_ZLIB_BUFFER 1024*1024*2

QByteArray compress(const QByteArray &src){
    QByteArray ret;
    ret.resize(compressBound(src.size()));
    uLongf destlen=ret.size();
    compress2(static_cast<Bytef*>(static_cast<void*>(ret.data())),&destlen,static_cast<const Bytef*>(static_cast<const void*>(src.data())),src.size(),9);
    ret.resize(destlen);
    return ret;
}

QByteArray uncompress(const QByteArray &src){
    QByteArray ret;
    ret.resize(MAX_ZLIB_BUFFER);
    uLongf destlen=MAX_ZLIB_BUFFER;
    uncompress(static_cast<Bytef*>(static_cast<void*>(ret.data())),&destlen,static_cast<const Bytef*>(static_cast<const void*>(src.data())),src.size());
    ret.resize(destlen);
    return ret;
}
