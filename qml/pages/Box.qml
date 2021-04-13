import QtQuick 2.0
import Sailfish.Silica 1.0
import FileCase 1.0
import QtWebKit 3.0
import QtWebKit.experimental 1.0

Page {
    id: boxpage

    allowedOrientations: appWindow.pagesOrientations

    property bool showError: false

    property bool autoDownload: false

    property string startFolder: "/"
    property string startFolderLink: "0"
    property bool newAccount: false
    property bool loaded: false

    property bool showWeb: false

    BoxClient { id: boxclient }

    backNavigation: startFolder!=="/" || newAccount

    function renameFile(path, filename) {
        boxclient.renameFile(path, filename)
    }

    function shareFile(fileid) {
        boxclient.shareFile(fileid)
    }

    function createFolder(filename) {
        driveclient.newFolder(filename)
    }

    onStatusChanged: {
        if(status === PageStatus.Activating) {
            if (startFolder==="start") {
                startFolder = "/"
                startFolderLink = "0"
                boxAccount = []
            }
            currentDeviceFolder = startFolder
            boxclient.setPath(startFolder)
        }
        if(status === PageStatus.Active) {
            if (!loaded) {
                boxclient.logIn(startFolderLink)
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
        target: boxclient

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

            if (ok && boxAccount.user===undefined)
                boxclient.getQuota()
        }

        onDataChanged: {
            loaded = true
        }

        onLinkShared: {
            if (currentLocation === "Box")
                itemLinkShared(url)
        }

        onUserChanged: {
            console.log("USER: " + account)
            boxAccount = account
        }

        onShowBanner: {
            ibanner.displayMessage(message, "box")
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
                enabled: boxAccount && boxAccount.user!==undefined
                onClicked: pageStack.push("BoxSettings.qml")
            }

            MenuItem {
                text: qsTr("New folder")
                enabled: boxAccount && boxAccount.user!==undefined
                onClicked: pageStack.push("NewFolder.qml")
            }

            MenuItem {
                text: qsTr("Paste here")
                visible: boxAction=="copy" || boxAction=="move"
                enabled: boxActionPathFull !== boxclient.path
                onClicked: {
                    if (boxAction=="copy")
                        boxclient.copyFile(boxActionPath, boxActionPathType)
                    else if (boxAction=="move")
                        boxclient.moveFile(boxActionPath, boxActionPathType)
                    boxAction = ""
                    boxActionPath = ""
                }
            }
        }


        MyHeader {
            id: header
            title: boxclient.path1==="/" ? "Box" : boxclient.path1
            description: boxclient.path2
        }

        /*IconButton {
            id: upBtn
            visible: boxPageLoaded
            enabled: boxclient.path!=="/" && !loadingFolder
            anchors.verticalCenter: header.verticalCenter
            anchors.left: parent.left
            anchors.leftMargin: Theme.paddingMedium
            icon.source: "image://theme/icon-m-up"
            onClicked: {
                boxclient.clearList()
                //controlPanel.open = false
                //browser.clearSelection()
                //loadingFolder = true
                boxclient.goUp()
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

            model: boxclient


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
                        boxclient.removeFile(model.fid, model.type===BoxClient.File? "file" : "folder")
                    })
                }

                onClicked: {
                    if (model.type===BoxClient.Folder) {
                        //driveclient.clearList()
                        //loadingFolderDrive = true
                        pageStack.push("Box.qml", {"startFolder":boxclient.path+"/"+model.name, "startFolderLink":model.fid, "loaded":false})
                    }
                    else
                    {
                        //browser.infoFiles(model.path)
                        //fileInfo.setFile()
                        pageStack.push("FileInfo.qml", {
                                           "cloudFile": true,
                                           "fileId": model.fid,
                                           "fileName": model.name,
                                           "fileLocation": "Box",
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
                            enabled: model.type===BoxClient.File
                            onClicked: {
                                bubbleMenu.hide()
                                utilities.transferFile("Box", "download", model.name, model.fid, model.size, model.realsize)
                            }
                        }

                        IconButton {
                            icon.source: "image://theme/icon-m-clipboard"
                            width: container.width/5
                            onClicked: {
                                bubbleMenu.hide()
                                boxAction = "copy"
                                boxActionPath = model.fid
                                boxActionPathType = model.type===BoxClient.Folder? "folders" : "files"
                                boxActionPathFull = startFolderLink
                            }
                        }

                        IconButton {
                            icon.source: "file:///usr/share/filecase/icons/toolbar-cut.png"
                            width: container.width/5
                            onClicked: {
                                bubbleMenu.hide()
                                boxAction = "move"
                                boxActionPath = model.fid
                                boxActionPathType = model.type===BoxClient.Folder? "folders" : "files"
                                boxActionPathFull = startFolderLink
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
                                                   "fileLocation": "Box",
                                                   "fileSize": model.type===BoxClient.File? model.size : "",
                                                   "fileModified": model.modified,
                                                   "fileCreated": model.created,
                                                   "fileType": fileInfo.getFileMimetype( model.type===BoxClient.File? model.name : "/home"),
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
                    boxclient.parseData(webView.url, message.data)
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
            onClicked: boxclient.logIn(startFolderLink)
        }

    }

}
