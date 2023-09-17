#include "sys/vfs.h"
#include "browser.h"
//#include "loadimage.h"
#include "qtfilecopier.h"
#include "filedeleter.h"
#include "utils.h"
#include "config.h"

//#include <gconf/2/gconf/gconf.h>
//#include <gconf/2/gconf/gconf-client.h>

#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QDateTime>
#include <QProcess>
#include <QDebug>
#include <QIcon>
#include <QStringList>
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QUrl>
#include <QVariantMap>

QSettings settings("cepiperez", "fileboxplus");
bool showHiddenFiles;
bool showRootFileSystem;

struct BrowserItem
{
    QString name;
    QString date;
    QString size;
    QString path;
    QString sec;
    QString exten;
    QString link;
    Browser::ItemType type;
};

class Browser::BrowserPrivate
{
public:
    BrowserPrivate(Browser * parent);
    ~BrowserPrivate();
    void populateItems();
    QList <BrowserItem * > items;
    QString path;
    Browser * const q;
    QFileSystemWatcher * watcher;
    //Thread *pepe;

    QString autoremove;
    QString clipboardAction;
    QStringList multiSelItems;
    int sortIndex;
    int sortOrder;
    int FoldersFirst;
    QString lastpath;
    int mlast;
    //QDBusInterface *iface;
    QString copyMsg1, copyMsg2;
    QString currentCopySource, currentCopyDest;
    QStringList copySources;
    QProcess* CMDproc;
    int cleaningCache;
    QString filter;

};

Browser::BrowserPrivate::BrowserPrivate(Browser * parent) :
        q(parent)
{
    lastpath = "";
    mlast = 0;
    filter = "";

    sortOrder = settings.value("SortOrder", "0").toInt();
    sortIndex = settings.value("SortIndex", "0").toInt();
    FoldersFirst = settings.value("FoldersFirst", "1").toInt();
    showHiddenFiles = settings.value("ShowHiddenFiles", "false").toString()== "true";

    //watcher = new QFileSystemWatcher(parent);
    //connect(watcher, SIGNAL(directoryChanged(const QString)), parent, SLOT(dirChanged(const QString)));

    CMDproc = new QProcess(parent);
    CMDproc->setProcessChannelMode(QProcess::ForwardedChannels);
    //connect(zipProc, SIGNAL( readyRead() ), parent, SLOT( preaded() ) );

}

Browser::BrowserPrivate::~BrowserPrivate()
{
    while(!items.isEmpty())
    {
        delete items.takeFirst();
    }
}

void Browser::dirChanged(const QString )
{
    clearItems();
    d->populateItems();

}

void Browser::showThumbnails(QString data)
{
    if ( data != settings.value("ShowThumbnails") )
    {
        settings.setValue("ShowThumbnails", data);
        settings.sync();
        clearItems();
        d->populateItems();
    }
}

void Browser::BrowserPrivate::populateItems()
{
    int cindex=0;
    mlast = 0;
    q->clearSelection();

    q->StartThread = 0;

    if ( path == "" )
        path = "/";

    QDir dir (path);

    QStringList filters;
    if ( filter == "*" )
        filters << "";
    else
        filters << "*" + filter + "*";

    qDebug() << "Populating items - Folder: " << path;

    QString dateFormat = settings.value("DateFormat","mm/dd/yyyy").toString().replace("mm","MM");

    //qDebug() << "FILTER: " << filters;
    dir.setNameFilters(filters);

    showHiddenFiles = settings.value("ShowHiddenFiles", "false").toString()== "true";

    QFileInfoList entries;
    if ( ! showHiddenFiles )
        entries = dir.entryInfoList(QDir::AllEntries | QDir::System | QDir::NoDotAndDotDot ,
                                    ( sortIndex==0 ? QDir::Name : sortIndex==1 ? QDir::Size : QDir::Time )
                                    | ( sortOrder==1 ? QDir::Reversed : QDir::IgnoreCase )
                                    | ( FoldersFirst==1 ? QDir::DirsFirst : QDir::IgnoreCase )
                                    | QDir::IgnoreCase);
    else
        entries = dir.entryInfoList(QDir::AllEntries | QDir::System | QDir::Hidden | QDir::NoDotAndDotDot ,
                                    ( sortIndex==0 ? QDir::Name : sortIndex==1 ? QDir::Size : QDir::Time )
                                    | ( sortOrder==1 ? QDir::Reversed : QDir::IgnoreCase )
                                    | ( FoldersFirst==1 ? QDir::DirsFirst : QDir::IgnoreCase )
                                    | QDir::IgnoreCase);

    //int entriesCount = entries.count();

    emit q->beginResetModel();

    q->beginInsertRows(QModelIndex(), 0, entries.count()-1);


    //QCoreApplication::processEvents();

    for (int i=0; i<entries.count(); ++i)
    {
        cindex = cindex + 1;

        QFileInfo fileInfo(entries.at(i));

        QString name = fileInfo.fileName();

        BrowserItem * item = new BrowserItem();
        item->name = name;

        item->date = QString("%1 %2").arg(fileInfo.lastModified().date().toString(dateFormat)).arg(fileInfo.lastModified().time().toString("HH:mm:ss"));
        item->size = QString("%1").arg(GetFileSize(fileInfo.size()));

        if ( sortIndex==0 )
        {
            item->sec = name.left(1).toUpper();
        }
        else if ( sortIndex==1 )
        {
            if ( fileInfo.size() < 1024*100 )
                item->sec = "< 100 k";
            else if ( fileInfo.size() < 1024*1024 )
                item->sec = "< 1 Mb";
            else if ( fileInfo.size() < 1024*1024*10 )
                item->sec = "< 10 Mb";
            else if ( fileInfo.size() < 1024*1024*100 )
                item->sec = "< 100 Mb";
            else if ( fileInfo.size() < 1024*1024*1024 )
                item->sec = "< 1 Gb";
            else
                item->sec = "> 1 Gb";

        }
        else if ( sortIndex==2 ) {

            item->sec = QString("%1").arg(fileInfo.lastModified().date().toString("MM/dd/yyyy"));
        }

        if ( fileInfo.isSymLink() )
            item->link = "yes";
        else
            item->link = "no";

        if ( fileInfo.isDir() )
            item->exten = "folder";
        else
        {
            if (settings.value("ShowThumbnails","false")=="true")
            {
                if ( name.toLower().endsWith(".jpg") )
                    item->exten = "image/jpeg";
                else if ( name.toLower().endsWith(".jpeg") )
                    item->exten = "image/jpeg";
                else if ( name.toLower().endsWith(".png") )
                    item->exten = "image/png";
                else if ( name.toLower().endsWith(".gif") )
                    item->exten = "image/gif";
                else if ( name.toLower().endsWith(".avi") )
                    item->exten = "video/x-msvideo";
                else if ( name.toLower().endsWith(".mp4") )
                    item->exten = "video/mp4";
                else if ( name.toLower().endsWith(".wmv") )
                    item->exten = "video/x-msvideo";
                else if ( name.toLower().endsWith(".3gp") )
                    item->exten = "video/3gpp";
                else
                    item->exten = GetExtension(name.toLower());

            } else {
                item->exten = GetExtension(name.toLower());
            }

        }

        item->path = fileInfo.absoluteFilePath();

        if ( item->path == lastpath )
            mlast = cindex;

        if(!fileInfo.isDir())
            item->type = Browser::File;
        else
            item->type = Browser::Folder;

        //QCoreApplication::processEvents();

        items.append(item);

    }


    q->endInsertRows();

    emit(q->countChanged());

    //emit(q->dataChanged(q->index(0) , q->index(entriesCount - 1)));

    emit q->endResetModel();

    /*if ( ! path.contains("non-existent-folder") )
    {
        QStringList wdirs = watcher->directories();
        if ( wdirs.count() > 0 ) watcher->removePaths(wdirs);
        watcher->addPath( path );
    }*/

}


Browser::Browser(QObject * parent) :
        QAbstractListModel(parent) , d (new BrowserPrivate(this))
{
    QHash <int, QByteArray> roles;
    roles.insert(NameRole , "name");
    roles.insert(DateRole , "date");
    roles.insert(SizeRole , "size");
    roles.insert(PathRole , "path");
    roles.insert(ExtenRole , "exten");
    roles.insert(SecRole , "sec");
    roles.insert(LinkRole , "link");
    roles.insert(TypeRole , "type");
    d->q->setRoleNames(roles);
}

QHash<int, QByteArray> Browser::roleNames() const {
  return m_roles;
}

void Browser::setRoleNames(const QHash<int, QByteArray>& roles) {
  m_roles = roles;
}


Browser::~Browser()
{
    delete d;
}

int Browser::rowCount(const QModelIndex & parent) const
{
    Q_UNUSED(parent)
    return d->items.size();
}

QVariant Browser::data(const QModelIndex & index , int role) const
{
    if(index.row() < 0 or index.row() > count())
    {
        return QVariant();
    }

    BrowserItem * item = d->items.at(index.row());
    switch(role)
    {
    case NameRole:
        return item->name;
        break;
    case DateRole:
        return item->date;
        break;
    case SizeRole:
        return item->size;
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
    case LinkRole:
        return item->link;
        break;
    case TypeRole:
        return item->type;
        break;
    default:
        return QVariant();
        break;
    }
}

int Browser::count() const
{
        return rowCount();
}

QString Browser::path() const
{
    QString tmp = d->path;
    if ( tmp.startsWith("//") )
            tmp.remove(0,1);
    d->path = tmp;
    return d->path;
}

QVariantMap Browser::get(int index)
{
    QVariantMap res;
    QString path = d->multiSelItems.at(index);

    for (int i=0; i< d->items.count(); ++ i)
    {
        if ( d->items.at(i)->path == path )
        {
            res.insert("name", QFileInfo(d->items.at(i)->path).fileName());
            res.insert("path", QFileInfo(d->items.at(i)->path).absoluteFilePath());
            res.insert("size", QString::number(QFileInfo(d->items.at(i)->path).size()));
            res.insert("tsize", GetFileSize(QFileInfo(d->items.at(i)->path).size()));
            break;
        }
    }
    return res;
}

QString Browser::cname(int index)
{
    QString result;
    if ( (d->items.at(index)->type==Browser::Folder) || (d->items.at(index)->type==Browser::FolderSel) )
        result = "a";
    else
        result = "b";

    /*if ( d->sortIndex == 0)
    {
        result += d->items.at(index)->name.toUpper();
    }

    else if ( d->sortIndex == 1)
    {
        result += d->items.at(index)->size;

    }
    else if ( d->sortIndex == 2)
    {
        QString r =  d->items.at(index)->date;
        int i = r.indexOf(" ");
        r.remove(i, r.length()-i);
        result += r;
    }*/
    result += d->items.at(index)->sec;

    return result;
}

QString Browser::path1() const
{
    return QFileInfo(d->path).fileName();
}

QString Browser::path2() const
{
    return QFileInfo(d->path).absolutePath();
}

int Browser::last() const
{
    return d->mlast;
}

void Browser::reload()
{
    clearItems();
    d->sortOrder = settings.value("SortOrder", "0").toInt();
    d->sortIndex = settings.value("SortIndex", "0").toInt();
    d->FoldersFirst = settings.value("FoldersFirst", "1").toInt();
    d->populateItems();
}

void Browser::setPath(const QString & path)
{
    //if(d->path != path)
    //{
        clearItems();
        d->path = path;
        emit(pathChanged());
        clearItems();
        d->populateItems();
    //}
    //QCoreApplication::processEvents();
}

void Browser::goUp()
{
    clearItems();
    d->lastpath = d->path;
    //qDebug() << "LASTPATH: " << d->lastpath;
    QString tmp = d->path;
    int i = tmp.lastIndexOf("/");
    tmp.remove(i, tmp.length()-i);
    d->path = tmp;
    clearItems();
    d->populateItems();
    emit(pathChanged());
}

void Browser::showRootFiles(bool show)
{
    showRootFileSystem = show;
    if ( !d->path.contains(Config::getHome()) && show==false )
        setPath(Config::getHome());
}

void Browser::showHidden(bool show)
{
    if (showHiddenFiles != show)
    {
        showHiddenFiles = show;
        clearItems();
        d->populateItems();
    }
}

void Browser::addToClipboard(const QString &file, QString autoselect)
{
    QStringList list = settings.value("Clipboard", "").toStringList();

    if (file=="selectedfiles")
    {
        for (int i=0; i<d->multiSelItems.count(); ++i)
            addToClipboard(d->multiSelItems.at(i), autoselect);
    }
    else
    {
        QString tmp = file;
        tmp.replace("//", "/");
        if ( ! list.contains(tmp) )
            list.append(tmp);
        settings.setValue("Clipboard", list);
        if ( autoselect == "true" )
        {
            QStringList slist = settings.value("ClipboardSelected", "").toStringList();
            if ( ! slist.contains(tmp) )
                slist.append(tmp);
            settings.setValue("ClipboardSelected", slist);
        }
    }
    settings.sync();
}

void Browser::addToStandardClipboard(const QString &file, QString autoselect)
{
    QStringList list = settings.value("StandardClipboard", "").toStringList();

    if (file=="selectedfiles")
    {
        for (int i=0; i<d->multiSelItems.count(); ++i)
            addToStandardClipboard(d->multiSelItems.at(i), autoselect);
    }
    else
    {
        QString tmp = file;
        tmp.replace("//", "/");
        if ( ! list.contains(tmp) )
            list.append(tmp);
        settings.setValue("StandardClipboard", list);
    }
    settings.sync();
}

/*void Browser::checkStandardClipboard(QString action)
{
    d->autoremove = "true";
    QStringList entries = settings.value("StandardClipboard","").toStringList();
    settings.setValue("DestPath", d->path);
    settings.sync();

    if (entries.count()==0)
    {
        //m_banner = "";
        //emit bannerChanged();
        return;
    }

    d->clipboardAction = action;

    qDebug() << "CHECKING STANDARD CLIPBOARD: " << entries << " TO " << d->path
                 << " - AUTOREMOVE: " << d->autoremove;
    if ( entries.count() > 0 )
        CopyFiles(entries);

}*/

void Browser::renameFile(const QString &file, const QString &newf)
{
    QString realname = file;
    if ( realname.startsWith("//") ) realname.remove(0, 1);
    QString newname = QFileInfo(file).absolutePath() + "/" + newf;
    if ( newname.startsWith("//") ) newname.remove(0, 1);
    QFileInfo sel = realname;
    bool res = false;

    if ( (sel.isDir()) && (sel.isWritable()) ) {
        QDir selected = realname;
        res = selected.rename(realname, newname);
    }
    if ( (sel.isFile()) && (sel.isWritable()) ) {
        QFile selected;
        res = selected.rename(realname, newname);
    }

    qDebug() << "RENAMING: " << realname << " TO " << newname << "... " << res;

    if ( res == true ) {
        emit d->q->fileRenamed(QFileInfo(newname).fileName());
        clearItems();
        d->populateItems();
    }

}

void Browser::removeFile(const QString &file, bool start)
{
    //qDebug() << "rem " << file;

    if (start)
        workErrors = 0;

    if ( file == "selectedfiles" )
    {
        //qDebug() << "Removing " << d->multiSelItems;
        for (int i=0; i<d->multiSelItems.count(); ++i)
            removeFile(d->multiSelItems.at(i));
    }
    else
    {
        //qDebug() << "Removing " << file;
        bool res;
        if ( QFileInfo(file).isSymLink() || QFileInfo(file).isFile() )
        {
            QFile f(file);
            res = f.remove();
            if (!res)
                ++workErrors;
        }
        else
        {
            QDir dir(file);
            if (dir.exists(file))
            {
                Q_FOREACH(QFileInfo info, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden  | QDir::AllDirs | QDir::Files, QDir::DirsFirst))
                {
                    if (info.isDir()) {
                        removeFile(info.absoluteFilePath());
                    } else {
                        QFile f(info.absoluteFilePath());
                        res = f.remove();
                        if (!res)
                            ++workErrors;
                    }
                }
                res = dir.rmdir(file);
                if (!res)
                    ++workErrors;
            }

        }
    }

    if (!start)
        return;

    if ( d->cleaningCache == 1 )
        return;

    if (workErrors==0)
        emit bannerChanged(tr("Delete process completed"), true);
    else
        emit bannerChanged(tr("There were errors deleting files!"), false);

    clearItems();
    d->populateItems();

}

void Browser::dbg(const QString &data)
{
    qDebug() << data;
}

void Browser::newFolder(QString name)
{
    QDir dir(d->path);
    dir.mkdir(name);
}


/*void Browser::copyFile1()
{
    qDebug() << d->clipboardAction << d->currentCopySource << d->currentCopyDest;
    QStringList args;
    args << "123456789abcd" << d->clipboardAction << d->currentCopySource << d->currentCopyDest;
    disconnect(d->CMDproc,0,0,0);
    connect(d->CMDproc, SIGNAL( finished(int) ), d->q, SLOT( cmdfinished(int) ) );
    d->CMDproc->start("/opt/FileboxPlus/bin/FileboxCMD", args);
    //d->CMDproc->waitForFinished(-1);
}*/

/*void Browser::cmdfinished(int)
{
    d->copySources.removeFirst();
    if (d->copySources.count() == 0)
    {
        emit bannerChanged(tr("Copying process is completed"), true);
        clearItems();
        d->populateItems();
    }
    else
    {
        d->currentCopySource = d->copySources.at(0);

        QString file = d->currentCopyDest + "/" + QFileInfo(d->currentCopySource).fileName();
        if (QFileInfo(file).exists() && d->copyOverwriteAll==0)
        {
            if (d->copySkipAll==1) {
                d->q->cmdfinished(0);
            }
            else
            {
                qDebug() << "Current file exists: " << file;
                d->copyMsg1 = QFileInfo(d->currentCopySource).fileName();
                d->copyMsg2 = tr("already extists in destination folder");
                emit copyDialog();
            }
        }
        else
        {
            d->q->copyFile1();
        }

    }

}*/



/*void Browser::CopyDialogResult(int result)
{
    //qDebug() << result;

    if ( result == 1 )
        d->copySkipAll = 1;
    else
        d->copySkipAll = 0;

    if ( result == 4 )
    {
        d->fileCopier->cancelAll();
        //d->copySources.clear();
        emit bannerChanged(tr("Copying process canceled"), true);
    }
    else
    {
        if ( result == 0 )
        {
            d->fileCopier->skip();
            //qDebug() << "Skip file";
            //cmdfinished();
        }
        else if ( result == 1 )
        {
            d->fileCopier->skipAll();
            //qDebug() << "Skip all files";
            //d->copySkipAll = 1;
            //cmdfinished();
        }
        else if ( result == 2 )
        {
            d->fileCopier->overwrite();
            //qDebug() << "Overwrite file";
           // d->q->copyFile1();
        }
        else if ( result == 3 )
        {
            d->fileCopier->overwriteAll();
            //qDebug() << "Overwrite all files";
            //d->copyOverwriteAll = 1;
            //d->q->copyFile1();
        }
    }

}*/

QString Browser::copyMessage1() const
{
    return d->copyMsg1;

}

QString Browser::copyMessage2() const
{
    return d->copyMsg2;

}


/*QString Browser::banner() const
{
    return m_banner;
}*/

void Browser::openTerminal(QString folder)
{
    //qDebug() << folder;
    QProcess * process = new QProcess();
    connect(process , SIGNAL(finished(int)) , process , SLOT(deleteLater()));
    process->setWorkingDirectory(folder);
    process->start("meego-terminal");
}

void Browser::cleanCache()
{
    d->cleaningCache = 1;
    qDebug() << "CLEANING THUMBNAILS CACHE";
    removeFile(Config::getHome() + "/.thumbnails/filecase");
    d->cleaningCache = 0;
    //d->iface->call("Cleanup");
}

void Browser::clearSelection()
{
    d->multiSelItems.clear();
    for (int i=0; i< d->items.count(); ++ i)
    {
        if ( d->items.at(i)->type == Browser::FileSel )
            d->items.at(i)->type = Browser::File;
        else if ( d->items.at(i)->type == Browser::FolderSel )
            d->items.at(i)->type = Browser::Folder;
    }
    m_multi1 = 0;
    m_multi2 = 0;
    m_multi3 = 0;

    emit(d->q->dataChanged(d->q->index(0) , d->q->index(d->items.count() - 1)));

}

void Browser::selectItem(QString name)
{
    //d->multiSelItems.clear();
    for (int i=0; i< d->items.count(); ++ i)
    {
        if ( d->items.at(i)->path == name )
        {
            if ( d->items.at(i)->type == Browser::File ) {
                d->items.at(i)->type = Browser::FileSel;
                d->multiSelItems.append(name);
                ++m_multi1;
                ++m_multi2;
            }
            else if ( d->items.at(i)->type == Browser::FileSel ) {
                d->items.at(i)->type = Browser::File;
                d->multiSelItems.removeAll(name);
                --m_multi1;
                --m_multi2;
            }
            else if ( d->items.at(i)->type == Browser::Folder ) {
                d->items.at(i)->type = Browser::FolderSel;
                d->multiSelItems.append(name);
                ++m_multi1;
                ++m_multi3;
            }
            else if ( d->items.at(i)->type == Browser::FolderSel ) {
                d->items.at(i)->type = Browser::Folder;
                d->multiSelItems.removeAll(name);
                --m_multi1;
                --m_multi3;
            }

            emit(dataChanged(index(i) , index(i)));

            break;
        }

    }

    //m_multi1: cantidad total de archivos y carpetas seleccionados
    //m_multi2: cantidad de archivos seleccionados
    //m_multi3: cantidad de carpetas seleccionadas
    //qDebug() << "EMITTING multiChanged(): " << m_multi1 << " " << m_multi2 << " " << m_multi3 ;
    emit multiChanged();
}

int Browser::xterm()
{
    if ( QFileInfo("/usr/bin/meego-terminal").exists() )
        return 1;
    else
        return 0;
}

int Browser::multi1() const
{
    return m_multi1;
}

int Browser::multi2() const
{
    return m_multi2;
}

int Browser::multi3() const
{
    return m_multi3;
}

void Browser::infoFiles(QString name)
{
    if (name == "selectedfiles")
        settings.setValue("InfoFiles", d->multiSelItems);
    else
        settings.setValue("InfoFiles", name);
    settings.sync();
}

void Browser::setSortOrder(QString name, QString order, QString first)
{
    d->sortIndex = name.toInt();
    d->sortOrder = order.toInt();
    d->FoldersFirst = first.toInt();
    settings.setValue("SortIndex", name);
    settings.setValue("SortOrder", order);
    settings.setValue("FoldersFirst", first);
    clearItems();
    d->populateItems();
}


void Browser::setFilter(QString filter)
{
    d->filter = filter;
    clearItems();
    d->populateItems();

}


QString Browser::getSelectedList()
{
    return d->multiSelItems.join("<filecaseseparator>");
}


QString Browser::manageList(QString action, QString list, QString items)
{
    QStringList mlist = list.split(",");
    QStringList mitems = items.split(",");

    for (int i=0; i<mitems.count(); ++i) {
        if (action=="add") mlist.append(mitems.at(i));
        else if (action=="remove") mlist.removeAll(mitems.at(i));
    }
    mlist.removeDuplicates();
    mlist.removeAll("");

    return mlist.join(",");
}

QString Browser::getSDcard()
{
    QString folder = "";

    /*v1: 
    QDir dir ("/media/sdcard");
    QFileInfoList entries = dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot );
    QListIterator<QFileInfo> entriesIterator (entries);
    if (entriesIterator.hasNext()) {
        QFileInfo fileInfo = entriesIterator.next();
        folder = fileInfo.absoluteFilePath();
    }*/

    /*v2:
    QProcess process;
    process.start("cat /proc/mounts");
    process.waitForFinished(-1);
    QString stdout = process.readAllStandardOutput();
    if (stdout.contains("/media/sdcard/")) {
        int i = stdout.indexOf("/media/sdcard/");
        stdout.remove(0, i);
        i = stdout.indexOf(" ");
        stdout = stdout.left(i);
        folder = stdout;
    }   
    qDebug() << "SD Card:" << stdout;*/

    //v3:
    if (QDir("/run/media/defaultuser").exists())
        folder = "/run/media/defaultuser";
    else if (QDir("/run/media/nemo").exists())
        folder = "/run/media/nemo";
    else if (QDir("/media/sdcard").exists())
        folder = "/media/sdcard";
    
    return folder;

}

void Browser::clearItems()
{
    emit d->q->beginResetModel();
    d->items.clear();
    emit d->q->endResetModel();
}
