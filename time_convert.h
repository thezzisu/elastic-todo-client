#ifndef TIME_CONVERT_H
#define TIME_CONVERT_H
#include<ctime>
#include <QDate>
#include <QTime>
#include <QDateTime>
#include <string>
inline std::tm convertToTm(const QDate& date, const QTime& time);
inline std::pair<QDate, QTime> convertFromTm(const std::tm& t);
inline QDateTime convertToQDateTime(const std::tm& tm);
inline std::tm isoToTm(const std::string& iso8601DateTime);
inline std::string tmToIso(const std::tm& time);
#endif // TIME_CONVERT_H
