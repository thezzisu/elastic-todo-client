#include "databasemanager.h"
#include <crow.h>
#include <QThread>
#include <QApplication>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QVariant>
#include <QSqlRecord>
#include "mapper.h"
#include "serverthread.h"
#include "event.h"
int main(int argc,char* argv[])
{
    QApplication a(argc,argv);
    CrowServerThread serverThread(34567);
    serverThread.start();
    return a.exec();
}

