#include "dialog.h"
#include <QApplication>
#include<QString>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Dialog w;
    QString str;
    str = str.fromLocal8Bit("���������ҳʽ�洢����ϵͳ�ķ���ʵ��");//�޸ı���
    w.setWindowTitle(str);
    w.show();//��ʾ����
    return a.exec();
}
