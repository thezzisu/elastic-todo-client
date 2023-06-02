#include "gpt.h"
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QEventLoop>
#include <QDebug>
#include "event.h"
QJsonArray gpt_handle(QString& input, Calendar* cal)
{
    QNetworkAccessManager* manager = new QNetworkAccessManager();
    QNetworkRequest request;

    request.setUrl(QUrl(GPT_ADDR));
    request.setRawHeader("Authorization", GPT_KEY);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject body;
    body.insert("prompt", input);
    body.insert("max_tokens", 1000);

    QEventLoop loop;
    QNetworkReply *reply = manager->post(request, QJsonDocument(body).toJson());

    QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();

    if (reply->error() == QNetworkReply::NoError) {
        QString response = reply->readAll();
        QJsonDocument jsonResponse = QJsonDocument::fromJson(response.toUtf8());
        QJsonObject jsonObject = jsonResponse.object();
        if(jsonObject.contains("choices") && jsonObject["choices"].isArray()){
            QJsonArray jsonArray = jsonObject["choices"].toArray();
            if(!jsonArray.isEmpty()){
                QJsonObject firstResponse = jsonArray.first().toObject();
                QString gptResponse = firstResponse["text"].toString();
                qDebug()<<"Res from GPT:"<<gptResponse;
                QJsonArray actions = parseGptResponse(gptResponse,cal);
                return actions;
            }
        }
    } else {
        qDebug() << "Error:" << reply->errorString();
    }
    return QJsonArray();  // return empty array in case of failure
}

QJsonArray parseGptResponse(const QString& response, Calendar* cal) {
    QJsonArray actions;
    QStringList operationList = response.split('\n');
    for (const QString& operation : operationList) {
        QString trimmedOperation = operation.trimmed();
        if (!trimmedOperation.isEmpty()) {
            QJsonObject actionObj;
            QStringList actionDetails = trimmedOperation.split("$$");
            if (!actionDetails.isEmpty() && actionDetails.count() == 6) {
                QString action = actionDetails[0];
                actionObj.insert("action", action);
                actionObj.insert("name", actionDetails[1].trimmed());
                actionObj.insert("description", actionDetails[2].trimmed());
                QString datetime = actionDetails[3] + "T" + actionDetails[4];
                actionObj.insert("datetime", datetime.trimmed());
                actionObj.insert("urgency", actionDetails[5].trimmed().toInt());
                if (action == "remove" || action == "Remove") {
                    QString closestId = cal->getClosestId(actionDetails[1].trimmed());
                    if (closestId != CANNOT_FIND) {
                        actionObj.insert("remove_id", closestId);
                    } else {
                        // the gpt output is not compatible with the current plan... so just ignore
                        qDebug() << "Failed to find id for removal: " << actionDetails[1];
                    }
                }
                actions.append(actionObj);
            } else {
                qDebug() << "Invalid operation: " << operation;
            }
        }
    }
    return actions;
}







