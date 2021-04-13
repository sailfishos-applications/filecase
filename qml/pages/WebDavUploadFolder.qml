import QtQuick 2.0
import Sailfish.Silica 1.0
import FileCase 1.0

Dialog {

    allowedOrientations: appWindow.pagesOrientations

    onAccepted: {
        webdav.setUploadFolder(webdav.path)
        pageStack.pop()
    }

    //canAccept: ren.text.length>0

    WebDav { id: webdav }

    //property bool loaded: false
    //property string startFolder: "/"

    onStatusChanged: {
        if(status === PageStatus.Activating) {
            webdav.logIn(currentLocation.replace("WebDav-",""), config.readConfig(currentLocation+"/upload_folder","/"))
        }
    }

    DialogHeader {
        id: header
        acceptText: qsTr("Done")
        cancelText: qsTr("Cancel")
        spacing: 0
    }

    MyHeader {
        id: header2
        title: webdav.path1==="/" ? webdav.service : webdav.path1
        description: webdav.path2
        anchors.top: header.bottom
        anchors.left: upBtn.right
        //anchors.leftMargin: Theme.paddingMedium
        anchors.right: parent.right
    }

    IconButton {
        id: upBtn
        enabled: webdav.path!=="/" && webdav.path!=="root"
        anchors.verticalCenter: header2.verticalCenter
        anchors.left: parent.left
        anchors.leftMargin: Theme.paddingLarge
        icon.source: "image://theme/icon-m-up"
        onClicked: {
            webdav.clearList()
            //controlPanel.open = false
            //browser.clearSelection()
            //loadingFolder = true
            webdav.goUp()
            //container.positionViewAtIndex(container.last, ListView.Center)
        }
    }

    Connections {
        target: webdav

        onRequestStarted: {
            loadingFolder = true
        }

        onRequestDone: {
            loadingFolder = false
        }
    }

    SilicaListView {
        id: container
        anchors.fill: parent
        anchors.topMargin: header.height + header2.height
        clip: true

        enabled: !loadingFolder

        model: webdav

        delegate: FileDelegate
        {
            enabled: model.exten==="folder"
            opacity: enabled? 1 : 0.5

            title: model.name
            description: model.modified + " - " + model.size
            imgsource: model.exten.indexOf("/home/nemo")===-1? "/usr/share/filecase/icons/" + model.exten + ".png" : model.exten
            mime: "image/png"

            onClicked: {
                loadingFolder = true
                webdav.setPath(model.path)
            }
        }


    }

    BusyIndicator {
        id: busyIndicator
        anchors.centerIn: parent
        visible: loadingFolder
        running: visible
        size: BusyIndicatorSize.Large
    }

    ViewPlaceholder {
        enabled: container.count===0 && !loadingFolder
        Behavior on opacity { FadeAnimation { duration: 10 } }
        text: qsTr("Folder is empty")
    }

}
