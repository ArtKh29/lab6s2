#include "event.h"

Event::Event(const QString& n, const QString& d, const QDate& dt, double c, const QString& f)
    : name(n), description(d), date(dt), cost(c), format(f) {}

bool Event::isValid() const
{
    if (name.trimmed().isEmpty()) return false;
    if (description.trimmed().isEmpty()) return false;
    if (!date.isValid()) return false;
    if (cost < 0) return false;

    QString f = format.toLower().trimmed();
    return (f == "онлайн" || f == "офлайн" || f == "гибридный");
}

QString Event::toJson() const
{
    QJsonObject obj;
    obj["name"] = name;
    obj["description"] = description;
    obj["date"] = date.toString(Qt::ISODate);
    obj["cost"] = cost;
    obj["format"] = format;
    return QString(QJsonDocument(obj).toJson(QJsonDocument::Compact));
}

Event Event::fromJson(const QJsonObject& obj)
{
    Event e;
    e.name = obj["name"].toString();
    e.description = obj["description"].toString();
    e.date = QDate::fromString(obj["date"].toString(), Qt::ISODate);
    e.cost = obj["cost"].toDouble();
    e.format = obj["format"].toString();
    return e;
}

bool Event::fromTxt(const QString& line, QString& error)
{
    // РАЗДЕЛИТЕЛЬ ТЕПЕРЬ "/"
    QStringList parts = line.split('/');
    if (parts.size() < 5) {
        error = "Неверный формат строки (нужно 5 полей через /)";
        return false;
    }

    name = parts[0].trimmed();
    description = parts[1].trimmed();

    date = QDate::fromString(parts[2].trimmed(), "yyyy-MM-dd");
    if (!date.isValid()) {
        error = "Неверная дата (формат: YYYY-MM-DD)";
        return false;
    }

    bool ok;
    cost = parts[3].trimmed().toDouble(&ok);
    if (!ok || cost < 0) {
        error = "Неверная стоимость (должна быть >= 0)";
        return false;
    }

    format = parts[4].trimmed();
    QString f = format.toLower();
    if (f != "онлайн" && f != "офлайн" && f != "гибридный") {
        error = "Неверный формат (допустимы: онлайн/офлайн/гибридный)";
        return false;
    }

    return true;
}
