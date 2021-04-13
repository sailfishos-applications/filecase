#ifndef BOX_H
#define BOX_H

#include <QAbstractListModel>
#include <QSettings>
#include <QStringList>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class QNetworkRequest;
class QNetworkReply;

#include "boxthumbnailer.h"
//#include "boxtransfers.h"


class BoxClient : public QAbstractListModel
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

    enum BoxClientRole
    {
        NameRole  = Qt::UserRole + 1 ,
        CreatedRole ,
        ModifiedRole ,
        SizeRole ,
        RealSizeRole,
        PathRole ,
        SecRole ,
        ExtenRole ,
        DExtenRole ,
        LinkRole ,
        IdRole ,
        PublicRole ,
        TypeRole
    };

    explicit BoxClient(QObject * parent = 0);
    virtual ~BoxClient();
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

    QString clientId, clientSecret, redirectURI;
    QString accessToken, refreshToken, accessCode, ticket;
    QString parentID;
    QString accessId, userId;

    QStringList history;

    double spaceUsed, spaceTotal;
    QString accountUser;

signals:
    void loggedIn(bool newaccount);
    void openUrl(QString url);
    void closeUrl(bool loged);
    void requestStarted();
    void requestDone(bool ok);
    void countChanged();
    void pathChanged();
    void fileRenamed(QString newname);
    void spaceChanged(QString spaceused, QString spacetotal, int realused, int realtotal);
    void userChanged(QVariantMap account);
    void showBanner(QString message);
    void linkShared(QString url);

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
    void moveFile(QString source, QString filetype);
    void removeFile(QString path, QString filetype);
    void copyFile(QString source, QString filetype);
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

    class BoxClientPrivate;
    BoxClientPrivate * const d;
    BoxThumbnailer *pepe;
    //BoxTransfers *transfers;

    QNetworkAccessManager* datos;
    QNetworkReply *reply;

};

#endif // BOX_H
