#include "databasemanager.h"
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

/*DatabaseManager::DatabaseManager(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DatabaseManager)
{
    ui->setupUi(this);
}*/

DatabaseManager::DatabaseManager(const QString&hostname,const int port,
                                 const QString&dbname,const QString&username,const QString&password,
                                 EventMapper * event_map_, CategoryMapper * category_map_)
{
    db = QSqlDatabase::addDatabase("QMYSQL");
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
bool DatabaseManager::init_db_event()
{
    QSqlQuery query;
    if (query.exec("SELECT * FROM Events")) {
        QSqlRecord record = query.record();
        while (query.next()) {
            std::string id = query.value(record.indexOf("id")).toString().toStdString();
            std::string name = query.value(record.indexOf("name")).toString().toStdString();
            int kindofevent = query.value(record.indexOf("kindofevent")).toInt();
            std::string description = query.value(record.indexOf("description")).toString().toStdString();
            QDate date = query.value(record.indexOf("date")).toDate();
            QTime time = query.value(record.indexOf("time")).toTime();
            QDate ddl_date = query.value(record.indexOf("ddl_date")).toDate();
            QTime ddl_time =  query.value(record.indexOf("ddl_time")).toTime();
            int inter_time = query.value(record.indexOf("inter_time")).toInt();
            int urgency = query.value(record.indexOf("urgency")).toInt();
            qDebug() << "Init Event:"<<id<<name<<kindofevent<<description<<date<<time<<ddl_date<<ddl_time<<inter_time<<urgency;
            Event* new_event;
            switch(kindofevent)
            {
            case EVENT: new_event= new Event(id, name, kindofevent, description, date,time, ddl_date,ddl_time, inter_time, urgency);
                break;
            case RECURRING: new_event= new RecurringEvent(id, name, kindofevent, description, date,time, inter_time, urgency);
                break;
            case DDL: new_event= new DdlEvent(id, name, kindofevent, description, date,time, urgency);
                break;
            case PROJECT: new_event= new ProjectEvent(id, name, kindofevent, description, date,time, urgency);
                break;
            default: qDebug() << "Init Event Unrec type of event";
            }

            = new Event(id, name, kindofevent, description, date,time, ddl_date,ddl_time, inter_time, urgency);
            if (!new_event)
            {
                qDebug() << "Init Event constructor error";
                return false;
            }
            event_map->addMapping(id, new_event);
        }
        return true;
    }
    else {
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
            Category* newCategory = new Category(uuid, name, description);
            if (!newCategory)
            {
                qDebug() << "Init Category constructor error";
                return false;
            }
            category_map->addMapping(uuid,newCategory);
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
            qDebug()<<parent_id<<child_id<<rel_type;
            Event * parent_p = event_map->getEvent(parent_id);
            Event * child_p = event_map->getEvent(child_id);
            parent_p->addSubEvent(child_p);
            parent_p->addfatherEvent(parent_p);
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
    if (query.exec("SELECT * FROM EventRelations")) {
        QSqlRecord record = query.record();
        while (query.next()) {
            std::string event_id = query.value(record.indexOf("event_id")).toString().toStdString();
            std::string category_id = query.value(record.indexOf("category_id")).toString().toStdString();
            qDebug()<<event_id<<category_id;
            Event * event_p = event_map->getEvent(event_id);
            Category * category_p = category_map->getEvent(category_id);
            event_p->addtag(category_p);
            category_p->addevent(event_p);
        }
        return true;
    }
    else
    {
        qDebug() << "Query failed: " << query.lastError();
        return false;
    }
}
bool DatabaseManager::addEvent(Event * event)
{
    QSqlQuery query;
    query.prepare("INSERT INTO Events (id, name, kindofevent, description, date, time, ddl_date, ddl_time, inter_time, urgency) "
                  "VALUES (:id, :name, :kindofevent, :description, :date, :time, :ddl_date, :ddl_time, :inter_time, :urgency)");
    query.bindValue(":id", event->getId());
    query.bindValue(":name", event->getName());
    query.bindValue(":kindofevent", event->getKindOfEvent()); // Assuming getKindOfEvent is a method in your Event class
    query.bindValue(":description", event->getDescription());
    query.bindValue(":date", event->getDate());
    query.bindValue(":time", event->getTime());
    query.bindValue(":ddl_date", event->getDdlDate()); // Assuming getDdlDate is a method in your Event class
    query.bindValue(":ddl_time", event->getDdlTime()); // Assuming getDdlTime is a method in your Event class
    query.bindValue(":inter_time", event->getInterTime()); // Assuming getInterTime is a method in your Event class
    query.bindValue(":urgency", event->getUrgency());

    if (!query.exec()) {
        qDebug() << "addEvent error:" << query.lastError();
        return false;
    }

    return true;
}

bool DatabaseManager::delEvent(Event * event)
{
    QSqlQuery query;
    query.prepare("DELETE FROM Events WHERE id = :id");
    query.bindValue(":id", event->getId());

    if (!query.exec()) {
        qDebug() << "delEvent error:" << query.lastError();
        return false;
    }

    return true;
}

bool DatabaseManager::modEvent(Event * event)
{
    bool status;
    status = delEvent(event);
    if (status==false){
        qDebug() << "ModEvent error";
        return false;
    }
    status = addEvent(event);
    return status;
}

std::vector<std::string> DatabaseManager::queryEventsByName(const std::string& name)
{
    QSqlQuery query;
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
    QSqlQuery query;
    query.prepare("SELECT id FROM Events WHERE date = :date");
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
    QSqlQuery query;
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
    QSqlQuery query;
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
    QSqlQuery query;
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
    QSqlQuery query;
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
    QSqlQuery query;
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



