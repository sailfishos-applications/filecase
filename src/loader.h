#ifndef LOADER_H
#define LOADER_H

#include <QStringList>
#include <QThread>
#include <QImage>
#include <QProcess>

class Loader : public QThread
{
   Q_OBJECT

public:
   Loader();
   virtual ~Loader();

   QStringList files;
   QString action;
   //QImage result;

public slots:
   void setAction(QString myaction = "");
   void setFiles(QStringList filelist);
   void run();

signals:
    void addLines(QString lines);
    void readFinished();
    void sizeFinished(double size);
    void sizeChanged(double size);


};

#endif // LOADER_H
