#include "dropboxclient.h"
#include "config.h"

#include "sys/vfs.h"
#include "utils.h"

#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QDateTime>
#include <QDebug>
#include <QIcon>
#include <QStringList>
#include <QString>
#include <QCryptographicHash>
#include <QList>

#include <QNetworkReply>
#include "dropbox/common.h"
#include "dropbox/dropoauth.h"
#include "dropbox/userdata.h"
#include "dropbox/dropbox.h"
#include "dropbox/qt-json/json.h"

struct DropboxClientItem
{
    QString name;
    QString date;
    QString size;
    QString realsize;
    QString path;
    QString sec;
    QString exten;
    QString dexten;
    QString link;
    DropboxClient::ItemType type;
};

class DropboxClient::DropboxClientPrivate
{
public:
    DropboxClientPrivate(DropboxClient * parent);
    ~DropboxClientPrivate();
    void populateItems();
    QList <DropboxClientItem * > items;
    QString path;
    DropboxClient * const q;
    QList<QVariantMap> archivos;
    QString filter;
};

DropboxClient::DropboxClientPrivate::DropboxClientPrivate(DropboxClient * parent) :
        q(parent)
{
    filter = "";

}

DropboxClient::DropboxClientPrivate::~DropboxClientPrivate()
{
    while(!items.isEmpty())
    {
        delete items.takeFirst();
    }
}

void DropboxClient::DropboxClientPrivate::populateItems()
{
    q->StartThread = 0;
    if (q->pepe->images.count()>0)
        q->pepe->clear();


    QList<QVariantMap> misarchivos;

    if (filter=="")
        misarchivos = archivos;
    else
    {
        for (int i=0; i<archivos.count(); ++i)
        {
            if (archivos[i].value("name").toString().toLower().contains(filter))
                misarchivos.append(archivos[i]);
        }
    }


    int entriesCount = misarchivos.count();

    emit q->beginResetModel();

    q->beginInsertRows(QModelIndex(), 0, entriesCount-1);

    QSettings settings("cepiperez","fileboxplus");


    for (int i=0; i<misarchivos.count(); ++i)
    {
        DropboxClientItem * item = new DropboxClientItem();

        item->name = misarchivos[i].value("name","").toString();

        item->path = misarchivos[i].value("path","").toString();

        item->date = misarchivos[i].value("date","").toString();

        item->size = GetFileSize(misarchivos[i].value("size","").toDouble());

        item->realsize = misarchivos[i].value("name","").toString();

        item->sec = misarchivos[i].value("sec","").toString();

        item->link = misarchivos[i].value("path","").toString();

        if ( misarchivos[i].value("type","").toString()=="folder" )
            item->exten = "folder";  //"../icons/folder.png";
        else
        {
            item->exten = GetExtension( item->name.toLower() );

            if ( ! QFileInfo(Config::getHome() + "/.thumbnails/filecase").exists() ) {
                QDir dir;
                dir.mkdir(Config::getHome() + "/.thumbnails/filecase");
            }

            if (settings.value("ShowThumbnails","false")=="true")
            {
                QString suffix = item->name.toLower();
                if ( (suffix.endsWith(".jpg")) || (suffix.endsWith(".jpeg")) ||
                     (suffix.endsWith(".png")) || (suffix.endsWith(".tiff")) ||
                     (suffix.endsWith(".tif")) || (suffix.endsWith(".bmp")) ||
                     (suffix.endsWith(".gif")) )
                     //(suffix.endsWith(".avi")) || (suffix.endsWith(".mp4")) ||
                     //(suffix.endsWith(".wmv")) || (suffix.endsWith(".3gp")) )
                {
                    QCryptographicHash md(QCryptographicHash::Md5);
                    QString file = "Dropbox/" + path + "/" + item->name;
                    file.replace("//","/");
                    md.addData(file.toUtf8());
                    QString tf = Config::getHome() + "/.thumbnails/filecase/"+ QString(md.result().toHex().constData()) + ".jpg";
                    if ( QFileInfo(tf).exists() ) {
                        item->exten = tf;
                    } else {
                        item->dexten = tf;
                        q->pepe->addImage(tf, path+"/"+item->name);
                        q->StartThread = 1;
                    }
                }

            }

        }

        item->path = item->path.replace("//","/");

        if( misarchivos[i].value("type","").toString()=="file" )
            item->type = DropboxClient::File;
        else
            item->type = DropboxClient::Folder;

        items.append(item);

    }


    q->endInsertRows();

    emit(q->countChanged());

    emit q->endResetModel();

    emit q->pathChanged();

    if ( q->StartThread == 1 )
    {
        q->pepe->done();
    }

}


DropboxClient::DropboxClient(QObject * parent) :
        QAbstractListModel(parent) , d (new DropboxClientPrivate(this))
{
    QHash <int, QByteArray> roles;
    roles.insert(NameRole , "name");
    roles.insert(DateRole , "date");
    roles.insert(SizeRole , "size");
    roles.insert(RealSizeRole , "realsize");
    roles.insert(PathRole , "path");
    roles.insert(ExtenRole , "exten");
    roles.insert(DExtenRole , "dexten");
    roles.insert(SecRole , "sec");
    roles.insert(LinkRole , "link");
    roles.insert(TypeRole , "type");
    d->q->setRoleNames(roles);

    m_working = false;

    pepe = new DropboxThumbnailer();
    connect (pepe, SIGNAL(imageLoaded(QString)), this, SLOT(imageLoaded(QString)));

    spaceUsed = 0;
    spaceTotal = 0;
    accountUser.clear();
}

QHash<int, QByteArray> DropboxClient::roleNames() const {
    return m_roles;
}

void DropboxClient::setRoleNames(const QHash<int, QByteArray>& roles) {
    m_roles = roles;
}

DropboxClient::~DropboxClient()
{
    delete d;
}

int DropboxClient::rowCount(const QModelIndex & parent) const
{
    Q_UNUSED(parent)
    return d->items.size();
}

QVariant DropboxClient::data(const QModelIndex & index , int role) const
{
    if(index.row() < 0 or index.row() > count())
    {
        return QVariant();
    }

    DropboxClientItem * item = d->items.at(index.row());
    switch(role)
    {
    case NameRole:
        return item->name;
        break;
    case DateRole:
        return item->date;
        break;
    case SizeRole:
        return item->size;
        break;
    case RealSizeRole:
        return item->realsize;
        break;
    case PathRole:
        return item->path;
        break;
    case SecRole:
        return item->sec;
        break;
    case ExtenRole:
        return item->exten;
        break;
    case DExtenRole:
        return item->dexten;
        break;
    case LinkRole:
        return item->link;
        break;
    case TypeRole:
        return item->type;
        break;
    default:
        return QVariant();
        break;
    }
}

int DropboxClient::count() const
{
        return rowCount();
}

QString DropboxClient::path() const
{
    QString tmp = d->path;
    if ( tmp.startsWith("//") )
            tmp.remove(0,1);
    if (tmp=="") tmp = "/";
    d->path = tmp;
    return d->path;
}

QString DropboxClient::path1() const
{
    if (d->path=="/")
        return d->path;
    else
        return QFileInfo(d->path).fileName();
}

QString DropboxClient::path2() const
{
    if (d->path=="/")
        return "";
    else
        return QFileInfo(d->path).path();
}

QString DropboxClient::cname(int index)
{
    QString result;
    if ( d->items.at(index)->type==DropboxClient::Folder )
        result = "a";
    else
        result = "b";

    result += d->items.at(index)->sec;

    return result;
}


void DropboxClient::reload()
{
    if (action.startsWith("rename")) {
        action = action.remove(0, 6);
        emit fileRenamed(QFileInfo(action).fileName());
    }
    d->q->requestMetadata(d->path);
}

void DropboxClient::setPath(QString path)
{
    path = path.replace("//","/");
    d->path = path;
}

void DropboxClient::setPath2(QString path)
{
    QSettings settings("cepiperez","fileboxplus");
    Common::userData->token = settings.value("Dropbox/access_token").toString();
    Common::userData->secret = settings.value("Dropbox/access_token_secret").toString();
    Common::userData->uid = settings.value("Dropbox/uid").toString();
    //if(d->path != path)
    //{
        d->path = path;
        d->path.replace("//","/");
        d->q->requestMetadata(d->path);
    //}
}

void DropboxClient::goUp()
{
    QString tmp = d->path;
    int i = tmp.lastIndexOf("/");
    tmp.remove(i, tmp.length()-i);
    if (tmp=="") tmp = "/";
    d->path = tmp;
    d->q->requestMetadata(d->path);
}

void DropboxClient::logIn(QString path)
{
    d->q->path1() = "/";
    d->q->path2() = "";
    emit d->q->pathChanged();

    QSettings settings("cepiperez","fileboxplus");
    if( (settings.value("Dropbox/access_token","").toString()!="") ||
        (settings.value("Dropbox/access_token_secret","").toString()!="") ||
        (settings.value("Dropbox/uid","").toString()!="") )
    {
        Common::userData->token = settings.value("Dropbox/access_token").toString();
        Common::userData->secret = settings.value("Dropbox/access_token_secret").toString();
        Common::userData->uid = settings.value("Dropbox/uid").toString();
        emit d->q->loggedIn(false);
        requestMetadata(path);
        return;
    }


    action = "request-token";
    emit d->q->requestStarted();
    QUrl url = Common::dropbox->apiToUrl(Dropbox::OAUTH_REQUESTTOKEN);
    QUrlQuery query;
    QNetworkRequest networkRequest(url);
    UserData userData;
    Common::oAuth->signRequestHeader("GET", &networkRequest, query, &userData);

    disconnect(Common::networkAccessManager,0,0,0);
    connect(Common::networkAccessManager, SIGNAL(finished(QNetworkReply*)),
        SLOT(handleNetworkReply(QNetworkReply*)));

    Common::networkAccessManager->get(networkRequest);

}

void DropboxClient::handleNetworkReply(QNetworkReply *networkReply)
{
    if (action == "request-token")
    {
        QString reply = QString::fromUtf8(networkReply->readAll());
        qDebug() << "REQUEST TOKEN DONE";

        requestUserData.token = reply.split("&").at(1).split("=").at(1);
        requestUserData.secret = reply.split("&").at(0).split("=").at(1);

        action = "authorize";
        QString url = Common::dropbox->apiToUrl(Dropbox::OAUTH_AUTHORIZE).toString();
        url += "?oauth2_token=" + requestUserData.token;

        qDebug() << "OPENING BROWSER...";
        emit d->q->openUrl(url);
        emit d->q->requestDone(false);

    }
    else if (action=="authorize")
    {
        qDebug() << "REQUEST AUTHORIZATION DONE";
        QString reply = networkReply->readAll();

        //qDebug() << reply;

        if (reply.contains("{\"error\":")) {
            d->q->closeUrl(true);
            action = "authorize";
            emit d->q->requestStarted();
            QUrl url = Common::dropbox->apiToUrl(Dropbox::OAUTH_ACCESSTOKEN);
            QUrlQuery query;
            QNetworkRequest networkRequest(url);
            Common::oAuth->signRequestHeader("GET", &networkRequest, query, &requestUserData);

            disconnect(Common::networkAccessManager,0,0,0);
            connect(Common::networkAccessManager, SIGNAL(finished(QNetworkReply*)),
                SLOT(handleNetworkReply(QNetworkReply*)));

            Common::networkAccessManager->get(networkRequest);
        }
        else
        {
            // update user data
            Common::userData->token = reply.split("&").at(1).split("=").at(1);
            Common::userData->secret = reply.split("&").at(0).split("=").at(1);
            Common::userData->uid = reply.split("&").at(2).split("=").at(1);

            if( !Common::userData->token.isEmpty() &&
                !Common::userData->secret.isEmpty() &&
                !Common::userData->uid.isEmpty() )
            {
                // LOGED IN!
                qDebug() << "LOG IN SUCCESS";
                //d->q->closeUrl();

                QSettings settings("cepiperez","fileboxplus");
                settings.setValue("Dropbox/access_token", Common::userData->token);
                settings.setValue("Dropbox/access_token_secret", Common::userData->secret);
                settings.setValue("Dropbox/uid", Common::userData->uid);
                settings.sync();

                emit d->q->loggedIn(true);

                //requestMetadata("/");

            }

        }

    }

}

void DropboxClient::parseData(QString dir, QString data)
{
    qDebug() << data;
    qDebug() << dir;
    //d->q->closeUrl();

    if (dir.startsWith("https://www.dropbox.com/1/oauth/authorize_submit"))
    {
        d->q->closeUrl(true);
        action = "authorize";
        emit d->q->requestStarted();
        QUrl url = Common::dropbox->apiToUrl(Dropbox::OAUTH_ACCESSTOKEN);
        QUrlQuery query;
        QNetworkRequest networkRequest(url);
        Common::oAuth->signRequestHeader("GET", &networkRequest, query, &requestUserData);

        disconnect(Common::networkAccessManager,0,0,0);
        connect(Common::networkAccessManager, SIGNAL(finished(QNetworkReply*)),
            SLOT(handleNetworkReply(QNetworkReply*)));

        Common::networkAccessManager->get(networkRequest);

    }
    else if (dir=="https://www.dropbox.com/m")
    {
        d->q->closeUrl(false);
    }

}

void DropboxClient::clearList()
{
    d->archivos.clear();
    emit d->q->beginResetModel();
    d->items.clear();
    emit d->q->endResetModel();
}

void DropboxClient::requestMetadata(QString path)
{
    clearList();
    emit d->q->requestStarted();

    QUrl url = Common::dropbox->apiToUrl(Dropbox::METADATA).toString() + path;
    QNetworkRequest networkRequest(url);
    Common::oAuth->signRequestHeader("GET", &networkRequest);

    disconnect(Common::networkAccessManager,0,0,0);

    connect(Common::networkAccessManager, SIGNAL(finished(QNetworkReply*)),
        SLOT(handleMetadata(QNetworkReply*)));

    Common::networkAccessManager->get(networkRequest);

}

void DropboxClient::handleMetadata(QNetworkReply *networkReply)
{
    QSettings settings("cepiperez","fileboxplus");

    QString dirJson = networkReply->readAll();
    qDebug() << dirJson;

    if (dirJson.contains("\"error\":"))
    {
        /*settings.remove("Dropbox/access_token");
        settings.remove("Dropbox/access_token_secret");
        settings.remove("Dropbox/uid");
        settings.sync();
        d->q->logIn("/");*/
        clearList();
        emit d->q->requestDone(false);
        return;
    }

    bool ok;
    QVariantMap jsonResult = QtJson::Json::parse(dirJson, ok).toMap();
    if(!ok || dirJson=="")
    {
        d->archivos.clear();
        d->populateItems();
        emit d->q->requestDone(false);
        return;
    }

    // update currentDirectory and ui->currentFolderLabel
    d->path = Common::dropbox->metaDataPathFromUrl(networkReply->url());
    //qDebug() << "CURRENT PATH: " << d->path;

    QString dateFormat = settings.value("DateFormat","mm/dd/yyyy").toString().replace("mm","MM");

    // add folders
    foreach(const QVariant &itemJson, jsonResult["contents"].toList())
    {
        QVariantMap itemMap = itemJson.toMap();

        QVariantMap item;


        if(itemMap["is_dir"].toBool() == true)
            item.insert("type", "folder");
        else
            item.insert("type", "file");

        QString f = itemMap["path"].toString();
        item.insert("name", QFileInfo(f).fileName());

        item.insert("path", f);

        QString predate = itemMap["modified"].toString();
        predate = predate.left(predate.length()-6);
        predate = predate.remove(0,5);
        predate = GetDate(predate);
        QDateTime modified = QDateTime::fromString(predate,"dd/MM/yyyy");
        //qDebug() << f << predate << modified;

        item.insert("date", modified.toString(dateFormat));

        if (itemMap["is_dir"].toBool() == true)
            item.insert("size", "0");
        else
            item.insert("size", itemMap["bytes"].toString());


        if(itemMap["is_dir"].toBool() == true)
            item.insert("sec", "a_"+QFileInfo(f).fileName().toLower());
        else
            item.insert("sec", "b_"+QFileInfo(f).fileName().toLower());

        d->archivos.append(item);

    }

    qSort(d->archivos.begin(), d->archivos.end(), sortVariantMap);

    d->populateItems();

    emit d->q->requestDone(true);

}

void DropboxClient::getQuota()
{
    QUrl url = Common::dropbox->apiToUrl(Dropbox::ACCOUNT_INFO);
    QNetworkRequest networkRequest(url);
    Common::oAuth->signRequestHeader("GET", &networkRequest);
    disconnect(Common::networkAccessManager,0,0,0);
    connect(Common::networkAccessManager, SIGNAL(finished(QNetworkReply*)),
            SLOT(handleInfo(QNetworkReply*)));
    Common::networkAccessManager->get(networkRequest);
}

void DropboxClient::handleInfo(QNetworkReply *networkReply)
{
    QString dirJson = networkReply->readAll();
    bool ok;

    qDebug() << dirJson;

    QVariantMap jsonResult = QtJson::Json::parse(dirJson, ok).toMap();
    if (!ok) return;

    QVariantMap quota = jsonResult["quota_info"].toMap();
    spaceUsed = quota["shared"].toDouble() + quota["normal"].toDouble();
    spaceTotal = quota["quota"].toDouble();


    accountUser.insert("user", jsonResult["display_name"].toString());
    accountUser.insert("email", jsonResult["email"].toString());
    accountUser.insert("picture", jsonResult["user"].toMap()["picture"].toMap()["url"].toString());
    accountUser.insert("spaceUsed", spaceUsed);
    accountUser.insert("spaceTotal", spaceTotal);
    accountUser.insert("spaceUsedText", GetFileSize(spaceUsed));
    accountUser.insert("spaceTotalText", GetFileSize(spaceTotal));



    //accountUser = jsonResult["display_name"].toString();
    //if (accountUser=="")
    //    accountUser = jsonResult["email"].toString();

    qDebug() << "USER:" << accountUser;
    //emit d->q->spaceChanged(GetFileSize(spaceUsed), GetFileSize(spaceTotal), spaceUsed/1024/1024, spaceTotal/1024/1024);
    emit d->q->userChanged(accountUser);
    emit d->q->requestDone(true);
}

void DropboxClient::newFolder(QString name)
{
    action = "newfolder";
    emit d->q->requestStarted();
    QUrl url = Common::dropbox->apiToUrl(Dropbox::FILEOPS_CREATEFOLDER);
    QUrlQuery query;
    query.addQueryItem("root", "dropbox");
    query.addQueryItem("path", d->path+"/"+name);
    url.setQuery(query);
    QNetworkRequest networkRequest(url);
    Common::oAuth->signRequestHeader("GET", &networkRequest, query);

    disconnect(Common::networkAccessManager,0,0,0);

    connect(Common::networkAccessManager, SIGNAL(finished(QNetworkReply*)),
        SLOT(reload()));

    Common::networkAccessManager->get(networkRequest);
}

void DropboxClient::renameFile(QString source, QString dest)
{
    action = "rename" + d->path + "/" + dest;

    emit d->q->requestStarted();
    QUrl url = Common::dropbox->apiToUrl(Dropbox::FILEOPS_MOVE);
    QUrlQuery query;
    query.addQueryItem("root", "dropbox");
    query.addQueryItem("from_path", source);
    query.addQueryItem("to_path", d->path + "/" + dest);
    url.setQuery(query);
    QNetworkRequest networkRequest(url);
    Common::oAuth->signRequestHeader("GET", &networkRequest, query);

    disconnect(Common::networkAccessManager,0,0,0);

    connect(Common::networkAccessManager, SIGNAL(finished(QNetworkReply*)),
        SLOT(reload()));

    Common::networkAccessManager->get(networkRequest);

}

void DropboxClient::moveFile(QString source, QString dest)
{
    qDebug() << "MOVING " << source << " TO " << d->path;

    action = "move";

    emit d->q->requestStarted();
    QUrl url = Common::dropbox->apiToUrl(Dropbox::FILEOPS_MOVE);
    QUrlQuery query;
    query.addQueryItem("root", "dropbox");
    query.addQueryItem("from_path", source);
    query.addQueryItem("to_path", d->path + "/" + QFileInfo(source).fileName());
    url.setQuery(query);
    QNetworkRequest networkRequest(url);
    Common::oAuth->signRequestHeader("GET", &networkRequest, query);

    disconnect(Common::networkAccessManager,0,0,0);

    connect(Common::networkAccessManager, SIGNAL(finished(QNetworkReply*)),
        SLOT(reload()));

    Common::networkAccessManager->get(networkRequest);
}

void DropboxClient::removeFile(QString path)
{
    action = "remove";
    emit d->q->requestStarted();
    QUrl url = Common::dropbox->apiToUrl(Dropbox::FILEOPS_DELETE);
    QUrlQuery query;
    query.addQueryItem("root", "dropbox");
    query.addQueryItem("path", path);
    url.setQuery(query);
    QNetworkRequest networkRequest(url);
    Common::oAuth->signRequestHeader("GET", &networkRequest, query);

    disconnect(Common::networkAccessManager,0,0,0);

    connect(Common::networkAccessManager, SIGNAL(finished(QNetworkReply*)),
        SLOT(reload()));

    Common::networkAccessManager->get(networkRequest);
}

void DropboxClient::copyFile(QString source)
{
    qDebug() << "COPYING " << source << " TO " << d->path;

    action = "copy";

    emit d->q->requestStarted();
    QUrl url = Common::dropbox->apiToUrl(Dropbox::FILEOPS_COPY);
    QUrlQuery query;
    query.addQueryItem("root", "dropbox");
    query.addQueryItem("from_path", source);
    query.addQueryItem("to_path", d->path + "/" + QFileInfo(source).fileName());
    url.setQuery(query);
    QNetworkRequest networkRequest(url);
    Common::oAuth->signRequestHeader("GET", &networkRequest, query);

    disconnect(Common::networkAccessManager,0,0,0);

    connect(Common::networkAccessManager, SIGNAL(finished(QNetworkReply*)),
        SLOT(reload()));

    Common::networkAccessManager->get(networkRequest);

}

void DropboxClient::imageLoaded(QString filename)
{
    for (int i=0; i<d->items.count(); ++i)
    {
        if (d->items.at(i)->dexten==filename)
        {
            d->items.at(i)->exten = filename;
            emit(d->q->dataChanged(d->q->index(i) , d->q->index(i)));
            break;
        }
    }
}



void DropboxClient::setFilter(QString filter)
{
    d->filter = filter.toLower();
    d->populateItems();

}

void DropboxClient::shareFile(QString filename)
{
    filename = filename.replace("//","/");
    emit d->q->requestStarted();
    QUrl url = Common::dropbox->apiToUrl(Dropbox::SHARES).toString() + filename;

    qDebug() << "SHARING " << url;

    QNetworkRequest networkRequest(url);
    Common::oAuth->signRequestHeader("GET", &networkRequest);

    disconnect(Common::networkAccessManager,0,0,0);

    connect(Common::networkAccessManager, SIGNAL(finished(QNetworkReply*)),
        SLOT(handleShare(QNetworkReply*)));

    Common::networkAccessManager->get(networkRequest);
}

void DropboxClient::handleShare(QNetworkReply *networkReply)
{
    QString dirJson = networkReply->readAll();
    qDebug() << dirJson;

    if (dirJson.contains("\"error\":"))
    {
        emit d->q->requestDone(true);
        return;
    }

    bool ok;
    QVariantMap jsonResult = QtJson::Json::parse(dirJson, ok).toMap();
    if(!ok)
    {
        emit d->q->requestDone(true);
        return;
    }

    QString shareLink = jsonResult["url"].toString();
    emit d->q->requestDone(true);
    emit d->q->linkShared(shareLink);

}

void DropboxClient::publicLink(QString filename)
{
    filename = filename.replace("//","/").remove("/Public");
    QSettings settings("cepiperez","fileboxplus");
    QString uid = settings.value("Dropbox/uid").toString();
    emit d->q->linkShared("http://dl.dropbox.com/u/" + uid + filename);
}

void DropboxClient::removeAccount()
{
    QSettings settings("cepiperez","fileboxplus");
    settings.remove("Dropbox/access_token");
    settings.remove("Dropbox/access_token_secret");
    settings.remove("Dropbox/uid");
    settings.sync();
    d->q->logIn("/");
}

void DropboxClient::mediaFile(QString filename)
{
    filename = filename.replace("//","/");
    emit d->q->requestStarted();
    QUrl url = Common::dropbox->apiToUrl(Dropbox::MEDIA).toString() + filename;

    qDebug() << "REQUESTING FOR STREAM" << url;

    QNetworkRequest networkRequest(url);
    Common::oAuth->signRequestHeader("GET", &networkRequest);

    disconnect(Common::networkAccessManager,0,0,0);

    connect(Common::networkAccessManager, SIGNAL(finished(QNetworkReply*)),
        SLOT(handleMedia(QNetworkReply*)));

    Common::networkAccessManager->get(networkRequest);
}

void DropboxClient::handleMedia(QNetworkReply *networkReply)
{
    QString dirJson = networkReply->readAll();
    qDebug() << dirJson;

    if (dirJson.contains("\"error\":"))
    {
        emit d->q->requestDone(true);
        return;
    }

    bool ok;
    QVariantMap jsonResult = QtJson::Json::parse(dirJson, ok).toMap();
    if(!ok)
    {
        emit d->q->requestDone(true);
        return;
    }

    QString shareLink = jsonResult["url"].toString();
    emit d->q->requestDone(true);
    QDesktopServices::openUrl(shareLink);

}

void DropboxClient::setUploadFolder(QString name)
{
    QSettings settings("cepiperez","fileboxplus");
    settings.setValue("Dropbox/upload_folder", name + "||" + d->path);
    settings.sync();
}
