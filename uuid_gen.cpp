#include <string>
#include "uuid_gen.h"
std::string generateUuid(){
    QUuid uuid = QUuid::createUuid();
    QString uuidStr = uuid.toString();
    uuidStr.remove(QRegularExpression("[{}]"));
    std::string ans = uuidStr.toStdString();
    return ans;
}
