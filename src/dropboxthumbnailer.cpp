#include "dropboxthumbnailer.h"
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
#include "dropbox/common.h"
#include "dropbox/dropoauth.h"
#include "dropbox/userdata.h"
#include "dropbox/dropbox.h"
#include "dropbox/qt-json/json.h"


DropboxThumbnailer::DropboxThumbnailer()
{

}

DropboxThumbnailer::~DropboxThumbnailer()
{

}

void DropboxThumbnailer::clear()
{
    images.clear();
    if (reply && reply->isRunning()) {
        reply->abort();
    }

}

void DropboxThumbnailer::addImage(QString filename, QString link)
{
    //filename.replace("//","/");
    QStringList m;
    m << filename << link;
    qDebug() << "ADDING IMAGE: " << filename << " - Link: " << link;
    images.append(m);
}

void DropboxThumbnailer::done()
{
    //this->start(QThread::IdlePriority);
    if (images.count()>0) {
        working = true;
        download(images.at(0));
    }
}

void DropboxThumbnailer::run()
{
    //if (images.count()>0)
    //    download(images.at(0));
}

void DropboxThumbnailer::download(QStringList image)
{
    QUrl url = Common::dropbox->apiToUrl(Dropbox::THUMBNAILS);
    QUrlQuery query;
    query.addQueryItem("root", "dropbox");
    query.addQueryItem("path", image[1]);
    query.addQueryItem("size", "m");
    url.setQuery(query);
    QNetworkRequest networkRequest(url);
    Common::oAuth->signRequestHeader("GET", &networkRequest, query);

    disconnect(Common::networkAccessManager,0,0,0);
    connect(Common::networkAccessManager, SIGNAL(finished(QNetworkReply*)),
            SLOT(handleNetworkReply(QNetworkReply*)));

    qDebug() << "DOWNLOADING " << image[1];

    reply = Common::networkAccessManager->get(networkRequest);


}

void DropboxThumbnailer::handleNetworkReply(QNetworkReply *networkReply)
{
    saveToDisk(images[0][0], networkReply);

    emit imageLoaded(images[0][0]);

    images.removeAt(0);
    if (images.count()>0)
        download(images.at(0));


}

bool DropboxThumbnailer::saveToDisk(const QString &filename, QIODevice *reply)
{
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly)) {
        fprintf(stderr, "Could not open %s for writing: %s\n",
                qPrintable(filename),
                qPrintable(file.errorString()));
        return false;
    }

    file.write(reply->readAll());
    file.close();

    return true;
}

