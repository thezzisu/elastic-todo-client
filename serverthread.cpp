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
#include <QCoreApplication>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include "gpt.h"
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
                    if (!dbm->delCategory(category_map->getCategory(todoTaskListId))){
                        crow::json::wvalue x;
                        x["error"] = "SQL delete error";
                        return crow::response(404,x);
                    }
                    cateorganizer->deleteCategory(todoTaskListId);
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
                    taskJson["kindofevent"] = QJsonValue::fromVariant(task->getKindOfEvent());
                    taskJson["title"] = QJsonValue::fromVariant(QString::fromStdString(task->getTitle()));
                    taskJson["description"] = QJsonValue::fromVariant(QString::fromStdString(task->getDescription()));
                    taskJson["createdAt"] = QJsonValue::fromVariant(convertToQDateTime(task->getCreateTime()).toString(Qt::ISODate));
                    taskJson["dueAt"] = QJsonValue::fromVariant(convertToQDateTime(task->getTime()).toString(Qt::ISODate));
                    taskJson["finishedAt"] = QJsonValue::fromVariant(convertToQDateTime(task->getFinishTime()).toString(Qt::ISODate));
                    taskJson["updatedAt"] = QJsonValue::fromVariant(convertToQDateTime(task->getUpdateTime()).toString(Qt::ISODate));
                    if (task->getKindOfEvent()==DDL){
                        taskJson["ddlAt"] = QJsonValue::fromVariant(convertToQDateTime(task->getDeadline()).toString(Qt::ISODate));
                    }
                    if (task->getKindOfEvent()==RECURRING)
                    {
                        taskJson["inter_time"] = QJsonValue::fromVariant(task->getInterval());
                    }
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
                taskJson["kindofevent"] = task->getKindOfEvent();
                taskJson["title"] = task->getTitle();
                taskJson["description"] = task->getDescription();
                taskJson["createdAt"] = convertToQDateTime(task->getCreateTime()).toString(Qt::ISODate).toStdString();
                taskJson["dueAt"] = convertToQDateTime(task->getTime()).toString(Qt::ISODate).toStdString();
                taskJson["finishedAt"] = convertToQDateTime(task->getFinishTime()).toString(Qt::ISODate).toStdString();
                taskJson["updatedAt"] = convertToQDateTime(task->getUpdateTime()).toString(Qt::ISODate).toStdString();
                if (task->getKindOfEvent()==DDL){
                    taskJson["ddlAt"] = convertToQDateTime(task->getDeadline()).toString(Qt::ISODate).toStdString();
                }
                if (task->getKindOfEvent()==RECURRING)
                {
                    taskJson["inter_time"] = task->getInterval();
                }
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
                int kindoftype = body["kindofevent"].i();
                Event* newTask;
                std::string ddlAt;
                int recur_interval;
                switch (kindoftype)
                {
                    case EVENT: newTask = new Event(false,dbm,new_uuid,title,isoToTm(dueAt),description,importance,
                                        kindoftype,isoToTm(createdAt),isoToTm(finishedAt),isoToTm(updatedAt)); break;
                    case DDL:
                        ddlAt = body["ddlAt"].s();
                        newTask = new DdlEvent(false,dbm,new_uuid,title,isoToTm(dueAt),description,importance,
                                               isoToTm(ddlAt),isoToTm(createdAt),isoToTm(finishedAt),isoToTm(updatedAt)); break;
                    case RECURRING:
                        recur_interval = body["inter_time"].i();
                        newTask = new RecurringEvent(false,dbm,new_uuid,title,isoToTm(dueAt),description,importance,recur_interval,
                                        isoToTm(createdAt),isoToTm(finishedAt),isoToTm(updatedAt));break;
                    case PROJECT: newTask = new ProjectEvent(false,dbm,new_uuid,title,isoToTm(dueAt),description,importance,
                                        isoToTm(createdAt),isoToTm(finishedAt),isoToTm(updatedAt));break;
                    default: return crow::response(404);
                }
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
                x["kindofevent"] = newTask->getKindOfEvent();
                if (kindoftype==DDL){
                    x["ddlAt"] = convertToQDateTime(newTask->getDeadline()).toString(Qt::ISODate).toStdString();
                }
                if (kindoftype==RECURRING)
                {
                    x["inter_time"] = newTask->getInterval();
                }
                return crow::response(201, x);
            });


        CROW_ROUTE(app, "/me/todo/lists/<string>/tasks/<string>").methods("PATCH"_method)
            ([&](const crow::request& req, const std::string& todoTaskListId, const std::string& taskId){
            // Get the event using taskId
            if (!event_map->EventidExists(taskId))
            {
                // cant find the task
                crow::json::wvalue error;
                error["error"] = "Task not found";
                return crow::response(404, error);
            }
            Event* eventToUpdate = event_map->getEvent(taskId);
            int kindofevent = eventToUpdate->getKindOfEvent();
            std::string ddlAt;
            int inter_time;
            auto body = crow::json::load(req.body);
            if (kindofevent == DDL){
                ddlAt = body["ddlAt"].s();
            }
            if (kindofevent == RECURRING)
            {
                inter_time = body["inter_time"].i();
            }

                std::string title = body["title"].s();
                std::string description = body["description"].s();
                std::string createdAt = body["createdAt"].s();
                std::string dueAt = body["dueAt"].s();
                std::string finishedAt = body["finishedAt"].s();
                std::string updatedAt = body["updatedAt"].s();
                int urgency = body["importance"].i();


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
                    if (kindofevent == DDL){
                        if (tmToIso(eventToUpdate->getDeadline())!=ddlAt)
                            eventToUpdate->setDeadline(isoToTm(ddlAt));
                    }
                    if (kindofevent == RECURRING)
                    {
                        if (inter_time!=eventToUpdate->getInterval())
                        {
                            eventToUpdate->setInterval(inter_time);
                        }
                    }
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
                updatedTask["kindofevent"] = eventToUpdate->getKindOfEvent();
                if (eventToUpdate->getKindOfEvent()==DDL){
                     updatedTask["ddlAt"] = convertToQDateTime(eventToUpdate->getDeadline()).toString(Qt::ISODate).toStdString();
                }
                if (eventToUpdate->getKindOfEvent()==RECURRING)
                {
                     updatedTask["inter_time"] = eventToUpdate->getInterval();
                }
                return crow::response(200, updatedTask);
            });


        CROW_ROUTE(app, "/me/todo/lists/<string>/tasks/<string>").methods("DELETE"_method)
            ([&](const std::string& todoTaskListId, const std::string& taskId){
            if (event_map->EventidExists(taskId)&&category_map->CategoryidExists(todoTaskListId))
                {//can find the task
                    Event * del_event = event_map->getEvent(taskId);
                    Category * thecate = category_map->getCategory(todoTaskListId);
                    if (!del_event->getFatherEvent().empty() || !del_event->getSubEvents().empty())
                    {
                        crow::json::wvalue x;
                        x["error"] = "PLEASE remove the constraint first";
                        return crow::response(404,x);
                    }
                    if (!dbm->deleventcate(del_event,thecate))
                    {
                        crow::json::wvalue x;
                        x["error"] = "constraint is not moved for delete";
                        return crow::response(404,x);
                    }
                    eventorganizer->deleteEvent(del_event);
                    thecate->deleteEvent(del_event);
                    event_map->removeMapping(taskId);
                    crow::json::wvalue suc;
                    suc["suc"]="suc no content";
                    if (!dbm->delEvent(del_event))
                    {
                        crow::json::wvalue x;
                        x["error"] = "constraint is not moved for delete";
                        return crow::response(404,x);
                    }
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
        CROW_ROUTE(app, "/me/todo/lists/<string>/gptreq").methods("POST"_method)
            ([&](const crow::request& req, const std::string& todoTaskListId) {

                auto x = crow::json::load(req.body);
                if (!x) {
                    return crow::response(400);
                }
                if (x.has("gptreq")) {
                    QString gptreq = QString::fromStdString(x["gptreq"].s());
                    QString current_date = QDate::currentDate().toString(Qt::ISODate);
                    QString prompt = GPT_PROMPT + "Today is" + current_date + "，" + gptreq;
                    QJsonArray gptResponse = gpt_handle(prompt,eventorganizer);
                    crow::json::wvalue result;
                    result["value"] = crow::json::load(QJsonDocument(gptResponse).toJson().toStdString());
                    return crow::response(result);
                } else {
                    return crow::response(400);
                }
            });
        CROW_ROUTE(app, "/me/todo/global_search").methods("POST"_method)
            ([&](const crow::request& req){
                auto x = crow::json::load(req.body);
                if (!x){
                    return crow::response(400);
                }
                if (x.has("keyword")){
                    std::string keyword = x["keyword"].s();
                    auto result = dbm->globalSearch(keyword);
                    std::vector<crow::json::wvalue> resArr;
                    for (const auto& relation : result)
                    {
                        crow::json::wvalue obj;
                        obj["which_type"] = relation.first;
                        obj["uuid"] = relation.second;
                        resArr.push_back(obj);
                    }
                    crow::json::wvalue res;
                    res["value"] = std::move(resArr);
                    return crow::response(res);
                }
                else{
                    return crow::response(400);
                }
            });
        CROW_ROUTE(app, "/me/todo/relationships").methods("POST"_method)
            ([&](const crow::request& req) {

                auto x = crow::json::load(req.body);
                if (!x || !x.has("parent_uuid") || !x.has("child_uuid")){
                    return crow::response(400);
                }
                std::string parent_uuid = x["parent_uuid"].s();
                std::string child_uuid = x["child_uuid"].s();
                if (event_map->EventidExists(parent_uuid)&&event_map->EventidExists(child_uuid))
                {
                    Event * parent_p = event_map->getEvent(parent_uuid);
                    Event * son_p = event_map->getEvent(child_uuid);
                    if (parent_p->getKindOfEvent()!=PROJECT||son_p->getKindOfEvent()!=PROJECT)
                    {
                        crow::json::wvalue res;
                        res["error"] = "parent/child is not project event";
                        return crow::response(404,res);
                    }
                    if (dbm->addeventevent(parent_p,son_p)) {
                        parent_p->addSubEvent(son_p);
                        son_p->addFatherEvent(parent_p);
                        return crow::response(200);
                    } else {
                        return crow::response(500);
                    }
                }
                else{
                    return crow::response(404);
                }

            });
        CROW_ROUTE(app, "/me/todo/relationships").methods("DELETE"_method)
            ([&](const crow::request& req) {

                auto x = crow::json::load(req.body);
                if (!x || !x.has("parent_uuid") || !x.has("child_uuid")){
                    return crow::response(400);
                }
                std::string parent_uuid = x["parent_uuid"].s();
                std::string child_uuid = x["child_uuid"].s();
                if (event_map->EventidExists(parent_uuid)&&event_map->EventidExists(child_uuid))
                {
                    Event * parent_p = event_map->getEvent(parent_uuid);
                    Event * son_p = event_map->getEvent(child_uuid);
                    if (parent_p->getKindOfEvent()!=PROJECT||son_p->getKindOfEvent()!=PROJECT)
                    {
                        crow::json::wvalue res;
                        res["error"] = "parent/child is not project event";
                        return crow::response(404,res);
                    }
                    if (dbm->deleventevent(parent_p,son_p)) {
                        parent_p->deleteSubEvent(son_p);
                        son_p->deleteFatherEvent(parent_p);
                        return crow::response(200);
                    } else {
                        return crow::response(500);
                    }
                }
                else{
                    return crow::response(404);
                }
            });
        CROW_ROUTE(app, "/me/todo/lists/<string>/tasks/<string>/children").methods("GET"_method)
        ([&](const crow::request& req, std::string todoTaskListId, std::string taskId) {
            if (!event_map->EventidExists(taskId))
            {
                  return crow::response(404);
            }
            Event* event = event_map->getEvent(taskId);
            auto children = event->getSubEvents();
            QJsonArray resultArray;
            for (Event* child : children) {
                QJsonObject childObject;
                childObject["child_uuid"] = QString::fromStdString(child->getId()); // Assuming there is a getUUID() method
                resultArray.push_back(childObject);
            }
            QJsonObject result;
            result["value"] = resultArray;
            QJsonDocument doc(result);
            return crow::response(doc.toJson(QJsonDocument::Compact).toStdString());
        });
        CROW_ROUTE(app, "/me/todo/lists/<string>/tasks/<string>/parent").methods("GET"_method)
            ([&](const crow::request& req, std::string todoTaskListId, std::string taskId) {
                if (!event_map->EventidExists(taskId))
                {
                    return crow::response(404);
                }
                Event* event = event_map->getEvent(taskId);
                auto parent = event->getFatherEvent();
                QJsonArray resultArray;
                for (Event* pa : parent) {
                    QJsonObject childObject;
                    childObject["parent_uuid"] = QString::fromStdString(pa->getId()); // Assuming there is a getUUID() method
                    resultArray.push_back(childObject);
                }
                QJsonObject result;
                result["value"] = resultArray;
                QJsonDocument doc(result);
                return crow::response(doc.toJson(QJsonDocument::Compact).toStdString());
            });
        app.bindaddr("0.0.0.0").port(port).multithreaded().run();
    }

