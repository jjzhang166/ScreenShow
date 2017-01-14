#include <QApplication>
#include <manager.h>
#include <QEventLoop>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    QApplication app(argc,argv);
    if(argc!=3){
        QMessageBox::warning(0,"Warning","参数错误");
        return 0;
    }
    Manager manager(QHostAddress(argv[1]),QString::fromLocal8Bit(argv[2]).toUInt());
    QEventLoop loop;
    QApplication::connect(&manager,SIGNAL(destroyed(QObject*)),&loop,SLOT(deleteLater()));
    loop.exec();
    return app.exec();
}
