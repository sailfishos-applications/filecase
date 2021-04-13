#ifndef BOXTHUMBNAILER_H
#define BOXTHUMBNAILER_H

#include <QStringList>
#include <QThread>
#include <QImage>

#include <QNetworkAccessManager>
#include <QNetworkReply>

class QNetworkRequest;
class QNetworkReply;

class BoxThumbnailer : public QThread
{
   Q_OBJECT

public:
   BoxThumbnailer();
   virtual ~BoxThumbnailer();

   QList<QStringList> images;
   QImage result;
   QString accessToken;
   bool working;

public slots:
   void clear();
   void addImage(QString filename, QString link);
   void download(QStringList image);
   void done();
   void run();

   void handleNetworkReply(QNetworkReply *networkReply);
   bool saveToDisk(const QString &filename, QIODevice *reply);


signals:
    void imageLoaded(QString filename);

private:
    QNetworkAccessManager* datos;
    QNetworkReply *reply;
};


#endif // BOXTHUMBNAILER_H
