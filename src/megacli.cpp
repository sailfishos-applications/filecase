#include "megacli.h"

#include "sys/vfs.h"
#include "utils.h"

#include <QDesktopServices>
//#include <QDeclarativeEngine>
//#include <QDeclarativeView>
#include <QDir>
#include <QFileInfo>
#include <QDateTime>
#include <QDebug>
#include <QIcon>
#include <QStringList>
#include <QString>
#include <QCryptographicHash>
#include <QList>
#include <QUrl>
#include <QUrlQuery>
#include <QJsonValue>
#include <QJsonDocument>
#include <QJsonObject>
#include <QVariantMap>
#include <QJsonArray>

#include <QNetworkReply>


struct MegaCliItem
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
    QString dlink;
    QString shared;
    MegaCli::ItemType type;
};

class MegaCli::MegaCliPrivate
{
public:
    MegaCliPrivate(MegaCli * parent);
    ~MegaCliPrivate();
    void populateItems();
    QList <MegaCliItem * > items;
    QString path;
    MegaCli * const q;
    QList<QVariantMap> archivos;
    QString filter;
};

MegaCli::MegaCliPrivate::MegaCliPrivate(MegaCli * parent) :
        q(parent)
{
    filter = "";

}

MegaCli::MegaCliPrivate::~MegaCliPrivate()
{
    while(!items.isEmpty())
    {
        delete items.takeFirst();
    }
}

void MegaCli::MegaCliPrivate::populateItems()
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

    //qDebug() << archivos;

    QSettings settings("cepiperez", "fileboxplus");

    int entriesCount = misarchivos.count();

    emit q->beginResetModel();

    q->beginInsertRows(QModelIndex(), 0, entriesCount-1);


    for (int i=0; i<misarchivos.count(); ++i)
    {
        MegaCliItem * item = new MegaCliItem();

        //qDebug() << misarchivos[i];

        item->name = misarchivos[i].value("name","").toString();

        item->created = misarchivos[i].value("created","").toString();

        item->modified = misarchivos[i].value("modified","").toString();

        item->size = GetFileSize(misarchivos[i].value("size","0").toString().toDouble());

        item->realsize = misarchivos[i].value("size","0").toString();

        item->sec = item->name.left(1).toUpper();

        item->link = misarchivos[i].value("link","").toString();

        item->dlink = misarchivos[i].value("dlink","").toString();

        item->shared = misarchivos[i].value("shared","").toString();

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
                     (suffix.endsWith(".bmp")) || (suffix.endsWith(".gif")) ||
                     (suffix.endsWith(".avi")) || (suffix.endsWith(".mp4")) ||
                     (suffix.endsWith(".wmv")) || (suffix.endsWith(".3gp")) )
                {
                    QString ef = ".jpg";
                    if (suffix.endsWith(".png")) ef = ".png";

                    QCryptographicHash md(QCryptographicHash::Md5);
                    QString file = "Mega/" + item->dlink;
                    file.replace("//","/");
                    md.addData(file.toUtf8());
                    QString tf = "/home/nemo/.thumbnails/filecase/"+ QString(md.result().toHex().constData()) + ef;
                    if ( QFileInfo(tf).exists() ) {
                        item->exten = tf;
                    } else if ( misarchivos[i].value("thumb","").toString() != "" ) {
                        item->dexten = tf;
                        q->pepe->addImage(tf, misarchivos[i].value("thumb","").toString());
                        q->StartThread = 1;
                    }
                }

            }

        }

        item->path = path + "/" + item->name;
        item->path = item->path.replace("//","/");

        if(misarchivos[i].value("type","").toString()=="folder")
            item->type = MegaCli::Folder;
        else
            item->type = MegaCli::File;

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


MegaCli::MegaCli(QObject * parent) :
        QAbstractListModel(parent) , d (new MegaCliPrivate(this))
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
    roles.insert(DownloadRole , "dlink");
    roles.insert(SharedRole , "shared");
    d->q->setRoleNames(roles);

    m_working = false;

    pepe = new MegaThumbnailer();
    connect (pepe, SIGNAL(imageLoaded(QString)), this, SLOT(imageLoaded(QString)));

    datos = new QNetworkAccessManager();

    d->path = "/";

    history.append("root");

    spaceUsed = 0;
    spaceTotal = 0;
    accountUser.clear();

}

QHash<int, QByteArray> MegaCli::roleNames() const {
    return m_roles;
}

void MegaCli::setRoleNames(const QHash<int, QByteArray>& roles) {
    m_roles = roles;
}

MegaCli::~MegaCli()
{
    delete d;
}

int MegaCli::rowCount(const QModelIndex & parent) const
{
    Q_UNUSED(parent)
    return d->items.size();
}

QVariant MegaCli::data(const QModelIndex & index , int role) const
{
    if(index.row() < 0 or index.row() > count())
    {
        return QVariant();
    }

    MegaCliItem * item = d->items.at(index.row());
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
    case DownloadRole:
        return item->dlink;
        break;
    case SharedRole:
        return item->shared;
        break;
    default:
        return QVariant();
        break;
    }
}

int MegaCli::count() const
{
        return rowCount();
}

QString MegaCli::path() const
{
    return d->path;
}

QString MegaCli::path1() const
{
    if (d->path=="/")
        return d->path;
    else
        return QFileInfo(d->path).fileName();
}

QString MegaCli::path2() const
{
    if (d->path=="/")
        return "";
    else
        return QFileInfo(d->path).path();
}

QString MegaCli::cname(int index)
{
    QString result;
    if ( d->items.at(index)->type==MegaCli::Folder )
        result = "a";
    else
        result = "b";

    result += d->items.at(index)->sec;

    return result;
}


void MegaCli::reload()
{
    d->q->requestMetadata(history.last());
}

void MegaCli::setPath(QString path)
{
    path = path.replace("//","/");
    d->path = path;
}

void MegaCli::setPath2(QString path, QString link)
{
    QSettings settings("cepiperez","fileboxplus");
    accessToken = settings.value("Mega/access_token").toString();
    refreshToken = settings.value("Mega/refresh_token").toString();
    if(d->path != path)
    {
        path = path.replace("//","/");
        d->path = path;
        history.append(link);
        d->q->requestMetadata(history.last());
    }
}

void MegaCli::goUp()
{
    QString tmp = d->path;
    int i = tmp.lastIndexOf("/");
    tmp.remove(i, tmp.length()-i);
    if (tmp=="") tmp = "/";
    d->path = tmp;
    history.removeLast();
    d->q->requestMetadata(history.last());
}

void MegaCli::logIn(QString username, QString password)
{
    /*d->q->path1() = "/";
    d->q->path2() = "";
    emit d->q->pathChanged();

    scope = "https://docs.google.com/feeds/%20https://spreadsheets.google.com/feeds/%20https://docs.googleusercontent.com/%20https://www.googleapis.com/auth/Mega%20https://www.googleapis.com/auth/Mega.file";
    clientID = "823981073892.apps.googleusercontent.com";
    redirectURI = "urn:ietf:wg:oauth:2.0:oob";
    endPoint = "https://accounts.google.com/o/oauth2/auth";
    clientSecret = "gzzbIF6rt-48xfDu7NohYKGV";

    QSettings settings("cepiperez","fileboxplus");
    if( (settings.value("Mega/access_token","").toString()!="") ||
        (settings.value("Mega/refresh_token","").toString()!="") )
    {
        accessToken = settings.value("Mega/access_token").toString();
        refreshToken = settings.value("Mega/refresh_token").toString();
        emit d->q->loggedIn(false);
        history.clear();
        history << path;
        requestMetadata(path);
        return;
    }


    action = "request-token";

    QString str = QString("%1?client_id=%2&redirect_uri=%3&response_type=code&scope=%4&approval_prompt=force&access_type=offline").
            arg(endPoint).arg(clientID).arg(redirectURI).arg(scope);

    d->q->openUrl(str);*/




    client->login(username.toLatin1(), QByteArray(password));


}

void MegaCli::handleNetworkReply(QNetworkReply *networkReply)
{

    QSettings settings("cepiperez","fileboxplus");
    QString replyStr = networkReply->readAll();
    qDebug() << replyStr;

    JSONParser jParser;

    accessToken = jParser.getParam(replyStr, "access_token");
    qDebug() << "ACCESS TOKEN: " << accessToken;

    settings.setValue("Mega/access_token", accessToken);

    QString newRefreshToken = jParser.getParam(replyStr, "refresh_token");
    if(!newRefreshToken.isEmpty())
    {
        refreshToken = newRefreshToken;
        qDebug() << "REFRESH TOKEN: " << refreshToken;
        settings.setValue("Mega/refresh_token", refreshToken);
    }

    if (!accessToken.isEmpty() && !refreshToken.isEmpty())
    {
        emit d->q->loggedIn(true);

        if (action=="request-token-new")
            requestMetadata(history.last());

    }

}

void MegaCli::parseData(QString dir, QString data)
{
    qDebug() << data;
    qDebug() << dir;
    //d->q->closeUrl();

    if (dir.startsWith("https://accounts.google.com/o/oauth2/approval") &&
        data.indexOf("<input id=\"code\" type=\"text\" readonly=\"readonly\" value=\""))
    {

        qDebug() << "PARSING CODE DATA";
        int x = data.indexOf("<input id=\"code\" ");
        data = data.remove(0,x);
        x = data.indexOf("value=\"");
        data = data.remove(0,x+7);
        x = data.indexOf("\"");
        data = data.remove(x,data.length()-x);
        codeStr = data;

        QByteArray params = "client_id=" + QByteArray(clientID.toLatin1());
        params += "&redirect_uri=";
        params += QByteArray(redirectURI.toLatin1());
        params += "&client_secret=";
        params += QByteArray(clientSecret.toLatin1());
        params += "&grant_type=authorization_code";
        params += "&code=" + codeStr.toLatin1();

        QUrl url("https://accounts.google.com/o/oauth2/token");
        QNetworkRequest request;
        request.setUrl(url);
        request.setRawHeader("Content-Type", "application/x-www-form-urlencoded");

        action = "request-token";

        disconnect(datos,0,0,0);
        connect(datos, SIGNAL(finished(QNetworkReply*)),this, SLOT(handleNetworkReply(QNetworkReply*)));
        datos->post(request, params);

        d->q->closeUrl(false);

    }
    else if (dir=="https://www.dropbox.com/m")
    {
        d->q->closeUrl(false);
    }

}

void MegaCli::clearList()
{
    d->archivos.clear();
    emit d->q->beginResetModel();
    d->items.clear();
    emit d->q->endResetModel();
}

void MegaCli::requestMetadata(QString path)
{
    clearList();
    emit d->q->requestStarted();

    path.remove("\"");

    QUrl url = "https://www.googleapis.com/Mega/v2/files?q='" + path + "'+in+parents+and+trashed=false";
    qDebug() << "OPEN FOLDER: " << url;
    QNetworkRequest networkRequest(url);
    networkRequest.setRawHeader("User-Agent", "FileCase");
    networkRequest.setRawHeader("GData-Version", "3.0");
    networkRequest.setRawHeader("Authorization",(QString("OAuth %1").arg(accessToken)).toLatin1());

    disconnect(datos,0,0,0);
    connect(datos, SIGNAL(finished(QNetworkReply*)),this, SLOT(handleMetadata(QNetworkReply*)));
    datos->get(networkRequest);
}

void MegaCli::handleMetadata(QNetworkReply *networkReply)
{
    QString dirJson = networkReply->readAll();
    //qDebug() << dirJson;

    QSettings settings("cepiperez","fileboxplus");
    QString dateFormat = settings.value("DateFormat","mm/dd/yyyy").toString().replace("mm","MM");


    if (dirJson.contains("Invalid Credentials"))
    {
        QUrl url("https://accounts.google.com/o/oauth2/token");
        QNetworkRequest request;
        request.setUrl(url);
        request.setRawHeader("Content-Type", "application/x-www-form-urlencoded");

        QByteArray params = "client_id=" + QByteArray(clientID.toLatin1());
        params += "&client_secret=";
        params += QByteArray(clientSecret.toLatin1());
        params += "&grant_type=refresh_token";
        params += "&refresh_token=" + refreshToken.toLatin1();

        action = "request-token-new";

        disconnect(datos,0,0,0);
        connect(datos, SIGNAL(finished(QNetworkReply*)),this, SLOT(handleNetworkReply(QNetworkReply*)));
        datos->post(request, params);
        return;
    }
    else
    {
        bool ok;
        QVariantMap jsonResult = QtJson::Json::parse(dirJson, ok).toMap();
        if(!ok)
        {
            qDebug() << "METADATA ERROR!";
            clearList();
            emit d->q->requestDone(false);
            return;
        }

        QJsonDocument jsonResponse = QJsonDocument::fromJson(dirJson.toUtf8());

        foreach(const QVariant &itemJson, jsonResponse.object().toVariantMap()["items"].toList())
        {
            QVariantMap itemMap = itemJson.toMap();

            //qDebug() << " ---------------------- ITEM ::: " << itemMap;

            QVariantMap item;

            QString filetype = itemMap["mimeType"].toString().split(".").last();

            if (filetype.contains("folder"))
                item.insert("type", "folder");
            else
                item.insert("type", "file");

            //QVariantMap tmp = itemMap["from"].toMap();
            //accountUser = tmp["name"].toString();

            //parentID = itemMap["parent_id"].toString();

            QString f = itemMap["title"].toString();
            item.insert("name", f);

            QString predate = itemMap["createdDate"].toString();
            predate = predate.left(10);
            QDateTime realdate = QDateTime::fromString(predate,"yyyy-MM-dd");
            item.insert("created", realdate.toString(dateFormat));


            QString premodified = itemMap["modifiedDate"].toString();
            premodified = premodified.left(10);
            QDateTime realmodified = QDateTime::fromString(premodified,"yyyy-MM-dd");
            item.insert("modified", realmodified.toString(dateFormat));

            if (filetype=="folder")
                item.insert("size", "0");
            else
                item.insert("size", itemMap["fileSize"].toString());

            if (filetype=="folder")
                item.insert("sec", "a_"+f.toLower());
            else
                item.insert("sec", "b_"+f.toLower());

            item.insert("dlink", itemMap["downloadUrl"].toString());
            item.insert("link", itemMap["id"].toString());

            if (filetype.contains("image/") || filetype.contains("video/"))
                item.insert("thumb", itemMap["thumbnailLink"].toString());



            //.append(thumb);
            //item.append(pub);

            d->archivos.append(item);
        }
        qSort(d->archivos.begin(), d->archivos.end(), sortVariantMap);

        d->populateItems();

        emit d->q->requestDone(true);
    }


}

void MegaCli::getQuota()
{
    QUrl url("https://www.googleapis.com/Mega/v2/about");
    QNetworkRequest networkRequest(url);
    networkRequest.setRawHeader("Content-Type", "application/json");
    networkRequest.setRawHeader("Authorization", "Bearer "+accessToken.toLatin1());

    disconnect(datos,0,0,0);
    connect(datos, SIGNAL(finished(QNetworkReply*)),this, SLOT(handleInfo(QNetworkReply*)));
    datos->get(networkRequest);
}

void MegaCli::handleInfo(QNetworkReply *networkReply)
{
    QString dirJson = networkReply->readAll();

    QJsonDocument jsonResponse = QJsonDocument::fromJson(dirJson.toUtf8());

    //qDebug() << jsonResponse.object().toVariantMap();

    bool ok;
    QVariantMap jsonResult = QtJson::Json::parse(dirJson, ok).toMap();
    if (!ok) return;

    accountUser.insert("user", jsonResult["user"].toMap()["displayName"].toString());
    accountUser.insert("email", jsonResult["user"].toMap()["emailAddress"].toString());
    accountUser.insert("picture", jsonResult["user"].toMap()["picture"].toMap()["url"].toString());
    accountUser.insert("spaceUsed", jsonResult["quotaBytesUsed"].toDouble() + jsonResult["quotaBytesUsedInTrash"].toDouble());
    accountUser.insert("spaceTotal", jsonResult["quotaBytesTotal"].toDouble());
    accountUser.insert("spaceUsedText", GetFileSize(jsonResult["quotaBytesUsed"].toDouble() + jsonResult["quotaBytesUsedInTrash"].toDouble()));
    accountUser.insert("spaceTotalText", GetFileSize(jsonResult["quotaBytesTotal"].toDouble()));

    emit d->q->userChanged(accountUser);
    emit d->q->requestDone(true);

}

void MegaCli::handleFileOps(QNetworkReply *networkReply)
{
    QString dirJson = networkReply->readAll();
    qDebug() << dirJson;

    if (action=="rename") {
        QJsonDocument jsonResponse = QJsonDocument::fromJson(dirJson.toUtf8());
        emit fileRenamed(jsonResponse.object().toVariantMap().value("title","").toString());
    }

    d->q->requestMetadata(history.last());
}

void MegaCli::newFolder(QString name)
{
    d->archivos.clear();
    emit d->q->requestStarted();

    action = "newfolder";

    qDebug() << "CREATING FOLDER: " << name;

    QUrl url("https://www.googleapis.com/Mega/v2/files");
    QNetworkRequest networkRequest(url);
    networkRequest.setRawHeader("Content-Type", "application/json");
    networkRequest.setRawHeader("Authorization", "Bearer "+accessToken.toLatin1());
    //networkRequest.setRawHeader("GData-Version", "3.0");
    //networkRequest.setRawHeader("Authorization",(QString("OAuth %1").arg(accessToken)).toLatin1());

    QString params = "{\n";
    params += " \"title\": \"" + name + "\",\n";
    params += " \"parents\" : [{\"id\": \"" + history.last() + "\"}],\n";
    params += " \"mimeType\": \"application/vnd.google-apps.folder\"\n";
    params += "}";

    //qDebug() << params;
    disconnect(datos,0,0,0);
    connect(datos, SIGNAL(finished(QNetworkReply*)),this, SLOT(handleFileOps(QNetworkReply*)));
    datos->post(networkRequest,params.toLatin1());

}

void MegaCli::renameFile(QString source, QString dest)
{
    d->archivos.clear();
    emit d->q->requestStarted();

    action = "rename";

    qDebug() << "RENAMING: " << source << " TO " << dest;

    QUrl url = "https://www.googleapis.com/Mega/v2/files/" + source;
    QNetworkRequest networkRequest(url);
    networkRequest.setRawHeader("Content-Type", "application/json");
    networkRequest.setRawHeader("Authorization", "Bearer "+accessToken.toLatin1());

    QString params = "{\n \"title\": \"" + dest + "\"\n}";
    //qDebug() << params;
    disconnect(datos,0,0,0);
    connect(datos, SIGNAL(finished(QNetworkReply*)),this, SLOT(handleFileOps(QNetworkReply*)));
    datos->put(networkRequest,params.toLatin1());
}

void MegaCli::moveFile(QString source)
{
    d->archivos.clear();
    emit d->q->requestStarted();

    action = "move";

    QUrl url = "https://www.googleapis.com/Mega/v2/files/" + source;
    QNetworkRequest networkRequest(url);
    networkRequest.setRawHeader("Content-Type", "application/json");
    networkRequest.setRawHeader("Authorization", "Bearer "+accessToken.toLatin1());

    QString params = "{\n";
    params += " \"parents\" : [{\"id\": \"" + history.last() + "\"}]\n";
    params += "}";

    qDebug() << params;
    disconnect(datos,0,0,0);
    connect(datos, SIGNAL(finished(QNetworkReply*)),this, SLOT(handleFileOps(QNetworkReply*)));
    datos->put(networkRequest,params.toLatin1());
}

void MegaCli::removeFile(QString path)
{
    d->archivos.clear();
    emit d->q->requestStarted();

    action = "remove";

    QUrl url = "https://www.googleapis.com/Mega/v2/files/" + path;
    QNetworkRequest networkRequest(url);
    networkRequest.setRawHeader("User-Agent", "Filebox");
    networkRequest.setRawHeader("GData-Version", "3.0");
    networkRequest.setRawHeader("Authorization",(QString("OAuth %1").arg(accessToken)).toLatin1());

    disconnect(datos,0,0,0);
    connect(datos, SIGNAL(finished(QNetworkReply*)),this, SLOT(handleFileOps(QNetworkReply*)));
    datos->deleteResource(networkRequest);
}

void MegaCli::copyFile(QString source)
{
    d->archivos.clear();
    emit d->q->requestStarted();

    QUrl url = "https://www.googleapis.com/Mega/v2/files/" + source + "/copy";
    QNetworkRequest networkRequest(url);
    networkRequest.setRawHeader("Content-Type", "application/json");
    networkRequest.setRawHeader("Authorization", "Bearer "+accessToken.toLatin1());

    QString params = "{\n";
    params += " \"parents\" : [{\"id\": \"" + history.last() + "\"}]\n";
    params += "}";

    qDebug() << url << params;
    disconnect(datos,0,0,0);
    connect(datos, SIGNAL(finished(QNetworkReply*)),this, SLOT(handleFileOps(QNetworkReply*)));
    datos->post(networkRequest,params.toLatin1());
}

void MegaCli::imageLoaded(QString filename)
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

QString MegaCli::getPreview(QString link, QString filename, QString mode)
{
    QString preview = "";
    if (mode=="download")
    {
        QCryptographicHash md(QCryptographicHash::Md5);
        QString file = "Mega/"+link;
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

void MegaCli::setFilter(QString filter)
{
    d->filter = filter.toLower();
    d->populateItems();

}

void MegaCli::setUploadFolder(QString name)
{
    QSettings settings("cepiperez","fileboxplus");
    settings.setValue("Mega/upload_folder", name + "||" + history.last());
    settings.sync();
}

void MegaCli::shareFile(QString filename)
{
    filename = filename.replace("//","/");
    emit d->q->requestStarted();
    QUrl url = "https://www.googleapis.com/Mega/v2/files/" + filename + "/permissions";
    QUrlQuery query;
    query.addQueryItem("sendNotificationEmails","false");
    url.setQuery(query);
    qDebug() << "SHARING " << url;
    QNetworkRequest networkRequest(url);
    networkRequest.setRawHeader("Content-Type", "application/json");
    networkRequest.setRawHeader("Authorization", "Bearer "+accessToken.toLatin1());

    QString params = "{\n";
    params += " \"role\" : \"reader\", \n";
    params += " \"type\" : \"anyone\" \n";
    params += "}";

    qDebug() << params;
    disconnect(datos,0,0,0);
    connect(datos, SIGNAL(finished(QNetworkReply*)),this, SLOT(handleShare(QNetworkReply*)));
    datos->post(networkRequest,params.toLatin1());

}

void MegaCli::handleShare(QNetworkReply *networkReply)
{
    QString dirJson = networkReply->readAll();
    //qDebug() << dirJson;

    if (dirJson.contains("\"error\":"))
    {
        emit d->q->requestDone(true);
        return;
    }

    QString shareLink = "ok";
    emit d->q->requestDone(true);
    emit d->q->linkShared(shareLink);

}

void MegaCli::unshareFile(QString filename)
{
    filename = filename.replace("//","/");
    emit d->q->requestStarted();
    QUrl url = "https://www.googleapis.com/Mega/v2/files/" + filename + "/permissions";
    QUrlQuery query;
    query.addQueryItem("sendNotificationEmails","false");
    url.setQuery(query);
    qDebug() << "SHARING " << url;
    QNetworkRequest networkRequest(url);
    networkRequest.setRawHeader("Content-Type", "application/json");
    networkRequest.setRawHeader("Authorization", "Bearer "+accessToken.toLatin1());

    QString params = "{\n";
    params += " \"role\" : \"reader\", \n";
    params += " \"type\" : \"user\", \n";
    params += " \"value\" : \"\" \n";
    params += "}";

    qDebug() << params;
    disconnect(datos,0,0,0);
    connect(datos, SIGNAL(finished(QNetworkReply*)),this, SLOT(handleUnshare(QNetworkReply*)));
    datos->put(networkRequest,params.toLatin1());

}

void MegaCli::handleUnshare(QNetworkReply *networkReply)
{
    QString dirJson = networkReply->readAll();
    qDebug() << dirJson;

    if (dirJson.contains("\"error\":"))
    {
        emit d->q->requestDone(true);
        return;
    }
    reload();

}

void MegaCli::removeAccount()
{
    QSettings settings("cepiperez","fileboxplus");
    settings.remove("Mega/access_token");
    settings.remove("Mega/refresh_token");
    settings.sync();
    d->q->logIn("root");
}
