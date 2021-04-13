import QtQuick 2.0
import Sailfish.Silica 1.0
import FileCase 1.0

Dialog {

    allowedOrientations: appWindow.pagesOrientations

    onAccepted: {
        var res = driveclient.path
        if (res==="") res = "/";
        driveclient.setUploadFolder(res)
        pageStack.pop()
    }

    DriveClient { id: driveclient }

    onStatusChanged: {
        if(status === PageStatus.Activating) {
            driveclient.setPath2(config.readConfig("Google Drive/upload_folder","/||root").split("||")[0],
                                 config.readConfig("Google Drive/upload_folder","/||root").split("||")[1])
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
        title: driveclient.path1==="/" ? "Google Drive" : driveclient.path1
        description: driveclient.path2
        anchors.top: header.bottom
        anchors.left: upBtn.right
        //anchors.leftMargin: Theme.paddingMedium
        anchors.right: parent.right
    }

    IconButton {
        id: upBtn
        enabled: driveclient.path!=="/" && driveclient.path!=="root"
        anchors.verticalCenter: header2.verticalCenter
        anchors.left: parent.left
        anchors.leftMargin: Theme.paddingLarge
        icon.source: "image://theme/icon-m-up"
        onClicked: {
            driveclient.clearList()
            //controlPanel.open = false
            //browser.clearSelection()
            //loadingFolder = true
            driveclient.goUp()
            //container.positionViewAtIndex(container.last, ListView.Center)
        }
    }

    Connections {
        target: driveclient

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

        model: driveclient

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
                driveclient.setPath2(model.path, model.link)
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
