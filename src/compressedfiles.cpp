#include "sys/vfs.h"
#include "compressedfiles.h"
#include "loader.h"
#include "utils.h"

#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QDateTime>
#include <QProcess>
#include <QDebug>
#include <QIcon>
#include <QStringList>

struct CompressedFilesItem
{
    QString name;
    QString date;
    QString path;
    QString exten;
    QString link;
    CompressedFiles::ItemType type;
};

class CompressedFiles::CompressedFilesPrivate
{
public:
    CompressedFilesPrivate(CompressedFiles * parent);
    ~CompressedFilesPrivate();
    void populateItems();
    QList <CompressedFilesItem * > items;
    QString path;
    CompressedFiles * const q;
    QProcess *pepe;
    QString compresor;
    QString zipFileName;
};

CompressedFiles::CompressedFilesPrivate::CompressedFilesPrivate(CompressedFiles * parent) :
        q(parent)
{
    pepe = new QProcess();
    connect( pepe, SIGNAL(finished(int)), parent, SLOT(unzipFinished(int)) );

}

CompressedFiles::CompressedFilesPrivate::~CompressedFilesPrivate()
{
    while(!items.isEmpty())
    {
        delete items.takeFirst();
    }
}


void CompressedFiles::loadZip(QString file)
{
    d->zipFileName = file;
    QFileInfo fileInfo(file);
    QString exten = fileInfo.fileName().toLower();

    if ( !QFileInfo("/usr/bin/unrar").exists() && exten.endsWith(".rar")) {
        emit unrarError();
        return;
    }

    if ( exten.endsWith(".zip") || exten.endsWith(".apk") )
        d->compresor = "unzip";
    else if ( exten.endsWith(".rar") )
        d->compresor = "unrar";
    else
        d->compresor = "tar";

    if (QFileInfo().exists("/home/nemo/.config/cepiperez/.tempfile"))
        QFile::remove("/home/nemo/.config/cepiperez/.tempfile");

    QString ttt1;
    if ( exten.endsWith(".zip") || exten.endsWith(".apk") )
        ttt1 = "unzip -l \"" + file + "\" | awk '{gsub($3,\"\")gsub($2,\"\")}; 1'";
    else if ( exten.endsWith(".rar") )
        ttt1 = "unrar v \"" + file + + "\"";
    else if ( exten.endsWith(".tar") )
        ttt1 = "tar -tvf \"" + file + "\" | awk '{gsub($1,\"\")gsub($4,\"\")gsub($3,\"\")gsub($1,\"\")}; 1'";
    else if ( exten.endsWith(".tar.gz") )
        ttt1 = "tar -tzvf \"" + file + "\" | awk '{gsub($1,\"\")gsub($4,\"\")gsub($3,\"\")gsub($1,\"\")}; 1'";
    else if ( exten.endsWith(".tar.bz2") )
        ttt1 = "tar -tjvf \"" + file + "\" | awk '{gsub($1,\"\")gsub($4,\"\")gsub($3,\"\")gsub($1,\"\")}; 1'";

    ttt1 += " > /home/nemo/.config/cepiperez/.tempfile";

    system(ttt1.toUtf8());

    //d->pepe->start(d->compresor);

    unzipFinished(0);

}

void CompressedFiles::unzipFinished(int res)
{
    Q_UNUSED(res)

    d->path = d->zipFileName;
    emit pathChanged();
    d->populateItems();
}

void CompressedFiles::cancelReading()
{
    d->pepe->close();
}

void CompressedFiles::CompressedFilesPrivate::populateItems()
{
    QString line; int linea=0;
    QString texto, texto2;

    QFile data( "/home/nemo/.config/cepiperez/.tempfile" );
    if (data.open(QFile::ReadOnly | QFile::Truncate))
    {
        int start=0;
        //int password = 0;
        int e_mydoc = 0;

        QTextStream out(&data);
        while ( !out.atEnd() )
        {
            if ( compresor == "unrar")
            {
                line = out.readLine();
                if ( line.indexOf("-----------------")!=-1 && start==0 ) { start=1; line = out.readLine(); }
                else if ( line.indexOf("-----------------")!=-1 && start==1 ) start=2;
                if ( start == 1 )
                {
                    texto2 = out.readLine();
                    texto2 = texto2.trimmed();
                    int ei = texto2.indexOf(" ");
                    line = line.trimmed();
                    texto2.remove ( ei, texto2.length()-ei );
                    if ( line.indexOf("*") == 0 ) { line.remove(0,1); /* password=1; */ }
                    if ( QString(texto2).toInt() > 0 ) texto += line.trimmed() + "\n";
                    if ( line != "" ) {
                        e_mydoc = e_mydoc + QString(texto2).toInt();
                    }
                }
            }
            else if ( compresor == "unzip")
            {
                line = out.readLine();

                if (line.indexOf("Length")>0 && line.indexOf("Name")>0) {
                    start = 1;
                    line = out.readLine();
                }

                if ( line.indexOf("Length")==-1 && line.indexOf("Archive")==-1
                     && line.indexOf("Name")==-1 && line.trimmed()!="" && start==1)
                {
                    texto2 = line;
                    int ei = line.lastIndexOf("   ");
                    line.remove ( 0, ei+3 );
                    texto2.remove ( ei, texto2.length()-ei );
                    if ( line.trimmed() != "" )
                    {
                        texto += line.trimmed() + "\n";
                        if ( line!="" && start==1 ) {
                            e_mydoc = e_mydoc + QString(texto2).toInt();
                        }
                    }
                }
            }
            else
            {
                line = out.readLine();
                texto2 = line;
                int ei = line.lastIndexOf("  ");
                line.remove ( 0, ei+1 );
                texto2.remove ( ei, texto2.length()-ei );
                //if ( QString(texto2).toInt() > 0 )
                    texto += line.trimmed() + "\n";
                if ( line != "" ) {
                    e_mydoc = e_mydoc + QString(texto2).toInt();
                    /*if ( line.indexOf("/home/user/MyDocs") > 0 ) e_mydoc = e_mydoc + QString(texto2).toInt();
                    else if ( line.indexOf("/home") > 0 ) e_home = e_home + QString(texto2).toInt();
                    else if ( line.indexOf("/media/mmc1") > 0 ) e_mmc= e_mmc + QString(texto2).toInt();
                    else if ( line.indexOf("/and/") > 0 ) e_other = e_other + QString(texto2).toInt();
                    else e_root = e_root + QString(texto2).toInt();*/
                }
            }

            ++linea;
        }
    }
    data.close();
    QStringList archivos = texto.split("\n");
    archivos.removeAll("");
    //qDebug() << archivos;

    q->beginInsertRows(QModelIndex() , 0 , archivos.count() - 1);

    for (int i=0; i < archivos.count(); ++i)
    {
        QString fileInfo = archivos.at(i);
        CompressedFilesItem * item = new CompressedFilesItem();

        int j = fileInfo.lastIndexOf("/");
        fileInfo.remove(0, j+1);
        if ( fileInfo.length() == 0 )
            fileInfo = archivos.at(i);

        item->name = fileInfo;
        QString desc1 = archivos.at(i);
        //desc1.remove(fileInfo);
        if ( archivos.at(i).contains("/") )
            item->date = "/" + desc1.left(desc1.length() -fileInfo.length() -1);
        else
            item->date = "/";

        //item->desc = archivos.at(i).remove(fileInfo);

        item->link = "compressed";

        item->path = archivos.at(i);

        if( ! archivos.at(i).endsWith("/") )
        {
            item->exten = GetExtension(item->name.toLower());
            item->type = CompressedFiles::File;
        }
        else
        {
            item->exten = "folder";  //"../icons/folder.png";
            item->type = CompressedFiles::Folder;
            item->name = fileInfo.left(fileInfo.length()-1);
            int j = item->name.lastIndexOf("/");
            item->name = item->name.remove(0, j+1);
            item->date = fileInfo.left(fileInfo.length()-1-item->name.length());
            if ( item->date=="" ) item->date = "/";
        }
        //qDebug() << "Adding " << archivos.at(i);

        //if ( archivos.at(i) != "" )
        items.append(item);

        //++i;
    }

    q->endInsertRows();

    emit(q->countChanged());

    emit(q->dataChanged(q->index(0) , q->index(archivos.count() - 1)));


}

CompressedFiles::CompressedFiles(QObject * parent) :
        QAbstractListModel(parent) , d (new CompressedFilesPrivate(this))
{
    // Roles
    QHash <int, QByteArray> roles;
    roles.insert(NameRole , "name");
    roles.insert(DateRole , "date");
    roles.insert(PathRole , "path");
    roles.insert(ExtenRole , "exten");
    roles.insert(LinkRole , "link");
    roles.insert(TypeRole , "type");
    d->q->setRoleNames(roles);
}

QHash<int, QByteArray> CompressedFiles::roleNames() const
{
    return m_roles;
}

void CompressedFiles::setRoleNames(const QHash<int, QByteArray>& roles)
{
    m_roles = roles;
}

CompressedFiles::~CompressedFiles()
{
    delete d;
}

int CompressedFiles::rowCount(const QModelIndex & parent) const
{
    Q_UNUSED(parent)
    return d->items.size();
}

QVariant CompressedFiles::data(const QModelIndex & index , int role) const
{
    if(index.row() < 0 or index.row() > count())
    {
            return QVariant();
    }

    CompressedFilesItem * item = d->items.at(index.row());
    switch(role)
    {
    case NameRole:
        return item->name;
        break;
    case DateRole:
        return item->date;
        break;
    case PathRole:
        return item->path;
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

int CompressedFiles::count() const
{
    return rowCount();
}

QString CompressedFiles::path() const
{
    return d->path;
}

QString CompressedFiles::banner() const
{
    return m_banner;
}

