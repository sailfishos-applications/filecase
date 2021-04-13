#ifndef SEARCH_H
#define SEARCH_H

#include <QAbstractListModel>
#include <QSettings>
#include <QStringList>

class Search : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(QString banner READ banner NOTIFY bannerChanged)
    Q_PROPERTY(int multi1 READ multi1 NOTIFY multiChanged)
    Q_PROPERTY(int multi2 READ multi2 NOTIFY multiChanged)
    Q_PROPERTY(int multi3 READ multi3 NOTIFY multiChanged)
    Q_ENUMS(ItemType)

public:
    Q_INVOKABLE QString getSelectedList();

    enum ItemType
    {
        Folder ,
        File ,
        FolderSel ,
        FileSel
    };

    enum SearchRole
    {
        NameRole  = Qt::UserRole + 1 ,
        DateRole ,
        FullPathRole ,
        PathRole ,
        SecRole ,
        ExtenRole ,
        LinkRole ,
        TypeRole
    };

    explicit Search(QObject * parent = 0);
    virtual ~Search();
    virtual int rowCount(const QModelIndex & parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex & index , int role = Qt::DisplayRole) const;
    int count() const;
    QString banner() const;
    int multi1() const;
    int multi2() const;
    int multi3() const;
    QString m_banner;
    int m_multi1;
    int m_multi2;
    int m_multi3;

signals:
    void countChanged();
    void bannerChanged(QString msg, bool ok);
    void multiChanged();
    void fileRenamed(QString newname);

public slots:
    void searchFiles(QString path, QString filter);
    void stopSearch();
    void pfinished(int code = 0);
    void preaded();

    void infoFiles(QString name);
    void selectItem(QString name);
    void clearSelection();
    void addToClipboard(const QString & file, QString autoselect);
    void addToStandardClipboard(const QString & file, QString autoselect);
    void removeFile(const QString & file, bool start = false);
    void renameFile(const QString & file, const QString & newname);
    void shareFile(const QString & file);
    void cleanSearch();

private:
    QHash<int, QByteArray> roleNames() const;
    void setRoleNames(const QHash<int, QByteArray>& roles);
    QHash<int, QByteArray> m_roles;

    class SearchPrivate;
    SearchPrivate * const d;

    int workErrors;

};

#endif // SEARCH_H
