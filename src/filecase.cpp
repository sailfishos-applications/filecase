#include <QtQuick>
#include <sailfishapp.h>
#include <QObject>
#include <QTextCodec>

#include "config.h"
#include "browser.h"
#include "fileinfo.h"
#include "utilities.h"
#include "clipboard.h"
#include "search.h"
#include "compressedfiles.h"
#include "driveclient.h"
#include "dropboxclient.h"
#include "skyclient.h"
#include "boxclient.h"
#include "webdav.h"

#include "transfers.h"

QStringList imagesToProcess;
Transfers *transfers;

int main(int argc, char *argv[])
{

    QTextCodec *linuxCodec = QTextCodec::codecForName("UTF-8");
    //QTextCodec::setCodecForTr(linuxCodec);
    //QTextCodec::setCodecForCStrings(linuxCodec);
    QTextCodec::setCodecForLocale(linuxCodec);


    QGuiApplication *app = SailfishApp::application(argc, argv);
    app->setOrganizationName("filecase");
    app->setApplicationName("filecase");

    transfers = new Transfers();

    QString lang;
    QTranslator translator;

    QSettings settings("cepiperez", "fileboxplus");
    lang = settings.value("Language", "undefined").toString();

    if (lang=="undefined")
    {
        lang=  QLocale().name();
        qDebug() << "Getting language from locale:" << lang;
    }
    else
    {
        qDebug() << "Stored language: " << lang;
    }

    if (QFile::exists("/usr/share/filecase/translations/"+ lang + ".qm"))
        translator.load("/usr/share/filecase/translations/" + lang);
    else
        translator.load("/usr/share/filecase/translations/en");

    app->installTranslator(&translator);


    QScopedPointer<QQuickView> window(SailfishApp::createView());
    window->setTitle("FileCase");

    //window->rootContext()->setContextProperty("view", window.data());
    //window->rootContext()->setContextProperty("app", app.data());
    window->engine()->addImportPath("/usr/share/filecase/qml");

    qmlRegisterType<Config>("FileCase", 1, 0, "Config");
    qmlRegisterType<Utilities>("FileCase", 1, 0, "Utilities");
    qmlRegisterType<Browser>("FileCase", 1, 0, "Browser");
    qmlRegisterType<FileInfo>("FileCase", 1, 0, "FilesInfo");
    qmlRegisterType<Clipboard>("FileCase", 1, 0, "Clipboard");
    qmlRegisterType<Search>("FileCase", 1, 0, "Search");
    qmlRegisterType<CompressedFiles>("FileCase", 1, 0, "CompressedFiles");

    qmlRegisterType<DriveClient>("FileCase", 1, 0, "DriveClient");
    qmlRegisterType<DropboxClient>("FileCase", 1, 0, "DropboxClient");
    qmlRegisterType<SkyClient>("FileCase", 1, 0, "SkyClient");
    qmlRegisterType<BoxClient>("FileCase", 1, 0, "BoxClient");
    qmlRegisterType<WebDav>("FileCase", 1, 0, "WebDav");

    window->setSource(SailfishApp::pathTo("qml/filecase.qml"));

    window->showFullScreen();
    return app->exec();
}

