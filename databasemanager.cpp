#include "databasemanager.h"
#include "mapper.h"
#include "time_convert.h"
#include "event.h"
#include "ui_databasemanager.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QApplication>
#include <QVariant>
#include <QSqlRecord>
#include <QDate>
#include <QTime>
#include <QDateTime>
#include<ctime>
#include "uuid_gen.h"
/*DatabaseManager::DatabaseManager(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DatabaseManager)
{
    ui->setupUi(this);
}*/

DatabaseManager::DatabaseManager(const QString&hostname,const int port,
                                 const QString&dbname,const QString&username,const QString&password,
                                 EventMapper * event_map_, CategoryMapper * category_map_,
                                 Calendar * eventorganizer_,
                                 CategoryOrganizer * cateorganizer_)
{
    db = QSqlDatabase::addDatabase("QMYSQL");
    HostName = hostname;
    Port = port;
    DatabaseName = dbname;
    UserName = username;
    PassWord = password;
    db.setHostName(hostname);
    db.setPort(port);
    db.setDatabaseName(dbname);
    db.setUserName(username);
    db.setPassword(password);
    if(!db.open())
    {
        qDebug() << "Fail to connect to the sql server" << db.lastError();
    }
    else
    {
        qDebug() << "Success to connect to the sql server";
    }
    event_map = event_map_;
    category_map = category_map_;
    eventorganizer = eventorganizer_;
    cateorganizer = cateorganizer_;
}

DatabaseManager::~DatabaseManager()
{
    db.close();
    //delete ui;
}
bool DatabaseManager::init_for_local()
{
    bool suc;
    suc = init_db_event();
    if (!suc) qDebug()<<"Database Event init error";
    suc = init_db_categories();
    if (!suc) qDebug()<<"Database categories init error";
    suc = init_db_relations();
    if (!suc) qDebug()<<"Database relations init error";
    suc = init_db_rel_cate();
    if (!suc) qDebug()<<"Database relations(categories) init error";
    return suc;
}
bool DatabaseManager::init_db_event() {
    QSqlQuery query;
    if (query.exec("SELECT * FROM Events")) {
        QSqlRecord record = query.record();
        while (query.next()) {
            std::string id = query.value(record.indexOf("id")).toString().toStdString();
            std::string name = query.value(record.indexOf("name")).toString().toStdString();
            int kindofevent = query.value(record.indexOf("kindofevent")).toInt();
            std::string description = query.value(record.indexOf("description")).toString().toStdString();

            QDateTime createdAt = query.value(record.indexOf("createdAt")).toDateTime();
            QDateTime dueAt = query.value(record.indexOf("dueAt")).toDateTime();
            QDateTime finishedAt = query.value(record.indexOf("finishedAt")).toDateTime();
            QDateTime updatedAt = query.value(record.indexOf("updatedAt")).toDateTime();

            std::tm tm_createdAt = convertToTm(createdAt.date(), createdAt.time());
            std::tm tm_dueAt = convertToTm(dueAt.date(), dueAt.time());
            std::tm tm_finishedAt = convertToTm(finishedAt.date(), finishedAt.time());
            std::tm tm_updatedAt = convertToTm(updatedAt.date(), updatedAt.time());

            int inter_time = query.value(record.indexOf("inter_time")).toInt();
            int urgency = query.value(record.indexOf("urgency")).toInt();
            int status = query.value(record.indexOf("status")).toInt();

            qDebug() << "Init Event:" << id << name << kindofevent << description
                     << createdAt << dueAt << finishedAt << updatedAt << inter_time << urgency << status;

            Event* new_event;
            switch(kindofevent) {
            case EVENT:
                new_event = new Event(this,id, name, tm_dueAt, description, urgency,kindofevent, tm_createdAt,
                                      tm_finishedAt,tm_updatedAt);
                break;
            case RECURRING:
                new_event = new RecurringEvent(this,id, name, tm_dueAt, description, urgency, inter_time, tm_createdAt,
                                               tm_finishedAt,tm_updatedAt);
                break;
            case DDL:
                new_event = new DdlEvent(this,id, name, tm_dueAt, description, urgency,tm_dueAt, tm_createdAt,
                                         tm_finishedAt,tm_updatedAt);
                break;
            case PROJECT:
                new_event = new ProjectEvent(this,id, name, tm_dueAt, description, urgency, tm_createdAt,
                                             tm_finishedAt,tm_updatedAt);
                break;
            default:
                qDebug() << "Init Event Unrec type of event";
            }

            if (!new_event) {
                qDebug() << "Init Event constructor error";
                return false;
            }
            event_map->addMapping(id, new_event);
            eventorganizer->addEvent(new_event);
        }
        return true;
    } else {
        qDebug() << "Query failed: " << query.lastError();
        return false;
    }
}

bool DatabaseManager::init_db_categories()
{
    QSqlQuery query;
    if (query.exec("SELECT * FROM Categories")) {
        QSqlRecord record = query.record();
        while (query.next()) {
            std::string uuid = query.value(record.indexOf("id")).toString().toStdString();
            std::string name = query.value(record.indexOf("name")).toString().toStdString();
            std::string description = query.value(record.indexOf("description")).toString().toStdString();
            qDebug()<<uuid<<name<<description;
            Category* newCategory = new Category(this,uuid, name);

            if (!newCategory)
            {
                qDebug() << "Init Category constructor error";
                return false;
            }
            category_map->addMapping(uuid,newCategory);
            cateorganizer->addCategory(newCategory->getId(),newCategory);
            //controlFunction(newCategory);
        }
        return true;
    }
    else
    {
        qDebug() << "Query failed: " << query.lastError();
        return false;
    }
}
bool DatabaseManager::init_db_relations()
{
    QSqlQuery query;
    if (query.exec("SELECT * FROM EventRelations")) {
        QSqlRecord record = query.record();
        while (query.next()) {
            std::string parent_id = query.value(record.indexOf("parent_id")).toString().toStdString();
            std::string child_id = query.value(record.indexOf("child_id")).toString().toStdString();
            std::string rel_type = query.value(record.indexOf("relation_type")).toString().toStdString();
            qDebug()<<"Init relations"<<parent_id<<child_id<<rel_type;
            Event * parent_p = event_map->getEvent(parent_id);
            Event * child_p = event_map->getEvent(child_id);
            parent_p->addSubEvent(child_p);
            parent_p->addFatherEvent(parent_p);
        }
        return true;
    }
    else
    {
        qDebug() << "Query failed: " << query.lastError();
        return false;
    }
}
bool DatabaseManager::init_db_rel_cate()
{
    QSqlQuery query;
    if (query.exec("SELECT * FROM EventCategories")) {
        QSqlRecord record = query.record();
        while (query.next()) {
            std::string event_id = query.value(record.indexOf("event_id")).toString().toStdString();
            std::string category_id = query.value(record.indexOf("category_id")).toString().toStdString();
            qDebug()<<"Init relation categories"<<event_id<<category_id;
            Event * event_p = event_map->getEvent(event_id);
            Category * category_p = category_map->getCategory(category_id);
            event_p->addTag(category_id);
            category_p->addEvent(event_p);
        }
        return true;
    }
    else
    {
        qDebug() << "Query failed: " << query.lastError();
        return false;
    }
}
bool DatabaseManager::addEvent(Event * event) {
    if (!openDatabase()) {
        qDebug() << "Database could not be opened!";
        return false;
    }
    QSqlQuery query(db);
    query.prepare("INSERT IGNORE INTO Events (id, name, kindofevent, description, createdAt, dueAt, finishedAt, updatedAt, inter_time, urgency, status) "
                  "VALUES (:id, :name, :kindofevent, :description, :createdAt, :dueAt, :finishedAt, :updatedAt, :inter_time, :urgency, :status)");
    query.bindValue(":id", QString::fromStdString(event->getId()));
    query.bindValue(":name", QString::fromStdString(event->getTitle()));
    query.bindValue(":kindofevent", event->getKindOfEvent());
    query.bindValue(":description", QString::fromStdString(event->getDescription()));

    QDateTime createdAt = convertToQDateTime(event->getCreateTime());
    query.bindValue(":createdAt", createdAt);

    QDateTime dueAt = convertToQDateTime(event->getTime());
    query.bindValue(":dueAt", dueAt);

    QDateTime finishedAt = convertToQDateTime(event->getFinishTime());
    query.bindValue(":finishedAt", finishedAt);

    QDateTime updatedAt = convertToQDateTime(event->getUpdateTime());
    query.bindValue(":updatedAt", updatedAt);

    query.bindValue(":inter_time", event->getInterval());
    query.bindValue(":urgency", event->getUrgency());
   // query.bindValue(":status", event->getStatus()); // Assuming getStatus is a method in your Event class

    if (!query.exec()) {
        qDebug() << "addEvent error:" << query.lastError();
        return false;
    }
    return true;
}


bool DatabaseManager::delEvent(Event * event)
{
    if (!openDatabase()) {
        qDebug() << "Database could not be opened!";
        return false;
    }
    QSqlQuery query(db);
    query.prepare("DELETE FROM EventCategories WHERE event_id = :event_id");
    query.bindValue(":event_id", QString::fromStdString(event->getId()));

    if (!query.exec()) {
        qDebug() << "delEvent error(del cate relation):" << query.lastError();
        return false;
    }

    query.prepare("DELETE FROM Events WHERE id = :id");
    query.bindValue(":id", QString::fromStdString(event->getId()));

    if (!query.exec()) {
        qDebug() << "delEvent error(del event):" << query.lastError();
        return false;
    }

    return true;
}

bool DatabaseManager::modEvent(Event * event)
{
    if (!openDatabase()) {
        qDebug() << "Database could not be opened!";
        return false;
    }
    QSqlQuery query(db);

    query.prepare("UPDATE Events SET "
                  "name = :name, "
                  "kindofevent = :kindofevent, "
                  "description = :description, "
                  "createdAt = :createdAt, "
                  "dueAt = :dueAt, "
                  "finishedAt = :finishedAt, "
                  "updatedAt = :updatedAt, "
                  "inter_time = :inter_time, "
                  "urgency = :urgency "
                  "WHERE id = :id");

    query.bindValue(":id", QString::fromStdString(event->getId()));
    query.bindValue(":name", QString::fromStdString(event->getTitle()));
    query.bindValue(":kindofevent", event->getKindOfEvent());
    query.bindValue(":description", QString::fromStdString(event->getDescription()));
    query.bindValue(":createdAt", convertToQDateTime(event->getCreateTime()).toString(Qt::ISODate));
    query.bindValue(":dueAt", convertToQDateTime(event->getDeadline()).toString(Qt::ISODate));
    query.bindValue(":finishedAt", convertToQDateTime(event->getFinishTime()).toString(Qt::ISODate));
    query.bindValue(":updatedAt", convertToQDateTime(event->getUpdateTime()).toString(Qt::ISODate));
    query.bindValue(":inter_time", event->getInterval());
    query.bindValue(":urgency", event->getUrgency());

    if (!query.exec()) {
        qDebug() << "Failed to modify event: " << query.lastError();
        return false;
    }

    return true;
}

std::vector<std::string> DatabaseManager::queryEventsByName(const std::string& name)
{
    if (!openDatabase()) {
        qDebug() << "Database could not be opened!";
        return std::vector<std::string>();
    }
    QSqlQuery query(db);
    query.prepare("SELECT id FROM Events WHERE name LIKE :name");
    query.bindValue(":name", "%" + QString::fromStdString(name) + "%");

    std::vector<std::string> ids;
    if (query.exec()) {
        while (query.next()) {
            ids.push_back(query.value(0).toString().toStdString());
        }
    } else {
        qDebug() << "queryEventsByName error:" << query.lastError();
    }

    return ids;
}


std::vector<std::string> DatabaseManager::queryEventsByDate(const QDate& date)
{
    if (!openDatabase()) {
        qDebug() << "Database could not be opened!";
        return std::vector<std::string>();
    }
    QSqlQuery query(db);
    query.prepare("SELECT id FROM Events WHERE DATE(dueAt) = :date");
    query.bindValue(":date", date);

    std::vector<std::string> ids;
    if (query.exec()) {
        while (query.next()) {
            ids.push_back(query.value(0).toString().toStdString());
        }
    } else {
        qDebug() << "queryEventsByDate error:" << query.lastError();
    }

    return ids;
}


std::vector<std::string> DatabaseManager::queryEventsByDescription(const std::string& description)
{
    if (!openDatabase()) {
        qDebug() << "Database could not be opened!";
        return std::vector<std::string>();
    }
    QSqlQuery query(db);
    query.prepare("SELECT id FROM Events WHERE description LIKE :description");
    query.bindValue(":description", "%" + QString::fromStdString(description) + "%");

    std::vector<std::string> ids;
    if (query.exec()) {
        while (query.next()) {
            ids.push_back(query.value(0).toString().toStdString());
        }
    } else {
        qDebug() << "queryEventsByDescription error:" << query.lastError();
    }

    return ids;
}

std::vector<std::string> DatabaseManager::queryCategoriesByName(const std::string& name)
{
    if (!openDatabase()) {
        qDebug() << "Database could not be opened!";
        return std::vector<std::string>();
    }
    QSqlQuery query(db);
    query.prepare("SELECT id FROM Categories WHERE name LIKE :name");
    query.bindValue(":name", "%" + QString::fromStdString(name) + "%");

    std::vector<std::string> ids;
    if (query.exec()) {
        while (query.next()) {
            ids.push_back(query.value(0).toString().toStdString());
        }
    } else {
        qDebug() << "queryCategoriesByName error:" << query.lastError();
    }

    return ids;
}

std::vector<std::string> DatabaseManager::queryCategoriesByDescription(const std::string& description)
{
    if (!openDatabase()) {
        qDebug() << "Database could not be opened!";
        return std::vector<std::string>();
    }
    QSqlQuery query(db);
    query.prepare("SELECT id FROM Categories WHERE description LIKE :description");
    query.bindValue(":description", "%" + QString::fromStdString(description) + "%");

    std::vector<std::string> ids;
    if (query.exec()) {
        while (query.next()) {
            ids.push_back(query.value(0).toString().toStdString());
        }
    } else {
        qDebug() << "queryCategoriesByDescription error:" << query.lastError();
    }

    return ids;
}
std::map<int, int> DatabaseManager::countEventsByKind()
{
    if (!openDatabase()) {
        qDebug() << "Database could not be opened!";
        return std::map<int, int>();
    }
    QSqlQuery query(db);
    query.prepare("SELECT kindofevent, COUNT(*) "
                  "FROM Events "
                  "GROUP BY kindofevent");

    std::map<int, int> counts;
    if (query.exec()) {
        while (query.next()) {
            counts[query.value(0).toInt()] = query.value(1).toInt();
        }
    } else {
        qDebug() << "countEventsByKind error:" << query.lastError();
    }

    return counts;
}
std::vector<std::pair<std::string, std::string>> DatabaseManager::globalSearch(const std::string& keyword)
{
    if (!openDatabase()) {
        qDebug() << "Database could not be opened!";
        return std::vector<std::pair<std::string, std::string>>(0);
    }
    QSqlQuery query(db);
    query.prepare("SELECT 'Event', id "
                  "FROM Events "
                  "WHERE name LIKE :keyword OR description LIKE :keyword "
                  "UNION ALL "
                  "SELECT 'Category', id "
                  "FROM Categories "
                  "WHERE name LIKE :keyword OR description LIKE :keyword");
    query.bindValue(":keyword", "%" + QString::fromStdString(keyword) + "%");

    std::vector<std::pair<std::string, std::string>> results;
    if (query.exec()) {
        while (query.next()) {
            results.push_back(std::make_pair(query.value(0).toString().toStdString(), query.value(1).toString().toStdString()));
        }
    } else {
        qDebug() << "globalSearch error:" << query.lastError();
    }

    return results;
}
bool DatabaseManager::addCategory(Category* category)
{
    if (!openDatabase()) {
        qDebug() << "Database could not be opened!";
        return false;
    }
    QSqlQuery query(db);
    query.prepare("INSERT IGNORE INTO Categories (id, name) "
                  "VALUES (:id, :name)");
    query.bindValue(":id", QString::fromStdString(category->getId()));
    query.bindValue(":name", QString::fromStdString(category->getTag()));

    if (!query.exec()) {
        qDebug() << "addCategory error:  "
                 << query.lastError();
        return false;
    }
    return true;
}
bool DatabaseManager::modCategory(Category* category)
{
    if (!openDatabase()) {
        qDebug() << "Database could not be opened!";
        return false;
    }
    QSqlQuery query(db);
    query.prepare("UPDATE Categories SET "
                  "name = :name "
                  "WHERE id = :id");
    query.bindValue(":name", QString::fromStdString(category->getTag()));
    query.bindValue(":id", QString::fromStdString(category->getId()));
    if (!query.exec()) {
        qDebug() << "Failed to modify Category: " << query.lastError();
        return false;
    }
    return true;
}
bool DatabaseManager::delCategory(Category* category)
{
    if (!openDatabase()) {
        qDebug() << "Database could not be opened!";
        return false;
    }
    QSqlQuery query(db);
    query.prepare("DELETE FROM EventCategories WHERE category_id = :category_id");
    query.bindValue(":category_id", QString::fromStdString(category->getId()));

    if (!query.exec()) {
        qDebug() << "delCategory error(del relations):  "
                 << query.lastError();
        return false;
    }
    query.prepare("DELETE FROM Events WHERE NOT EXISTS (SELECT * FROM EventCategories "
                  "WHERE EventCategories.event_id = Events.id) ");
    if (!query.exec()) {
        qDebug() << "delCategory error(del cate sub tasks):  "
                 << query.lastError();
        return false;
    }
    query.prepare("DELETE FROM Categories WHERE id = :id");
    query.bindValue(":id", QString::fromStdString(category->getId()));

    if (!query.exec()) {
        qDebug() << "delCategory error(del categories):  "
                 << query.lastError();
        return false;
    }

    return true;
}


bool DatabaseManager::openDatabase() {
    db = QSqlDatabase::addDatabase("QMYSQL", QUuid::createUuid().toString());

    db.setHostName(HostName);
    db.setPort(Port);
    db.setUserName(UserName);
    db.setPassword(PassWord);
    db.setDatabaseName(DatabaseName);

    if (!db.open()) {
        qDebug() << "Could not open database: " << db.lastError();
        return false;
    }

    return true;
}
bool DatabaseManager::addeventcate(Event* event, Category * category)
{
    if (!openDatabase()) {
        qDebug() << "Database could not be opened!";
        return false;
    }
    QSqlQuery query(db);
    query.prepare("INSERT INTO EventCategories (event_id, category_id) VALUES (:event_id, :category_id)");
    query.bindValue(":event_id", QString::fromStdString(event->getId()));
    query.bindValue(":category_id", QString::fromStdString(category->getId()));

    if (!query.exec()) {
        qDebug() << "addeventcate error: " << query.lastError();
        return false;
    }

    return true;
}
bool DatabaseManager::deleventcate(Event* event, Category * category)
{
    if (!openDatabase()) {
        qDebug() << "Database could not be opened!";
        return false;
    }
    QSqlQuery query(db);
    query.prepare("DELETE FROM EventCategories WHERE event_id = :event_id AND category_id = :category_id");
    query.bindValue(":event_id", QString::fromStdString(event->getId()));
    query.bindValue(":category_id", QString::fromStdString(category->getId()));

    if (!query.exec()) {
        qDebug() << "deleventcate error: " << query.lastError();
        return false;
    }

    return true;
}

