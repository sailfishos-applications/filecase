#include "loader.h"
#include "utils.h"

#include <QCryptographicHash>
#include <QUrl>
#include <qstring.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <QImage>
#include <QSettings>
#include <QDebug>

Loader::Loader()
{
    files.clear();
    action = "";
}

Loader::~Loader()
{

}

void Loader::setFiles(QStringList filelist)
{
    files = filelist;
}

void Loader::setAction(QString myaction)
{
    action = myaction;
}


void Loader::run()
{
    if (action == "fileinfo-files")
    {
        double size = 0;
        for (int fi=0; fi < files.count(); ++fi)
        {
            //qDebug() << "SIZE: " << size;
            if ( QFileInfo(files[fi]).isFile() ) size += QFileInfo(files[fi]).size();
            else size += GetFolderSize( files[fi] );
            emit sizeChanged(size);
        }


        emit sizeFinished(size);
    }
    else if (action == "readtext")
    {
        int step = 0;
        QString lines;
        QFile data(files.at(0));
        if (data.open(QFile::ReadOnly | QFile::Truncate))
        {
            QTextStream out(&data);
            while ( !out.atEnd() ) {
                lines += out.readLine()+"\n";
                ++step;

                if (step==50) {
                    step = 0;
                    emit addLines(lines);
                    lines = "";
                }
            }
        }
        data.close();
        emit addLines(lines);
        emit readFinished();
    }

}
