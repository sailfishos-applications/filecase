#include "config.h"

#include <QSettings>

QSettings sets("cepiperez", "fileboxplus");

Config::Config(QQuickItem *parent)
    : QQuickItem(parent)
{

}

void Config::setConfig(QString data1, QString data2)
{
    sets.setValue(data1, data2);
    sets.sync();
}

QString Config::readConfig(QString data1, QString data2)
{
    return sets.value(data1, data2).toString();
}

void Config::removeConfig(QString data1)
{
    sets.remove(data1);
    sets.sync();
}
