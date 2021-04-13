#include "skythumbnailer.h"

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


SkyThumbnailer::SkyThumbnailer()
{
    datos = new QNetworkAccessManager(this);
}

SkyThumbnailer::~SkyThumbnailer()
{

}

void SkyThumbnailer::clear()
{
    images.clear();
    if (reply && reply->isRunning()) {
        reply->abort();
    }

}

void SkyThumbnailer::addImage(QString filename, QString link)
{
    filename.replace("//","/");
    QStringList m;
    m << filename << link;
    qDebug() << "ADDING IMAGE: " << filename << " - Link: " << link;
    images.append(m);
}

void SkyThumbnailer::done()
{
    //this->start(QThread::IdlePriority);
    if (images.count()>0) {
        working = true;
        download(images.at(0));
    }
}

void SkyThumbnailer::run()
{
    //if (images.count()>0)
    //    download(images.at(0));
}

void SkyThumbnailer::download(QStringList image)
{
    QSettings settings("cepiperez","fileboxplus");
    accessToken = settings.value("OneDrive/access_token","").toString();

    QUrl url = image[1];
    QNetworkRequest networkRequest(url);
    //networkRequest.setRawHeader("User-Agent", "Filebox");
    //networkRequest.setRawHeader("GData-Version", "3.0");
    //networkRequest.setRawHeader("Authorization",(QString("OAuth %1").arg(accessToken)).toLatin1());
    networkRequest.setRawHeader("Authorization", "Bearer "+accessToken.toLatin1());

    disconnect(datos,0,0,0);
    connect(datos, SIGNAL(finished(QNetworkReply*)),
            SLOT(handleNetworkReply(QNetworkReply*)));

    //qDebug() << "DOWNLOADING " << image[1];

    reply = datos->get(networkRequest);


}

void SkyThumbnailer::handleNetworkReply(QNetworkReply *networkReply)
{
    saveToDisk(images[0][0], networkReply);

    emit imageLoaded(images[0][0]);

    images.removeAt(0);
    if (images.count()>0)
        download(images.at(0));

}

bool SkyThumbnailer::saveToDisk(const QString &filename, QIODevice *reply)
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
