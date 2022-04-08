#include "generator.h"

Generator::Generator(QObject *parent)
    : QObject(parent)
{

}

QString Generator::errorString()
{
    return property("errorString").toString();
}

bool Generator::setErrorString(const QString &error)
{
    return setProperty("errorString", error);
}

bool Generator::setProperty(const QString &name, const QVariant &value)
{
    return QObject::setProperty(name.toLatin1(), value);
}

QVariant Generator::property(const QString &name) const
{
    return QObject::property(name.toLatin1());
}
