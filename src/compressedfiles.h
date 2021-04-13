#ifndef COMPRESSEDFILES_H
#define COMPRESSEDFILES_H

#include <QAbstractListModel>
#include <QSettings>
#include <QStringList>
#include <QProcess>

class CompressedFiles : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(QString path READ path NOTIFY pathChanged)
    Q_PROPERTY(QString banner READ banner NOTIFY bannerChanged)
    Q_ENUMS(ItemType)

public:
    enum ItemType
    {
        Folder ,
        File ,
        FolderSel ,
        FileSel
    };

    enum CompressedFilesRole
    {
        NameRole  = Qt::UserRole + 1 ,
        DateRole ,
        PathRole ,
        ExtenRole ,
        LinkRole ,
        TypeRole
    };

    explicit CompressedFiles(QObject * parent = 0);
    virtual ~CompressedFiles();
    virtual int rowCount(const QModelIndex & parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex & index , int role = Qt::DisplayRole) const;
    int count() const;
    QString path() const;
    QString banner() const;
    QString m_banner;
    //QString getcname(int index) const;


signals:
    void countChanged();
    void pathChanged();
    void bannerChanged();
    void multiChanged();
    void unrarError();

public slots:
    void loadZip(QString file);
    void unzipFinished(int res);
    void cancelReading();

private:
    QHash<int, QByteArray> roleNames() const;
    void setRoleNames(const QHash<int, QByteArray>& roles);
    QHash<int, QByteArray> m_roles;

    class CompressedFilesPrivate;
    CompressedFilesPrivate * const d;
};


#endif // COMPRESSEDFILES_H
