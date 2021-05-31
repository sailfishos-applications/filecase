#include "webdav.h"

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
#include <QUrlQuery>
#include <QVariantMap>
#include <QNetworkReply>
#include <QBuffer>

#include "dropbox/qt-json/json.h"
#include "drive/jsonparser.h"

struct WebDavItem
{
    QString name;
    QString created;
    QString modified;
    QString size;
    QString realsize;
    QString path;
    QString sec;
    QString exten;
    QString dexten;
    QString link;
    QString fid;
    QString pub;
    WebDav::ItemType type;
};

class WebDav::WebDavPrivate
{
public:
    WebDavPrivate(WebDav * parent);
    ~WebDavPrivate();
    void populateItems();
    QList <WebDavItem * > items;
    QString path;
    WebDav * const q;
    QList<QVariantMap> archivos;
    QString filter;
    bool creatingAccount;

};

WebDav::WebDavPrivate::WebDavPrivate(WebDav * parent) :
        q(parent)
{
    filter = "";
    creatingAccount = false;
}

WebDav::WebDavPrivate::~WebDavPrivate()
{
    while(!items.isEmpty())
    {
        delete items.takeFirst();
    }
}

void WebDav::WebDavPrivate::populateItems()
{
    q->StartThread = 0;
    //if (q->pepe->images.count()>0)
    //    q->pepe->clear();


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


    QSettings settings("cepiperez","fileboxplus");

    int entriesCount = misarchivos.count();

    emit q->beginResetModel();

    q->beginInsertRows(QModelIndex(), 0, entriesCount-1);

    for (int i=0; i<misarchivos.count(); ++i)
    {
        WebDavItem * item = new WebDavItem();

        //qDebug() << misarchivos[i];

        item->name = misarchivos[i].value("name","").toString();

        item->created = misarchivos[i].value("created","").toString();

        item->modified = misarchivos[i].value("modified","").toString();

        item->size = GetFileSize(misarchivos[i].value("size","").toDouble());

        item->realsize = misarchivos[i].value("size","0").toString();

        item->sec = item->name.left(1).toUpper();

        item->path = path + "/" + item->name;
        item->path = item->path.replace("//","/");

        if ( misarchivos[i].value("type","").toString()=="folder" )
            item->exten = "folder";  //"../icons/folder.png";
        else
        {
            item->exten = GetExtension( item->name.toLower() );

            if ( ! QFileInfo("/home/nemo/.thumbnails/filecase").exists() ) {
                QDir dir;
                dir.mkdir("/home/nemo/.thumbnails/filecase");
            }

            if (settings.value("ShowThumbnails","false")=="true")
            {
                QString suffix = item->name.toLower();
                if ( (suffix.endsWith(".jpg")) || (suffix.endsWith(".jpeg")) ||
                     (suffix.endsWith(".png")) || (suffix.endsWith(".tiff")) ||
                     (suffix.endsWith(".tif")) || (suffix.endsWith(".bmp")) ||
                     (suffix.endsWith(".avi")) || (suffix.endsWith(".mp4")) ||
                     (suffix.endsWith(".wmv")) || (suffix.endsWith(".3gp")) )
                {
                    QCryptographicHash md(QCryptographicHash::Md5);
                    QString file = "WebDav-" + q->m_service + "/" + misarchivos[i].value("id","").toString();
                    file.replace("//","/");
                    md.addData(file.toUtf8());
                    QString tf = "/home/nemo/.thumbnails/filecase/"+ QString(md.result().toHex().constData()) + ".jpg";
                    if ( QFileInfo(tf).exists() ) {
                        item->exten = tf;
                    } else if (misarchivos[i].value("thumb","").toString()!="" ) {
                        item->dexten = tf;
                        //q->pepe->addImage(tf, misarchivos[i].value("thumb","").toString());
                        q->StartThread = 1;
                    }
                }

            }

        }

        item->link = misarchivos[i].value("link","").toString();
        item->fid = misarchivos[i].value("id","").toString();

        item->pub = misarchivos[i].value("public","").toString();

        if(misarchivos[i].value("type","").toString()=="file")
            item->type = WebDav::File;
        else
            item->type = WebDav::Folder;

        items.append(item);
    }

    q->endInsertRows();

    emit(q->countChanged());

    emit q->endResetModel();

    emit q->pathChanged();

    if ( q->StartThread == 1 )
    {
        //q->pepe->done();
    }

}


WebDav::WebDav(QObject * parent) :
        QAbstractListModel(parent) , d (new WebDavPrivate(this))
{
    QHash <int, QByteArray> roles;
    roles.insert(NameRole , "name");
    roles.insert(CreatedRole , "created");
    roles.insert(ModifiedRole , "modified");
    roles.insert(SizeRole , "size");
    roles.insert(RealSizeRole , "realsize");
    roles.insert(PathRole , "path");
    roles.insert(ExtenRole , "exten");
    roles.insert(DExtenRole , "dexten");
    roles.insert(SecRole , "sec");
    roles.insert(LinkRole , "link");
    roles.insert(TypeRole , "type");
    roles.insert(IdRole , "fid");
    roles.insert(PublicRole , "pub");
    d->q->setRoleNames(roles);

    m_working = false;

    //pepe = new SkyThumbnailer();
    //connect (pepe, SIGNAL(imageLoaded(QString)), this, SLOT(imageLoaded(QString)));

    datos = new QNetworkAccessManager();

    d->path = "/";

    spaceUsed = 0;
    spaceTotal = 0;
    accountUser.clear();
}

QHash<int, QByteArray> WebDav::roleNames() const {
    return m_roles;
}

void WebDav::setRoleNames(const QHash<int, QByteArray>& roles) {
    m_roles = roles;
}

WebDav::~WebDav()
{
    delete d;
}

int WebDav::rowCount(const QModelIndex & parent) const
{
    Q_UNUSED(parent)
    return d->items.size();
}

QVariant WebDav::data(const QModelIndex & index , int role) const
{
    if(index.row() < 0 or index.row() > count())
    {
        return QVariant();
    }

    WebDavItem * item = d->items.at(index.row());
    switch(role)
    {
    case NameRole:
        return item->name;
        break;
    case CreatedRole:
        return item->created;
        break;
    case ModifiedRole:
        return item->modified;
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
    case IdRole:
        return item->fid;
        break;
    case PublicRole:
        return item->pub;
        break;
    default:
        return QVariant();
        break;
    }
}

int WebDav::count() const
{
        return rowCount();
}

QString WebDav::path() const
{
    QString tmp = d->path;
    if ( tmp.startsWith("//") )
            tmp.remove(0,1);
    if (tmp=="") tmp = "/";
    d->path = tmp;
    return d->path;
}

QString WebDav::path1() const
{
    if (d->path=="/")
        return d->path;
    else
        return QFileInfo(d->path).fileName();
}

QString WebDav::path2() const
{
    if (d->path=="/")
        return "";
    else
        return QFileInfo(d->path).path();
}

QString WebDav::cname(int index)
{
    QString result;
    if ( d->items.at(index)->type==WebDav::Folder )
        result = "a";
    else
        result = "b";

    result += d->items.at(index)->sec;

    return result;
}


void WebDav::reload()
{
    d->q->requestMetadata(d->path);
}

void WebDav::setPath(QString path)
{
    qDebug() << "PATH: " << path;
    path = path.replace("//","/");
    d->path = path;
    d->q->requestMetadata(d->path);
}

void WebDav::goUp()
{
    QString tmp = d->path;
    int i = tmp.lastIndexOf("/");
    tmp.remove(i, tmp.length()-i);
    if (tmp=="") tmp = "/";
    d->path = tmp;
    d->q->requestMetadata(d->path);
}

void WebDav::logIn(QString service, QString path)
{
    d->creatingAccount = false;

    m_service = service;
    emit serviceChanged();

    emit requestStarted();
    QString serv = "WebDav-" + service;
    QSettings settings("cepiperez","fileboxplus");

    d->path = path;
    m_path = d->path;

    emit pathChanged();

    //accountUser.clear();
    //emit userChanged(accountUser);

    account.clear();
    account.insert("host", settings.value(serv+"/host","").toString());
    account.insert("path", settings.value(serv+"/path","").toString());
    account.insert("user", settings.value(serv+"/user","").toString());
    account.insert("pass", settings.value(serv+"/pass","").toString());
    account.insert("port", settings.value(serv+"/port","").toString());
    account.insert("name", service);

    startService(settings.value(serv+"/host","").toString(),
                 settings.value(serv+"/path","").toString(),
                 settings.value(serv+"/user","").toString(),
                 settings.value(serv+"/pass","").toString(),
                 settings.value(serv+"/port","").toString());

}


void WebDav::clearList()
{
    d->archivos.clear();
    emit d->q->beginResetModel();
    d->items.clear();
    emit d->q->endResetModel();
}

void WebDav::requestMetadata(QString path)
{
    clearList();
    emit d->q->requestStarted();
    m_path = path + "/";
    m_path.replace("//", "/");

    disconnect(&parser, 0, 0, 0);
    disconnect(&qwebdav, 0, 0, 0);

    connect(&parser, SIGNAL(finished()), this, SLOT(printList()));
    connect(&parser, SIGNAL(errorChanged(QString)), this, SLOT(printError(QString)));
    connect(&qwebdav, SIGNAL(errorChanged(QString)), this, SLOT(printError(QString)));

    parser.listDirectory(&qwebdav, m_path);

}

void WebDav::handleMetadata(QNetworkReply *networkReply)
{
    QSettings settings("cepiperez","fileboxplus");

    QString dirJson = networkReply->readAll();
    //qDebug() << dirJson;

    if (dirJson.contains(" has expired"))
    {
        qDebug() << "REQUESTING NEW TOKEN";

        action = "request-token";
        QUrl url("https://login.live.com/oauth20_token.srf");
        QNetworkRequest request;
        request.setUrl(url);
        request.setRawHeader("Content-Type", "application/x-www-form-urlencoded");

        QByteArray params = "client_id=" + clientId.toLatin1();
        params += "&client_secret=" + clientSecret.toLatin1();
        params += "&refresh_token=" + refreshToken.toLatin1();
        params += "&redirect_uri=https://login.live.com/oauth20_desktop.srf";
        params += "&grant_type=refresh_token";

        disconnect(datos,0,0,0);
        connect(datos, SIGNAL(finished(QNetworkReply*)),this, SLOT(handleNetworkReply(QNetworkReply*)));
        datos->post(request, params);
        return;
    }

    bool ok;
    QVariantMap jsonResult = QtJson::Json::parse(dirJson, ok).toMap();
    if(!ok)
    {
        qDebug() << "METADATA ERROR!";
        d->archivos.clear();
        d->populateItems();
        emit d->q->requestDone(false);
        return;
    }
    //qDebug() << jsonResult;

    QString dateFormat = settings.value("DateFormat","mm/dd/yyyy").toString().replace("mm","MM");

    // add folders
    foreach(const QVariant &itemJson, jsonResult["data"].toList())
    {
        QVariantMap itemMap = itemJson.toMap();


        //qDebug() << "ITEM:--------------------------\n" << itemMap;

        QVariantMap item;
        if(itemMap["type"].toString()=="folder" || itemMap["type"].toString()=="album")
            item.insert("type", "folder");
        else
            item.insert("type", "file");

        QVariantMap tmp = itemMap["from"].toMap();

        parentID = itemMap["parent_id"].toString();

        QString f = itemMap["name"].toString();
        item.insert("name", f);

        QString predate = itemMap["updated_time"].toString();
        predate = predate.left(10);
        QDateTime modified = QDateTime::fromString(predate,"yyyy-MM-dd");
        item.insert("modified", modified.toString(dateFormat));

        predate = itemMap["created_time"].toString();
        predate = predate.left(10);
        modified = QDateTime::fromString(predate,"yyyy-MM-dd");
        item.insert("created", modified.toString(dateFormat));


        if(itemMap["type"].toString()=="folder" || itemMap["type"].toString()=="album")
            item.insert("size", "0");
        else
            item.insert("size", itemMap["size"].toString());

        if(itemMap["type"].toString()=="folder" || itemMap["type"].toString()=="album")
            item.insert("sec", "a_"+f.toLower());
        else
            item.insert("sec", "b_"+f.toLower());

        item.insert("link", itemMap["upload_location"].toString());
        item.insert("id", itemMap["id"].toString());


        QString thumb = "";
        foreach(const QVariant &itemJson2, itemMap["images"].toList())
        {
            QVariantMap itemMap2 = itemJson2.toMap();
            if (itemMap2["type"].toString()=="album")
                thumb = itemMap2["source"].toString();
        }

        if (thumb == "") {
            thumb = itemMap["picture"].toString();
        }


        QString pub = "false";
        QVariantMap tmp2 = itemMap["shared_with"].toMap();
        if (tmp2["access"].toString()=="Public")
            pub = "true";

        item.insert("thumb", thumb);
        item.insert("public", pub);

        d->archivos.append(item);

    }

    qSort(d->archivos.begin(), d->archivos.end(), sortVariantMap);

    d->populateItems();

    /*if (!spaceTotal>0)
    {
        QUrl url("https://apis.live.net/v5.0/me/skydrive/quota");
        QUrlQuery query;
        query.addQueryItem("access_token", accessToken.toLatin1());
        url.setQuery(query);
        QNetworkRequest request(url);
        disconnect(datos,0,0,0);
        connect(datos, SIGNAL(finished(QNetworkReply*)),this, SLOT(handleInfo(QNetworkReply*)));
        datos->get(request);
    }
    else
    {
        emit d->q->requestDone(true);
    }*/

}

void WebDav::handleInfo()
{
    QString dirJson = reply->readAll();
    qDebug() << dirJson;

    QString used = dirJson;

    if (used.contains("<d:quota-used-bytes>"))
    {
        int i = used.indexOf("<d:quota-used-bytes>");
        used.remove(0, i);
        used.remove("<d:quota-used-bytes>");
        i = used.indexOf("<");
        used = used.left(i);
    } else
        used = "0";

    QString avail = dirJson;

    if (avail.contains("<d:quota-available-bytes>"))
    {
        int i = avail.indexOf("<d:quota-available-bytes>");
        avail.remove(0, i);
        avail.remove("<d:quota-available-bytes>");
        i = avail.indexOf("<");
        avail = avail.left(i);
    } else
        avail = "0";

    //double tot = used.toDouble() + avail.toDouble();
    qDebug() << "SPACE:" << used << avail;

    accountUser.clear();
    accountUser.insert("user", account.value("user","").toString());
    accountUser.insert("email", "");
    accountUser.insert("picture", "");
    accountUser.insert("spaceUsed", used.toDouble());
    accountUser.insert("spaceTotal", avail.toDouble());
    accountUser.insert("spaceUsedText", GetFileSize(used.toDouble()));
    accountUser.insert("spaceTotalText", GetFileSize(avail.toDouble()));

    emit d->q->userChanged(accountUser);
    emit d->q->requestDone(true);

}

void WebDav::newFolder(QString name)
{
    action = "newfolder";

    d->archivos.clear();
    emit d->q->requestStarted();

    QString d = m_path + "/" + name;
    d.replace("//", "/");

    reply = qwebdav.mkdir(d);

    disconnect(reply, 0, 0, 0);
    connect(reply, SIGNAL(finished()), this, SLOT(handleFileOps()));

    /*QUrl url = "https://apis.live.net/v5.0/" + parentID;
    QNetworkRequest networkRequest(url);
    networkRequest.setRawHeader("Content-Type", "application/json");
    networkRequest.setRawHeader("Authorization", "Bearer "+accessToken.toLatin1());

    QString params = "{\n";
    params += " \"name\": \"" + name + "\"\n";
    params += "}";

    //qDebug() << params;
    disconnect(datos,0,0,0);
    connect(datos, SIGNAL(finished(QNetworkReply*)),this, SLOT(handleFileOps(QNetworkReply*)));
    datos->post(networkRequest,params.toLatin1());*/
}

void WebDav::handleFileOps()
{
    qDebug() << reply->readAll();

    if (action.startsWith("rename")) {
        action = action.remove(0,6);
        emit fileRenamed(action);
    }

    d->q->requestMetadata(d->path);
}

void WebDav::renameFile(QString source, QString dest)
{
    action = "rename" + dest;

    d->archivos.clear();
    emit d->q->requestStarted();

    if (source.endsWith("/"))
        source.chop(1);


    QString dp = QFileInfo(source).absolutePath() + "/" + dest;
    dp.replace("//", "/");

    qDebug() << "RENAMING " << source << " TO " << dp;

    reply = qwebdav.move(source, dp);

    disconnect(reply, 0, 0, 0);
    connect(reply, SIGNAL(finished()), this, SLOT(handleFileOps()));

    //qDebug() << reply->readAll();

    //qDebug() << "RENAMING FOLDER TO " << dest;

    /*QUrl url = "https://apis.live.net/v5.0/" + source;
    QNetworkRequest networkRequest(url);
    networkRequest.setRawHeader("Content-Type", "application/json");
    networkRequest.setRawHeader("Authorization", "Bearer "+accessToken.toLatin1());

    QString params = "{\n";
    params += " \"name\": \"" + dest + "\"\n";
    params += "}";

    //qDebug() << params;
    disconnect(datos,0,0,0);
    connect(datos, SIGNAL(finished(QNetworkReply*)),this, SLOT(handleFileOps(QNetworkReply*)));
    datos->put(networkRequest,params.toLatin1());*/

}

void WebDav::moveFile(QString source)
{
    action = "move";

    d->archivos.clear();
    emit d->q->requestStarted();

    QString dp = QFileInfo(m_path).absolutePath() + "/" + QFileInfo(source).fileName();
    dp.replace("//", "/");

    reply = qwebdav.move(source, dp, true);

    disconnect(reply, 0, 0, 0);
    connect(reply, SIGNAL(finished()), this, SLOT(handleFileOps()));

}

void WebDav::removeFile(QString path)
{
    action = "remove";

    d->archivos.clear();
    emit d->q->requestStarted();
    //if (path.endsWith("/"))
    //    path.chop(1);

    qDebug() << "REMOVING " << path;

    reply = qwebdav.remove(path);

    disconnect(reply, 0, 0, 0);
    connect(reply, SIGNAL(finished()), this, SLOT(handleFileOps()));


}

void WebDav::copyFile(QString source)
{
    action = "copy";

    d->archivos.clear();
    emit d->q->requestStarted();

    QString dp = QFileInfo(m_path).absolutePath() + "/" + QFileInfo(source).fileName();
    dp.replace("//", "/");

    qDebug() << "COPYING " << source << " TO " << dp;


    reply = qwebdav.copy(source, dp, true);

    disconnect(reply, 0, 0, 0);
    connect(reply, SIGNAL(finished()), this, SLOT(handleFileOps()));

}

void WebDav::imageLoaded(QString filename)
{
    qDebug() << "Image loaded" << filename;
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

QString WebDav::getPreview(QString link, QString filename, QString mode)
{
    Q_UNUSED(link)

    QString preview = "";
    if (mode=="download")
    {
        QCryptographicHash md(QCryptographicHash::Md5);
        QString file = "WebDav-" + m_service + "/" + filename;
        file.replace("//","/");
        md.addData(file.toUtf8());
        QString ef = ".jpg";
        if (filename.endsWith(".png")) ef = ".png";
        QString tf = "/home/nemo/.thumbnails/filecase/"+ QString(md.result().toHex().constData()) + ef;
        if (QFileInfo(tf).exists())
            preview = tf;
        else
            preview = GetExtension(filename.toLower());
    }
    else
    {
        if ( filename.toLower().endsWith(".jpg") )
            preview = "image/jpeg";
        else if ( filename.toLower().endsWith(".jpeg") )
            preview = "image/jpeg";
        else if ( filename.toLower().endsWith(".png") )
            preview = "image/png";
        else if ( filename.toLower().endsWith(".gif") )
            preview = "image/gif";
        else if ( filename.toLower().endsWith(".avi") )
            preview = "video/x-msvideo";
        else if ( filename.toLower().endsWith(".mp4") )
            preview = "video/mp4";
        else if ( filename.toLower().endsWith(".wmv") )
            preview = "video/x-msvideo";
        else if ( filename.toLower().endsWith(".3gp") )
            preview = "video/3gpp";
        else
            preview = GetExtension(filename.toLower());
    }
    return preview;
}

void WebDav::setFilter(QString filter)
{
    d->filter = filter.toLower();
    d->populateItems();

}

void WebDav::shareFile(QString filename)
{
    d->archivos.clear();
    emit d->q->requestStarted();

    QUrl url = "https://apis.live.net/v5.0/" + filename + "/shared_read_link";
    QNetworkRequest networkRequest(url);
    //networkRequest.setRawHeader("Content-Type", "application/json");
    networkRequest.setRawHeader("Authorization", "Bearer "+accessToken.toLatin1());

    disconnect(datos,0,0,0);
    connect(datos, SIGNAL(finished(QNetworkReply*)),this, SLOT(handleShare(QNetworkReply*)));
    datos->get(networkRequest);

}

void WebDav::handleShare(QNetworkReply *networkReply)
{
    QString dirJson = networkReply->readAll();
    //qDebug() << dirJson;

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

    qDebug() << "LINK: " << jsonResult["link"].toString();
    QString shareLink = jsonResult["link"].toString();
    emit d->q->requestDone(true);
    emit d->q->linkShared(shareLink);

}

void WebDav::publicLink(QString filename)
{
    d->archivos.clear();
    emit d->q->requestStarted();

    QUrl url = "https://apis.live.net/v5.0/" + filename + "/shared_read_link";
    QNetworkRequest networkRequest(url);
    //networkRequest.setRawHeader("Content-Type", "application/json");
    networkRequest.setRawHeader("Authorization", "Bearer "+accessToken.toLatin1());

    disconnect(datos,0,0,0);
    connect(datos, SIGNAL(finished(QNetworkReply*)),this, SLOT(handleShare(QNetworkReply*)));
    datos->get(networkRequest);
}

void WebDav::removeAccount()
{
    QSettings settings("cepiperez","fileboxplus");
    settings.remove("SKY/access_token");
    settings.remove("SKY/refresh_token");
    settings.sync();
    //d->q->logIn();
}

void WebDav::setUploadFolder(QString name)
{
    Q_UNUSED(name)

    QSettings settings("cepiperez","fileboxplus");
    settings.setValue("WebDav-" + m_service + "/upload_folder", d->path);
    settings.sync();
}

void WebDav::createAccount(QString name, QString host, QString path, QString user, QString pass, QString port)
{
    d->creatingAccount = true;

    m_service = name;
    emit serviceChanged();

    d->path = "/";
    m_path = d->path;

    //host.remove("http://").remove("https://");

    qDebug() << "Creating WebDav account: " << name << host << path << user << pass;

    account.clear();
    account.insert("name", name.trimmed());
    account.insert("host", host.trimmed());
    account.insert("path", path.trimmed());
    account.insert("user", user.trimmed());
    account.insert("pass", pass.trimmed());
    account.insert("port", port.trimmed());

    startService(host.trimmed(), path.trimmed(), user.trimmed(), pass.trimmed(), port.trimmed());

}

void WebDav::startService(QString host, QString path, QString user, QString pass, QString port)
{
    m_error = "";

    QString ctype;
    if (host.startsWith("http://")) {
        ctype = "http";
    } else {
        ctype = "https";
    }

    if (port=="") {
        if (ctype=="http")
            port = "80";
        else
            port = "443";
    }

    host.remove("http://").remove("https://");

    qwebdav.setConnectionSettings(ctype=="http"? QWebdav::HTTP : QWebdav::HTTPS,
                                  host, path, user, pass, port.toInt());

    disconnect(&parser, 0, 0, 0);
    disconnect(&qwebdav, 0, 0, 0);

    connect(&parser, SIGNAL(finished()), this, SLOT(printList()));
    connect(&parser, SIGNAL(errorChanged(QString)), this, SLOT(printError(QString)));
    connect(&qwebdav, SIGNAL(errorChanged(QString)), this, SLOT(printError(QString)));
    //m_path = "/";

    setPath(m_path);
    //parser.listDirectory(&qwebdav, m_path);

}

void WebDav::printList()
{
    QString name = account.value("name","").toString();

    //QNetworkReply* reply = qobject_cast<QNetworkReply*>(QObject::sender());
    //QList<QWebdavItem> list = parser.getList();
    //QWebdavItem item;
    //foreach(item, list) {

    if (m_error!="") {
        qDebug() << "Error in response!";
        emit requestDone(false);
        return;
    }


    if (d->creatingAccount) {
        QSettings settings("cepiperez","fileboxplus");
        settings.setValue("WebDav-"+name+"/host", account.value("host","").toString());
        settings.setValue("WebDav-"+name+"/path", account.value("path","").toString());
        settings.setValue("WebDav-"+name+"/user", account.value("user","").toString());
        settings.setValue("WebDav-"+name+"/pass", account.value("pass","").toString());
        settings.setValue("WebDav-"+name+"/port", account.value("port","").toString());
        settings.sync();
        emit accountCreated();
        d->creatingAccount = false;
        return;
    }

    QSettings settings("cepiperez","fileboxplus");

    QList<QWebdavItem> list = parser.getList();
    QWebdavItem item;

    foreach(item, list) {
        qDebug() << item.name() << item.path();

        QVariantMap map;
        if(item.isDir())
            map.insert("type", "folder");
        else
            map.insert("type", "file");

        QString f = item.name();
        map.insert("name", f);

        QString dateFormat = settings.value("DateFormat","mm/dd/yyyy").toString().replace("mm","MM");

        QDateTime modified = item.lastModified();
        map.insert("modified", modified.toString(dateFormat));

        modified = item.createdAt();
        map.insert("created", modified.toString(dateFormat));


        if(item.isDir())
            map.insert("size", "0");
        else
            map.insert("size", item.size());

        if(item.isDir())
            map.insert("sec", "a_"+f.toLower());
        else
            map.insert("sec", "b_"+f.toLower());

        map.insert("link", item.path());
        map.insert("id", item.path());


        QString thumb = "";
        /*foreach(const QVariant &itemJson2, itemMap["images"].toList())
        {
            QVariantMap itemMap2 = itemJson2.toMap();
            if (itemMap2["type"].toString()=="album")
                thumb = itemMap2["source"].toString();
        }

        if (thumb == "") {
            thumb = itemMap["picture"].toString();
        }*/

        map.insert("thumb", thumb);
        //map.insert("public", pub);

        QString pre = m_path;
        if (item.path() != pre)
            d->archivos.append(map);
    }

    qSort(d->archivos.begin(), d->archivos.end(), sortVariantMap);

    d->populateItems();

    emit requestDone(true);

}

void WebDav::getQuota()
{
    qDebug() << "Getting quota";
    emit requestStarted();

    QStringList props;
    props << "quota-available-bytes" << "quota-used-bytes";

    disconnect(&parser, 0, 0, 0);
    disconnect(&qwebdav, 0, 0, 0);

    action = "quota";
    reply = qwebdav.propfind2("/", props);

    disconnect(reply, 0, 0, 0);

    connect(reply, SIGNAL(finished()), this, SLOT(handleInfo()));

}

void WebDav::printError(QString errorMsg)
{
    qDebug() << "QWebdav::printErrors()  errorMsg == " << errorMsg;
    m_error = errorMsg;
    emit accountError(m_error);
}

void WebDav::replySkipRead()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(QObject::sender());
    if (reply==0) {
        qDebug() << "Reply returns 0";
        return;
    }

    QByteArray ba = reply->readAll();

    qDebug() << "QWebdav::replySkipRead()   skipped " << ba.size() << " reply->url() == " << reply->url().toString(QUrl::RemoveUserInfo);
}

