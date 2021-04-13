import QtQuick 2.0
import Sailfish.Silica 1.0
import FileCase 1.0
import QtWebKit 3.0
import QtWebKit.experimental 1.0

Page {
    id: drivepage

    allowedOrientations: appWindow.pagesOrientations

    property bool showError: false

    property bool autoDownload: false

    property string startFolder: "/"
    property string startFolderLink: "0"
    property bool newAccount: false
    property bool loaded: false

    property bool showWeb: false

    DriveClient { id: driveclient }

    backNavigation: startFolder!=="/" || newAccount

    function renameFile(path, filename) {
        driveclient.renameFile(path, filename)
    }

    function shareFile(fileid) {
        driveclient.shareFile(fileid)
    }

    function createFolder(filename) {
        driveclient.newFolder(filename)
    }

    onStatusChanged: {
        if(status === PageStatus.Activating) {
            if (startFolder==="start") {
                startFolder = "/"
                startFolderLink = "root"
                driveAccount = []
            }
            currentDeviceFolder = startFolder
            driveclient.setPath(startFolder)
        }
        if(status === PageStatus.Active) {
            if (!loaded) {
                driveclient.logIn(startFolderLink)
            }
            if (showWeb) {
                console.log("Reloading page")
                webView.reload()
            }
            if (!newAccount)
                pageStack.pushAttached(Qt.resolvedUrl("FoldersPanel.qml"))
        }
    }

    Connections {
        target: driveclient

        onLoggedIn: {
            if (newaccount) {
                cloudModel.clear()
                utilities.loadClouds()
                pageStack.pop()
            }
        }

        onOpenUrl: {
            showWeb = true
            webView.url = url
        }

        onCloseUrl: {
            showWeb = false
            //webView.url = ""
            if (!loged) {
                loaded = false
            }
        }

        onCountChanged: {
            loaded = true
        }

        onRequestStarted: {
            showWeb = false
            loaded = false
        }

        onRequestDone: {
            loaded = true
            showError = !ok

            if (ok && driveAccount.user===undefined)
                driveclient.getQuota()
        }

        onDataChanged: {
            loaded = true
        }

        onLinkShared: {
            if (currentLocation === "Google Drive")
                itemLinkShared(url)
        }

        onUserChanged: {
            console.log("USER: " + account)
            driveAccount = account
        }

        onShowBanner: {
            ibanner.displayMessage(message, "drive")
        }

        onFileRenamed: itemRenamed(newname)
    }


    SilicaFlickable {
        id: listado
        anchors.fill: parent

        PullDownMenu {
            enabled: loaded && !showWeb

            MenuItem {
                text: qsTr("Settings")
                enabled: driveAccount && driveAccount.user!==undefined
                onClicked: pageStack.push("DriveSettings.qml")
            }

            MenuItem {
                text: qsTr("New folder")
                enabled: driveAccount && driveAccount.user!==undefined
                onClicked: pageStack.push("NewFolder.qml")
            }

            MenuItem {
                text: qsTr("Paste here")
                visible: driveAction=="copy" || driveAction=="move"
                enabled: driveActionPathFull !== driveclient.path
                onClicked: {
                    if (driveAction=="copy")
                        driveclient.copyFile(driveActionPath)
                    else if (action=="move")
                        driveclient.moveFile(driveActionPath)
                    driveAction = ""
                    driveActionPath = ""
                }
            }
        }


        MyHeader {
            id: header
            title: driveclient.path1==="/" ? "Google Drive" : driveclient.path1
            description: driveclient.path2
        }

        /*IconButton {
            id: upBtn
            visible: drivePageLoaded
            enabled: driveclient.path!=="/" && driveclient.path!=="root" && !loadingFolder
            anchors.verticalCenter: header.verticalCenter
            anchors.left: parent.left
            anchors.leftMargin: Theme.paddingMedium
            icon.source: "image://theme/icon-m-up"
            onClicked: {
                driveclient.clearList()
                //controlPanel.open = false
                //browser.clearSelection()
                //loadingFolder = true
                driveclient.goUp()
                //container.positionViewAtIndex(container.last, ListView.Center)
            }
        }*/

        SilicaListView {
            id: container
            visible: !showWeb

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

            model: driveclient


            delegate: FileDelegate
            {
                enabled: !busyIndicator.visible

                title: model.name
                description: model.modified + " - " + model.size
                //size: model.size
                imgsource: model.exten.indexOf("/home/nemo")===-1? "/usr/share/filecase/" + iconTheme + "/" + model.exten + ".png" : model.exten
                mime: "image/png"
                //link: model.link
                //type: model.type
                //sec: model.sec

                function removeItem() {
                    remorseAction(qsTr("Deleting"),
                    function() {
                        driveclient.removeFile(model.link)
                    })
                }

                onClicked: {
                    if (model.type===DriveClient.Folder) {
                        //driveclient.clearList()
                        //loadingFolderDrive = true
                        pageStack.push("Drive.qml", {"startFolder":driveclient.path+"/"+model.name, "startFolderLink":model.link, "loaded":false})
                    }
                    else
                    {
                        //browser.infoFiles(model.path)
                        //fileInfo.setFile()
                        pageStack.push("FileInfo.qml", {
                                           "cloudFile": true,
                                           "fileId": model.link,
                                           "fileName": model.name,
                                           "fileLocation": "Google Drive",
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
                            enabled: model.type===DriveClient.File
                            onClicked: {
                                bubbleMenu.hide()
                                utilities.transferFile("Google Drive", "download", model.name, model.dlink, model.size, model.realsize)
                            }
                        }

                        IconButton {
                            icon.source: "image://theme/icon-m-clipboard"
                            width: container.width/5
                            enabled: model.type===DriveClient.File
                            onClicked: {
                                bubbleMenu.hide()
                                driveAction = "copy"
                                driveActionPath = model.link
                                driveActionPathType = model.type
                                driveActionPathFull = startFolderLink
                            }
                        }

                        IconButton {
                            icon.source: "file:///usr/share/filecase/icons/toolbar-cut.png"
                            width: container.width/5
                            enabled: model.type===DriveClient.File
                            onClicked: {
                                bubbleMenu.hide()
                                driveAction = "move"
                                driveActionPath = model.link
                                driveActionPathType = model.type
                                driveActionPathFull = startFolderLink
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
                                                   "fileId": model.link,
                                                   "fileName": model.name,
                                                   "fileLocation": "Google Drive",
                                                   "fileSize": model.type===DriveClient.File? model.size : "",
                                                   "fileModified": model.modified,
                                                   "fileCreated": "",
                                                   "fileType": fileInfo.getFileMimetype(model.type===DriveClient.File? model.name : "/home"),
                                                   "fileImage": imgsource
                                               })
                            }
                        }

                    }

                }
            }

        }


        SilicaFlickable {
            id: flickArea
            anchors.fill: parent
            anchors.topMargin: header.height
            //contentWidth: webView.width
            //contentHeight: webView.height
            flickableDirection: Flickable.HorizontalAndVerticalFlick
            visible: showWeb
            clip: true

            WebView {
                id: webView
                anchors.fill: parent
                smooth: true
                visible: showWeb

                experimental.preferences.navigatorQtObjectEnabled: true;

                onLoadingChanged: {
                    if (status===WebView.LoadSucceededStatus)
                        experimental.evaluateJavaScript("navigator.qt.postMessage(document.documentElement.outerHTML)");
                        //driveclient.parseData(webView.url, webView.html)
                }
                onNavigationRequested: {
                    console.log("Requested " + url)
                    request.action = WebView.AcceptRequest;
                }

                experimental.onMessageReceived: {
                    driveclient.parseData(webView.url, message.data)
                }

            }

        }

        BusyIndicator {
            id: busyIndicator
            anchors.centerIn: parent
            visible: !loaded && !showWeb
            running: visible
            size: BusyIndicatorSize.Large
        }

        ViewPlaceholder {
            id: emptyText
            enabled: container.count===0 && loaded && !showWeb
            Behavior on opacity { FadeAnimation { duration: 10 } }
            text: showError? qsTr("Error accessing folder") : qsTr("Folder is empty")
        }

        IconButton {
            anchors.top: emptyText.bottom
            anchors.topMargin: Theme.paddingLarge
            visible: emptyText.enabled && showError
            icon.source: "image://theme/icon-m-refresh"
            anchors.horizontalCenter: parent.verticalCenter
            onClicked: driveclient.logIn(startFolderLink)
        }

    }

}
