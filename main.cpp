#include "databasemanager.h"
#include <crow.h>
#include <QThread>
#include <QApplication>
#include "serverthread.h"
#include "connect.h"
int main(int argc,char* argv[])
{
    QApplication a(argc,argv);
    CrowServerThread serverThread(PORT);
    serverThread.start();
    return a.exec();
}

