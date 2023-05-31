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
QT_BEGIN_NAMESPACE
namespace Ui { class DatabaseManager; }
QT_END_NAMESPACE
class Event;
class Category;
class EventMapper;
class CategoryMapper;
class Calendar;
class CategoryOrganizer;
class DatabaseManager : public QDialog
{
    Q_OBJECT

public:
    //DatabaseManager(QWidget *parent = nullptr);
    DatabaseManager(const QString&hostname,const int port,
    const QString&dbname,const QString&username,const QString&password,
                    EventMapper * event_map, CategoryMapper * category_map,
                    Calendar * eventorganizer,
                    CategoryOrganizer * cateorganizer);
    ~DatabaseManager();
    // sync with local
    bool addEvent(Event* event);
    bool delEvent(Event* event);
    bool modEvent(Event* event);
    //sync with local
    bool addCategory(Category * category);
    bool delCategory(Category * category);
    bool modCategory(Category * category);
    //event with cate
    bool addeventcate(Event* event,Category * category);
    bool deleventcate(Event* event,Category * category);
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
    Calendar * eventorganizer;
    CategoryOrganizer * cateorganizer;
    QSqlDatabase db_thread;
    bool openDatabase();
    QString HostName;
    int Port;
    QString UserName;
    QString PassWord;
    QString DatabaseName;
};




#endif // DATABASEMANAGER_H
