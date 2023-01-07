#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setStyle("fusion");
    QCoreApplication::setOrganizationName("cuteKB");
    QCoreApplication::setOrganizationDomain("cuteKB.pl");
    QCoreApplication::setApplicationName("cuteKB");
    MainWindow w;
    QSize size(16,16);
    w.setIconSize(size);
    w.show();

    return a.exec();
}
