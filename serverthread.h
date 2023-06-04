#ifndef SERVERTHREAD_H
#define SERVERTHREAD_H
#include <crow.h>
#include <QThread>
#include <QApplication>
#include <QDebug>
#include <QVariant>
class EventMapper;
class CategoryMapper;
class Calendar;
class CategoryOrganizer;
class DatabaseManager;
class CrowServerThread : public QThread {
public:
    CrowServerThread(int port_);
protected:
    void run() override;
private:
    EventMapper* event_map;
    CategoryMapper* category_map;
    int port;
    CategoryOrganizer * cateorganizer;
    Calendar * eventorganizer;
    DatabaseManager * dbm;
    DatabaseManager * init_dbm;
};

#endif // SERVERTHREAD_H
