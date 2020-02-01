#include "dialog.h"
#include <QApplication>
#include<QString>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Dialog w;
    QString str;
    str = str.fromLocal8Bit("计算机虚拟页式存储管理系统的仿真实现");//修改标题
    w.setWindowTitle(str);
    w.show();//显示窗口
    return a.exec();
}
