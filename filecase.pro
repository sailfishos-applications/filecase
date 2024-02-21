# NOTICE:
#
# Application name defined in TARGET has a corresponding QML filename.
# If name defined in TARGET is changed, the following needs to be done
# to match new name:
#   - corresponding QML filename must be changed
#   - desktop icon filename must be changed
#   - desktop filename must be changed
#   - icon definition filename in desktop file must be changed
#   - translation filenames have to be changed

# The name of your application
TARGET = filecase

i18n_files.files = translations
i18n_files.path = /usr/share/$$TARGET

INSTALLS += i18n_files

CONFIG += sailfishapp

QT += core xml

DEFINES += QWEBDAVITEM_EXTENDED_PROPERTIES DEBUG_WEBDAV

SOURCES += src/filecase.cpp \
    src/browser.cpp \
    src/config.cpp \
    src/qtfilecopier.cpp \
    src/utils.cpp \
    src/filedeleter.cpp \
    src/fileinfo.cpp \
    src/loader.cpp \
    src/thumbgenerator.cpp \
    src/clipboard.cpp \
    src/search.cpp \
    src/driveclient.cpp \
    src/drivethumbnailer.cpp \
    src/drive/jsonparser.cpp \
    src/drive/json.cpp \
    src/dropboxclient.cpp \
    src/dropboxthumbnailer.cpp \
    src/dropbox/common.cpp \
    src/dropbox/consumerdata.cpp \
    src/dropbox/dropbox.cpp \
    src/dropbox/dropoauth.cpp \
    src/dropbox/userdata.cpp \
    src/dropbox/util.cpp \
    src/skyclient.cpp \
    src/skythumbnailer.cpp \
    src/compressedfiles.cpp \
    src/utilities.cpp \
    src/webdav.cpp \
    src/qwebdavlib/qnaturalsort.cpp \
    src/qwebdavlib/qwebdav.cpp \
    src/qwebdavlib/qwebdavdirparser.cpp \
    src/qwebdavlib/qwebdavitem.cpp \
    src/transfers.cpp \
    src/boxclient.cpp \
    src/boxthumbnailer.cpp

OTHER_FILES += qml/filecase.qml \
    rpm/filecase.spec \
    rpm/filecase.yaml \
    translations/*.ts \
    filecase.desktop \
    qml/pages/MainPage.qml \
    qml/pages/CoverPage.qml \
    qml/pages/MyHeader.qml \
    qml/pages/Settings.qml \
    qml/pages/FileInfo.qml \
    qml/pages/ValueItem.qml \
    qml/pages/RenameFile.qml \
    qml/pages/ClipboardPage.qml \
    qml/pages/Banner.qml \
    qml/pages/NewFolder.qml \
    qml/pages/FoldersPanel.qml \
    qml/pages/FileDelegate.qml \
    qml/pages/FolderDelegate.qml \
    qml/pages/SelectLanguage.qml \
    qml/pages/SearchPage.qml \
    qml/pages/Drive.qml \
    qml/pages/DriveSettings.qml \
    qml/pages/DriveUploadFolder.qml \
    qml/pages/TransfersDelegate.qml \
    qml/pages/SelDownloadFolder.qml \
    qml/pages/UploadFiles.qml \
    qml/pages/Dropbox.qml \
    qml/pages/DropboxSettings.qml \
    qml/pages/DropboxUploadFolder.qml \
    qml/pages/SkyDrive.qml \
    qml/pages/SkySettings.qml \
    qml/pages/SkyUploadFolder.qml \
    qml/pages/TransfersPage.qml \
    qml/pages/ButtonHeader.qml \
    qml/pages/Compressed.qml \
    qml/pages/ExtractDialog.qml \
    qml/pages/CompressDialog.qml \
    qml/pages/WebDavPage.qml \
    qml/pages/WebDavSettings.qml \
    qml/pages/WebDavUploadFolder.qml \
    qml/pages/AddWebDavAccount.qml \
    qml/pages/AddAccount.qml \
    qml/pages/Box.qml \
    qml/pages/BoxSettings.qml \
    qml/pages/BoxUploadFolder.qml \
    rpm/filecase.changes \
    qml/pages/TextViewer.qml \
    qml/pages/TextEditor.qml \
    qml/pages/AboutPage.qml

# to disable building translations every time, comment out the
# following CONFIG line
CONFIG += sailfishapp_i18n
TRANSLATIONS += translations/ca.ts \
                translations/de.ts \
                translations/es.ts \
                translations/fa.ts \
                translations/fi.ts \
                translations/fr.ts \
                translations/it.ts \
                translations/nl.ts \
                translations/ru.ts \
                translations/sv.ts \
                translations/zh.ts \
                translations/zh_HK.ts \
                translations/zh_TW.ts

HEADERS += \
    src/browser.h \
    src/config.h \
    src/qtfilecopier.h \
    src/utils.h \
    src/filedeleter.h \
    src/fileinfo.h \
    src/loader.h \
    src/thumbgenerator.h \
    src/clipboard.h \
    src/search.h \
    src/driveclient.h \
    src/drivethumbnailer.h \
    src/drive/jsonparser.h \
    src/drive/json.h \
    src/dropboxclient.h \
    src/dropboxthumbnailer.h \
    src/dropbox/common.h \
    src/dropbox/consumerdata.h \
    src/dropbox/dropbox.h \
    src/dropbox/dropoauth.h \
    src/dropbox/userdata.h \
    src/dropbox/util.h \
    src/skyclient.h \
    src/skythumbnailer.h \
    src/compressedfiles.h \
    src/utilities.h \
    src/webdav.h \
    src/qwebdavlib/qnaturalsort.h \
    src/qwebdavlib/qwebdav.h \
    src/qwebdavlib/qwebdav_global.h \
    src/qwebdavlib/qwebdavdirparser.h \
    src/qwebdavlib/qwebdavitem.h \
    src/transfers.h \
    src/boxclient.h \
    src/boxthumbnailer.h

icons.files = icons
icons.path = /usr/share/filecase/

INSTALLS += icons
