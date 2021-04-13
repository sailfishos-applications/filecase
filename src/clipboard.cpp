#include "clipboard.h"
#include "utils.h"

#include <QCryptographicHash>
#include <QDir>
#include <QFileInfo>
#include <QDateTime>
#include <QProcess>
#include <QDebug>
#include <QIcon>

struct ClipboardItem
{
    QString name;
    QString date;
    QString path;
    QString exten;
    Clipboard::ItemType type;
};

class Clipboard::ClipboardPrivate
{
public:
    ClipboardPrivate(Clipboard * parent);
    ~ClipboardPrivate();
    void populateItems();
    QList <ClipboardItem * > items;
    QStringList selectedFiles;
    Clipboard * const q;
};

Clipboard::ClipboardPrivate::ClipboardPrivate(Clipboard * parent) : q(parent)
{

}

Clipboard::ClipboardPrivate::~ClipboardPrivate()
{
    while(!items.isEmpty())
    {
        delete items.takeFirst();
    }
}

void Clipboard::ClipboardPrivate::populateItems()
{
    QSettings settings("cepiperez", "fileboxplus");
    QStringList entries1 = settings.value("Clipboard","").toStringList();
    QStringList entriesSelected = settings.value("ClipboardSelected","").toStringList();

    entries1.removeDuplicates();
    entriesSelected.removeDuplicates();
    selectedFiles.clear();

    QFileInfoList entries;
    QStringList entriesnew;
    for (int i=0; i < entries1.count(); ++i)
    {
        if ( (entries1.at(i) != "") && (QFileInfo(entries1.at(i)).exists()) )
        {
            entries.append ( QFileInfo(entries1.at(i)) );
            entriesnew.append( entries1.at(i) );
        }
    }
    settings.setValue("Clipboard", entriesnew);

    QStringList entriesselectednew;
    for (int i=0; i < entriesSelected.count(); ++i)
    {
        if ( (entriesnew.contains(entriesSelected.at(i))) && (QFileInfo(entriesSelected.at(i)).exists()) )
        {
            entriesselectednew.append( entriesSelected.at(i) );
            selectedFiles.append( entriesSelected.at(i) );
        }
    }
    entriesSelected = entriesselectednew;
    settings.setValue("ClipboardSelected", entriesselectednew);
    settings.sync();


    int entriesCount = entries.count();

    emit q->beginResetModel();
    items.clear();
    emit q->endResetModel();

    emit q->beginResetModel();

    q->beginInsertRows(QModelIndex(), 0, entriesCount-1);

    for(int i=0; i<entriesCount; i++)
    {
        items.append(new ClipboardItem);
    }

    q->endInsertRows();


    QListIterator<QFileInfo> entriesIterator (entries);
    QListIterator<ClipboardItem *> itemsIterator(items);

    while(entriesIterator.hasNext() && itemsIterator.hasNext())
    {
        QFileInfo fileInfo = entriesIterator.next();
        QString name = fileInfo.fileName();

        ClipboardItem * item = itemsIterator.next();
        item->name = name;

        item->date = fileInfo.absolutePath();

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

        if ( entriesSelected.contains(fileInfo.absoluteFilePath()) )
            item->type = Clipboard::FileSel;
        else
            item->type = Clipboard::File;

        //qDebug() << "Adding file: " << name << " - type " << item->type;


    }

    q->m_multi1 = selectedFiles.count();
    emit(q->multiChanged());
    emit(q->countChanged());
    emit q->endResetModel();


}

Clipboard::Clipboard(QObject * parent) :
        QAbstractListModel(parent) , d (new ClipboardPrivate(this))
{
        // Roles
    QHash <int, QByteArray> roles;
    roles.insert(NameRole , "name");
    roles.insert(DateRole , "date");
    roles.insert(PathRole , "path");
    roles.insert(ExtenRole , "exten");
    roles.insert(TypeRole , "type");
    d->q->setRoleNames(roles);
    //d->populateItems();
}

QHash<int, QByteArray> Clipboard::roleNames() const {
  return m_roles;
}

void Clipboard::setRoleNames(const QHash<int, QByteArray>& roles) {
  m_roles = roles;
}

Clipboard::~Clipboard()
{
    delete d;
}

int Clipboard::rowCount(const QModelIndex & parent) const
{
    Q_UNUSED(parent)
    return d->items.size();
}

QVariant Clipboard::data(const QModelIndex & index , int role) const
{
    if(index.row() < 0 or index.row() > count())
    {
            return QVariant();
    }

    ClipboardItem * item = d->items.at(index.row());
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
    case TypeRole:
        return item->type;
        break;
    default:
        return QVariant();
        break;
    }
}

int Clipboard::count() const
{
    return rowCount();
}

void Clipboard::selectFile(const QString &file)
{
    for (int i=0; i< d->items.count(); ++ i)
    {
        if ( d->items.at(i)->path == file )
        {
            if ( d->items.at(i)->type == Clipboard::FileSel )
            {
                d->items.at(i)->type = Clipboard::File;
                d->selectedFiles.removeAll(file);
            }
            else
            {
                d->items.at(i)->type = Clipboard::FileSel;
                d->selectedFiles.append(file);
            }
        }
    }
    d->selectedFiles.removeDuplicates();
    QSettings settings("cepiperez", "fileboxplus");
    settings.setValue("ClipboardSelected", d->selectedFiles);
    settings.sync();
    m_multi1 = d->selectedFiles.count();
    emit multiChanged();
    emit(dataChanged(index(0) , index(d->items.count() - 1)));

}

int Clipboard::multi1() const
{
    return m_multi1;
}

void Clipboard::removeSelected()
{
    QSettings settings("cepiperez", "fileboxplus");
    QStringList entries = settings.value("Clipboard","").toStringList();
    QStringList newfiles;
    for (int i=0; i< entries.count(); ++i)
    {
        if ( ! d->selectedFiles.contains(entries.at(i)) )
            newfiles.append( entries.at(i) );
    }
    settings.setValue("Clipboard", newfiles);
    settings.remove("ClipboardSelected");
    settings.sync();
    d->populateItems();

}

void Clipboard::setClipboard()
{
    QSettings settings("cepiperez", "fileboxplus");
    QStringList newfiles;
    for (int i=0; i< d->items.count(); ++ i)
    {
        if ( d->items.at(i)->type == Clipboard::FileSel )
            newfiles.append( d->items.at(i)->path );
    }
    settings.setValue("ClipboardSelected", newfiles);
    settings.sync();

}

void Clipboard::reload()
{
    d->populateItems();
}

