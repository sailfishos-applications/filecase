#include "transfers.h"
#include "config.h"

#include "utils.h"

#include <QCryptographicHash>
#include <QUrl>
#include <qstring.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <QImage>
#include <QSettings>
#include <QDebug>

#include <QNetworkReply>
#include "drive/jsonparser.h"
#include "dropbox/common.h"
#include "dropbox/dropoauth.h"
#include "dropbox/userdata.h"
#include "dropbox/dropbox.h"

Transfers::Transfers()
{
    datos = new QNetworkAccessManager();
    working = false;
    emit workingChanged(working);
}

Transfers::~Transfers()
{

}

void Transfers::addFile(QString service, QString name, QString link, QString mode, QString realsize)
{
    qDebug() << "ADDING FILE: " << service << " - Mode: " << mode << " - Link:" << link;

    QStringList transfer;
    transfer << service << mode << name << link << realsize;

    media.append(transfer);

    //if (media.count()==1 && start)
    //    startTranfers();

}

void Transfers::removeFile(QString filename)
{
    qDebug() << "REMOVING TRANSFER FILE: " << filename;

    for (int i=0; i<media.count(); ++i)
    {
        qDebug() << "CHECKING MEDIA: " << media.at(i);
        if (media.at(i).at(3)==filename)
        {
            if (i==0 && working) {
                qDebug() << "STOPPING TRANSFERS";
                stopTransfers();
            }

            qDebug() << "REMOVING MEDIA: " << media.at(i);
            media.removeAt(i);
            qDebug() << "REMOVED FROM MEDIA";

            break;
        }
    }
}

void Transfers::startTranfers()
{
    //this->start(QThread::IdlePriority);
    if (media.count()>0) {
        transferFile(media.at(0));
    }
}

void Transfers::stopTransfers()
{
    if (media[0][1]=="download") {
        if (downloadNetworkReply && downloadNetworkReply->isRunning()) {
            qDebug() << "STOPPING ALL DOWNLOADS";
            disconnect(downloadNetworkReply,0,0,0);
            downloadNetworkReply->abort();
        }

        if (downloadFile.isOpen()) {
            qDebug() << "CLOSING DOWNLOAD FILE";
            downloadFile.close();
            if (QFileInfo(currentFile).exists()) {
                QFile::remove(currentFile);
            }
        }
    }
    else
    {
        if (uploadNetworkReply && uploadNetworkReply->isRunning()) {
            qDebug() << "STOPPING ALL UPLOADS";
            disconnect(uploadNetworkReply,0,0,0);
            uploadNetworkReply->abort();
        }

        /*if (uploadFile && uploadFile->isOpen()) {
            qDebug() << "CLOSING UPLOAD FILE";
            delete uploadFile;
        }*/
    }

    emit progressChanged(media[0][3], 0, "");
    working = false;
    emit workingChanged(working);

}

void Transfers::transferFile(QStringList file)
{
    QSettings settings("cepiperez","fileboxplus");

    accessToken = settings.value(file.at(0)+"/access_token","").toString();

    if (file.at(1)=="download")
    {
        qDebug() << "DOWNLOADING FILE: " << file;
        QString downloadPath = settings.value(file.at(0)+"/download_folder",Config::getHome() + "/Downloads").toString();

        working = true;
        emit workingChanged(working);

        QString tmp = file.at(3);

        currentFile = downloadPath + "/" + QFileInfo(file.at(2)).fileName();
        currentFile = currentFile.replace("//","/").replace("//","/");

        if (QFileInfo(currentFile).exists()) {
            QFile::remove(currentFile);
        }

        downloadFile.setFileName(currentFile);
        if(!downloadFile.open(QFile::WriteOnly))
        {
            qDebug() << "ERROR OPENING FILE FOR DOWNLOAD!!!";
            return;
        }

        if (file.at(0).startsWith("OneDrive"))
        {
            tmp = tmp.remove("/content/");
            tmp = tmp + "/content";
            tmp = tmp + "?download=true";

            QUrl url(tmp);
            qDebug() << "STARTING " << url << " token: " << accessToken;
            QNetworkRequest networkRequest(url);
            //networkRequest.setRawHeader("User-Agent", "Filebox");
            networkRequest.setRawHeader("Authorization", "Bearer "+accessToken.toLatin1());

            disconnect(datos, 0, 0, 0);

            downloadNetworkReply = datos->get(networkRequest);
            disconnect(downloadNetworkReply, 0, 0, 0);

            connect(downloadNetworkReply, SIGNAL(finished()), this, SLOT(transferDownloadFinished()) );
            connect(downloadNetworkReply, SIGNAL(readyRead()), this, SLOT(handleDownloadReadyRead()) );
            connect(downloadNetworkReply, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(handleDownloadProgress(qint64, qint64)));
        }

        else if (file.at(0).startsWith("Box"))
        {
            tmp = "https://api.box.com/2.0/files/" + file.at(3) + "/content";

            QUrl url(tmp);
            qDebug() << "STARTING " << url << " token: " << accessToken;
            QNetworkRequest networkRequest(url);
            networkRequest.setRawHeader("Authorization", "Bearer "+accessToken.toLatin1());

            disconnect(datos, 0, 0, 0);

            downloadNetworkReply = datos->get(networkRequest);
            disconnect(downloadNetworkReply, 0, 0, 0);

            connect(downloadNetworkReply, SIGNAL(finished()), this, SLOT(transferDownloadFinished()) );
            connect(downloadNetworkReply, SIGNAL(readyRead()), this, SLOT(handleDownloadReadyRead()) );
            connect(downloadNetworkReply, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(handleDownloadProgress(qint64, qint64)));
        }

        else if (file.at(0).startsWith("Google Drive"))
        {
            QUrl url = file.at(3);
            QNetworkRequest networkRequest(url);
            networkRequest.setRawHeader("User-Agent", "Filebox");
            networkRequest.setRawHeader("GData-Version", "3.0");
            networkRequest.setRawHeader("Authorization",(QString("OAuth %1").arg(accessToken)).toLatin1());

            disconnect(datos,0,0,0);
            //connect(datos, SIGNAL(finished(QNetworkReply*)), SLOT(handleNetworkReply(QNetworkReply*)));

            downloadNetworkReply = datos->get(networkRequest);

            disconnect(downloadNetworkReply,0,0,0);
            connect(downloadNetworkReply, SIGNAL(finished()), this, SLOT(transferDownloadFinished()) );
            //connect(downloadNetworkReply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(transferError()) );
            connect(downloadNetworkReply, SIGNAL(readyRead()), this, SLOT(handleDownloadReadyRead()) );
            connect(downloadNetworkReply, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(handleDownloadProgress(qint64, qint64)));
        }

        else if (file.at(0).startsWith("Dropbox"))
        {
            QSettings settings("cepiperez","fileboxplus");
            Common::userData->token = settings.value("Dropbox/access_token").toString();
            Common::userData->secret = settings.value("Dropbox/access_token_secret").toString();
            Common::userData->uid = settings.value("Dropbox/uid").toString();

            QUrl url = Common::dropbox->apiToUrl(Dropbox::FILES).toString() + QString(file.at(3)).replace("//","/");
            QNetworkRequest networkRequest(url);
            Common::oAuth->signRequestHeader("GET", &networkRequest);

            disconnect(datos,0,0,0);
            //connect(datos, SIGNAL(finished(QNetworkReply*)), SLOT(handleNetworkReply(QNetworkReply*)));

            downloadNetworkReply = datos->get(networkRequest);

            disconnect(downloadNetworkReply,0,0,0);
            connect(downloadNetworkReply, SIGNAL(finished()), this, SLOT(transferDownloadFinished()) );
            //connect(downloadNetworkReply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(transferError()) );
            connect(downloadNetworkReply, SIGNAL(readyRead()), this, SLOT(handleDownloadReadyRead()) );
            connect(downloadNetworkReply, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(handleDownloadProgress(qint64, qint64)));
        }

        else if (file.at(0).startsWith("WebDav-"))
        {
            QString host = settings.value(file.at(0)+"/host","").toString();
            QString path = settings.value(file.at(0)+"/path","").toString();
            QString user = settings.value(file.at(0)+"/user","").toString();
            QString pass = settings.value(file.at(0)+"/pass","").toString();

            QString ctype;
            if (host.startsWith("http://")) {
                ctype = "http";
            } else {
                ctype = "https";
            }

            host.remove("http://").remove("https://");

            qwebdav.setConnectionSettings(ctype=="http"? QWebdav::HTTP : QWebdav::HTTPS,
                                          host, "", user, pass, ctype=="http"? 80 : 443);

            tmp = path + tmp;
            tmp.replace("//", "/");

            downloadFile.setFileName(currentFile);
            if(!downloadFile.open(QFile::WriteOnly))
            {
                qDebug() << "ERROR OPENING FILE FOR DOWNLOAD!!!";
                return;
            }

            qDebug() << "Downloading WebDav: " << tmp;

            disconnect(&qwebdav, 0, 0, 0);
            qwebdav.get(tmp, &downloadFile);

            connect(&qwebdav, SIGNAL(readReadyRead(QNetworkReply*)), this, SLOT(handleDownloadReadyRead2(QNetworkReply*)));
            connect(&qwebdav, SIGNAL(readFinished(QNetworkReply*)), this, SLOT(transferDownloadFinished2(QNetworkReply*)));
            connect(&qwebdav, SIGNAL(errorChanged(QString)), this, SLOT(printError(QString)));
            connect(&qwebdav, SIGNAL(fileProgress(qint64,qint64)), this, SLOT(handleDownloadProgress(qint64, qint64)));
        }



    }
    else if (file.at(1)=="upload")
    {
        qDebug() << "UPLOADING FILE: " << file;

        if (settings.value("OneDrive/upload_folder","").toString()=="") {
            settings.setValue("OneDrive/upload_folder", "/||https://apis.live.net/v5.0/me/skydrive/files");
            settings.sync();
        }


        QStringList uploadPath = settings.value(file.at(0)+"/upload_folder","/||/").toString().split("||");

        currentFile = QFileInfo(file.at(3)).fileName();
        currentFile = uploadPath[1] + "/" + currentFile;
        currentFile = currentFile.replace("https:/","https://");
        currentFile = currentFile.replace("//","/");
        uploadFile = new QFile(file.at(3));

        if(!uploadFile->open(QFile::ReadOnly))
        {
            qDebug() << "ERROR OPENING FILE FOR UPLOAD!!!";
            return;
        }

        working = true;
        emit workingChanged(working);

        QByteArray myFile = uploadFile->readAll();

        if (file.at(0).startsWith("OneDrive"))
        {
            QUrl url(currentFile);
            QNetworkRequest request(url);
            request.setRawHeader( "Authorization", "Bearer "+accessToken.toLatin1());

            qDebug() << "UPLOADING " << request.url();

            disconnect(datos,0,0,0);

            uploadNetworkReply = datos->put(request, myFile);

            disconnect(uploadNetworkReply,0,0,0);
            connect(uploadNetworkReply, SIGNAL(finished()), this, SLOT(transferUploadFinished()) );
            connect(uploadNetworkReply, SIGNAL(uploadProgress(qint64, qint64)), this, SLOT(handleDownloadProgress(qint64, qint64)));

        }

        else if (file.at(0).startsWith("Box"))
        {
            QString filename = QFileInfo(file.at(3)).fileName();
            QString dest = uploadPath[1];
            if (dest=="/") dest="0";
            QString params = "{\"name\":\"" + filename + "\", \"parent\": { \"id\":\"" + dest + "\"} }";

            QUrl url("https://upload.box.com/api/2.0/files/content");
            QUrlQuery query;
            query.addQueryItem("attributes", params);
            url.setQuery(query);

            QNetworkRequest request(url);
            request.setRawHeader("Authorization", "Bearer "+accessToken.toLatin1());

            QString boundary = "---------------------------916552054217479323188478885";

            QByteArray parts(QString("--" + boundary + "\r\n").toLatin1());
            parts += "Content-Disposition: form-data; name=\"file\"; filename=\""+ filename +"\"\r\n";
            parts += "Content-Type: application/octet-stream\r\n\r\n";
            parts += myFile;
            parts += "\r\n";
            parts += QString("--" + boundary + "\r\n").toLatin1();
            parts += "Content-Disposition: form-data; name=\"upload\"\r\n\r\n";
            parts += "Uploader\r\n";
            parts += QString("--" + boundary + "--\r\n").toLatin1();

            request.setHeader(QNetworkRequest::ContentTypeHeader, "multipart/form-data; boundary=" + boundary);
            request.setHeader(QNetworkRequest::ContentLengthHeader, QString::number(parts.length()));

            disconnect(datos,0,0,0);
            uploadNetworkReply = datos->post(request, parts);

            disconnect(uploadNetworkReply,0,0,0);
            connect(uploadNetworkReply, SIGNAL(finished()), this, SLOT(transferUploadFinished()) );
            connect(uploadNetworkReply, SIGNAL(uploadProgress(qint64, qint64)), this, SLOT(handleDownloadProgress(qint64, qint64)));

        }

        else if (file.at(0).startsWith("Google Drive"))
        {
            QFileInfo fi(file.at(3));
            QString ext = fi.suffix();
            QString title = fi.fileName();
            QString contentType = getContentTypeByExtension(ext.toLower());

            QByteArray params = "--foo_bar_baz\nContent-Type: application/json; charset=UTF-8\n\n";
            params += "{\n";
            params += " \"title\": \"" + title.toLatin1() + "\"";

            if (settings.value("Google Drive/upload_folder","").toString()!="") {
                params += ",\n \"parents\" : [{\"id\": \"" + uploadPath[1].toLatin1() + "\"}]\n";
            }

            params += "}\n\n--foo_bar_baz\nContent-Type: " + contentType.toLatin1() + "\n\n";

            qDebug() << params;

            params += myFile;
            params += "\n--foo_bar_baz--\n";


            QUrl url("https://www.googleapis.com/upload/drive/v2/files");
            QUrlQuery query;
            query.addQueryItem("uploadType", "multipart");
            url.setQuery(query);
            QNetworkRequest request(url);
            request.setRawHeader("Host", "www.googleapis.com");
            request.setRawHeader("Authorization", "Bearer "+ accessToken.toLatin1());
            request.setRawHeader("Content-Type", "multipart/related; boundary=\"foo_bar_baz\"");
            request.setRawHeader("Content-Length", QString::number(params.size()).toLatin1());

            qDebug() << "UPLOADING TO GOOGLE DRIVE: " << url;

            disconnect(datos,0,0,0);

            uploadNetworkReply = datos->post(request, params);

            disconnect(uploadNetworkReply,0,0,0);
            connect(uploadNetworkReply, SIGNAL(finished()), this, SLOT(transferUploadFinished()) );
            connect(uploadNetworkReply, SIGNAL(uploadProgress(qint64, qint64)), this, SLOT(handleDownloadProgress(qint64, qint64)));

        }

        else if (file.at(0).startsWith("Dropbox"))
        {
            int fileBytes = QFileInfo(file.at(3)).size();

            QSettings settings("cepiperez","fileboxplus");
            Common::userData->token = settings.value("Dropbox/access_token").toString();
            Common::userData->secret = settings.value("Dropbox/access_token_secret").toString();
            Common::userData->uid = settings.value("Dropbox/uid").toString();

            QUrl url = Common::dropbox->apiToUrl(Dropbox::FILESPUT).toString() + currentFile;
            qDebug() << "UPLOADING TO DROPBOX: " << file << url;

            QNetworkRequest networkRequest(url);
            Common::oAuth->signRequestHeader("PUT", &networkRequest);
            networkRequest.setRawHeader("Content-Length",QString("%1").arg(fileBytes).toLatin1());

            disconnect(datos,0,0,0);
            //connect(datos, SIGNAL(finished(QNetworkReply*)), SLOT(handleNetworkReply(QNetworkReply*)));

            uploadNetworkReply = Common::networkAccessManager->put(networkRequest, myFile);

            disconnect(uploadNetworkReply,0,0,0);
            connect(uploadNetworkReply, SIGNAL(finished()), this, SLOT(transferUploadFinished()) );
            connect(uploadNetworkReply, SIGNAL(uploadProgress(qint64, qint64)), this, SLOT(handleDownloadProgress(qint64, qint64)));

        }

        else if (file.at(0).startsWith("WebDav-"))
        {
            QString host = settings.value(file.at(0)+"/host","").toString();
            QString path = settings.value(file.at(0)+"/path","").toString();
            QString user = settings.value(file.at(0)+"/user","").toString();
            QString pass = settings.value(file.at(0)+"/pass","").toString();

            QString ctype;
            if (host.startsWith("http://")) {
                ctype = "http";
            } else {
                ctype = "https";
            }

            host.remove("http://").remove("https://");

            qwebdav.setConnectionSettings(ctype=="http"? QWebdav::HTTP : QWebdav::HTTPS,
                                          host, path, user, pass, ctype=="http"? 80 : 443);

            currentFile.replace("//", "/");

            qDebug() << "Uploading WebDav: " << currentFile;

            disconnect(&qwebdav, 0, 0, 0);
            uploadNetworkReply = qwebdav.put(currentFile, myFile);

            disconnect(uploadNetworkReply,0,0,0);
            connect(uploadNetworkReply, SIGNAL(finished()), this, SLOT(transferUploadFinished()) );
            connect(uploadNetworkReply, SIGNAL(uploadProgress(qint64, qint64)), this, SLOT(handleDownloadProgress(qint64, qint64)));
        }

    }

}

void Transfers::printError(QString error)
{
    qDebug() << "Error transfering file: " << error;
    stopTransfers();
}

void Transfers::slotError(QNetworkReply::NetworkError error)
{
    qDebug() << "NetworkManager::slotError error code:" << error;

    if(error == QNetworkReply::AuthenticationRequiredError)
    {
        uploadNetworkReply->abort();
    }

    if(error == QNetworkReply::UnknownNetworkError)
        qDebug() << "\n*******************\nIf this error occur, please make sure that you have openssl installed (also you can try just copy libeay32.dll and ssleay32.dll files from Qt SDK QtCreator/bin folder into your folder where your program .exe file located (tested on non-static compilation only))\n*******************\n";
}

void Transfers::slotSslErrors(const QList<QSslError>& )
{
    qDebug() << "error!";

}

void Transfers::handleDownloadReadyRead()
{
    //qDebug() << "WRITTING FILE";
    downloadFile.write(downloadNetworkReply->readAll());
}

void Transfers::handleDownloadReadyRead2(QNetworkReply *networkReply)
{
    //qDebug() << "WRITTING FILE";
    downloadFile.write(networkReply->readAll());
}

void Transfers::handleDownloadProgress(qint64 received, qint64 total)
{
    // avoid errors
    if (received == 0)
        return;

    if (total <= 0)
        total = media[0][4].toDouble();

    int temp = (received*100) / total;
    if (currentProgress != temp) {
        currentProgress = temp;
        emit progressChanged(media[0][3], temp, GetFileSize(received));
    }

}


void Transfers::handleNetworkReply(QNetworkReply *networkReply)
{
    qDebug() << "REQUEST UPLOAD DONE!\n" << networkReply->rawHeader("Location");

    if (media[0][0]=="Google Drive")
    {
        QString replyStr = networkReply->readAll();
        qDebug() << replyStr;

        JSONParser jParser;

        accessToken = jParser.getParam(replyStr, "access_token");
        qDebug() << "ACCESS TOKEN: " << accessToken;

        QSettings settings("cepiperez","fileboxplus");
        settings.setValue("Google Drive/access_token", accessToken);

        QString newRefreshToken = jParser.getParam(replyStr, "refresh_token");
        if(!newRefreshToken.isEmpty())
        {
            qDebug() << "REFRESH TOKEN: " << newRefreshToken;
            settings.setValue("Google Drive/refresh_token", newRefreshToken);
        }

        if (!accessToken.isEmpty() && !newRefreshToken.isEmpty())
        {
            startTranfers();
        }

    }
    else if (media[0][0]=="OneDrive")
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
        QString refreshToken = tmp;

        QSettings settings("cepiperez","fileboxplus");
        settings.setValue("OneDrive/access_token", accessToken);
        settings.setValue("OneDrive/refresh_token", refreshToken);
        settings.sync();

        startTranfers();

    }



}

QUrl Transfers::redirectUrl(const QUrl &possibleRedirectUrl, const QUrl &oldRedirectUrl) const
{
    QUrl redirectUrl;
    if(!possibleRedirectUrl.isEmpty() &&
       possibleRedirectUrl != oldRedirectUrl) {
            redirectUrl = possibleRedirectUrl;
    }
    return redirectUrl;
}

void Transfers::transferDownloadFinished2(QNetworkReply *networkReply)
{
    downloadNetworkReply = networkReply;
    transferDownloadFinished();
}

void Transfers::transferDownloadFinished()
{

    if (downloadNetworkReply->readAll().contains("Invalid Credentials") && media[0][0]=="Google Drive")
    {
        QUrl url("https://accounts.google.com/o/oauth2/token");
        QNetworkRequest request;
        request.setUrl(url);
        request.setRawHeader("Content-Type", "application/x-www-form-urlencoded");

        QSettings settings("cepiperez","fileboxplus");
        QString clientID = "823981073892.apps.googleusercontent.com";
        QString clientSecret = "gzzbIF6rt-48xfDu7NohYKGV";
        QString refreshToken = settings.value("Google Drive/refresh_token").toString();

        QByteArray params = "client_id=" + QByteArray(clientID.toLatin1());
        params += "&client_secret=";
        params += QByteArray(clientSecret.toLatin1());
        params += "&grant_type=refresh_token";
        params += "&refresh_token=" + refreshToken.toLatin1();

        disconnect(datos,0,0,0);
        connect(datos, SIGNAL(finished(QNetworkReply*)),this, SLOT(handleNetworkReply(QNetworkReply*)));
        datos->post(request, params);
        return;
    }

    //emit showBanner("TRANSFER FINISHED");
    if(downloadNetworkReply->error() != QNetworkReply::NoError) {
        qDebug() << "DOWNLOAD NOT FINISHED: " << downloadNetworkReply->errorString() ;
        stopTransfers();

        if (downloadNetworkReply->errorString().contains("Host requires authentication") && media[0][0]=="OneDrive")
        {
            qDebug() << "REQUESTING NEW TOKEN";

            QUrl url("https://login.live.com/oauth20_token.srf");
            QNetworkRequest request;
            request.setUrl(url);
            request.setRawHeader("Content-Type", "application/x-www-form-urlencoded");

            QSettings settings("cepiperez","fileboxplus");
            QString clientId = "000000004C0CB4E7";
            QString clientSecret = "lABMSvVTZfWRhck5qBs25C1l1s4qyp0t";
            QString refreshToken = settings.value("OneDrive/refresh_token").toString();

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


    }
    else
    {
        QVariant possibleRedirectUrl = downloadNetworkReply->attribute(QNetworkRequest::RedirectionTargetAttribute);
        _urlRedirectedTo = this->redirectUrl(possibleRedirectUrl.toUrl(), _urlRedirectedTo);
        if(!_urlRedirectedTo.isEmpty())
        {
            qDebug() << "REDIRECTING!!!!! " << _urlRedirectedTo.toString();
            //agregarUsuario(_urlRedirectedTo.toString().remove(0,1));
            QUrl url = _urlRedirectedTo;
            QNetworkRequest networkRequest(url);
            networkRequest.setRawHeader("Authorization", "Bearer "+accessToken.toLatin1());

            disconnect(datos,0,0,0);

            downloadNetworkReply = datos->get(networkRequest);

            disconnect(downloadNetworkReply,0,0,0);

            connect(downloadNetworkReply, SIGNAL(finished()), this, SLOT(transferDownloadFinished()) );
            connect(downloadNetworkReply, SIGNAL(readyRead()), this, SLOT(handleDownloadReadyRead()) );
            connect(downloadNetworkReply, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(handleDownloadProgress(qint64, qint64)));
        }
        else
        {
            qDebug() << "DOWNLOAD FINISHED OK " << downloadNetworkReply->readAll();
            downloadFile.close();
            emit fileTransfered(media[0][3]);
            working = false;
            emit workingChanged(working);
        }

    }

}

void Transfers::transferUploadFinished()
{
    //qDebug() << uploadNetworkReply->readAll();

    if(uploadNetworkReply->error() != QNetworkReply::NoError) {
        qDebug() << "UPLOAD NOT FINISHED: " << uploadNetworkReply->errorString();
        stopTransfers();

        if (uploadNetworkReply->errorString().contains("Host requires authentication") && media[0][0]=="Google Drive") {
            QUrl url("https://accounts.google.com/o/oauth2/token");
            QNetworkRequest request;
            request.setUrl(url);
            request.setRawHeader("Content-Type", "application/x-www-form-urlencoded");

            QSettings settings("cepiperez","fileboxplus");
            QString clientID = "823981073892.apps.googleusercontent.com";
            QString clientSecret = "gzzbIF6rt-48xfDu7NohYKGV";
            QString refreshToken = settings.value("Google Drive/refresh_token").toString();

            QByteArray params = "client_id=" + QByteArray(clientID.toLatin1());
            params += "&client_secret=";
            params += QByteArray(clientSecret.toLatin1());
            params += "&grant_type=refresh_token";
            params += "&refresh_token=" + refreshToken.toLatin1();

            disconnect(datos,0,0,0);
            connect(datos, SIGNAL(finished(QNetworkReply*)),this, SLOT(handleNetworkReply(QNetworkReply*)));
            datos->post(request, params);
            return;
        }

        if (uploadNetworkReply->errorString().contains("Host requires authentication") && media[0][0]=="OneDrive") {
            QUrl url("https://login.live.com/oauth20_token.srf");
            QNetworkRequest request;
            request.setUrl(url);
            request.setRawHeader("Content-Type", "application/x-www-form-urlencoded");

            QSettings settings("cepiperez","fileboxplus");
            QString clientId = "000000004C0CB4E7";
            QString clientSecret = "lABMSvVTZfWRhck5qBs25C1l1s4qyp0t";
            QString refreshToken = settings.value("OneDrive/refresh_token").toString();

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


    } else {
        qDebug() << "UPLOAD FINISHED OK";
        emit fileTransfered(media[0][3]);
        working = false;
        emit workingChanged(working);
    }

}

void Transfers::handleUploadProgress(qint64 sent, qint64 total)
{
    if (sent == 0 || total == 0)
        return;

    emit progressChanged(media[0][3], (sent*100) / total, GetFileSize(sent));

}

QString Transfers::getContentTypeByExtension(const QString& ext)
{
    QString contentType;

    if(ext == "doc" || ext == "docx") contentType = "application/msword";
    else if(ext == "xls") contentType = "application/vnd.ms-excel";
    else if(ext == "ppt" || ext == "pptx") contentType = "application/vnd.ms-powerpoint";
    else if(ext == "pdf") contentType = "application/pdf";
    else if(ext == "exe") contentType = "application/x-msdos-program";
    else if(ext == "rar") contentType = "application/rar";
    else if(ext == "png") contentType = "image/png";
    else if(ext == "rtf") contentType = "application/rtf";
    else if(ext == "tar") contentType = "application/x-tar";
    else if(ext == "zip") contentType = "application/zip";
    else if(ext == "jpeg" || ext == "jpg" || ext == "jpe") contentType = "image/jpeg";
    else if(ext == "gif") contentType = "image/gif";
    else if(ext == "wav") contentType = "application/x-wav";
    else if(ext == "tiff" || ext == "tif") contentType = "image/tiff";
    else if(ext == "txt" || ext == "cpp" || ext == "h" || ext == "c") contentType = "text/plain";
    else if(ext == "mpeg" || ext == "mpg" || ext == "mpe" ) contentType = "video/mpeg";
    else if(ext == "qt" || ext == "mov") contentType = "video/quicktime";
    else if(ext == "mp4") contentType = "video/mp4";
    else if(ext == "3gp") contentType = "video/3gpp";
    else if(ext == "avi") contentType = "video/x-msvideo";
    else if(ext == "qvi") contentType = "video/x-msvideo";
    else if(ext == "video/x-sgi-movie") contentType = "movie";
    else if(ext == "exe") contentType = "application/x-msdos-program";
    else contentType = "";
    return contentType;
}
