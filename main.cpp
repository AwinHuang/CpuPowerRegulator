#include "CpuPowerRegulator.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    CpuPowerRegulator w;
    w.show();
    return a.exec();
}
