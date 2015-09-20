#include <QtGui/QApplication>
#include "topwidget.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    TopWidget w;
    w.show();
    return a.exec();
}
