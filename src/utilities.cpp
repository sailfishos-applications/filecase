#include "sys/vfs.h"
#include "utilities.h"
#include "transfers.h"
#include "utils.h"

#include <QSettings>
#include <QFileInfo>
#include <QProcess>


extern Transfers *transfers;

Utilities::Utilities(QQuickItem *parent)
    : QQuickItem(parent)
{
    m_working = false;

    transfers = new Transfers();
    connect (transfers, SIGNAL(progressChanged(QString,int,QString)), this, SIGNAL(progressChanged(QString,int,QString)));
    connect (transfers, SIGNAL(workingChanged(bool)), this, SLOT(setWorking(bool)));
    connect (transfers, SIGNAL(fileTransfered(QString)), this, SLOT(fileTransfered(QString)));

    zipFile = "";
    zipBase = "";

    zipProc = new QProcess();
    connect(zipProc, SIGNAL( finished(int) ), this, SLOT( pfinished(int) ) );

    pepe = new Loader();
    connect(pepe, SIGNAL(finished()), this, SIGNAL(readTextFinished()));
    connect(pepe, SIGNAL(addLines(QString)), this, SIGNAL(addTextLines(QString)));

}

void Utilities::loadBookmarks()
{
    QSettings settings("cepiperez", "fileboxplus");
    QStringList entries = settings.value("Bookmarks","").toStringList();
    entries.removeAll("");
    for (int i=0; i<entries.count(); ++i)
    {
        emit appendBookmark(entries.at(i));
    }

}

void Utilities::addToBookmarks(QString folder)
{
    QSettings settings("cepiperez", "fileboxplus");
    QString nf = folder;
    if ( nf.startsWith("//") )
        nf.remove(0, 1);
    QStringList entries = settings.value("Bookmarks","").toStringList();
    if ( !entries.contains(nf) )
    {
        entries.append(nf);
        settings.setValue("Bookmarks", entries);
        settings.sync();
        emit bookmarkAdded(nf);
    }
}

void Utilities::removeFromBookmarks(QString folder)
{
    QSettings settings("cepiperez", "fileboxplus");
    QString nf = folder;
    if ( nf.startsWith("//") )
        nf.remove(0, 1);
    QStringList entries = settings.value("Bookmarks","").toStringList();
    entries.removeAll(nf);
    settings.setValue("Bookmarks", entries);
    settings.sync();
    emit bookmarkRemoved(folder);
}

void Utilities::openFile(QString file)
{
    QProcess * process = new QProcess();
    process->start(QString("xdg-open \"%2\"").arg(file));

}

bool Utilities::fileExists(QString filename)
{
    return QFileInfo(filename).exists();
}

void Utilities::loadWebDavs()
{
    QSettings settings("cepiperez", "fileboxplus");
    Q_FOREACH (QString group, settings.childGroups())
    {
        if (group.startsWith("WebDav-"))
        {
            emit appendWebDav(group.remove("WebDav-"));
        }
    }
}

void Utilities::loadClouds()
{
    QSettings settings("cepiperez", "fileboxplus");

    if (settings.value("Box/access_token","") != "")
        emit appendCloud("Box", "Box.qml", "box");

    if (settings.value("Dropbox/access_token","") != "")
        emit appendCloud("Dropbox", "Dropbox.qml", "dropbox");

    if (settings.value("Google Drive/access_token","") != "")
        emit appendCloud("Google Drive", "Drive.qml", "gdrive");

    if (settings.value("OneDrive/access_token","") != "")
        emit appendCloud("OneDrive", "SkyDrive.qml", "sky");

}


/*void Utilities::removeWebDav(QString service)
{

}*/

/*void Utilities::removeCloud(QString service)
{

}*/


void Utilities::loadTransfers()
{
    emit clearTransfers();
    transfers->media.clear();

    QSettings settings("cepiperez","fileboxplus");
    QStringList media = settings.value("Transfers", QStringList()).toStringList();

    for (int i=0; i<media.count(); ++i)
    {
        QStringList m = media.at(i).split("||");
        transfers->addFile(m.at(0), m.at(2), m.at(3), m.at(1), m.at(5));
        QString exten = getPreview(m.at(0), m.at(3), m.at(2), m.at(1));
        emit addTransferFile(m.at(0), m.at(1), m.at(2), m.at(3), m.at(4), m.at(5), exten);
    }

    emit loadTransfersCompleted(media.count());

    QStringList cmedia = settings.value("CompletedTransfers", QStringList()).toStringList();

    for (int i=0; i<cmedia.count(); ++i)
    {
        QStringList m = cmedia.at(i).split("||");
        QString exten = getPreview(m.at(0), m.at(3), m.at(2), m.at(1));
        emit addTransferFile(m.at(0), m.at(1), m.at(2), m.at(3), m.at(4), m.at(5), exten, true);
    }


}

void Utilities::cleanCompleted()
{
    QSettings settings("cepiperez","fileboxplus");

    QStringList cmedia = settings.value("CompletedTransfers", QStringList()).toStringList();

    for (int i=0; i<cmedia.count(); ++i)
    {
        QStringList m = cmedia.at(i).split("||");
        emit removeTransferFile(m.at(3), true);
    }

    settings.remove("CompletedTransfers");
    settings.sync();
    //loadTransfers();
}

void Utilities::transferFile(QString service, QString mode, QString name, QString link, QString filesize, QString realsize)
{
    QSettings settings("cepiperez","fileboxplus");
    bool add = true;

    QStringList media = settings.value("Transfers", QStringList()).toStringList();

    for (int i=0; i<media.count(); ++i)
    {
        QStringList m = media.at(i).split("||");
        if (m.at(3) == link) {
            add = false;
            break;
        }
    }

    if (add)
    {
        /*double freeSpace = spaceTotal - spaceUsed;
        if ( mode=="upload" && QFileInfo(link).size()>freeSpace )
        {
            emit d->q->showBanner("Not enough space to upload the file!");
            return;
        }*/

        //QString exten = getPreview(service, link, name, mode);

        QString t = service + "||" + mode + "||" + name+ "||" + link + "||" + filesize + "||" + realsize;
        media.append(t);
        settings.setValue("Transfers", media);
        settings.sync();

        transfers->addFile(service, name, link, mode, realsize);

        //emit addTransferFile(service, mode, name, link, filesize, realsize, exten);
        loadTransfers();

        if (transfers->media.count()>0 && !transfers->working) {
            transfers->startTranfers();
        }
    }

}

QString Utilities::getPreview(QString service, QString link, QString filename, QString mode)
{
    QString preview = "";
    if (mode=="download")
    {
        QCryptographicHash md(QCryptographicHash::Md5);
        QString file = service + "/" + link;
        file.replace("//","/");
        md.addData(file.toUtf8());
        QString ef = ".jpg";
        if (filename.endsWith(".png") || service=="Box") ef = ".png";
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

void Utilities::removeTransfer(QString link, bool completed)
{
    QSettings settings("cepiperez","fileboxplus");

    qDebug() << "Removing transfer: " << link;

    QString key;

    if (!completed)
        key = "Transfers";
    else
        key = "CompletedTransfers";

    QStringList media = settings.value(key, QStringList()).toStringList();

    for (int i=0; i<media.count(); ++i)
    {
        QStringList m = media.at(i).split("||");
        if (m.at(3) == link) {
            media.removeAt(i);
            if (!completed) transfers->removeFile(link);
            break;
        }
    }

    if (media.count()>0)
        settings.setValue(key, media);
    else
        settings.remove(key);

    settings.sync();

    emit removeTransferFile(link, completed);
    emit loadTransfersCompleted(transfers->media.count());

}

void Utilities::setWorking(bool working)
{
    m_working = working;
    emit workingChanged();
}

void Utilities::startTransfers()
{
    transfers->startTranfers();
}

void Utilities::stopTransfers()
{
    transfers->stopTransfers();
}

void Utilities::fileTransfered(QString link)
{
    QSettings settings("cepiperez","fileboxplus");

    QStringList media = settings.value("Transfers", QStringList()).toStringList();
    QStringList completedmedia = settings.value("CompletedTransfers", QStringList()).toStringList();

    QStringList x;

    for (int i=0; i<media.count(); ++i)
    {
        QStringList m  = media.at(i).split("||");
        if (m.at(3) == link) {
            x = m;
            completedmedia.append(media.at(i));
            media.removeAt(i);
            break;
        }
    }

    if (media.count()>0)
        settings.setValue("Transfers", media);
    else
        settings.remove("Transfers");

    settings.setValue("CompletedTransfers", completedmedia);

    settings.sync();

    emit removeTransferFile(link, false);
    QString exten = getPreview(x.at(0), x.at(3), x.at(2), x.at(1));
    emit addTransferFile(x.at(0), x.at(1), x.at(2), x.at(3), x.at(4), x.at(5), exten, true);
    //loadTransfers();

    //qDebug() << "TRANSFERS: " << transfers->media;
    transfers->media.removeFirst();
    emit loadTransfersCompleted(transfers->media.count());
    //qDebug() << "TRANSFERS: " << transfers->media;

    if (transfers->media.count()>0)
        transfers->startTranfers();
    else
        emit showBanner(tr("All transfers have been completed"));
}

void Utilities::extractFiles(QString file, QString password)
{
    emit utilWorking(true, "");
    QSettings settings("cepiperez","fileboxplus");
    zipFile = file;
    zipBase = settings.value("ExtractFolder", "/home/nemo").toString();
    copyDest = zipBase;
    startExtracting(zipFile, zipBase, password);
}

void Utilities::startExtracting(QString file1, QString base, QString password)
{
    QSettings settings("cepiperez","fileboxplus");
    QString tf = base + "/filebox-check";
    QFile destFile(tf);
    if (!destFile.open(QIODevice::WriteOnly))
    {
        emit bannerChanged(tr("You don't have permissions to proceed"), false);
        emit utilWorking(false, "");
        return;
    }
    destFile.remove();

    //QString base = settings.value("ExtractFolder","/home/nemo/Documents").toString();
    zipProc->setWorkingDirectory(base);
    zipcanceled = 0;
    zipProcCMD = "extract";

    QStringList arguments;
    QString compresor;
    QString keep = settings.value("KeepOldFiles","No").toString();

    QFileInfo fileInfo(file1);
    QString exten = fileInfo.fileName().toLower();
    //int ei = exten.lastIndexOf( "." );
    //exten.remove ( 0, ei + 1 );

    if ( exten.endsWith(".zip") || exten.endsWith(".apk") ) {
        compresor = "unzip";
        if ( keep=="Yes") arguments << "-n" << "-P" << password << file1 << "-d" << base;
        else arguments << "-o" << "-P" << password << file1 << "-d" << base;
    }
    else if ( exten.endsWith(".rar") )
    {
        compresor = "unrar";
        if ( password == "" ) password="-";
        QString pass2 = "-p" + password;
        if ( keep=="Yes") arguments << "x" << "-o-" << pass2 << "-idp" << file1 << base;
        else arguments << "x" << "-o+" << pass2 << "-idp" << file1 << base;
    }
    else if ( exten.endsWith(".tar") )
    {
        compresor = "tar";
        if ( keep=="Yes") arguments << "-xvf" << file1 << "-C" << base;
        else arguments << "-xvf" << file1 << "-C" << base;
    }
    else if ( exten.endsWith(".tar.gz") )
    {
        compresor = "tar";
        if ( keep=="Yes") arguments << "-xzvf" << file1 << "-C" << base;
        else arguments << "-xzvf" << file1 << "-C" << base;
    }
    else if ( exten.endsWith(".tar.bz2") )
    {
        compresor = "tar";
        if ( keep=="Yes") arguments << "-xjvf" << file1 << "-C" << base;
        else arguments << "-xjvf" << file1 << "-C" << base;
    }

    zipProc->start(compresor, arguments);

}


void Utilities::pfinished(int code)
{
    qDebug() << "PROCESS FINISHED! - Code: " << code;
    //qDebug() << d->zipProc->readAllStandardError();
    QString error = zipProc->readAllStandardError();
    qDebug() << error;

    if ( zipProcCMD == "extract" )
    {
        if ( zipcanceled == 1 )
            emit bannerChanged(tr("Extracting process canceled"), true);
        else
        {
            if ( error.contains("incorrect password") )
                emit bannerChanged(tr("Extraction error: incorrect password"), false);
            else
                emit bannerChanged(tr("Extraction completed"), true);
        }
    }
    else if ( zipProcCMD == "compress" )
    {
        if ( zipcanceled == 1 )
            emit bannerChanged(tr("Compressing process canceled"), true);
        else
            emit bannerChanged(tr("Compression completed"), true);
    }
    emit utilWorking(false, copyDest);

}

void Utilities::cancelExtracting()
{
    zipFile = "";
    zipBase = "";
    emit bannerChanged(tr("Extracting process canceled"), true);
    emit utilWorking(false, copyDest);
}

void Utilities::stopExtracting()
{
    zipcanceled = 1;
    zipProc->terminate();
}

void Utilities::compressFiles(QString files, QString curpath, QString tarfile, QString format)
{
    emit utilWorking(true, "");
    QStringList myfiles = files.split("<filecaseseparator>");
    QStringList newFiles;

    for (int i=0; i<myfiles.count(); ++i)
    {
        QString f = myfiles.at(i);
        f.remove(curpath+"/");
        newFiles.append(f);
    }

    qDebug() << "Files: " << newFiles;
    qDebug() << "Path: " << tarfile;
    qDebug() << "Format: " << format;

    copyDest = curpath;

    zipProc->setWorkingDirectory(curpath);
    zipcanceled = 0;
    zipProcCMD = "compress";

    QStringList arguments;
    QString proc;

    if ( (format=="tar") || (format=="tar.gz") || (format=="tar.bz2") )
        proc = "tar";
    else if ( format=="zip" )
        proc = "zip";

    if ( format == "tar")
        arguments << "-cvf" << tarfile+"."+format << newFiles;
    else if ( format == "tar.gz")
        arguments << "-czvf" << tarfile+"."+format << newFiles;
    else if ( format == "tar.bz2")
        arguments << "-cjvf" << tarfile+"."+format << newFiles;
    else if ( format == "zip")
        arguments << "-r" << tarfile+"."+format << newFiles;

    zipProc->start(proc, arguments);

}


void Utilities::checkClipboard(QString action, QString dest, QString autoremove)
{
    QSettings settings("cepiperez","fileboxplus");
    autoremove = autoremove;
    QStringList entries = settings.value("ClipboardSelected","").toStringList();
    copyDest = dest;
    settings.sync();

    if (entries.count()==0)
    {
        return;
    }

    clipboardAction = action;

    qDebug() << "CHECKING CLIPBOARD: " << entries << " TO " << copyDest
                 << " - AUTOREMOVE: " << autoremove;
    if ( entries.count() > 0 )
        CopyFiles(entries);

}

void Utilities::CopyFiles(QStringList files)
{
    emit utilWorking(true, "");

    QStringList entries = files;

    QString tf = copyDest + "/filebox-check";
    QFile destFile(tf);
    if (!destFile.open(QIODevice::WriteOnly))
    {
        emit bannerChanged(tr("You don't have permissions to proceed"), false);
        emit utilWorking(false, "");
        return;
    }

    destFile.remove();

    struct statfs stat;
    double freeSize = 0;
    double TargetSize = 0;
    QByteArray ba = copyDest.toUtf8();
    const char *str1 = ba.data();
    statfs ( str1, &stat );
    freeSize = stat.f_bsize*stat.f_bfree;
    for (int fi=0; fi < entries.count(); ++fi)
    {
        if ( QFileInfo(entries[fi]).isFile() ) TargetSize += QFileInfo(entries[fi]).size();
        else TargetSize += GetFolderSize( entries[fi] );
    }
    if ( freeSize < TargetSize )
    {
        emit bannerChanged(tr("There's no enough space to proceed"), false);
        emit utilWorking(false, "");
        return;
    }

    if ( clipboardAction == "link" )
    {
        if ( ! QFile::link(entries.at(0), copyDest + "/filebox-checking") )
        {
            emit bannerChanged(tr("Cannot create symlinks in destination path"), false);
            emit utilWorking(false, "");
            return;
        }
    }
    QFile(copyDest + "/filebox-checking").remove();

    copycanceled = 0;
    //qDebug() << "ACTION: " << d->clipboardAction;
    copySkipAll = 0;
    copyOverwriteAll = 0;

    fileCopier = new QtFileCopier();
    connect(fileCopier, SIGNAL(finished(int, bool)), this, SLOT(copyFinished(int, bool)));
    connect(fileCopier, SIGNAL(done(bool)), this, SLOT(copyDone(bool)));
    connect(fileCopier, SIGNAL(error(int,QtFileCopier::Error,bool)), this, SLOT(copyError(int,QtFileCopier::Error,bool)));

    if ( clipboardAction == "link" )
        fileCopier->copyFiles(entries, copyDest, QtFileCopier::MakeLinks);
    else if ( clipboardAction == "move" )
        fileCopier->moveFiles(entries, copyDest);
    else if ( clipboardAction == "copy" )
        fileCopier->copyFiles(entries, copyDest);

}

void Utilities::copyFinished(int, bool)
{
    QSettings settings("cepiperez","fileboxplus");
    //qDebug() <<  "ID: " << id << " - RES:" << res;
    QStringList entries = settings.value("ClipboardSelected","").toStringList();

    for (int i=0; i<entries.count(); ++i)
    {
        QString destfile = copyDest + "/" + QFileInfo(entries.at(i)).fileName();
        if ( QFileInfo(destfile).exists() )
        {
            qDebug() << "COPYING OK: " << destfile;
            /*if ( d->clipboardAction=="move")
            {
                if (QFileInfo(destfile).size()==QFileInfo(entries.at(i)).size())
                    removeFile(entries.at(i));
            }*/
            if ( autoremove=="true")
            {
                //qDebug() << "REMOVING FROM CLIPBOARD: " << entries.at(i);
                QStringList clip = settings.value("Clipboard","").toStringList();
                clip.removeAll( entries.at(i) );
                settings.setValue("Clipboard", clip);
                settings.sync();
            }
        }
        else
            qDebug() << "ERROR COPYING: " << destfile;
    }
    emit utilWorking(false, copyDest);
}

void Utilities::copyDone(bool res)
{
    disconnect(fileCopier, SIGNAL(finished(int, bool)), this, SLOT(copyFinished(int, bool)));
    disconnect(fileCopier, SIGNAL(done(bool)), this, SLOT(copyDone(bool)));
    disconnect(fileCopier, SIGNAL(error(int,QtFileCopier::Error,bool)), this, SLOT(copyError(int,QtFileCopier::Error,bool)));
    delete fileCopier;

    if ( copycanceled==1 )
        return;

    if (res == true)
        emit bannerChanged(tr("There were errors copying files!"), false);
    else
        emit bannerChanged(tr("Copying process is completed"), true);

    emit utilWorking(false, copyDest);

}

void Utilities::stopCopying()
{
    copycanceled = 1;
    fileCopier->cancelAll();

    /*d->CMDproc->terminate();
    disconnect(d->CMDproc,0,0,0);


    if (d->clipboardAction=="copy" || d->clipboardAction=="move") {
        d->copySources.clear();
        QString file = d->currentCopyDest + "/" + QFileInfo(d->currentCopySource).fileName();
        qDebug() << "Removing unfinished file: " << file;
        QStringList args;
        args << "123456789abcd" << "delete" << file;
        d->CMDproc->start("/opt/FileboxPlus/bin/FileboxCMD", args);
    }*/

    emit bannerChanged(tr("Copying process canceled"), true);
    emit utilWorking(false, copyDest);
}

void Utilities::copyError(int id, QtFileCopier::Error error, bool stopped)
{
    //qDebug() << id << error << stopped;
    QString file = fileCopier->destinationFilePath(id) + "/" + QFileInfo(fileCopier->sourceFilePath(id)).fileName();
    qDebug() << "Current file: " << file;
    if ( error == QtFileCopier::DestinationExists ) {
        if ( copySkipAll == 1 )
        {
            //qDebug() << "skipping all files...";
            fileCopier->skip();
        }
        else
        {

            //copyMsg1 = QFileInfo(d->fileCopier->sourceFilePath(id)).fileName();
            //copyMsg2 = tr("already extists in destination folder");
            //emit copyDialog();
        }
    }
    else {
        fileCopier->skip();
    }
}

void Utilities::getTextFile(QString filename)
{
    /*QString lines;
    QFile data(filename);
    if (data.open(QFile::ReadOnly | QFile::Truncate))
    {
        QTextStream out(&data);
        while ( !out.atEnd() )
            lines += out.readLine()+"\n";
    }
    data.close();
    return lines;*/
    pepe->setAction("readtext");
    QStringList filelist;
    filelist << filename;
    pepe->setFiles(filelist);
    pepe->start();
    pepe->setPriority(QThread::IdlePriority);

}

void Utilities::saveTextFile(QString filename, QString filedata)
{
    if ( QFileInfo(filename).exists() )
        QFile::remove(filename);
    QFile file(filename);
    file.open( QIODevice::Truncate | QIODevice::Text | QIODevice::ReadWrite);
    QTextStream out(&file);
    out << filedata;
    file.close();
}
