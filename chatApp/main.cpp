#include "chatclientform.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ChatClientForm w;
    w.show();
    return a.exec();
}
