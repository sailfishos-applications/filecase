#include "thumbgenerator.h"
#include <QCryptographicHash>
#include <QUrl>
#include <QImageReader>
#include <qstring.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <QImage>
#include <QSettings>
#include <QDebug>

#include <QLibrary>

typedef QImage (*CreateThumbnailFunc)(const QString &fileName, const QSize &requestedSize, bool crop);


ThumbGenerator::ThumbGenerator()
{

}

ThumbGenerator::~ThumbGenerator()
{

}

void ThumbGenerator::run()
{
    if (imagesToProcess.count()==0)
        return;

    generate(imagesToProcess.at(0));
}

void ThumbGenerator::generate(QString filename)
{
    if (filename.endsWith(".jpg") || filename.endsWith(".jpeg") || filename.endsWith(".png"))
    {

        QFileInfo fileInfo = filename;

        QString exten = fileInfo.fileName().toLower();
        int ei = exten.lastIndexOf( "." );
        exten.remove ( 0, ei + 1 );

        QString thumb;
        QCryptographicHash md(QCryptographicHash::Md5);
        QString file = fileInfo.absoluteFilePath();
        file.replace("#","%2523");
        file.replace(",","%2C");
        file.replace(" ","%20");
        file = "file://"+file;
        md.addData(file.toUtf8());
        QString tf = "/home/nemo/.thumbnails/whatsup/"+ QString(md.result().toHex().constData()) + ".jpeg";
        thumb = tf;

        if ( !QFileInfo(tf).exists() )
        {
            //Utilities::logData("Generating thumbnail for " + fileInfo.absoluteFilePath());
            QImage result;
            QImageReader reader(fileInfo.absoluteFilePath());
            reader.setAutoTransform(true);
            QImage img = reader.read();
            if ( img.height() > img.width() ) {
                result = img.scaledToWidth(480,Qt::SmoothTransformation);
                //result = result.copy(0,result.height()/2-90,180,180);
            }
            else if ( img.height() < img.width() )
            {
                result = img.scaledToHeight(480,Qt::SmoothTransformation);
                //result = result.copy(result.width()/2-90,0,180,180);
            }
            else if ( img.height() > 480 )
            {
                result = img.scaled(480, 480, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
                //result = result.copy(0,0,180,180);
            }
            else
            {
                result = img;
            }
            result.save( tf, "JPEG" );

        }
        emit imgLoaded(tf);
    }
    else if (filename.endsWith(".mp4") || filename.endsWith(".3gp") || filename.endsWith(".avi") || filename.endsWith(".wmv"))
    {
        QString thumb;
        QFileInfo fileInfo = filename;
        QCryptographicHash md(QCryptographicHash::Md5);
        QString file = fileInfo.absoluteFilePath();
        file.replace("#","%2523");
        file.replace(",","%2C");
        file.replace(" ","%20");
        file = "file://"+file;
        md.addData(file.toUtf8());
        QString tf = "/home/nemo/.thumbnails/whatsup/"+ QString(md.result().toHex().constData()) + ".jpeg";
        thumb = tf;

        if ( !QFileInfo(tf).exists() )
        {
            //Utilities::logData("Generating video thumbnail for " + fileInfo.absoluteFilePath());
            //VideoThumbnailer *thumbnailer = new VideoThumbnailer;
            //QImage result = thumbnailer->createThumbnail(filename.remove("file://"), QSize(170,170), true);
            QImage image;

            static CreateThumbnailFunc createThumbnail = (CreateThumbnailFunc)QLibrary::resolve(
                        QLatin1String("/usr/lib/qt5/qml/org/nemomobile/thumbnailer/thumbnailers/libvideothumbnailer.so"), "createThumbnail");

            if (createThumbnail) {
                image = createThumbnail(filename, QSize(800,800), false);
            }

            image.save( tf, "JPEG" );
        }
        emit imgLoaded(tf);
    }

    imagesToProcess.removeAt(0);

    if (imagesToProcess.count()>0)
        generate(imagesToProcess.at(0));

}
