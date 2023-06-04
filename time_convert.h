#ifndef TIME_CONVERT_H
#define TIME_CONVERT_H
#include<ctime>
#include <QDate>
#include <QTime>
#include <QDateTime>
#include <string>
std::tm convertToTm(const QDate& date, const QTime& time);
std::pair<QDate, QTime> convertFromTm(const std::tm& t);
QDateTime convertToQDateTime(const std::tm& tm);
std::tm isoToTm(const std::string& iso8601DateTime);
std::string tmToIso(const std::tm& time);
#endif // TIME_CONVERT_H
