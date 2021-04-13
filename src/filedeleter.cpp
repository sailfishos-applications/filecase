#include "filedeleter.h"
#include <QCoreApplication>
#include <QFileInfo>
#include <QMutex>
#include <QDir>

#define SYNCRONIZED_BLOCK QMutexLocker locker(&mutex)

struct FileDeleterPrivate {
    QMutex mutex;
    bool canceled;
    bool preparing;
    quint64 deleteCount;
    quint64 itemCount;
    QFileInfoList list;

    void setCanceled(bool c) {
        SYNCRONIZED_BLOCK;
        canceled = c;
    }

    bool isCanceled() {
        SYNCRONIZED_BLOCK;
        return canceled;
    }
};

FileDeleter::FileDeleter(const QFileInfoList& list, QObject *parent) :
    QThread(parent),
    d_ptr(new FileDeleterPrivate)
{
    d_ptr->list = list;
}

FileDeleter::~FileDeleter()
{
    delete d_ptr;
}

void FileDeleter::run()
{
    QCoreApplication::processEvents();
    d_ptr->setCanceled(false);
    d_ptr->itemCount = 0;
    d_ptr->preparing = true;
    deleteItems(d_ptr->list);
    d_ptr->deleteCount = 0;
    d_ptr->preparing = false;
    deleteItems(d_ptr->list);
    emit endDelete(d_ptr->isCanceled());
}

void FileDeleter::abortDelete()
{
    d_ptr->setCanceled(true);
}

void FileDeleter::deleteItems(const QFileInfoList &items)
{

    foreach(QFileInfo item, items) {
        if (d_ptr->isCanceled())
            return;
        if (item.isSymLink())
            deleteFile(item);
        else if (item.isDir())
            deleteDir(QDir(item.canonicalFilePath()));
        else
            deleteFile(item);
    }
}

void FileDeleter::deleteFile(const QFileInfo &file)
{
    if (d_ptr->preparing) {
        d_ptr->itemCount++;
        emit prepareProgress(d_ptr->itemCount);
    } else {
        if (!QFile::remove(file.absoluteFilePath())) {
            emit fail(file.absoluteFilePath());
            d_ptr->setCanceled(true);
        } else {
            d_ptr->deleteCount++;
            emit delProgress(
                    double(d_ptr->deleteCount)/d_ptr->itemCount*100.0,
                    file.absoluteFilePath());
        }
    }
}

void FileDeleter::deleteDir(const QDir &dir)
{
    if (d_ptr->preparing) {
        d_ptr->itemCount++;
        emit prepareProgress(d_ptr->itemCount);
    }
    deleteItems(dir.entryInfoList(QDir::NoDotAndDotDot|QDir::AllEntries));
    if (!d_ptr->isCanceled()) {
        if (!d_ptr->preparing) {
            if (!QDir::root().rmdir(dir.absolutePath())) {
                emit fail(dir.absolutePath());
                d_ptr->setCanceled(true);
            } else {
                d_ptr->deleteCount++;
                emit delProgress(
                        double(d_ptr->deleteCount)/d_ptr->itemCount*100.0,
                        dir.absolutePath()
                );
            }
        }
    }
}
