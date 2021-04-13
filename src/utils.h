#ifndef UTILS_H
#define UTILS_H

#include <QString>
#include <QVariantMap>

bool sortVariantMap(const QVariantMap &d1, const QVariantMap &d2);
bool sortStringList(const QStringList &d1, const QStringList &d2);
double GetFolderSize ( const QString &path );
QString GetFileSize ( double size );
QString GetExtension ( QString name );
QString GetDate ( QString date );

#endif // UTILS_H
