#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QApplication>
#include <QVariant>
#include <QSqlRecord>
#include <QDialog>
#include "event.h"
#include "mapper.h"
QT_BEGIN_NAMESPACE
namespace Ui { class DatabaseManager; }
QT_END_NAMESPACE

class DatabaseManager : public QDialog
{
    Q_OBJECT

public:
    //DatabaseManager(QWidget *parent = nullptr);
    DatabaseManager(const QString&hostname,const int port,
    const QString&dbname,const QString&username,const QString&password,
                    EventMapper * event_map, CategoryMapper * category_map);
    ~DatabaseManager();
    // sync with local
    bool addEvent(Event* event);
    bool delEvent(Event* event);
    bool modEvent(Event* event);
    // only for initialization
    bool init_for_local();
    // for convenient queries
    // events
    std::vector<std::string> queryEventsByName(const std::string& name);
    std::vector<std::string> queryEventsByDate(const QDate& date);
    std::vector<std::string> queryEventsByDescription(const std::string& description);
    // categories
    std::vector<std::string> queryCategoriesByName(const std::string& name);
    std::vector<std::string> queryCategoriesByDescription(const std::string& description);

    // query how many events (four types respectively)
    std::map<int, int> countEventsByKind();
    // global research. Output: The first item is 'Event' or 'Category', the second is id.
    std::vector<std::pair<std::string, std::string>> globalSearch(const std::string& keyword);



private:
    QSqlDatabase db;
    EventMapper * event_map;
    CategoryMapper * category_map;
    bool init_db_event();
    bool init_db_categories();
    bool init_db_relations();
    bool init_db_rel_cate();
    //Ui::DatabaseManager *ui;
};




#endif // DATABASEMANAGER_H
