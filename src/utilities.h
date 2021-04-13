#ifndef UTILITIES_H
#define UTILITIES_H

#include <QQuickItem>
#include <QProcess>

#include "qtfilecopier.h"
#include "loader.h"

class Utilities : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(bool isWorking READ isWorking NOTIFY workingChanged)

public:
    Q_INVOKABLE bool fileExists(QString filename);
    //Q_INVOKABLE QString getTextFile(QString filename);

    Utilities(QQuickItem *parent = 0);

    QString getPreview(QString service, QString link, QString filename, QString mode);

    bool isWorking() const { return m_working; }
    bool m_working;

    int zipcanceled;

    QProcess* zipProc;
    QString zipProcCMD;
    QString zipFile, zipBase;

    QtFileCopier *fileCopier;
    QString autoremove;
    QString clipboardAction;
    int copycanceled;
    int copySkipAll, copyOverwriteAll;
    QString copyDest;


    Loader *pepe;


signals:
    void utilWorking(bool work, QString target);

    void bannerChanged(QString msg, bool ok);

    void appendBookmark(QString folder);
    void bookmarkRemoved(QString folder);
    void bookmarkAdded(QString folder);

    void appendWebDav(QString service);
    void webDavRemoved(QString service);

    void appendCloud(QString service, QString qmlfile, QString locate);
    void cloudRemoved(QString service, QString locate);

    void showBanner(QString message);

    void addTransferFile(QString service, QString mode, QString name, QString link, QString filesize, QString realsize, QString exten, bool completed=false);
    void progressChanged(QString link, int val, QString curprog);
    void workingChanged();
    void removeTransferFile(QString link, bool completed);
    void loadTransfersCompleted(int queued);
    void clearTransfers();

    void readTextFinished();
    void addTextLines(QString lines);


public slots:
    void loadBookmarks();
    void addToBookmarks(QString folder);
    void removeFromBookmarks(QString folder);
    void openFile(QString file);

    void extractFiles(QString file, QString password = "");
    void startExtracting(QString file, QString dest, QString password);
    void pfinished(int code = 0);
    void stopExtracting();
    void cancelExtracting();
    void compressFiles(QString files, QString curpath, QString tarfile, QString format);

    void checkClipboard(QString action, QString dest, QString autoremove);
    void CopyFiles(QStringList files);
    void copyDone(bool res);
    void copyFinished(int id, bool res);
    void copyError(int id,QtFileCopier::Error error, bool stopped);
    void stopCopying();

    void loadWebDavs();
    void loadClouds();

    void loadTransfers();
    void transferFile(QString service, QString mode, QString name, QString link, QString filesize, QString realsize);
    void removeTransfer(QString link, bool completed);
    void fileTransfered(QString link);
    void startTransfers();
    void stopTransfers();
    void cleanCompleted();
    void setWorking(bool working);

    void getTextFile(QString filename);
    void saveTextFile(QString filename, QString filedata);



};

#endif // UTILITIES_H
