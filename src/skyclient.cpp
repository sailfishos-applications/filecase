#include "skyclient.h"

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

struct SkyClientItem
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
    SkyClient::ItemType type;
};

class SkyClient::SkyClientPrivate
{
public:
    SkyClientPrivate(SkyClient * parent);
    ~SkyClientPrivate();
    void populateItems();
    QList <SkyClientItem * > items;
    QString path;
    SkyClient * const q;
    QList<QVariantMap> archivos;
    QString filter;
};

SkyClient::SkyClientPrivate::SkyClientPrivate(SkyClient * parent) :
        q(parent)
{
    filter = "";

}

SkyClient::SkyClientPrivate::~SkyClientPrivate()
{
    while(!items.isEmpty())
    {
        delete items.takeFirst();
    }
}

void SkyClient::SkyClientPrivate::populateItems()
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


    QSettings settings("cepiperez","fileboxplus");

    if (settings.value("OneDrive/upload_folder","").toString()=="")
        settings.setValue("OneDrive/upload_folder", "/||https://apis.live.net/v5.0/me/skydrive/files");

    int entriesCount = misarchivos.count();

    emit q->beginResetModel();

    q->beginInsertRows(QModelIndex(), 0, entriesCount-1);

    for (int i=0; i<misarchivos.count(); ++i)
    {
        SkyClientItem * item = new SkyClientItem();

        //qDebug() << misarchivos[i];

        item->name = misarchivos[i].value("name","").toString();

        item->created = misarchivos[i].value("created","").toString();

        item->modified = misarchivos[i].value("modified","").toString();

        item->size = GetFileSize(misarchivos[i].value("size","").toDouble());

        item->realsize = misarchivos[i].value("size","0").toString();

        item->sec = item->name.left(1).toUpper();

        item->path = path + "/" + item->name;
        item->path = item->path.replace("//","/");

        item->link = misarchivos[i].value("link","").toString();
        item->fid = misarchivos[i].value("id","").toString();

        item->pub = misarchivos[i].value("public","").toString();

        if(misarchivos[i].value("type","").toString()=="file")
            item->type = SkyClient::File;
        else
            item->type = SkyClient::Folder;

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
                    QString file = "OneDrive/" + item->link;
                    file.replace("//","/");
                    md.addData(file.toUtf8());
                    QString tf = "/home/nemo/.thumbnails/filecase/"+ QString(md.result().toHex().constData()) + ".jpg";
                    if ( QFileInfo(tf).exists() ) {
                        item->exten = tf;
                    } else if (misarchivos[i].value("thumb","").toString()!="" ) {
                        item->dexten = tf;
                        q->pepe->addImage(tf, misarchivos[i].value("thumb","").toString());
                        q->StartThread = 1;
                    }
                }

            }

        }

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


SkyClient::SkyClient(QObject * parent) :
        QAbstractListModel(parent) , d (new SkyClientPrivate(this))
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

    pepe = new SkyThumbnailer();
    connect (pepe, SIGNAL(imageLoaded(QString)), this, SLOT(imageLoaded(QString)));


    datos = new QNetworkAccessManager();

    d->path = "/";
    history.append("/");

    spaceUsed = 0;
    spaceTotal = 0;
    accountUser = "";
}

QHash<int, QByteArray> SkyClient::roleNames() const {
    return m_roles;
}

void SkyClient::setRoleNames(const QHash<int, QByteArray>& roles) {
    m_roles = roles;
}

SkyClient::~SkyClient()
{
    delete d;
}

int SkyClient::rowCount(const QModelIndex & parent) const
{
    Q_UNUSED(parent)
    return d->items.size();
}

QVariant SkyClient::data(const QModelIndex & index , int role) const
{
    if(index.row() < 0 or index.row() > count())
    {
        return QVariant();
    }

    SkyClientItem * item = d->items.at(index.row());
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

int SkyClient::count() const
{
        return rowCount();
}

QString SkyClient::path() const
{
    QString tmp = d->path;
    if ( tmp.startsWith("//") )
            tmp.remove(0,1);
    if (tmp=="") tmp = "/";
    d->path = tmp;
    return d->path;
}

QString SkyClient::path1() const
{
    if (d->path=="/")
        return d->path;
    else
        return QFileInfo(d->path).fileName();
}

QString SkyClient::path2() const
{
    if (d->path=="/")
        return "";
    else
        return QFileInfo(d->path).path();
}

QString SkyClient::cname(int index)
{
    QString result;
    if ( d->items.at(index)->type==SkyClient::Folder )
        result = "a";
    else
        result = "b";

    result += d->items.at(index)->sec;

    return result;
}


void SkyClient::reload()
{
    d->q->requestMetadata(history.last());
}

void SkyClient::setPath(QString path)
{
    path = path.replace("//","/");
    d->path = path;
}

void SkyClient::setPath2(QString path, QString link)
{
    qDebug() << "PATH: " << path << " - LINK: " << link;

    QSettings settings("cepiperez","fileboxplus");
    accessToken = settings.value("OneDrive/access_token").toString();
    refreshToken = settings.value("OneDrive/refresh_token").toString();

    if(d->path != path)
    {
        path = path.replace("//","/");
        d->path = path;
        history.append(link);
        d->q->requestMetadata(history.last());
    }
}

void SkyClient::goUp()
{
    QString tmp = d->path;
    int i = tmp.lastIndexOf("/");
    tmp.remove(i, tmp.length()-i);
    if (tmp=="") tmp = "/";
    d->path = tmp;
    history.removeLast();
    d->q->requestMetadata(history.last());
}

void SkyClient::logIn(QString path)
{
    d->q->path1() = "/";
    d->q->path2() = "";
    emit d->q->pathChanged();

    clientId = "000000004C0CB4E7";
    clientSecret = "lABMSvVTZfWRhck5qBs25C1l1s4qyp0t";

    QSettings settings("cepiperez","fileboxplus");
    if( (settings.value("OneDrive/access_token","").toString()!="") ||
        (settings.value("OneDrive/refresh_token","").toString()!="") )
    {
        accessToken = settings.value("OneDrive/access_token").toString();
        refreshToken = settings.value("OneDrive/refresh_token").toString();
        emit d->q->loggedIn(false);
        history.clear();
        history << path;
        requestMetadata(path);
        return;
    }

    QString scope = "wl.skydrive, wl.skydrive_update, wl.offline_access";
    action = "request-token";
    emit d->q->requestStarted();
    QString purl = "https://login.live.com/oauth20_authorize.srf?";
    purl += "client_id=" + clientId + "&scope=" + scope + "&response_type=code&";
    purl += "redirect_uri=https://login.live.com/oauth20_desktop.srf";

    d->q->openUrl(purl);

}

void SkyClient::handleNetworkReply(QNetworkReply *networkReply)
{
    if (action.startsWith("request-token"))
    {
        QString reply = QString::fromUtf8(networkReply->readAll());
        qDebug() << "REQUEST TOKEN DONE\n" << reply;

        QString tmp = reply;
        int i = tmp.indexOf("access_token");
        tmp = tmp.remove(0, i+15);
        i = tmp.indexOf("\"");
        tmp = tmp.remove(i, tmp.length()-i);
        accessToken = tmp;

        tmp = reply;
        i = tmp.indexOf("refresh_token");
        tmp = tmp.remove(0, i+16);
        i = tmp.indexOf("\"");
        tmp = tmp.remove(i, tmp.length()-i);
        refreshToken = tmp;

        //qDebug() << accessToken << "\n";

        //qDebug() << refreshToken << "\n";

        //accessToken = jParser.getParam(reply, "access_token").remove("\"").remove("}");
        //refreshToken = jParser.getParam(reply, "refresh_token").remove("\"").remove("}");

        QSettings settings("cepiperez","fileboxplus");
        settings.setValue("OneDrive/access_token", accessToken);
        settings.setValue("OneDrive/refresh_token", refreshToken);
        settings.sync();

        emit d->q->loggedIn(true);

        if (action == "request-token-new")
            requestMetadata(history.last());

    }

}

void SkyClient::parseData(QString dir, QString data)
{
    Q_UNUSED(data)

    //qDebug() << dir;
    //qDebug() << data;

    if (dir.contains("https://login.live.com/oauth20_desktop.srf?code="))
    {
        action = "request-token";
        QString code = dir;
        int i = dir.indexOf("?code=");
        accessCode = code.remove(0, i+6);

        QUrl url("https://login.live.com/oauth20_token.srf");
        QNetworkRequest request;
        request.setUrl(url);
        request.setRawHeader("Content-Type", "application/x-www-form-urlencoded");

        QByteArray params = "client_id=" + clientId.toLatin1();
        params += "&client_secret=" + clientSecret.toLatin1();
        params += "&code=" + accessCode.toLatin1();
        params += "&redirect_uri=https://login.live.com/oauth20_desktop.srf";
        params += "&grant_type=authorization_code";

        disconnect(datos,0,0,0);
        connect(datos, SIGNAL(finished(QNetworkReply*)),this, SLOT(handleNetworkReply(QNetworkReply*)));
        datos->post(request, params);
        return;

    }
    /*else if (dir=="https://www.Sky.com/m")
    {
        d->q->closeUrl(false);
    }*/

}

void SkyClient::clearList()
{
    d->archivos.clear();
    emit d->q->beginResetModel();
    d->items.clear();
    emit d->q->endResetModel();
}

void SkyClient::requestMetadata(QString path)
{
    clearList();
    emit d->q->requestStarted();

    QUrl url;

    if (path=="/")
        url = "https://apis.live.net/v5.0/me/skydrive/files";
    else
        url = path;

    QUrlQuery query;
    query.addQueryItem("access_token", accessToken.toLatin1());
    url.setQuery(query);
    QNetworkRequest request(url);
    qDebug() << "OPENING " << url;

    disconnect(datos,0,0,0);
    connect(datos, SIGNAL(finished(QNetworkReply*)),this, SLOT(handleMetadata(QNetworkReply*)));
    datos->get(request);

}

void SkyClient::handleMetadata(QNetworkReply *networkReply)
{
    QSettings settings("cepiperez","fileboxplus");

    QString dirJson = networkReply->readAll();
    //qDebug() << dirJson;

    if (dirJson.contains(" has expired"))
    {
        qDebug() << "REQUESTING NEW TOKEN";

        action = "request-token-new";
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
        accountUser = tmp["name"].toString();

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

    emit d->q->requestDone(true);

}

void SkyClient::getQuota()
{
    emit d->q->requestStarted();
    QUrl url("https://apis.live.net/v5.0/me/skydrive/quota");
    QUrlQuery query;
    query.addQueryItem("access_token", accessToken.toLatin1());
    url.setQuery(query);
    QNetworkRequest request(url);
    disconnect(datos,0,0,0);
    connect(datos, SIGNAL(finished(QNetworkReply*)),this, SLOT(handleInfo(QNetworkReply*)));
    datos->get(request);
}

void SkyClient::handleInfo(QNetworkReply *networkReply)
{
    QString dirJson = networkReply->readAll();
    qDebug() << dirJson;

    bool ok;
    QVariantMap jsonResult = QtJson::Json::parse(dirJson, ok).toMap();
    if (!ok) return;

    spaceUsed = jsonResult["quota"].toDouble() - jsonResult["available"].toDouble();
    spaceTotal = jsonResult["quota"].toDouble();

    qDebug() << "SPACE: " << spaceUsed << " OF " << spaceTotal;
    //emit d->q->spaceChanged(GetFileSize(spaceUsed), GetFileSize(spaceTotal), spaceUsed/1024/1024, spaceTotal/1024/1024);

    QVariantMap userdata;

    userdata.insert("user", accountUser);
    userdata.insert("email", "");
    userdata.insert("picture", "");
    userdata.insert("spaceUsed", jsonResult["quota"].toDouble() - jsonResult["available"].toDouble());
    userdata.insert("spaceTotal", jsonResult["quota"].toDouble());
    userdata.insert("spaceUsedText", GetFileSize(jsonResult["quota"].toDouble() - jsonResult["available"].toDouble()));
    userdata.insert("spaceTotalText", GetFileSize(jsonResult["quota"].toDouble()));

    emit d->q->userChanged(userdata);
    emit d->q->requestDone(true);
}

void SkyClient::newFolder(QString name)
{
    action = "newfolder";

    d->archivos.clear();
    emit d->q->requestStarted();

    QUrl url = "https://apis.live.net/v5.0/" + parentID;
    QNetworkRequest networkRequest(url);
    networkRequest.setRawHeader("Content-Type", "application/json");
    networkRequest.setRawHeader("Authorization", "Bearer "+accessToken.toLatin1());

    QString params = "{\n";
    params += " \"name\": \"" + name + "\"\n";
    params += "}";

    //qDebug() << params;
    disconnect(datos,0,0,0);
    connect(datos, SIGNAL(finished(QNetworkReply*)),this, SLOT(handleFileOps(QNetworkReply*)));
    datos->post(networkRequest,params.toLatin1());
}

void SkyClient::handleFileOps(QNetworkReply *networkReply)
{
    QString dirJson = networkReply->readAll();
    qDebug() << dirJson;

    if (action.startsWith("rename")) {
        action = action.remove(0,6);
        emit fileRenamed(action);
    }

    d->q->requestMetadata(history.last());
}

void SkyClient::renameFile(QString source, QString dest)
{
    action = "rename" + dest;

    d->archivos.clear();
    emit d->q->requestStarted();

    //qDebug() << "RENAMING FOLDER TO " << dest;

    QUrl url = "https://apis.live.net/v5.0/" + source;
    QNetworkRequest networkRequest(url);
    networkRequest.setRawHeader("Content-Type", "application/json");
    networkRequest.setRawHeader("Authorization", "Bearer "+accessToken.toLatin1());

    QString params = "{\n";
    params += " \"name\": \"" + dest + "\"\n";
    params += "}";

    //qDebug() << params;
    disconnect(datos,0,0,0);
    connect(datos, SIGNAL(finished(QNetworkReply*)),this, SLOT(handleFileOps(QNetworkReply*)));
    datos->put(networkRequest,params.toLatin1());
}

void SkyClient::moveFile(QString source)
{
    action = "move";

    d->archivos.clear();
    emit d->q->requestStarted();

    QUrl url = "https://apis.live.net/v5.0/" + source;
    QNetworkRequest networkRequest(url);
    networkRequest.setRawHeader("Content-Type", "application/json");
    networkRequest.setRawHeader("Authorization", "Bearer "+accessToken.toLatin1());

    QString dest = history.last();
    dest.remove("/files/");
    dest.remove("https://apis.live.net/v5.0/");

    //qDebug() << "MOVING " << source << " TO " << dest;

    QByteArray *queryText = new QByteArray( "{\n \"destination\": \"" + dest.toLatin1() + "\"\n }");

    QBuffer *query = new QBuffer(queryText);

    //qDebug() << url << params;
    disconnect(datos,0,0,0);
    connect(datos, SIGNAL(finished(QNetworkReply*)),this, SLOT(handleFileOps(QNetworkReply*)));
    datos->sendCustomRequest(networkRequest, "MOVE", query);
}

void SkyClient::removeFile(QString path)
{
    action = "remove";

    d->archivos.clear();
    emit d->q->requestStarted();

    qDebug() << "REMOVING " << path;

    QUrl url = "https://apis.live.net/v5.0/" + path;
    QNetworkRequest networkRequest(url);
    //networkRequest.setRawHeader("Content-Type", "application/json");
    networkRequest.setRawHeader("Authorization", "Bearer "+accessToken.toLatin1());

    disconnect(datos,0,0,0);
    connect(datos, SIGNAL(finished(QNetworkReply*)),this, SLOT(handleFileOps(QNetworkReply*)));
    datos->deleteResource(networkRequest);
}

void SkyClient::copyFile(QString source)
{
    action = "copy";

    d->archivos.clear();
    emit d->q->requestStarted();

    QUrl url = "https://apis.live.net/v5.0/" + source;
    QNetworkRequest networkRequest(url);
    networkRequest.setRawHeader("Content-Type", "application/json");
    networkRequest.setRawHeader("Authorization", "Bearer "+accessToken.toLatin1());

    QString dest = history.last();
    dest.remove("/files/");
    dest.remove("https://apis.live.net/v5.0/");

    qDebug() << "COPING " << source << " TO " << dest;

    QByteArray *queryText = new QByteArray( "{\n \"destination\": \"" + dest.toLatin1() + "\"\n }");

    QBuffer *query = new QBuffer(queryText);

    //qDebug() << url << params;
    disconnect(datos,0,0,0);
    connect(datos, SIGNAL(finished(QNetworkReply*)),this, SLOT(handleFileOps(QNetworkReply*)));
    datos->sendCustomRequest(networkRequest, "COPY", query);
}

void SkyClient::imageLoaded(QString filename)
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




void SkyClient::setFilter(QString filter)
{
    d->filter = filter.toLower();
    d->populateItems();

}

void SkyClient::shareFile(QString filename)
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

void SkyClient::handleShare(QNetworkReply *networkReply)
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

void SkyClient::publicLink(QString filename)
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

void SkyClient::removeAccount()
{
    QSettings settings("cepiperez","fileboxplus");
    settings.remove("OneDrive/access_token");
    settings.remove("OneDrive/refresh_token");
    settings.sync();
    d->q->logIn("/");
}

void SkyClient::setUploadFolder(QString name)
{
    QSettings settings("cepiperez","fileboxplus");
    settings.setValue("OneDrive/upload_folder", name + "||" + history.last());
    settings.sync();
}
