#ifndef GPT_H
#define GPT_H
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
class Calendar;
QJsonArray parseGptResponse(const QString& response, Calendar* cal);
QJsonArray gpt_handle(QString& input, Calendar* cal);
const QString GPT_PROMPT = "Suppose you're a schedule planning expert and someone wants you to arrange, modify, or delete plans. "
                           "Each operation should be output on a separate line, specific items are "
                           "seperated by '$$'. If the user wants to add a new task, that line should start with 'add', "
                           "followed by the task's name, description, date (format like 2023-06-01), time(like: 18:00:00), and urgency level (a number from 0-2). "
                           "Please strictly follow this format: 'add$$Review$$This is a review$$2023-06-15$$15:50:00$$1'. "
                           "If the user indeed wants to delete a task(usually not), that output "
                           "line should start with 'remove', followed by the task's name and some properties like 'add' instruct, "
                           "then please strictly follow this format: 'remove$$Review$$Go over the book$$2023-12-19$$14:34$$2' . "
                           "If the user wants to modify a task, you should output a 'remove' line like the above,"
                           " and an 'add' line like the above mentioned, and the format is also the same as above. "
                           "Here is the user's request:";
const QByteArray GPT_KEY = "Bearer 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";
const QString GPT_ADDR = "https://api.openai.com/v1/engines/text-davinci-003/completions";
const QString CANNOT_FIND = "CANNOT_FIND";
#endif // GPT_H
