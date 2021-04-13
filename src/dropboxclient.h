#ifndef DropboxClient_H
#define DropboxClient_H

#include <QAbstractListModel>
#include <QSettings>
#include <QStringList>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class QNetworkRequest;
class QNetworkReply;

#include "dropboxthumbnailer.h"
//#include "dropboxtransfers.h"

#include "dropbox/userdata.h"

class DropboxClient : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(QString path READ path WRITE setPath NOTIFY pathChanged)
    Q_PROPERTY(QString path1 READ path1 NOTIFY pathChanged)
    Q_PROPERTY(QString path2 READ path2 NOTIFY pathChanged)
    Q_ENUMS(ItemType)

public:
    Q_INVOKABLE QString cname(int index);

    enum ItemType
    {
        Folder ,
        File
    };

    enum DropboxClientRole
    {
        NameRole  = Qt::UserRole + 1 ,
        DateRole ,
        SizeRole ,
        RealSizeRole ,
        PathRole ,
        SecRole ,
        ExtenRole ,
        DExtenRole ,
        LinkRole ,
        TypeRole
    };

    explicit DropboxClient(QObject * parent = 0);
    virtual ~DropboxClient();
    virtual int rowCount(const QModelIndex & parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex & index , int role = Qt::DisplayRole) const;
    int count() const;
    QString path() const;
    QString path1() const;
    QString path2() const;
    int StartThread;
    bool m_working;

    QString action;
    QStringList media;


    double spaceUsed, spaceTotal;
    QVariantMap accountUser;

signals:
    void openUrl(QString url);
    void closeUrl(bool loged);
    void requestStarted();
    void requestDone(bool ok);
    void countChanged();
    void pathChanged();
    void progressChanged(QString filename, int val, QString curprog);
    void spaceChanged(QString spaceused, QString spacetotal, int realused, int realtotal);
    void userChanged(QVariantMap account);
    void showBanner(QString message);
    void linkShared(QString url);
    void fileRenamed(QString newname);
    void loggedIn(bool newaccount);

public slots:
    void logIn(QString path);
    void clearList();
    void getQuota();
    void handleInfo(QNetworkReply *networkReply);
    void handleNetworkReply(QNetworkReply *networkReply);
    void parseData(QString dir, QString data);
    void requestMetadata(QString path);
    void handleMetadata(QNetworkReply *networkReply);
    void setPath(QString path);
    void setPath2(QString path);
    void goUp();
    void newFolder(QString name);
    void renameFile(QString source, QString newname);
    void moveFile(QString source, QString dest = "");
    void removeFile(QString path);
    void copyFile(QString source);
    void reload();
    void shareFile(QString filename);
    void handleShare(QNetworkReply *networkReply);
    void publicLink(QString filename);
    void mediaFile(QString filename);
    void handleMedia(QNetworkReply *networkReply);

    void removeAccount();


    void setFilter(QString filter);
    void setUploadFolder(QString name);
    void imageLoaded(QString filename);

private:
    QHash<int, QByteArray> roleNames() const;
    void setRoleNames(const QHash<int, QByteArray>& roles);
    QHash<int, QByteArray> m_roles;

    class DropboxClientPrivate;
    DropboxClientPrivate * const d;
    DropboxThumbnailer *pepe;
    //DropboxTransfers *transfers;
    UserData requestUserData;
};

#endif // DropboxClient_H
