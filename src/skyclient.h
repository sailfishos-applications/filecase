#ifndef SKY_H
#define SKY_H

#include <QAbstractListModel>
#include <QSettings>
#include <QStringList>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class QNetworkRequest;
class QNetworkReply;

#include "skythumbnailer.h"
//#include "skytransfers.h"


class SkyClient : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(QString path READ path NOTIFY pathChanged)
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

    enum SkyClientRole
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

    explicit SkyClient(QObject * parent = 0);
    virtual ~SkyClient();
    virtual int rowCount(const QModelIndex & parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex & index , int role = Qt::DisplayRole) const;
    int count() const;
    QString path() const;
    QString path1() const;
    QString path2() const;
    int StartThread;

    QString action;
    QStringList media;


    QString clientId, clientSecret;
    QString accessToken, refreshToken, accessCode;
    QString parentID;

    QStringList history;

    double spaceUsed, spaceTotal;
    QString accountUser;

signals:
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
    void handleFileOps(QNetworkReply *networkReply);
    void setPath(QString path);
    void setPath2(QString path, QString link);
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

    class SkyClientPrivate;
    SkyClientPrivate * const d;
    SkyThumbnailer *pepe;
    //SkyTransfers *transfers;

    QNetworkAccessManager* datos;
    QNetworkReply *reply;

};

#endif // SKY_H
