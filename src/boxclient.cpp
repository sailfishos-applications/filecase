#include "boxclient.h"

#include "sys/vfs.h"
#include "utils.h"
#include "config.h"

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
#include <QBuffer>

#include "dropbox/qt-json/json.h"
#include "drive/jsonparser.h"

struct BoxClientItem
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
    BoxClient::ItemType type;
};

class BoxClient::BoxClientPrivate
{
public:
    BoxClientPrivate(BoxClient * parent);
    ~BoxClientPrivate();
    void populateItems();
    QList <BoxClientItem * > items;
    QString path;
    BoxClient * const q;
    QList<QVariantMap> archivos;
    QString filter;
};

BoxClient::BoxClientPrivate::BoxClientPrivate(BoxClient * parent) :
        q(parent)
{
    filter = "";

}

BoxClient::BoxClientPrivate::~BoxClientPrivate()
{
    while(!items.isEmpty())
    {
        delete items.takeFirst();
    }
}

void BoxClient::BoxClientPrivate::populateItems()
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

    int entriesCount = misarchivos.count();

    emit q->beginResetModel();

    q->beginInsertRows(QModelIndex(), 0, entriesCount-1);


    for (int i=0; i<misarchivos.count(); ++i)
    {

        BoxClientItem * item = new BoxClientItem();
        item->name = misarchivos[i].value("name","").toString();

        item->modified = misarchivos[i].value("modified","").toString();
        item->created = misarchivos[i].value("created","").toString();
        item->size = GetFileSize(misarchivos[i].value("size","0").toString().toDouble());
        item->realsize = misarchivos[i].value("size","0").toString();

        item->sec = item->name.left(1).toUpper();

        item->path = q->history.last();
        //item->path = path + "/" + item->name;
        //item->path = item->path.replace("//","/");

        item->link = misarchivos[i].value("shared","").toString();;
        item->fid = misarchivos[i].value("id","").toString();;

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
                     (suffix.endsWith(".tif")) || (suffix.endsWith(".bmp")) )
                {
                    QCryptographicHash md(QCryptographicHash::Md5);
                    QString file = "Box/" + item->fid;
                    file.replace("//","/");
                    md.addData(file.toUtf8());
                    QString tf = Config::getHome() + "/.thumbnails/filecase/"+ QString(md.result().toHex().constData()) + ".png";
                    if ( QFileInfo(tf).exists() ) {
                        item->exten = tf;
                    } else {
                        item->dexten = tf;
                        q->pepe->addImage(tf, misarchivos[i].value("id","").toString());
                        q->StartThread = 1;
                    }
                }

            }

        }

        //item->pub = misarchivos[i][8];

        if( misarchivos[i].value("type","").toString()=="file" )
            item->type = BoxClient::File;
        else
            item->type = BoxClient::Folder;

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


BoxClient::BoxClient(QObject * parent) :
        QAbstractListModel(parent) , d (new BoxClientPrivate(this))
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

    pepe = new BoxThumbnailer();
    connect (pepe, SIGNAL(imageLoaded(QString)), this, SLOT(imageLoaded(QString)));

    datos = new QNetworkAccessManager();

    d->path = "/";
    history.append("0");

    spaceUsed = 0;
    spaceTotal = 0;
    accountUser = "";
}

QHash<int, QByteArray> BoxClient::roleNames() const {
    return m_roles;
}

void BoxClient::setRoleNames(const QHash<int, QByteArray>& roles) {
    m_roles = roles;
}

BoxClient::~BoxClient()
{
    delete d;
}

int BoxClient::rowCount(const QModelIndex & parent) const
{
    Q_UNUSED(parent)
    return d->items.size();
}

QVariant BoxClient::data(const QModelIndex & index , int role) const
{
    if(index.row() < 0 or index.row() > count())
    {
        return QVariant();
    }

    BoxClientItem * item = d->items.at(index.row());
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

int BoxClient::count() const
{
        return rowCount();
}

QString BoxClient::path() const
{
    QString tmp = d->path;
    if ( tmp.startsWith("//") )
            tmp.remove(0,1);
    if (tmp=="") tmp = "/";
    d->path = tmp;
    return d->path;
}

QString BoxClient::path1() const
{
    if (d->path=="/")
        return d->path;
    else
        return QFileInfo(d->path).fileName();
}

QString BoxClient::path2() const
{
    if (d->path=="/")
        return "";
    else
        return QFileInfo(d->path).path();
}

QString BoxClient::cname(int index)
{
    QString result;
    if ( d->items.at(index)->type==BoxClient::Folder )
        result = "a";
    else
        result = "b";

    result += d->items.at(index)->sec;

    return result;
}


void BoxClient::reload()
{
    d->q->requestMetadata(history.last());
}

void BoxClient::setPath(QString path)
{
    path = path.replace("//","/");
    d->path = path;
}

void BoxClient::setPath2(QString path, QString link)
{
    QSettings settings("cepiperez","fileboxplus");
    accessToken = settings.value("Box/access_token").toString();
    refreshToken = settings.value("Box/refresh_token").toString();
    path = path.replace("//","/");
    d->path = path;
    history.append(link);
    d->q->requestMetadata(history.last());
}

void BoxClient::goUp()
{
    QString tmp = d->path;
    int i = tmp.lastIndexOf("/");
    tmp.remove(i, tmp.length()-i);
    if (tmp=="") tmp = "/";
    d->path = tmp;
    if (history.count()>1)
        history.removeLast();
    d->q->requestMetadata(history.last());
}

void BoxClient::logIn(QString path)
{
    d->q->path1() = "/";
    d->q->path2() = "";
    emit d->q->pathChanged();

    clientId = "5rm92vy1rv9sdhdh3tslbq3uofu5e1jt";
    clientSecret = "8rEvxZOaQB3Mn2OPrFb7iqKv6q8q4qeA";
    redirectURI = "https://app.box.com/services/filecase";

    QSettings settings("cepiperez","fileboxplus");
    if (settings.value("Box/access_token","").toString()!="")
    {
        qDebug() << "ALREADY LOGED IN";
        accessToken = settings.value("Box/access_token").toString();
        refreshToken = settings.value("Box/refresh_token").toString();
        //accessId = settings.value("Box/access_id").toString();
        //userId = settings.value("Box/user_id").toString();
        emit d->q->loggedIn(false);
        history.clear();
        history << path;
        requestMetadata(path);
        return;
    }


    qDebug() << "NOT LOGED IN";

    action = "request-token";
    emit d->q->requestStarted();
    QString purl = "https://app.box.com/api/oauth2/authorize?response_type=code&client_id=" +
            clientId + "&redirect_uri=" + redirectURI + "&state=security_token";

    /*QNetworkRequest request;
    request.setUrl(purl);
    disconnect(datos,0,0,0);
    connect(datos, SIGNAL(finished(QNetworkReply*)),this, SLOT(handleNetworkReply(QNetworkReply*)));
    datos->get(request);
    return;*/

    d->q->openUrl(purl);

}

void BoxClient::handleNetworkReply(QNetworkReply *networkReply)
{
    QString reply = QString::fromUtf8(networkReply->readAll());
    qDebug() << reply;

    if (action == "request-ticket")
    {
        if (!reply.contains("get_ticket_ok"))
        {
            logIn(d->path);
        }
        else
        {
            QString tmp = reply;
            int i = tmp.indexOf("<ticket>");
            tmp = tmp.remove(0, i+8);
            i = tmp.indexOf("</ticket>");
            tmp = tmp.remove(i, tmp.length()-i);
            ticket = tmp;

            d->q->openUrl("https://m.box.com/api/1.0/auth/"+ticket);
            emit d->q->requestDone(false);
        }
    }
    if (action.startsWith("request-token"))
    {
        if (!reply.contains("access_token") && !reply.contains("refresh_token"))
        {
            logIn(d->path);
        }
        else
        {
            QString tmp = reply;
            int i = tmp.indexOf("access_token\":");
            tmp = tmp.remove(0, i+15);
            i = tmp.indexOf("\"");
            accessToken = tmp.left(i);

            tmp = reply;
            i = tmp.indexOf("refresh_token\":");
            tmp = tmp.remove(0, i+16);
            i = tmp.indexOf("\"");
            refreshToken = tmp.left(i);

            //JSONParser jParser;
            //accessToken = jParser.getParam(reply, "access_token");
            //refreshToken = jParser.getParam(reply, "refresh_token");

            QSettings settings("cepiperez","fileboxplus");
            settings.setValue("Box/access_token", accessToken);
            settings.setValue("Box/refresh_token", refreshToken);
            settings.sync();

            //emit d->q->requestDone(true);
            //emit d->q->spaceChanged(GetFileSize(spaceUsed), GetFileSize(spaceTotal), spaceUsed/1024/1024, spaceTotal/1024/1024);
            //emit d->q->userChanged(accountUser);

            if (action == "request-token-new") {
                requestMetadata(history.last());
                emit d->q->loggedIn(true);
            } else {
                emit d->q->loggedIn(true);
            }

        }

    }

}

void BoxClient::parseData(QString dir, QString data)
{
    Q_UNUSED(data)

    qDebug() << dir;
    //qDebug() << data;

    if (dir.startsWith("https://app.box.com/services/filecase?state=security_token&code="))
    {
        action = "request-token";
        QString code = dir;
        int i = dir.indexOf("?code=");
        accessCode = code.remove(0, i+6);

        QString url = "https://app.box.com/api/oauth2/token";

        QByteArray params = "grant_type=authorization_code";
        params += "&code=";
        params += accessCode;
        params += "&client_id=";
        params += clientId;
        params += "&client_secret=";
        params += clientSecret;

        emit closeUrl(false);

        QNetworkRequest request;
        request.setUrl(QUrl(url));

        disconnect(datos,0,0,0);
        connect(datos, SIGNAL(finished(QNetworkReply*)),this, SLOT(handleNetworkReply(QNetworkReply*)));
        datos->post(request, params);
        //datos->get(request);
        return;

    }
    /*else if (dir=="https://www.Sky.com/m")
    {
        d->q->closeUrl(false);
    }*/

}

void BoxClient::clearList()
{
    d->archivos.clear();
    emit d->q->beginResetModel();
    d->items.clear();
    emit d->q->endResetModel();
}

void BoxClient::requestMetadata(QString path)
{
    clearList();
    emit d->q->requestStarted();

    QUrl url;
    url = "https://api.box.com/2.0/folders/" + path + "/items?fields=name,size,id,type,created_at,modified_at,shared_link";


    QNetworkRequest request(url);
    qDebug() << "OPENING " << url;
    request.setRawHeader("Authorization", "Bearer "+accessToken.toLatin1());

    disconnect(datos,0,0,0);
    connect(datos, SIGNAL(finished(QNetworkReply*)),this, SLOT(handleMetadata(QNetworkReply*)));
    datos->get(request);

}

void BoxClient::handleMetadata(QNetworkReply *networkReply)
{
    QString dirJson = networkReply->readAll();
    qDebug() << dirJson;

    if (dirJson.isEmpty()) {

        action = "request-token-new";
        QString url = "https://app.box.com/api/oauth2/token";
        QByteArray params = "grant_type=refresh_token";
        params += "&refresh_token=";
        params += refreshToken;
        params += "&client_id=";
        params += clientId;
        params += "&client_secret=";
        params += clientSecret;

        QNetworkRequest request;
        request.setUrl(QUrl(url));

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

    QSettings settings("cepiperez","fileboxplus");
    QString dateFormat = settings.value("DateFormat","mm/dd/yyyy").toString().replace("mm","MM");

    // add folders
    //QVariantMap res = jsonResult["item_collection"].toMap();
    //qDebug() << res;

    foreach(const QVariant &itemJson, jsonResult["entries"].toList())
    {
        QVariantMap itemMap = itemJson.toMap();

        QVariantMap item;
        if(itemMap["type"].toString()=="folder")
            item.insert("type", "folder");
        else
            item.insert("type", "file");

        //QVariantMap tmp = itemMap["from"].toMap();
        //accountUser = tmp["name"].toString();

        parentID = history.last();

        QString f = itemMap["name"].toString();
        item.insert("name", f);

        QString predate = itemMap["created_at"].toString();
        predate = predate.left(10);
        QDateTime modified = QDateTime::fromString(predate,"yyyy-MM-dd");
        item.insert("created", modified.toString(dateFormat));

        predate = itemMap["modified_at"].toString();
        predate = predate.left(10);
        modified = QDateTime::fromString(predate,"yyyy-MM-dd");
        item.insert("modified", modified.toString(dateFormat));

        item.insert("size", itemMap["size"].toString());

        if(itemMap["type"].toString()=="folder")
            item.insert("sec", "a_"+f.toLower());
        else
            item.insert("sec", "b_"+f.toLower());

        //item.append(itemMap["upload_location"].toString());
        item.insert("id", itemMap["id"].toString());


        QString thumb = "";
        /*foreach(const QVariant &itemJson2, itemMap["images"].toList())
        {
            QVariantMap itemMap2 = itemJson2.toMap();
            if (itemMap2["type"].toString()=="album")
                thumb = itemMap2["source"].toString();
        }*/

        item.insert("thumb", thumb);

        QVariantMap tmp = itemMap["shared_link"].toMap();
        item.insert("shared", tmp.value("url","").toString());

        d->archivos.append(item);

    }

    qSort(d->archivos.begin(), d->archivos.end(), sortVariantMap);

    qDebug() << "Populate items";
    d->populateItems();

    qDebug() << "Done";

    emit d->q->requestDone(true);
}

void BoxClient::getQuota()
{
    qDebug() << "Getting quota";
    QUrl url("https://api.box.com/2.0/users/me");
    QNetworkRequest request(url);
    //QString auth = "BoxAuth api_key=" + clientId + "&auth_token=" + accessToken;
    request.setRawHeader("Authorization", "Bearer "+accessToken.toLatin1());
    disconnect(datos,0,0,0);
    connect(datos, SIGNAL(finished(QNetworkReply*)),this, SLOT(handleInfo(QNetworkReply*)));
    datos->get(request);
}


void BoxClient::handleInfo(QNetworkReply *networkReply)
{
    QString dirJson = networkReply->readAll();
    qDebug() << dirJson;

    bool ok;
    QVariantMap jsonResult = QtJson::Json::parse(dirJson, ok).toMap();
    if (!ok) return;

    spaceUsed = jsonResult["space_used"].toDouble();
    spaceTotal = jsonResult["space_amount"].toDouble();
    accountUser = jsonResult["name"].toString();

    qDebug() << "SPACE: " << spaceUsed << " OF " << spaceTotal;

    QVariantMap userdata;

    userdata.insert("user", accountUser);
    userdata.insert("email", jsonResult["login"].toString());
    userdata.insert("picture", jsonResult["avatar_url"].toString());
    userdata.insert("spaceUsed", jsonResult["space_used"].toDouble());
    userdata.insert("spaceTotal", jsonResult["space_amount"].toDouble());
    userdata.insert("spaceUsedText", GetFileSize(jsonResult["space_used"].toDouble()));
    userdata.insert("spaceTotalText", GetFileSize(jsonResult["space_amount"].toDouble()));

    emit d->q->userChanged(userdata);
    emit d->q->requestDone(true);
}

void BoxClient::newFolder(QString name)
{
    action = "newfolder";

    d->archivos.clear();
    emit d->q->requestStarted();

    QUrl url("https://api.box.com/2.0/folders");
    QNetworkRequest request(url);
    //QString auth = "BoxAuth api_key=" + clientId + "&auth_token=" + accessToken;
    request.setRawHeader("Authorization", "Bearer "+accessToken.toLatin1());

    QString params = "{ \"name\":\"" + name.trimmed() + "\", \"parent\": ";
    params += "{ \"id\":\"" + history.last() + "\"}}";

    qDebug() << params;
    disconnect(datos,0,0,0);
    connect(datos, SIGNAL(finished(QNetworkReply*)),this, SLOT(handleFileOps(QNetworkReply*)));
    datos->post(request,params.toLatin1());
}

void BoxClient::handleFileOps(QNetworkReply *networkReply)
{
    QString dirJson = networkReply->readAll();
    qDebug() << dirJson;

    if (action.startsWith("rename")) {
        action = action.remove(0,6);
        emit fileRenamed(action);
    }

    d->q->requestMetadata(history.last());
}

void BoxClient::renameFile(QString source, QString newname)
{
    d->archivos.clear();
    emit d->q->requestStarted();

    action = "rename" + newname;

    QString filetype;

    for (int i=0; i<d->items.count(); ++i)
    {
        if (d->items.at(i)->fid==source)
        {
            qDebug() << "founded " << d->items.at(i)->fid << d->items.at(i)->type;
            if (d->items.at(i)->type==BoxClient::File)
                filetype = "files";
            else
                filetype = "folders";
            break;
        }
    }

    //qDebug() << "RENAMING " << source << " TO " << newname;

    QUrl url = "https://api.box.com/2.0/" + filetype + "/" + source;
    QNetworkRequest request(url);
    //QString auth = "BoxAuth api_key=" + clientId + "&auth_token=" + accessToken;
    request.setRawHeader("Authorization", "Bearer "+accessToken.toLatin1());

    QString params = "{\"name\":\"" + newname + "\"}";

    qDebug() << url;
    disconnect(datos,0,0,0);
    connect(datos, SIGNAL(finished(QNetworkReply*)),this, SLOT(handleFileOps(QNetworkReply*)));
    datos->put(request,params.toLatin1());
}

void BoxClient::moveFile(QString source, QString filetype)
{
    action = "move";

    d->archivos.clear();
    emit d->q->requestStarted();

    QUrl url = "https://api.box.com/2.0/" + filetype + "/" + source;
    QNetworkRequest request(url);
    //QString auth = "BoxAuth api_key=" + clientId + "&auth_token=" + accessToken;
    request.setRawHeader("Authorization", "Bearer "+accessToken.toLatin1());

    QString params = "{ \"parent\": { \"id\":\"" + history.last() + "\"} }";

    qDebug() << "MOVING " << source << " TO " << history.last();

    qDebug() << url << params;
    disconnect(datos,0,0,0);
    connect(datos, SIGNAL(finished(QNetworkReply*)),this, SLOT(handleFileOps(QNetworkReply*)));
    datos->put(request,params.toLatin1());

}

void BoxClient::removeFile(QString path, QString filetype)
{
    action = "remove";

    d->archivos.clear();
    emit d->q->requestStarted();

    //qDebug() << "REMOVING " << path;

    QUrl url;
    if (filetype=="folder")
        url = "https://api.box.com/2.0/folders/" + path + "?recursive=true";
    else
        url = "https://api.box.com/2.0/files/" + path;
    QNetworkRequest request(url);
    //QString auth = "BoxAuth api_key=" + clientId + "&auth_token=" + accessToken;
    request.setRawHeader("Authorization", "Bearer "+accessToken.toLatin1());

    disconnect(datos,0,0,0);
    connect(datos, SIGNAL(finished(QNetworkReply*)),this, SLOT(handleFileOps(QNetworkReply*)));
    datos->deleteResource(request);

}

void BoxClient::copyFile(QString source, QString filetype)
{
    action = "copy";

    d->archivos.clear();
    emit d->q->requestStarted();

    QUrl url = "https://api.box.com/2.0/" + filetype + "/" + source + "/copy";
    QNetworkRequest request(url);
    //QString auth = "BoxAuth api_key=" + clientId + "&auth_token=" + accessToken;
    request.setRawHeader("Authorization", "Bearer "+accessToken.toLatin1());

    QString params = "{\"parent\": { \"id\":\"" + history.last() + "\"}}";

    qDebug() << "COPING " << source << " TO " << history.last();

    qDebug() << url << params;
    disconnect(datos,0,0,0);
    connect(datos, SIGNAL(finished(QNetworkReply*)),this, SLOT(handleFileOps(QNetworkReply*)));
    datos->post(request,params.toLatin1());

}

void BoxClient::imageLoaded(QString filename)
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


void BoxClient::setFilter(QString filter)
{
    d->filter = filter.toLower();
    d->populateItems();

}

void BoxClient::shareFile(QString filename)
{
    qDebug() << "Sharing file: " << filename;

    bool found = false;
    for (int i=0; i<d->items.count(); ++i)
    {
        if (d->items.at(i)->fid==filename && d->items.at(i)->link!="")
        {
            emit d->q->linkShared(d->items.at(i)->link);
            found = true;
            break;
        }
    }

    if (found)
        return;

    emit d->q->requestStarted();

    QUrl url = "https://api.box.com/2.0/files/" + filename;
    QNetworkRequest request(url);
    //QString auth = "BoxAuth api_key=" + clientId + "&auth_token=" + accessToken;
    request.setRawHeader("Authorization", "Bearer "+accessToken.toLatin1());

    QString params = "{\"shared_link\": { \"access\":\"open\"}}";

    qDebug() << url << params;
    disconnect(datos,0,0,0);
    connect(datos, SIGNAL(finished(QNetworkReply*)),this, SLOT(handleShare(QNetworkReply*)));
    datos->put(request,params.toLatin1());



}

void BoxClient::handleShare(QNetworkReply *networkReply)
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

    QVariantMap tmp = jsonResult["shared_link"].toMap();

    QString shareLink = tmp.value("url","").toString();
    emit d->q->requestDone(true);
    emit d->q->linkShared(shareLink);
    //requestMetadata(history.last());

}

void BoxClient::publicLink(QString filename)
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

void BoxClient::removeAccount()
{
    QSettings settings("cepiperez","fileboxplus");
    settings.remove("Box/access_token");
    //settings.remove("Box/refresh_token");
    settings.sync();
    history.clear();
    history.append("0");
    emit d->q->requestDone(false);
    d->q->logIn("0");
}

void BoxClient::setUploadFolder(QString name)
{
    QSettings settings("cepiperez","fileboxplus");
    settings.setValue("Box/upload_folder", name + "||" + history.last());
    settings.sync();
}
