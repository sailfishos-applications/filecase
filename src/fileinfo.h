#ifndef FILEINFO_H
#define FILEINFO_H

#include "loader.h"
#include "thumbgenerator.h"
#include <QQuickItem>


class FileInfo : public QQuickItem
{
    Q_OBJECT

    Q_PROPERTY(QVariantMap data READ data NOTIFY fileChanged)

public:
    //Q_INVOKABLE QVariantMap getFileInfo(QString filelist);
    Q_INVOKABLE QVariantMap getFileMimetype(QString filename);
    Q_INVOKABLE QString getFilePreview(QString filename);

    FileInfo(QQuickItem *parent = 0);

    QVariantMap data() const;

    Loader *pepe;

public slots:
    void stopInfoThread();
    void setFile(QString filelist);
    void setSizeInfo(double size);
    void setSizeInfoLoading(double size);

signals:
    void fileChanged();
    void thumbReady(QString imageGenerated);

private:
    ThumbGenerator *thumbnailer;
    QVariantMap m_data;

};

#endif // FILEINFO_H
