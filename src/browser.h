#ifndef BROWSER_H
#define BROWSER_H

#include <QFileSystemWatcher>
#include <QAbstractListModel>
#include <QSettings>
#include <QStringList>

class Browser : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(QString path READ path WRITE setPath NOTIFY pathChanged)
    Q_PROPERTY(QString path1 READ path1 NOTIFY pathChanged)
    Q_PROPERTY(QString path2 READ path2 NOTIFY pathChanged)
    Q_PROPERTY(int last READ last NOTIFY pathChanged)
    //Q_PROPERTY(QString banner READ banner NOTIFY bannerChanged)
    Q_PROPERTY(int multi1 READ multi1 NOTIFY multiChanged)
    Q_PROPERTY(int multi2 READ multi2 NOTIFY multiChanged)
    Q_PROPERTY(int multi3 READ multi3 NOTIFY multiChanged)
    Q_PROPERTY(QString copyMessage1 READ copyMessage1 NOTIFY copyDialog)
    Q_PROPERTY(QString copyMessage2 READ copyMessage2 NOTIFY copyDialog)
    Q_ENUMS(ItemType)

public:
    Q_INVOKABLE QString getSelectedList();
    Q_INVOKABLE QString manageList(QString action, QString list, QString items);
    Q_INVOKABLE QString cname(int index);
    Q_INVOKABLE QVariantMap get(int index);
    Q_INVOKABLE QString getSDcard();
    Q_INVOKABLE int xterm();

    enum ItemType
    {
        Folder ,
        File ,
        FolderSel ,
        FileSel
    };

    enum BrowserRole
    {
        NameRole  = Qt::UserRole + 1 ,
        DateRole ,
        SizeRole ,
        PathRole ,
        SecRole ,
        ExtenRole ,
        LinkRole ,
        TypeRole
    };

    explicit Browser(QObject * parent = 0);
    virtual ~Browser();
    virtual int rowCount(const QModelIndex & parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex & index , int role = Qt::DisplayRole) const;
    int count() const;
    QString path() const;
    QString path1() const;
    QString path2() const;
    //QString banner() const;
    QString copyMessage1() const;
    QString copyMessage2() const;
    int last() const;
    int multi1() const;
    int multi2() const;
    int multi3() const;
    //QString m_banner;
    int m_multi1;
    int m_multi2;
    int m_multi3;
    int StartThread;
    //QString getcname(int index) const;


signals:
    void countChanged();
    void pathChanged();
    void bannerChanged(QString msg, bool ok);
    void multiChanged();
    void copyDialog();
    void askPassword();

    void fileRenamed(QString newname);

public slots:
    //void copyFile1();
    //void cmdfinished(int code = 0);

    void setFilter(QString filter);
    void setSortOrder(QString name, QString order, QString first);
    void infoFiles(QString name);
    void selectItem(QString name);
    void clearSelection();
    void cleanCache();

    //void CopyDialogResult(int result);
    void newFolder(QString name);
    void openTerminal(QString folder);
    void showThumbnails(QString data);
    void dirChanged(const QString folder);

    //void checkStandardClipboard(QString action);
    void addToClipboard(const QString & file, QString autoselect);
    void addToStandardClipboard(const QString & file, QString autoselect);
    void removeFile(const QString & file, bool start = false);
    void renameFile(const QString & file, const QString & newname);
    void showRootFiles(bool show = false);
    void showHidden(bool show = false);
    void setPath(const QString & path);
    void goUp();
    void dbg(const QString & data);
    void reload();
    void clearItems();

private:
    QHash<int, QByteArray> roleNames() const;
    void setRoleNames(const QHash<int, QByteArray>& roles);
    QHash<int, QByteArray> m_roles;

    class BrowserPrivate;
    BrowserPrivate * const d;

    int workErrors;
};

#endif // BROWSER_H
