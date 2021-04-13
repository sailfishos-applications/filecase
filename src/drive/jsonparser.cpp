#include "jsonparser.h"
#include <QRegExp>
#include <QStringList>

JSONParser::JSONParser()
{
}

QString JSONParser::getParam(const QString& jsonStr, const QString& lval)
{
    QString optStr = jsonStr;
    optStr.remove(QRegExp("[ \"]"));
    QStringList parseStrs = optStr.split("\n");
    QString rval("");

    for (int i = 0; i < parseStrs.count(); ++i)
    {
        QStringList exp = parseStrs[i].split(",");

        for(int j = 0; j < exp.count(); ++j)
        {
            QStringList token = exp[j].split(":");

            if(token[0] == lval)
            {
                rval = token[1];
                break;
            }
        }
    }

    return rval;
}
