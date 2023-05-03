#include "PointsSearch.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    PointsSearch w;
    w.show();
    return a.exec();
}
