#ifndef WEBDAV_H
#define WEBDAV_H

#include <QAbstractListModel>
#include <QSettings>
#include <QStringList>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class QNetworkRequest;
class QNetworkReply;

//#include "skythumbnailer.h"
//#include "skytransfers.h"

#include "qwebdavlib/qwebdav.h"
#include "qwebdavlib/qwebdavdirparser.h"
#include "qwebdavlib/qwebdavitem.h"

class WebDav : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(QString path READ path NOTIFY pathChanged)
    Q_PROPERTY(QString path1 READ path1 NOTIFY pathChanged)
    Q_PROPERTY(QString path2 READ path2 NOTIFY pathChanged)
    Q_PROPERTY(QString service READ service NOTIFY serviceChanged)
    Q_ENUMS(ItemType)

public:
    Q_INVOKABLE QString cname(int index);

    enum ItemType
    {
        Folder ,
        File
    };

    enum WebDavRole
    {
        NameRole  = Qt::UserRole + 1 ,
        CreatedRole ,
        ModifiedRole ,
        SizeRole ,
        RealSizeRole ,
        PathRole ,
        SecRole ,
        ExtenRole ,
        DExtenRole ,
        LinkRole ,
        IdRole ,
        PublicRole ,
        TypeRole
    };

    explicit WebDav(QObject * parent = 0);
    virtual ~WebDav();
    virtual int rowCount(const QModelIndex & parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex & index , int role = Qt::DisplayRole) const;
    int count() const;
    QString path() const;
    QString path1() const;
    QString path2() const;
    QString service() const { return m_service; }
    bool isWorking() const { return m_working; }
    int StartThread;
    bool m_working;

    QString action;
    QStringList media;
    QString m_service;

    QString getPreview(QString link, QString filename, QString mode);

    QString clientId, clientSecret;
    QString accessToken, refreshToken, accessCode;
    QString parentID;

    double spaceUsed, spaceTotal;
    QVariantMap accountUser;


signals:
    void accountCreated();
    void accountError(QString message);

    void openUrl(QString url);
    void closeUrl(bool loged);
    void requestStarted();
    void requestDone(bool ok);
    void countChanged();
    void pathChanged();
    void spaceChanged(QString spaceused, QString spacetotal, int realused, int realtotal);
    void userChanged(QVariantMap account);
    void showBanner(QString message);
    void linkShared(QString url);
    void fileRenamed(QString newname);
    void serviceChanged();

public slots:
    void startService(QString host, QString path, QString user, QString pass, QString port);
    void createAccount(QString name, QString host, QString path, QString user, QString pass, QString port);
    void printList();
    void printError(QString errorMsg);
    void replySkipRead();

    void logIn(QString service, QString path);
    void clearList();
    void getQuota();
    void handleInfo();
    void requestMetadata(QString path);
    void handleMetadata(QNetworkReply *networkReply);
    void handleFileOps();
    void setPath(QString path);
    void goUp();
    void newFolder(QString name);
    void renameFile(QString source, QString newname);
    void moveFile(QString source);
    void removeFile(QString path);
    void copyFile(QString source);
    void reload();
    void shareFile(QString filename);
    void handleShare(QNetworkReply *networkReply);
    void publicLink(QString filename);

    void removeAccount();

    void setFilter(QString filter);
    void setUploadFolder(QString name);
    void imageLoaded(QString filename);

private:
    QHash<int, QByteArray> roleNames() const;
    void setRoleNames(const QHash<int, QByteArray>& roles);
    QHash<int, QByteArray> m_roles;

    class WebDavPrivate;
    WebDavPrivate * const d;
    //SkyThumbnailer *pepe;
    //SkyTransfers *transfers;

    QNetworkAccessManager* datos;
    QNetworkReply *reply;


    QWebdav qwebdav;
    QWebdavDirParser parser;
    QString m_path;
    QVariantMap account;
    QList<QNetworkReply *> replylist;
    QString m_error;

};

#endif // WEBDAV_H
