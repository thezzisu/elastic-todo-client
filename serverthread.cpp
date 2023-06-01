#include <crow.h>
#include <QThread>
#include <QApplication>
#include <QDebug>
#include <QVariant>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include "event.h"
#include "mapper.h"
#include "serverthread.h"
#include "uuid_gen.h"
#include "databasemanager.h"
#include "time_convert.h"
#include "connect.h"
    CrowServerThread::CrowServerThread(int port_)
    {
        event_map = new EventMapper();
        category_map = new CategoryMapper();

        eventorganizer = new Calendar();
        cateorganizer = new CategoryOrganizer();
        port = port_;

        /*init_dbm = new DatabaseManager("103.140.229.50",3306,"stu2100012734","stu2100012734",
                                                  "stu2100012734",event_map,category_map,eventorganizer,cateorganizer);
        if (!init_dbm->init_for_local())
            qDebug()<<"DB Init error"<<Qt::endl;*/
    }

    void CrowServerThread::run(){
        crow::SimpleApp app;
        //open another driver for the service thread
        dbm = new DatabaseManager(DBADDR,DBPORT,DBUSERNAME,DBPASSWORD,
                                  DBDATABASE,event_map,category_map,eventorganizer,cateorganizer);
        if (!dbm->init_for_local())
            qDebug()<<"DB (another Thread) error";
        // a test for server
        CROW_ROUTE(app, "/multiply/<int>/<int>")
        ([](const crow::request& req, int a, int b){
            crow::json::wvalue x;
            x["number1"] = a;
            x["number2"] = b;
            x["product"] = a * b;
            return x;
        });

        // lists(categories)
        /* list lists(categories) */
        CROW_ROUTE(app, "/me/todo/lists")
        ([this](){
            std::map<std::string, Category*> todoLists =
            cateorganizer->getAllCategories();

            QJsonArray array;
            for (const auto& todoList : todoLists) {
                QJsonObject list;
                list["id"] = QString::fromStdString(todoList.first);
                list["displayName"] = QString::fromStdString((todoList.second)->getTag());
                array.push_back(list);
            }
            QJsonObject finalObject;
            finalObject["value"] = array;

            QJsonDocument doc(finalObject);
            std::string jsonString = doc.toJson(QJsonDocument::Compact).toStdString();

            return crow::response(200,jsonString);
        });


        /* get list(category) */
        CROW_ROUTE(app, "/me/todo/lists/<string>")
        ([&](const std::string& todoTaskListId){
            if (!(category_map->CategoryidExists)(todoTaskListId)) {
                crow::json::wvalue x;
                x["error"] = "no such list(category)";
                return crow::response(404,x);
            }
            Category* todoList = cateorganizer->getCategory(todoTaskListId);
            crow::json::wvalue x;
            x["id"] = todoList->getId();
            x["displayName"] = todoList->getTag();
            return crow::response(200, x);
        });
        /* Add list(Category) */
        CROW_ROUTE(app, "/me/todo/lists").methods("POST"_method)
            ([&](const crow::request& req){
                auto body = crow::json::load(req.body);
                if (!body) {
                    // parsing error
                    crow::json::wvalue error;
                    error["error"] = "Invalid JSON";
                    return crow::response(400, error);
                }
                // get the name
                std::string displayName = body["displayName"].s();
                std::string temp_uuid = generateUuid();
                Category * new_cate = new Category(false,dbm,temp_uuid,displayName);
                cateorganizer->addCategory(temp_uuid,new_cate);

                category_map->addMapping(temp_uuid,new_cate);
                Category* todoList = cateorganizer->getCategory(temp_uuid);
                crow::json::wvalue x;
                x["id"] = todoList->getId();
                x["displayName"] = todoList->getTag();
                return crow::response(201, x);
            });
        /* Edify Category */
        CROW_ROUTE(app, "/me/todo/lists/<string>").methods("PATCH"_method)
            ([&](const crow::request& req, const std::string& todoTaskListId){
                // parse the json first.
                auto body = crow::json::load(req.body);
                if (!body) {
                    // Show the error in parsing
                    crow::json::wvalue error;
                    error["error"] = "Invalid JSON";
                    return crow::response(400, error);
                }

                if (!(category_map->CategoryidExists)(todoTaskListId))
                {
                    // there is no such cate id
                    crow::json::wvalue error;
                    error["error"] = "No Such Category(List)";
                    return crow::response(404, error);
                }

                // get displayName
                std::string displayName = body["displayName"].s();

                // update the list
                ((category_map->getCategory)(todoTaskListId))->setTag(displayName);

                crow::json::wvalue x;
                x["id"] = todoTaskListId;
                x["displayName"] = ((category_map->getCategory)(todoTaskListId))->getTag();
                return crow::response(200, x);
            });
        /* Delete list Category */
        CROW_ROUTE(app, "/me/todo/lists/<string>").methods("DELETE"_method)
            ([&](const std::string& todoTaskListId){

                if ((category_map->CategoryidExists)(todoTaskListId)) {
                    cateorganizer->deleteCategory(todoTaskListId);
                    dbm->delCategory(category_map->getCategory(todoTaskListId));
                    category_map->removeMapping(todoTaskListId);
                    crow::json::wvalue x;
                    x["suc"] = "suc no content";
                    return crow::response(204,x);
                } else {
                    crow::json::wvalue error;
                    error["error"] = "Todo list not found";
                    return crow::response(404, error);
                }
            });


        //for events(tasks)
        CROW_ROUTE(app, "/me/todo/lists/<string>/tasks").methods("GET"_method)
            ([&](const std::string& todoTaskListId){
                if (!category_map->CategoryidExists(todoTaskListId))
                {
                    crow::json::wvalue x;
                    x["error"] = "no such list";
                    return crow::response(404,x);
                }
                Category * alist= (cateorganizer->getCategory)(todoTaskListId);
                std::set<Event*> tasks = alist->getAllEvents();

                QJsonArray taskArray;
                for (const auto& task : tasks) {
                    QJsonObject taskJson;
                    taskJson["id"] = QString::fromStdString(task->getId());
                    taskJson["importance"] = QJsonValue::fromVariant(task->getUrgency());
                    taskJson["title"] = QJsonValue::fromVariant(QString::fromStdString(task->getTitle()));
                    taskJson["description"] = QJsonValue::fromVariant(QString::fromStdString(task->getDescription()));
                    taskJson["createdAt"] = QJsonValue::fromVariant(convertToQDateTime(task->getCreateTime()).toString(Qt::ISODate));
                    taskJson["dueAt"] = QJsonValue::fromVariant(convertToQDateTime(task->getTime()).toString(Qt::ISODate));
                    taskJson["finishedAt"] = QJsonValue::fromVariant(convertToQDateTime(task->getFinishTime()).toString(Qt::ISODate));
                    taskJson["updatedAt"] = QJsonValue::fromVariant(convertToQDateTime(task->getUpdateTime()).toString(Qt::ISODate));
                    taskArray.push_back(taskJson);
                }
                QJsonObject rootObj;
                rootObj["value"] = taskArray;
                QString jsonString = QJsonDocument(rootObj).toJson();

                return crow::response(200, jsonString.toUtf8().data());
            });

        CROW_ROUTE(app, "/me/todo/lists/<string>/tasks/<string>").methods("GET"_method)
            ([&](const std::string& todoTaskListId, const std::string& taskId){
                if (!event_map->EventidExists(taskId))
                {
                    crow::json::wvalue x;
                    x["error"] = "no such task";
                    return crow::response(404,x);
                }
                Event* task = event_map->getEvent(taskId);
                crow::json::wvalue taskJson;
                taskJson["id"] = task->getId();
                taskJson["importance"] = task->getUrgency();
                taskJson["title"] = task->getTitle();
                taskJson["description"] = task->getDescription();
                taskJson["createdAt"] = convertToQDateTime(task->getCreateTime()).toString(Qt::ISODate).toStdString();
                taskJson["dueAt"] = convertToQDateTime(task->getTime()).toString(Qt::ISODate).toStdString();
                taskJson["finishedAt"] = convertToQDateTime(task->getFinishTime()).toString(Qt::ISODate).toStdString();
                taskJson["updatedAt"] = convertToQDateTime(task->getUpdateTime()).toString(Qt::ISODate).toStdString();
                return crow::response(200, taskJson);
            });
        /* add a new task */
        CROW_ROUTE(app, "/me/todo/lists/<string>/tasks").methods("POST"_method)
            ([&](const crow::request& req, const std::string& todoTaskListId) {
                auto body = crow::json::load(req.body);
                if (!body) {
                    // Show the error in parsing
                    crow::json::wvalue error;
                    error["error"] = "Invalid JSON";
                    return crow::response(400, error);
                }
                if (!category_map->CategoryidExists(todoTaskListId))
                {
                    crow::json::wvalue x;
                    x["error"] = "no such list";
                    return crow::response(404,x);
                }
                std::string title = body["title"].s();
                std::string description = body["description"].s();
                int importance = body["importance"].i();

                std::string createdAt = body["createdAt"].s();
                std::string dueAt = body["dueAt"].s();
                std::string finishedAt = body["finishedAt"].s();
                std::string updatedAt = body["updatedAt"].s();

                std::string new_uuid = generateUuid();
                int kindoftype = EVENT;
                Event* newTask = new Event(false,dbm,new_uuid,title,isoToTm(dueAt),description,importance,
                                           kindoftype,isoToTm(createdAt),isoToTm(finishedAt),isoToTm(updatedAt));
                Category* thecate = category_map->getCategory(todoTaskListId);
                thecate->addEvent(newTask);
                event_map->addMapping(new_uuid,newTask);
                eventorganizer->addEvent(newTask);
                newTask->addTag(todoTaskListId);
                dbm->addeventcate(newTask,thecate);
                crow::json::wvalue x;
                x["id"] = newTask->getId();
                x["importance"] = newTask->getUrgency();
                x["title"] = newTask->getTitle();
                x["description"] = newTask->getDescription();
                x["createdAt"] = convertToQDateTime(newTask->getCreateTime()).toString(Qt::ISODate).toStdString();
                x["dueAt"] = convertToQDateTime(newTask->getTime()).toString(Qt::ISODate).toStdString();
                x["finishedAt"] = convertToQDateTime(newTask->getFinishTime()).toString(Qt::ISODate).toStdString();
                x["updatedAt"] = convertToQDateTime(newTask->getUpdateTime()).toString(Qt::ISODate).toStdString();

                return crow::response(201, x);
            });


        CROW_ROUTE(app, "/me/todo/lists/<string>/tasks/<string>").methods("PATCH"_method)
            ([&](const crow::request& req, const std::string& todoTaskListId, const std::string& taskId){
                auto body = crow::json::load(req.body);

                std::string title = body["title"].s();
                std::string description = body["description"].s();
                std::string createdAt = body["createdAt"].s();
                std::string dueAt = body["dueAt"].s();
                std::string finishedAt = body["finishedAt"].s();
                std::string updatedAt = body["updatedAt"].s();
                int urgency = body["importance"].i();

                // Get the event using taskId
                if (!event_map->EventidExists(taskId))
                {
                    // cant find the task
                    crow::json::wvalue error;
                    error["error"] = "Task not found";
                    return crow::response(404, error);
                }
                Event* eventToUpdate = event_map->getEvent(taskId);
                if (eventToUpdate) {
                    // save the sql resources
                    if (eventToUpdate->getTitle()!=title)
                        eventToUpdate->setTitle(title);
                    if (eventToUpdate->getDescription()!=description)
                        eventToUpdate->setDescription(description);
                    if (tmToIso(eventToUpdate->getCreateTime())!=createdAt)
                        eventToUpdate->setCreateTime(isoToTm(createdAt));
                    if (tmToIso(eventToUpdate->getTime())!=dueAt)
                        eventToUpdate->setTime(isoToTm(dueAt));
                    if (tmToIso(eventToUpdate->getFinishTime())!=finishedAt)
                        eventToUpdate->setFinishTime(isoToTm(finishedAt));
                    if (tmToIso(eventToUpdate->getUpdateTime())!=updatedAt)
                        eventToUpdate->setUpdateTime(isoToTm(updatedAt));
                    if (eventToUpdate->getUrgency()!=urgency)
                        eventToUpdate->setUrgency(urgency);
                }

                crow::json::wvalue updatedTask;
                updatedTask["id"] = eventToUpdate->getId();
                updatedTask["title"] = eventToUpdate->getTitle();
                updatedTask["description"] = eventToUpdate->getDescription();
                updatedTask["createdAt"] = tmToIso(eventToUpdate->getCreateTime());
                updatedTask["dueAt"] = tmToIso(eventToUpdate->getTime());
                updatedTask["finishedAt"] = tmToIso(eventToUpdate->getFinishTime());
                updatedTask["updatedAt"] = tmToIso(eventToUpdate->getUpdateTime());
                updatedTask["importance"] = eventToUpdate->getUrgency();
                return crow::response(200, updatedTask);
            });


        CROW_ROUTE(app, "/me/todo/lists/<string>/tasks/<string>").methods("DELETE"_method)
            ([&](const std::string& todoTaskListId, const std::string& taskId){
            if (event_map->EventidExists(taskId)&&category_map->CategoryidExists(todoTaskListId))
                {//can find the task
                    Event * del_event = event_map->getEvent(taskId);
                    Category * thecate = category_map->getCategory(todoTaskListId);
                    dbm->deleventcate(del_event,thecate);
                    eventorganizer->deleteEvent(del_event);
                    thecate->deleteEvent(del_event);
                    event_map->removeMapping(taskId);
                    crow::json::wvalue suc;
                    suc["suc"]="suc no content";
                    dbm->delEvent(del_event);
                    delete del_event;
                    return crow::response(204,suc);
                }
                else {
                // cant find the task
                    crow::json::wvalue error;
                    error["error"] = "Task/List not found";
                    return crow::response(404, error);
                }
            });
        app.port(port).multithreaded().run();
    }

