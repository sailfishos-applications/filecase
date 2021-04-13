import QtQuick 2.0
import Sailfish.Silica 1.0
import FileCase 1.0

Dialog {

    id: dialog

    allowedOrientations: appWindow.pagesOrientations

    property string zipfile: ""


    onAccepted: {
        utilities.extractFiles(zipfile, ren.text)
        //pageStack.pop()
    }

    onStatusChanged: {
        if (status===PageStatus.Activating)
            startupFolder.value = getName(config.readConfig("ExtractFolder", "/home/nemo"))
    }


    DialogHeader {
        id: header
        acceptText: qsTr("Done")
        cancelText: qsTr("Cancel")
        spacing: 0
    }

    SilicaFlickable {
        anchors.fill: parent
        anchors.topMargin: header.height
        contentHeight: col1.height+Theme.paddingLarge
        clip: true

        Column {
            id: col1
            width: parent.width
            spacing: Theme.paddingLarge

            PageHeader {
                id: header2
                title: qsTr("Extract files")
                width: parent.width
            }

            TextField {
                id: ren
                width: parent.width
                placeholderText: qsTr("Password (if required)")
                label: qsTr("Password (if required)")
                //text: name
                focus: true
                EnterKey.enabled: ren.text.length>0
                EnterKey.iconSource: "image://theme/icon-m-enter-accept"
                EnterKey.onClicked: dialog.accept()
            }

            ValueButton {
                id: startupFolder
                label: qsTr("Destination")
                onClicked: {
                    currentLocation = "extract"
                    pageStack.push("SelDownloadFolder.qml")
                }
            }

        }


    }


}
