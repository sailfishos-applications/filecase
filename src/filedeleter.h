#ifndef FILEDELETER_H
#define FILEDELETER_H

#include <QThread>
#include <QFileInfo>

class QDir;

struct FileDeleterPrivate;

class FileDeleter : public QThread
{
Q_OBJECT
public:
    explicit FileDeleter(const QFileInfoList& items, QObject *parent = 0);
    virtual ~FileDeleter();

    virtual void run();

signals:
    void prepareProgress(int filesCounted);
    void delProgress(int percent, const QString& item);
    void endDelete(bool canceled);
    void fail(const QString& itemOfFail);

public slots:
    void abortDelete();

private:
    FileDeleterPrivate *d_ptr;

    void deleteItems(const QFileInfoList& items);
    void deleteFile(const QFileInfo& file);
    void deleteDir(const QDir& dir);
};

#endif // FILEDELETER_H
