import QtQuick 2.0
import Sailfish.Silica 1.0
import FileCase 1.0

Dialog {

    allowedOrientations: appWindow.pagesOrientations

    onAccepted: {
        var res = dropboxclient.path
        if (res==="") res = "/";
        dropboxclient.setUploadFolder(res)
        pageStack.pop()
    }

    DropboxClient { id: dropboxclient }

    onStatusChanged: {
        if(status === PageStatus.Activating) {
            dropboxclient.setPath2(config.readConfig("Dropbox/upload_folder","/"))
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
        title: dropboxclient.path1==="/" ? "Dropbox" : dropboxclient.path1
        description: dropboxclient.path2
        anchors.top: header.bottom
        anchors.left: upBtn.right
        //anchors.leftMargin: Theme.paddingMedium
        anchors.right: parent.right
    }

    IconButton {
        id: upBtn
        enabled: dropboxclient.path!=="/" && dropboxclient.path!=="root"
        anchors.verticalCenter: header2.verticalCenter
        anchors.left: parent.left
        anchors.leftMargin: Theme.paddingLarge
        icon.source: "image://theme/icon-m-up"
        onClicked: {
            dropboxclient.clearList()
            //controlPanel.open = false
            //browser.clearSelection()
            //loadingFolder = true
            dropboxclient.goUp()
            //container.positionViewAtIndex(container.last, ListView.Center)
        }
    }

    Connections {
        target: dropboxclient

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

        model: dropboxclient

        delegate: FileDelegate
        {
            enabled: model.exten==="folder"
            opacity: enabled? 1 : 0.5

            title: model.name
            description: model.modified + " - " + model.size
            imgsource: model.exten.indexOf(config.getHome())===-1? "/usr/share/filecase/icons/" + model.exten + ".png" : model.exten
            mime: "image/png"

            onClicked: {
                loadingFolder = true
                dropboxclient.setPath2(model.link)
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
