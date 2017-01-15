#include <QApplication>
#include <picshow.h>
#include <QHostAddress>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    QApplication app(argc,argv);
    if(argc!=3){
        QMessageBox::warning(0,"Warning","参数错误");
        return 0;
    }
    PicShow picshow(QHostAddress(argv[1]),QString::fromLocal8Bit(argv[2]).toUInt());
    picshow.show();
    return app.exec();
}
