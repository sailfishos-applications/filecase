# NOTICE:
#
# Application name defined in TARGET has a corresponding QML filename.
# If the name defined in TARGET is changed, the following must be done
# to match the new name:
#   - the corresponding QML filename must be adapted
#   - the desktop icon filename must be adapted
#   - the name of the `.desktop` file must be adapted
#   - the icon definition filename in `.desktop` file must be adapted
#   - the original translation strings `.ts` filename has to be adapted

# The name of your application; should default to the name of the project file,
# see https://doc.qt.io/qt-5/qmake-common-projects.html#building-an-application
TARGET = filecase

CONFIG += sailfishapp

QT += core xml

DEFINES += QWEBDAVITEM_EXTENDED_PROPERTIES DEBUG_WEBDAV
DEFINES += VERSION=\\\"$${VERSION}\\\"

SOURCES += \
    src/boxclient.cpp \
    src/boxthumbnailer.cpp \
    src/browser.cpp \
    src/clipboard.cpp \
    src/compressedfiles.cpp \
    src/config.cpp \
    src/drive/json.cpp \
    src/drive/jsonparser.cpp \
    src/driveclient.cpp \
    src/drivethumbnailer.cpp \
    src/dropbox/common.cpp \
    src/dropbox/consumerdata.cpp \
    src/dropbox/dropbox.cpp \
    src/dropbox/dropoauth.cpp \
    src/dropbox/qt-json/json.cpp \
    src/dropbox/userdata.cpp \
    src/dropbox/util.cpp \
    src/dropboxclient.cpp \
    src/dropboxthumbnailer.cpp \
    src/filecase.cpp \
    src/filedeleter.cpp \
    src/fileinfo.cpp \
    src/loader.cpp \
    src/megacli.cpp \
    src/qtfilecopier.cpp \
    src/qwebdavlib/qnaturalsort.cpp \
    src/qwebdavlib/qwebdav.cpp \
    src/qwebdavlib/qwebdavdirparser.cpp \
    src/qwebdavlib/qwebdavitem.cpp \
    src/search.cpp \
    src/skyclient.cpp \
    src/skythumbnailer.cpp \
    src/thumbgenerator.cpp \
    src/transfers.cpp \
    src/utilities.cpp \
    src/utils.cpp \
    src/webdav.cpp

OTHER_FILES += \
    rpm/filecase.spec \
    rpm/filecase.changes \
    rpm/filecase.rpmlintrc \
    translations/*.ts \
    filecase.desktop \
    qml/filecase.qml \
    qml/pages/AboutPage.qml \
    qml/pages/AddAccount.qml \
    qml/pages/AddWebDavAccount.qml \
    qml/pages/Banner.qml \
    qml/pages/Box.qml \
    qml/pages/BoxSettings.qml \
    qml/pages/BoxUploadFolder.qml \
    qml/pages/ButtonHeader.qml \
    qml/pages/ClipboardPage.qml \
    qml/pages/CompressDialog.qml \
    qml/pages/Compressed.qml \
    qml/pages/CoverPage.qml \
    qml/pages/Drive.qml \
    qml/pages/DriveSettings.qml \
    qml/pages/DriveTransfers.qml \
    qml/pages/DriveUploadFolder.qml \
    qml/pages/Dropbox.qml \
    qml/pages/DropboxSettings.qml \
    qml/pages/DropboxTransfers.qml \
    qml/pages/DropboxUploadFolder.qml \
    qml/pages/ExtractDialog.qml \
    qml/pages/FileDelegate.qml \
    qml/pages/FileInfo.qml \
    qml/pages/FolderDelegate.qml \
    qml/pages/FoldersPanel.qml \
    qml/pages/MainPage.qml \
    qml/pages/MyHeader.qml \
    qml/pages/NewFolder.qml \
    qml/pages/RenameFile.qml \
    qml/pages/SearchPage.qml \
    qml/pages/SelDownloadFolder.qml \
    qml/pages/SelectLanguage.qml \
    qml/pages/Settings.qml \
    qml/pages/SkyDrive.qml \
    qml/pages/SkySettings.qml \
    qml/pages/SkyTransfers.qml \
    qml/pages/SkyUploadFolder.qml \
    qml/pages/TextEditor.qml \
    qml/pages/TextViewer.qml \
    qml/pages/TransfersDelegate.qml \
    qml/pages/TransfersPage.qml \
    qml/pages/UploadFiles.qml \
    qml/pages/ValueItem.qml \
    qml/pages/WebDavPage.qml \
    qml/pages/WebDavSettings.qml \
    qml/pages/WebDavUploadFolder.qml

# to disable building translations every time,
# comment out the following CONFIG line
CONFIG += sailfishapp_i18n
TRANSLATIONS += \
    translations/ca.ts \
    translations/de.ts \
    translations/es.ts \
    translations/et.ts \
    translations/fa.ts \
    translations/fi.ts \
    translations/fr.ts \
    translations/it.ts \
    translations/nl.ts \
    ### translations/pl.ts \ ###
    translations/ru.ts \
    translations/sv.ts \
    translations/zh.ts \
    translations/zh_HK.ts \
    translations/zh_TW.ts

HEADERS += \
    src/boxclient.h \
    src/boxthumbnailer.h \
    src/browser.h \
    src/clipboard.h \
    src/compressedfiles.h \
    src/config.h \
    src/drive/json.h \
    src/drive/jsonparser.h \
    src/driveclient.h \
    src/drivethumbnailer.h \
    src/dropbox/common.h \
    src/dropbox/consumerdata.h \
    src/dropbox/dropbox.h \
    src/dropbox/dropoauth.h \
    src/dropbox/qt-json/json.h \
    src/dropbox/userdata.h \
    src/dropbox/util.h \
    src/dropboxclient.h \
    src/dropboxthumbnailer.h \
    src/filedeleter.h \
    src/fileinfo.h \
    src/loader.h \
    src/megacli.h \
    src/qtfilecopier.h \
    src/qwebdavlib/qnaturalsort.h \
    src/qwebdavlib/qwebdav.h \
    src/qwebdavlib/qwebdav_global.h \
    src/qwebdavlib/qwebdavdirparser.h \
    src/qwebdavlib/qwebdavitem.h \
    src/search.h \
    src/skyclient.h \
    src/skythumbnailer.h \
    src/thumbgenerator.h \
    src/transfers.h \
    src/utilities.h \
    src/utils.h \
    src/webdav.h

icons.files = icons
icons.path = /usr/share/$$TARGET

INSTALLS += icons
