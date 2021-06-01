#include "fileinfo.h"
#include "utils.h"
#include "config.h"

#include <QMimeDatabase>
#include <QCryptographicHash>
#include <QFileInfo>
#include <QDateTime>
#include <QSettings>
#include <QString>
#include <QDir>


FileInfo::FileInfo(QQuickItem *parent)
     : QQuickItem(parent)
{
    thumbnailer = new ThumbGenerator();
    connect(thumbnailer, SIGNAL(imgLoaded(QString)), this, SIGNAL(thumbReady(QString)));

}

void FileInfo::setFile(QString filelist)
{
    QStringList entries = filelist.split("<filecaseseparator>");

    //qDebug() << "READING FILES: " << entries;

    pepe = new Loader();
    pepe->setAction("fileinfo-files");
    pepe->setFiles(entries);

    connect (pepe, SIGNAL(sizeChanged(double)), this, SLOT(setSizeInfoLoading(double)) );
    connect (pepe, SIGNAL(sizeFinished(double)), this, SLOT(setSizeInfo(double)) );

    m_data.clear();

    if ( entries.count() == 1 )
    {
        QFileInfo fileInfo = entries.at(0);
        m_data.insert("name", fileInfo.fileName());
        //m_size = fileInfo.isFile() ? GetFileSize(fileInfo.size()) :
        //                             GetFileSize(GetFolderSize(fileInfo.absoluteFilePath()));
        m_data.insert("size", "0 kb " + tr("(loading)"));
        m_data.insert("path", fileInfo.absolutePath());
        m_data.insert("created", fileInfo.created().date().toString("MM/dd/yyyy"));
        m_data.insert("modified", fileInfo.lastModified().date().toString("MM/dd/yyyy"));

        QMimeDatabase db;
        QMimeType ftype = db.mimeTypeForFile(fileInfo.absoluteFilePath());
        m_data.insert("comment", ftype.comment());
        m_data.insert("mime", ftype.name());

        QString m_name = fileInfo.fileName();
        QString m_icono;
        if ( fileInfo.isDir() )
            m_icono = "folder";  //"../icons/folder.png";
        else if ( m_name.toLower().endsWith(".jpg") )
            m_icono = "image/jpeg";
        else if ( m_name.toLower().endsWith(".jpeg") )
            m_icono = "image/jpeg";
        else if ( m_name.toLower().endsWith(".png") )
            m_icono = "image/png";
        else if ( m_name.toLower().endsWith(".gif") )
            m_icono = "image/gif";
        else if ( m_name.toLower().endsWith(".avi") )
            m_icono = "video/x-msvideo";
        else if ( m_name.toLower().endsWith(".mp4") )
            m_icono = "video/mp4";
        else if ( m_name.toLower().endsWith(".wmv") )
            m_icono = "video/x-msvideo";
        else if ( m_name.toLower().endsWith(".3gp") )
            m_icono = "video/3gpp";
        else
            m_icono = GetExtension(m_name.toLower());

        m_data.insert("icon", m_icono);

        if ( fileInfo.isSymLink() )
            m_data.insert("target", fileInfo.symLinkTarget());
        else
            m_data.insert("target", "");

    }
    else
    {
        m_data.insert("name", QString::number(entries.count()) + " " + tr("items"));
        m_data.insert("path", QFileInfo(entries.at(0)).absolutePath());
        m_data.insert("size", "0 kb " + tr("(loading)"));
        m_data.insert("created", "");
        m_data.insert("modified", "");
        m_data.insert("icon", "");
        m_data.insert("mime", "");
        m_data.insert("comment", "");
        m_data.insert("target", "");
    }

    pepe->start();
    pepe->setPriority(QThread::IdlePriority);

    emit fileChanged();
}

void FileInfo::stopInfoThread()
{
    //qDebug() << "STOP THREAD";
    if ( pepe->isRunning() )
        pepe->quit();

}

void FileInfo::setSizeInfoLoading(double size)
{
    //qDebug() << "Size changed!" << size;
    m_data.remove("size");
    m_data.insert("size", GetFileSize(size) + " " + tr("(loading)"));
    emit fileChanged();
}

void FileInfo::setSizeInfo(double size)
{
    //qDebug() << "Size changed!" << size;
    m_data.remove("size");
    m_data.insert("size", GetFileSize(size));
    emit fileChanged();
}

QVariantMap FileInfo::data() const
{
    return m_data;
}

QString FileInfo::getFilePreview(QString filename)
{
    QDir dir;
    if (!QFileInfo(Config::getHome() + "/.thumbnails/whatsup").exists())
        dir.mkdir(Config::getHome() + "/.thumbnails/whatsup");

    filename.remove("file://");
    QCryptographicHash md(QCryptographicHash::Md5);
    QString file = filename;
    file.replace("#","%2523");
    file.replace(",","%2C");
    file.replace(" ","%20");
    file = "file://"+file;
    md.addData(file.toUtf8());
    QString tf = Config::getHome() + "/.thumbnails/whatsup/"+ QString(md.result().toHex().constData()) + ".jpeg";
    if (QFileInfo(tf).exists())
        return "file://" + tf;
    else {
        thumbnailer->generate(filename);
        return "file://" + tf;
    }
}

QVariantMap FileInfo::getFileMimetype(QString filename)
{
    QMimeDatabase db;
    QMimeType ftype = db.mimeTypeForFile(filename);
    QVariantMap res;
    res.insert("comment", ftype.comment());
    res.insert("name", ftype.name());
      return res;
}
