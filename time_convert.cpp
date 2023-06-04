#include "time_convert.h"
std::tm convertToTm(const QDate& date, const QTime& time) {
    std::tm t;
    t.tm_year = date.year() - 1900; // Year since 1900
    t.tm_mon = date.month() - 1; // 0-11
    t.tm_mday = date.day(); // 1-31
    t.tm_hour = time.hour(); // 0-23
    t.tm_min = time.minute(); // 0-59
    t.tm_sec = time.second(); // 0-61 (0-60 in C++11 and later)
    return t;
}

std::pair<QDate, QTime> convertFromTm(const std::tm& t) {
    QDate date(t.tm_year + 1900, t.tm_mon + 1, t.tm_mday);
    QTime time(t.tm_hour, t.tm_min, t.tm_sec);
    return std::make_pair(date, time);
}
QDateTime convertToQDateTime(const std::tm& tm) {
    QDate date(tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
    QTime time(tm.tm_hour, tm.tm_min, tm.tm_sec);
    return QDateTime(date, time);
}
std::tm isoToTm(const std::string& iso8601DateTime) {
    QDateTime dateTime = QDateTime::fromString(QString::fromStdString(iso8601DateTime), Qt::ISODate);
    qint64 timestampSecs = dateTime.toSecsSinceEpoch();
    std::time_t timestamp = static_cast<std::time_t>(timestampSecs);
    std::tm *tmDateTime = std::gmtime(&timestamp);
    //use it safer
    std::tm tmDateTimeCopy = *tmDateTime;
    return tmDateTimeCopy;
}
std::string tmToIso(const std::tm& time) {
    time_t time_t_type = std::mktime(const_cast<std::tm*>(&time));
    QDateTime qdt = QDateTime::fromSecsSinceEpoch(static_cast<qint64>(time_t_type));
    return qdt.toString(Qt::ISODate).toStdString();
}
