#include <QtGui/QApplication>
#include "mainwindow.h"
#include "generatetitle.h"
#include <opendcp.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    GenerateTitle g;
    w.show();

    return a.exec();
}
