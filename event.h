#ifndef EVENT_H
#define EVENT_H

#include <QString>
#include <QDate>
#include <QJsonObject>
#include <QJsonDocument>

class Event
{
public:
    Event() = default;
    Event(const QString& n, const QString& d, const QDate& dt, double c, const QString& f);

    // Поля согласно варианту 7 "Мероприятие"
    QString name;        // Название
    QString description; // Описание
    QDate date;          // Дата
    double cost;         // Стоимость
    QString format;      // Формат (онлайн/офлайн/гибридный)

    bool isValid() const;
    QString toJson() const;
    static Event fromJson(const QJsonObject& obj);
    bool fromTxt(const QString& line, QString& error);
};

#endif // EVENT_H
