#include "sys/vfs.h"
#include "search.h"
#include "utils.h"
#include "config.h"

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

struct SearchItem
{
    QString name;
    QString date;
    QString fullpath;
    QString path;
    QString sec;
    QString exten;
    QString link;
    Search::ItemType type;
};

class Search::SearchPrivate
{
public:
    SearchPrivate(Search * parent);
    ~SearchPrivate();
    void populateSearch(QStringList archivos);
    QList <SearchItem * > items;
    QString path;
    Search * const q;
    QStringList multiSelItems;
    QStringList searchResults;
    QProcess* searchProc;
    QString searchProcCMD;
    int searchcanceled;

};

Search::SearchPrivate::SearchPrivate(Search * parent) :
        q(parent)
{
    searchProc = new QProcess(parent);
    connect(searchProc, SIGNAL( readyRead() ), parent, SLOT( preaded() ) );
    connect(searchProc, SIGNAL( finished(int) ), parent, SLOT( pfinished(int) ) );

}

Search::SearchPrivate::~SearchPrivate()
{
    while(!items.isEmpty())
    {
        delete items.takeFirst();
    }
}

Search::Search(QObject * parent) :
        QAbstractListModel(parent) , d (new SearchPrivate(this))
{
    QHash <int, QByteArray> roles;
    roles.insert(NameRole , "name");
    roles.insert(DateRole , "date");
    roles.insert(FullPathRole , "fullpath");
    roles.insert(PathRole , "path");
    roles.insert(ExtenRole , "exten");
    roles.insert(SecRole , "sec");
    roles.insert(LinkRole , "link");
    roles.insert(TypeRole , "type");
    d->q->setRoleNames(roles);
}

QHash<int, QByteArray> Search::roleNames() const {
  return m_roles;
}

void Search::setRoleNames(const QHash<int, QByteArray>& roles) {
  m_roles = roles;
}


Search::~Search()
{
    delete d;
}

int Search::rowCount(const QModelIndex & parent) const
{
    Q_UNUSED(parent)
    return d->items.size();
}

QVariant Search::data(const QModelIndex & index , int role) const
{
    if(index.row() < 0 or index.row() > count())
    {
        return QVariant();
    }

    SearchItem * item = d->items.at(index.row());
    switch(role)
    {
    case NameRole:
        return item->name;
        break;
    case DateRole:
        return item->date;
        break;
    case FullPathRole:
        return item->fullpath;
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

int Search::count() const
{
        return rowCount();
}

void Search::addToClipboard(const QString &file, QString autoselect)
{
    QSettings settings("cepiperez", "fileboxplus");
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

void Search::addToStandardClipboard(const QString &file, QString autoselect)
{
    QSettings settings("cepiperez", "fileboxplus");
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

void Search::renameFile(const QString &file, const QString &newf)
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

        for (int i=0; i< d->items.count(); ++ i)
        {
            if ( d->items.at(i)->path == file ) {
                d->items.at(i)->name = newf;
                emit(d->q->dataChanged(d->q->index(i) , d->q->index(i)));
                break;
            }
        }
        emit d->q->fileRenamed(newname);
    }

}

void Search::removeFile(const QString &file, bool start)
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

    if (workErrors==0)
        emit bannerChanged(tr("Delete process completed"), true);
    else
        emit bannerChanged(tr("There were errors deleting files!"), false);

    cleanSearch();
    d->populateSearch(d->searchResults);

}

void Search::shareFile(const QString &file)
{
    if ( !d->path.contains(Config::getHome()) )
    {
        emit bannerChanged(tr("You can't share files outside home folder"), false);
        return;
    }

/*#ifndef Q_WS_MAEMO_5
    ShareUiInterface shareIf("com.nokia.ShareUi");
    if ( file == "selectedfiles" )
    {
        shareIf.share(d->multiSelItems);
    }
    else
    {
        QStringList items;
        items.append(file);
        shareIf.share(items);
    }
#endif*/

}

QString Search::banner() const
{
    return m_banner;
}

void Search::clearSelection()
{
    d->multiSelItems.clear();
    for (int i=0; i< d->items.count(); ++ i)
    {
        if ( d->items.at(i)->type == Search::FileSel )
            d->items.at(i)->type = Search::File;
        else if ( d->items.at(i)->type == Search::FolderSel )
            d->items.at(i)->type = Search::Folder;
    }
    m_multi1 = 0;
    m_multi2 = 0;
    m_multi3 = 0;

    emit(d->q->dataChanged(d->q->index(0) , d->q->index(d->items.count() - 1)));

}

void Search::selectItem(QString name)
{
    d->multiSelItems.clear();
    int cantfiles = 0;
    int cantfolders = 0;
    for (int i=0; i< d->items.count(); ++ i)
    {
        if ( d->items.at(i)->path == name )
        {
            if ( d->items.at(i)->type == Search::File )
                d->items.at(i)->type = Search::FileSel;
            else if ( d->items.at(i)->type == Search::FileSel )
                d->items.at(i)->type = Search::File;
            else if ( d->items.at(i)->type == Search::Folder )
                d->items.at(i)->type = Search::FolderSel;
            else if ( d->items.at(i)->type == Search::FolderSel )
                d->items.at(i)->type = Search::Folder;

//          qDebug() << "SELECTING " << d->items.at(i)->path << " - " << d->items.at(i)->type;
        }

        if ( d->items.at(i)->type > 1 )
        {
            d->multiSelItems.append(d->items.at(i)->path);
            if ( d->items.at(i)->type == 2 )
                 ++cantfolders;
            else
                ++cantfiles;
        }
        emit(dataChanged(index(i) , index(i)));
    }

    //m_multi1: cantidad total de archivos y carpetas seleccionados
    m_multi1 = d->multiSelItems.count();
    //m_multi2: cantidad de archivos seleccionados
    m_multi2 = cantfiles;
    //m_multi3: cantidad de carpetas seleccionadas
    m_multi3 = cantfolders;
    //qDebug() << "EMITTING multiChanged(): " << m_multi1 << " " << m_multi2 << " " << m_multi3 ;
    emit multiChanged();
}



int Search::multi1() const
{
    return m_multi1;
}

int Search::multi2() const
{
    return m_multi2;
}

int Search::multi3() const
{
    return m_multi3;
}

void Search::infoFiles(QString name)
{
    QSettings settings("cepiperez", "fileboxplus");
    if (name == "selectedfiles")
        settings.setValue("InfoFiles", d->multiSelItems);
    else
        settings.setValue("InfoFiles", name);
    settings.sync();
}

void Search::pfinished(int code)
{
    qDebug() << "PROCESS FINISHED! - Code: " << code;
    //qDebug() << d->zipProc->readAllStandardError();
    QString error = d->searchProc->readAllStandardError();
    qDebug() << error;

    if ( d->searchcanceled == 1 )
        emit bannerChanged(tr("Search canceled"), true);
    else
        emit bannerChanged(tr("Search completed"), true);

    QString res =  d->searchProc->readAllStandardOutput();
    QStringList lista = res.split("\n");
    qDebug() << "SEARCH LIST: " << lista;
    d->searchResults << lista;

    if (lista.count()>0) {
        cleanSearch();
        d->populateSearch(d->searchResults);
    }

}

void Search::searchFiles(QString path, QString filter)
{
    d->searchResults.clear();
    qDebug() << "PATH: " << path << " FILTER: " << filter;
    d->searchProc->setWorkingDirectory("/");
    d->searchcanceled = 0;
    d->searchProcCMD = "search";
    QStringList arguments;
    arguments << path << "-iname" << "*"+filter+"*";
    d->searchProc->start("find", arguments);

}

void Search::stopSearch()
{
    d->searchcanceled = 1;
    d->searchProc->terminate();
}

void Search::SearchPrivate::populateSearch(QStringList archivos)
{
    QSettings settings("cepiperez", "fileboxplus");
    bool showHidden = settings.value("ShowHiddenFiles", "false").toString()== "true";

    archivos.removeAll("");
    QStringList archivos2;

    emit q->beginResetModel();

    for (int i=0; i < archivos.count(); ++i)
    {
        if ( QFileInfo(archivos.at(i)).exists() && !archivos2.contains(archivos.at(i)) ) {

            bool hiddenFile = QFileInfo(archivos.at(i)).absolutePath().contains(".") ||
                              QFileInfo(archivos.at(i)).fileName().startsWith(".");

            if ( !hiddenFile || showHidden )
                archivos2.append(archivos.at(i));
        }
    }

    q->beginInsertRows(QModelIndex() , 0 , archivos2.count() - 1);

    for (int i=0; i < archivos2.count(); ++i)
    {
        QFileInfo fileInfo = archivos2.at(i);
        QString name = fileInfo.fileName();
        SearchItem * item = new SearchItem();
        item->name = name;

        item->fullpath = fileInfo.absoluteFilePath();
        item->path = fileInfo.absolutePath();
        item->sec = "";

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


        if(!fileInfo.isDir())
            item->type = Search::File;
        else
            item->type = Search::Folder;



        items.append(item);

    }

    q->endInsertRows();

    emit(q->countChanged());

    emit q->endResetModel();

}

void Search::preaded()
{
    if ( d->searchProcCMD == "search" )
    {
        QString res =  d->searchProc->readAllStandardOutput();
        QStringList lista = res.split("\n");
        //qDebug() << "READED: " << lista;
        d->searchResults << lista;
        //d->populateSearch(lista);
    }


}

QString Search::getSelectedList()
{
    return d->multiSelItems.join("<filecaseseparator>");
}

void Search::cleanSearch()
{
    emit d->q->beginResetModel();
    d->items.clear();
    emit d->q->endResetModel();
}
