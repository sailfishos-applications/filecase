#ifndef CLIPBOARD_H
#define CLIPBOARD_H

#include <QAbstractListModel>
#include <QSettings>

class Clipboard : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(int multi1 READ multi1 NOTIFY multiChanged)
    Q_ENUMS(ItemType)

public:
    enum ItemType
    {
        File ,
        FileSel
    };

    enum ClipboardRole
    {
        NameRole  = Qt::UserRole + 1 ,
        DateRole ,
        PathRole ,
        ExtenRole ,
        TypeRole
    };

    explicit Clipboard(QObject * parent = 0);
    virtual ~Clipboard();
    virtual int rowCount(const QModelIndex & parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex & index , int role = Qt::DisplayRole) const;
    int count() const;
    int m_multi1;
    int multi1() const;
    QString path() const;

signals:
    void countChanged();
    void multiChanged();

public slots:
    void reload();
    void setClipboard();
    void selectFile(const QString & file);
    void removeSelected();

private:
    QHash<int, QByteArray> roleNames() const;
    void setRoleNames(const QHash<int, QByteArray>& roles);
    QHash<int, QByteArray> m_roles;

    class ClipboardPrivate;
    ClipboardPrivate * const d;
};


#endif // CLIPBOARD_H
