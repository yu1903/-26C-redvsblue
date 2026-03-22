#include "cell.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    cell w;
    w.show();
    return QCoreApplication::exec();
}
