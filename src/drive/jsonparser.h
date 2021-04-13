#ifndef JSONPARSER_H
#define JSONPARSER_H

#include <QString>

class JSONParser
{
public:
    JSONParser();

public:
    QString getParam(const QString& jsonStr, const QString& lval);
};

#endif // JSONPARSER_H
