#ifndef THUMBGENERATOR_H
#define THUMBGENERATOR_H

#include <QStringList>
#include <QThread>
#include <QImage>

//#include "videothumbnailer.h"

extern QStringList imagesToProcess;

class ThumbGenerator : public QThread
{
   Q_OBJECT

public:
   ThumbGenerator();
   virtual ~ThumbGenerator();

public slots:
   void run();
   void generate(QString file);

signals:
    void finished();
    void imgLoaded(QString file);

};

#endif // THUMBGENERATOR_H
