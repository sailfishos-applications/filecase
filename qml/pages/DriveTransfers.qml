import QtQuick 2.0
import Sailfish.Silica 1.0
import FileCase 1.0

Page {
    id: drivepage

    property bool loaded: false
    property string action: "nothing"
    property string sourcePath: ""
    property string sourcePathType: ""

    property bool isWorking

    Connections {
        target: driveclient

        /*onWorkingChanged: {
            isWorking = working;
        }*/
    }

    MyHeader {
        id: header
        title: qsTr("Transfers")
    }


    IconButton {
        enabled: driveTransfers.count>0
        anchors.left: parent.left
        anchors.leftMargin: Theme.paddingLarge
        anchors.verticalCenter: header.verticalCenter
        icon.source: driveclient.isWorking? "image://theme/icon-m-pause" : "image://theme/icon-m-play"
        onClicked: {
            if (!driveclient.isWorking)
                driveclient.startTransfers()
            else
                driveclient.stopTransfers()

        }
    }


    SilicaListView {
        id: container
        anchors.fill: parent
        anchors.topMargin: header.height
        clip: true

        model: driveTransfers

        delegate: TransfersDelegate
        {
            name: model.name
            preview: model.mode==="download"? (model.exten.indexOf("/home")>-1? model.exten : "/usr/share/filecase/" + iconTheme + "/" + model.exten + ".png") :
                     model.exten.indexOf("/")===-1? "/usr/share/filecase/" + iconTheme + "/" + model.exten + ".png" : model.path
            mime: "image/png"
            mode: model.mode
            path: model.path
            progress: model.progress
            filesize: model.size

            onRemoved: {
                driveclient.removeTransfer(model.path)
            }

            Component.onCompleted:
                console.log("ADD TRANSFER: " + model.name + " - " + model.path + " - " + model.exten)

        }

    }

}
