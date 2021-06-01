import QtQuick 2.0
import Sailfish.Silica 1.0
import FileCase 1.0

Page {
    id: webdavpage

    allowedOrientations: appWindow.pagesOrientations

    property bool showError: false

    property string service

    property bool autoDownload: false

    property string startFolder: "/"
    property bool newAccount: false
    property bool loaded: false

    WebDav { id: webdav }

    function renameFile(path, filename) {
        webdav.renameFile(path, filename)
    }

    function createFolder(filename) {
        webdav.newFolder(filename)
    }

    /*function shareFile(fileid) {
        webdav.shareFile(fileid)
    }*/

    onStatusChanged: {
        if(status === PageStatus.Activating) {
            if (startFolder==="start") {
                startFolder = "/"
            }
            currentDeviceFolder = startFolder
        }
        if(status === PageStatus.Active) {
            if (!loaded) {
                webdav.logIn(service, startFolder)
            }
            pageStack.pushAttached(Qt.resolvedUrl("FoldersPanel.qml"))
        }
    }

    Connections {
        target: webdav

        onCountChanged: {
            loaded = true
        }

        onRequestStarted: {
            loaded = false
        }

        onRequestDone: {
            loaded = true
            showError = !ok

            console.log("Request done: " + ok + " - " + webdavAccount.user)

            if (ok && webdavAccount.user===undefined)
                webdav.getQuota()
        }

        onDataChanged: {
            loaded = true
        }

        onLinkShared: {
            if (container.selFileType===WebDav.File)
                sharedLink = "https://docs.google.com/open?id=" + container.selFileLink
            else
                sharedLink = "https://docs.google.com/folder/d/" + container.selFileLink + "/edit"
            //var sDialog = ObjectCreator.createObject(Qt.resolvedUrl("SharedInfo.qml"), appWindow.pageStack);
            //sDialog.open();
        }

        onFileRenamed: itemRenamed(newname)

        onUserChanged: {
            console.log("USER: " + account)
            webdavAccount = account
        }

        onShowBanner: {
            ibanner.displayMessage(message, "webdav")
        }
    }


    SilicaFlickable {
        id: listado
        anchors.fill: parent

        PullDownMenu {
            enabled: loaded

            MenuItem {
                text: qsTr("Settings")
                enabled: webdav.user!==""
                onClicked: pageStack.push("WebDavSettings.qml")
            }

            MenuItem {
                text: qsTr("New folder")
                onClicked: pageStack.push("NewFolder.qml")
            }

            MenuItem {
                text: qsTr("Paste here")
                visible: webdavAction=="copy" || webdavAction=="move"
                enabled: webdavActionPathFull !== webdav.path
                onClicked: {
                    if (webdavAction=="copy")
                        webdav.copyFile(webdavActionPath)
                    else if (action=="move")
                        webdav.moveFile(webdavActionPath)
                    webdavAction = ""
                    webdavActionPath = ""
                }
            }
        }


        MyHeader {
            id: header
            title: webdav.path1==="/" ? service : webdav.path1
            description: webdav.path2
        }

        /*IconButton {
            id: upBtn
            enabled: webdav.path!=="/" && !loadingFolder
            anchors.verticalCenter: header.verticalCenter
            anchors.left: parent.left
            anchors.leftMargin: Theme.paddingMedium
            icon.source: "image://theme/icon-m-up"
            onClicked: {
                webdav.clearList()
                //controlPanel.open = false
                //browser.clearSelection()
                //loadingFolder = true
                webdav.goUp()
                //container.positionViewAtIndex(container.last, ListView.Center)
            }
        }*/

        SilicaListView {
            id: container
            //visible: !skyShowWeb

            enabled: loaded
            opacity: enabled? 1 : 0.5
            Behavior on opacity { FadeAnimation {} }

            property string selFileName
            property string selFilePath
            property string selFileType
            property string selFileExten
            property string selFileLink
            property string selFileDLink
            property string selFileShared
            property string selFileSize
            property string path
            property int last
            property string sectiontext

            anchors.fill: parent
            anchors.topMargin: header.height
            clip: true

            model: webdav


            delegate: FileDelegate
            {
                enabled: !busyIndicator.visible

                title: model.name
                description: model.modified + " - " + model.size
                //size: model.size
                imgsource: model.exten.indexOf(config.getHome())===-1? "/usr/share/filecase/" + iconTheme + "/" + model.exten + ".png" : model.exten
                mime: "image/png"
                //link: model.link
                //type: model.type
                //sec: model.sec

                function removeItem() {
                    remorseAction(qsTr("Deleting"),
                    function() {
                        webdav.removeFile(model.link)
                    })
                }

                onClicked: {
                    if (model.type===WebDav.Folder) {
                        //webdav.setPath(model.path)
                        pageStack.push("WebDavPage.qml", {"service":service, "startFolder":model.path, "loaded":false})
                    }
                    else
                    {
                        //browser.infoFiles(model.path)
                        //fileInfo.setFile()
                        pageStack.push("FileInfo.qml", {
                                           "cloudFile": true,
                                           "fileId": model.fid,
                                           "fileName": model.name,
                                           "fileLocation": service,
                                           "fileSize": model.size,
                                           "fileModified": model.modified,
                                           "fileCreated": model.created,
                                           "fileType": fileInfo.getFileMimetype(model.name),
                                           "fileImage": imgsource
                                       })
                    }
                }

                menu: ContextMenu {
                    id: bubbleMenu

                    Row {
                        anchors.horizontalCenter: parent.horizontalCenter
                        height: Theme.itemSizeSmall
                        //spacing: conv_view.isPortrait? Theme.paddingLarge : Theme.paddingLarge*2.5

                        IconButton {
                            icon.source: "image://theme/icon-m-cloud-download"
                            width: container.width/5
                            enabled: model.type===WebDav.File
                            onClicked: {
                                bubbleMenu.hide()
                                utilities.transferFile("WebDav-"+service, "download", model.name, model.link, model.size, model.realsize)
                            }
                        }

                        IconButton {
                            icon.source: "image://theme/icon-m-clipboard"
                            width: container.width/5
                            onClicked: {
                                bubbleMenu.hide()
                                webdavAction = "copy"
                                webdavActionPath = model.link
                                webdavActionPathType = model.type
                                webdavActionPathFull = webdav.path
                            }
                        }

                        IconButton {
                            icon.source: "file:///usr/share/filecase/icons/toolbar-cut.png"
                            width: container.width/5
                            onClicked: {
                                bubbleMenu.hide()
                                webdavAction = "move"
                                webdavActionPath = model.fid
                                webdavActionPathType = model.type
                                webdavActionPathFull = webdav.path
                            }
                        }

                        IconButton {
                            icon.source: "image://theme/icon-m-delete"
                            width: container.width/5
                            onClicked:{
                                bubbleMenu.hide()
                                removeItem()
                            }
                        }

                        /*IconButton {
                            icon.source: "image://theme/icon-m-link"
                            width: container.width/5
                            onClicked: {
                                bubbleMenu.hide()
                                //sharedName = container.selFileName
                                //sharedIcon = container.selFileExten
                                //driveclient.shareFile(model.dlink)
                            }
                        }*/

                        IconButton {
                            icon.source: "file:///usr/share/filecase/icons/toolbar-info.png"
                            width: container.width/5
                            onClicked: {
                                bubbleMenu.hide()
                                pageStack.push("FileInfo.qml", {
                                                   "cloudFile": true,
                                                   "fileId": model.fid,
                                                   "fileName": model.name,
                                                   "fileLocation": service,
                                                   "fileSize": model.type===SkyClient.File? model.size : "",
                                                   "fileModified": model.modified,
                                                   "fileCreated": model.created,
                                                   "fileType": model.type===SkyClient.File? fileInfo.getFileMimetype(model.name) : "",
                                                   "fileImage": imgsource
                                               })
                            }
                        }

                    }

                }
            }

        }


        BusyIndicator {
            id: busyIndicator
            anchors.centerIn: parent
            visible: !loaded
            running: visible
            size: BusyIndicatorSize.Large
        }

        ViewPlaceholder {
            enabled: container.count===0 && loaded
            Behavior on opacity { FadeAnimation { duration: 10 } }
            text: showError? qsTr("Error accessing folder") : qsTr("Folder is empty")
        }

    }

}
