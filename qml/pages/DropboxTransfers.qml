import QtQuick 2.0
import Sailfish.Silica 1.0
import FileCase 1.0

Page {
    id: dropboxpage

    property bool loaded: false
    property string action: "nothing"
    property string sourcePath: ""
    property string sourcePathType: ""

    property bool isWorking

    Connections {
        target: dropboxclient

        /*onWorkingChanged: {
            isWorking = working;
        }*/
    }

    MyHeader {
        id: header
        title: qsTr("Transfers")
    }


    IconButton {
        enabled: dropboxTransfers.count>0
        anchors.left: parent.left
        anchors.leftMargin: Theme.paddingLarge
        anchors.verticalCenter: header.verticalCenter
        icon.source: dropboxclient.isWorking? "image://theme/icon-m-pause" : "image://theme/icon-m-play"
        onClicked: {
            if (!dropboxclient.isWorking)
                dropboxclient.startTransfers()
            else
                dropboxclient.stopTransfers()

        }
    }


    SilicaListView {
        id: container
        anchors.fill: parent
        anchors.topMargin: header.height
        clip: true

        model: dropboxTransfers

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
                dropboxclient.removeTransfer(model.path)
            }

            Component.onCompleted:
                console.log("ADD TRANSFER: " + model.name + " - " + model.path + " - " + model.exten)

        }

    }

}
