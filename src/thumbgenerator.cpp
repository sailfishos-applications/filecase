#include "config.h"
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
#include <QProcess>
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
        QString tf = Config::getHome() + "/.thumbnails/whatsup/" + QString(md.result().toHex().constData()) + ".jpeg";
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
        file = "file://" + file;
        md.addData(file.toUtf8());
        QString tf = Config::getHome() + "/.thumbnails/whatsup/" + QString(md.result().toHex().constData()) + ".jpeg";
        thumb = tf;

        if (!QFileInfo(thumb).exists())
        {
            QString command = "ffmpeg"; //used to replace  "/usr/lib/qt5/qml/org/nemomobile/thumbnailer/thumbnailers/libvideothumbnailer.so" solution
            QStringList arguments;
            arguments << "-i" << fileInfo.absoluteFilePath()
                      << "-ss" << "00:00:01"                 // Arbitrary time can be anything (1 second here)
                      << "-vframes" << "1"
                      << "-q:v" << "2"                       // Quality factor. Lower is better. Higher gives lower bitrate. 2 is 1735 kb/s Check https://ffmpeg.org/ffmpeg-codecs.html#Options-22
                      << thumb;

            QProcess process;
            process.start(command, arguments);
            process.waitForFinished();

            if (process.exitCode() != 0) {
                //Need to test what might go wrong..should add .close()
                qDebug() << "ffmpeg ERROR: " << file;
            }
        }
        emit imgLoaded(thumb);
    }

    imagesToProcess.removeAt(0);

    if (imagesToProcess.count()>0)
        generate(imagesToProcess.at(0));

}
