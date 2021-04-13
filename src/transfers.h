#ifndef TRANSFERS_H
#define TRANSFERS_H

#include <QStringList>
#include <QThread>
#include <QImage>

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QFile>

#include "qwebdavlib/qwebdav.h"

//class QNetworkRequest;
//class QNetworkReply;

class Transfers : public QThread
{
   Q_OBJECT

public:
   Transfers();
   virtual ~Transfers();

   QString getContentTypeByExtension(const QString& ext);

   QList<QStringList> media;
   QString currentFile;
   QString accessToken;
   int currentProgress;
   bool working;

public slots:
   void addFile(QString service, QString name, QString link, QString mode, QString realsize);
   void removeFile(QString filename);
   void startTranfers();
   void stopTransfers();
   void transferFile(QStringList file);

   void transferDownloadFinished();
   void transferDownloadFinished2(QNetworkReply *networkReply);
   void transferUploadFinished();
   void handleDownloadReadyRead();
   void handleDownloadReadyRead2(QNetworkReply *networkReply);
   void handleDownloadProgress(qint64 received, qint64 total);
   void handleUploadProgress(qint64 sent, qint64 total);
   void handleNetworkReply(QNetworkReply *networkReply);

   void printError(QString error);

   virtual void slotError(QNetworkReply::NetworkError error);
   virtual void slotSslErrors(const QList<QSslError>& errors);


signals:
    void workingChanged(bool working);
    void progressChanged(QString filename, int val, QString curprog);
    void fileTransfered(QString link);
    void showBanner(QString info);

private:
    QNetworkAccessManager* datos;
    QNetworkReply *downloadNetworkReply;
    QNetworkReply *uploadNetworkReply;

    QFile downloadFile;
    QFile *uploadFile;

    QUrl _originalUrl;
    QUrl _urlRedirectedTo;

    QWebdav qwebdav;

    QUrl redirectUrl(const QUrl& possibleRedirectUrl, const QUrl& oldRedirectUrl) const;

};



#endif // TRANSFERS_H
