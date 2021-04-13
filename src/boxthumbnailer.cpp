#include "boxthumbnailer.h"

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


BoxThumbnailer::BoxThumbnailer()
{
    datos = new QNetworkAccessManager(this);
}

BoxThumbnailer::~BoxThumbnailer()
{

}

void BoxThumbnailer::clear()
{
    images.clear();
    if (reply && reply->isRunning()) {
        reply->abort();
    }

}

void BoxThumbnailer::addImage(QString filename, QString link)
{
    filename.replace("//","/");
    QStringList m;
    m << filename << link;
    qDebug() << "ADDING IMAGE: " << filename << " - Link: " << link;
    images.append(m);
}

void BoxThumbnailer::done()
{
    //this->start(QThread::IdlePriority);
    if (images.count()>0) {
        working = true;
        download(images.at(0));
    }
}

void BoxThumbnailer::run()
{
    //if (images.count()>0)
    //    download(images.at(0));
}

void BoxThumbnailer::download(QStringList image)
{
    QSettings settings("cepiperez","fileboxplus");
    accessToken = settings.value("Box/access_token","").toString();

    QUrl url = "https://api.box.com/2.0/files/" + image[1] + "/thumbnail.png?min_height=256&min_width=256";
    QNetworkRequest networkRequest(url);
    networkRequest.setRawHeader("Authorization", "Bearer "+accessToken.toLatin1());

    disconnect(datos,0,0,0);
    connect(datos, SIGNAL(finished(QNetworkReply*)),
            SLOT(handleNetworkReply(QNetworkReply*)));

    qDebug() << "DOWNLOADING " << url;

    reply = datos->get(networkRequest);


}

void BoxThumbnailer::handleNetworkReply(QNetworkReply *networkReply)
{
    qDebug() << networkReply->size();
    /*QImage img;
    img = QImage::fromData(networkReply->readAll());

   if ( img.height() > img.width() ) {
        result = img.scaledToWidth(64,Qt::SmoothTransformation);
        result = result.copy(0,result.height()/2-32,64,64);
    }
    else if ( img.height() < img.width() )
    {
        result = img.scaledToHeight(64,Qt::SmoothTransformation);
        result = result.copy(result.width()/2-32,0,64,64);
    }
    else if ( img.height() > 64 )
    {
        result = img.scaled(64, 64, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        result = result.copy(0,0,64,64);
    }
    else
    {
        result = img;
    }

    QCryptographicHash md(QCryptographicHash::Md5);
    QString file = "Box" + images[0][0];
    md.addData(file.toUtf8());
    QString tf = "/home/user/.thumbnails/filebox/"+ QString(md.result().toHex().constData()) + ".png";

    result.save( tf, "PNG" );*/

    if (networkReply->size()==0)
        return;

    saveToDisk(images[0][0], networkReply);

    emit imageLoaded(images[0][0]);

    images.removeAt(0);
    if (images.count()>0)
        download(images.at(0));

}

bool BoxThumbnailer::saveToDisk(const QString &filename, QIODevice *reply)
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
