#include "utils.h"

#include <QDir>


bool sortVariantMap(const QVariantMap &d1, const QVariantMap &d2)
{
    return d1.value("sec","").toString() < d2.value("sec","").toString();
}

bool sortStringList(const QStringList &d1, const QStringList &d2)
{
    return d1.at(0) < d2.at(0);
}

QString GetFileSize(double size)
{
    QString ss;
    if (size < 1024) {
      ss = QString("%1 b").arg(size);
    } else if (size < 1024*1024) {
      ss = QString("%1").arg( (double)(size) / 1024);
      size = ss.indexOf(QChar('.')); if ( size > -1 ) { ss.remove(size+3, ss.length()-size-3); }
      ss = ss + " Kb";
    } else if (size < 1024*1024*1024) {
      ss = QString("%1").arg( (double)(size) / 1024 / 1024);
      size = ss.indexOf(QChar('.')); if ( size > -1 ) { ss.remove(size+3, ss.length()-size-3); }
      ss = ss + " Mb";
    } else {
        ss = QString("%1").arg( (double)(size) / 1024 / 1024 / 1024);
        size = ss.indexOf(QChar('.')); if ( size > -1 ) { ss.remove(size+3, ss.length()-size-3); }
        ss = ss + " Gb";
      }
    return ss;
}


double GetFolderSize (const QString &path )
{
  //recursive
  QDir dir ( path );
  QStringList::Iterator it;
  double size = 0;
  QStringList files = dir.entryList ( QDir::AllEntries | QDir::NoDotAndDotDot );
  it = files.begin();
  while ( it != files.end() ) {
    size += QFileInfo ( path, *it ).size();
    ++it;
  }
  QStringList dirs = dir.entryList ( QDir::Dirs );
  it = dirs.begin();
  while ( it != dirs.end() ) {
    if ( *it != "." && *it != ".." )
      size += GetFolderSize ( path + "/" + *it );
    ++it;
  }
  return size;
}

QString GetExtension(QString name)
{
    QString exten = "";
    if ( name.endsWith(".jpg") || name.endsWith(".jpeg")
         || name.endsWith(".png") || name.endsWith(".svg")
         || name.endsWith(".gif") )
        exten = "image";
    else if ( name.endsWith(".txt") )
        exten = "txt";
    else if ( name.endsWith(".ini") || name.endsWith(".conf")
              || name.endsWith(".cfg") || name.endsWith(".inf")
              || name.endsWith(".spec") || name.endsWith(".desktop") )
        exten = "info";
    else if ( name.endsWith(".sh") )
        exten = "exec";
    else if ( name.endsWith(".mp4") || name.endsWith(".mpg")
              || name.endsWith(".mpeg") || name.endsWith(".avi")
              || name.endsWith(".wmv") || name.endsWith(".3gp")
              || name.endsWith(".flv") )
        exten = "video";
    else if ( name.endsWith(".mp3") || name.endsWith(".aac")
              || name.endsWith(".wav") || name.endsWith(".wma")
              || name.endsWith(".m4a") || name.endsWith(".ogg")
              || name.endsWith(".oga") || name.endsWith(".xm")
              || name.endsWith(".mod") || name.endsWith(".pls")
              || name.endsWith(".m3u") )
        exten = "audio";
    else if ( name.endsWith(".pls") || name.endsWith(".m3u") )
        exten = "pls";
    else if ( name.endsWith(".doc") || name.endsWith(".docx") )
        exten = "document";
    else if ( name.endsWith(".xls") || name.endsWith(".xlsx") )
        exten = "spreadsheet";
    else if ( name.endsWith(".odt") )
        exten = "document";
    else if ( name.endsWith(".ppt") )
        exten = "presentation";
    else if ( name.endsWith(".ods") )
        exten = "spreadsheet";
    else if ( name.endsWith(".pdf") )
        exten = "pdf";
    else if ( name.endsWith(".deb") )
        exten = "deb";
    else if ( name.endsWith(".rpm") )
        exten = "package";
    else if ( name.endsWith(".vcf") )
        exten = "vcard";
    else if ( name.endsWith(".apk") )
        exten = "android";
    else if ( name.endsWith(".tar") || name.endsWith(".tar.gz")
              || name.endsWith(".zip") || name.endsWith(".tar.bz2")
              || name.endsWith(".rar") )
        exten = "tar";
    else
        exten = "unknown";

    return exten;

}

QString GetDate(QString date)
{
    date.replace("Jan","01");
    date.replace("Feb","02");
    date.replace("Mar","03");
    date.replace("Apr","04");
    date.replace("May","05");
    date.replace("Jun","06");
    date.replace("Jul","07");
    date.replace("Aug","08");
    date.replace("Sep","09");
    date.replace("Oct","10");
    date.replace("Nov","11");
    date.replace("Dec","12");
    date.remove(date.length()-9,9);
    date.replace(" ","/");
    return date;
}
