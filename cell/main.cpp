#include "cell.h"
#include "StartWindow.h"  // 新增

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
  //  cell w;

    // 先显示启动主界面
    StartWindow w;

    w.show();
    return QCoreApplication::exec();
}
