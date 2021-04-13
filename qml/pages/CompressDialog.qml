import QtQuick 2.0
import Sailfish.Silica 1.0
import FileCase 1.0

Dialog {

    id: dialog

    allowedOrientations: appWindow.pagesOrientations

    property string filelist: ""
    property string format: "tar.gz"

    canAccept: ren.text!==""

    onAccepted: {
        utilities.compressFiles(filelist, currentDeviceFolder, ren.text, format)
        pageStack.pop()
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
                title: qsTr("Compress files")
                width: parent.width
            }

            TextField {
                id: ren
                width: parent.width
                placeholderText: qsTr("File name")
                label: qsTr("File name")
                //text: name
                focus: true
                EnterKey.enabled: ren.text.length>0
                EnterKey.iconSource: "image://theme/icon-m-enter-accept"
                EnterKey.onClicked: dialog.accept()
            }

            ComboBox {
                label: qsTr("Format")
                menu: ContextMenu {
                    MenuItem { text: ".zip"; enabled: utilities.fileExists("/usr/bin/zip") }
                    MenuItem { text: ".tar" }
                    MenuItem { text: ".tar.gz" }
                    MenuItem { text: ".tar.bz2" }
                }
                currentIndex: 2
                onCurrentItemChanged: {
                    if (currentIndex == 0) format = "zip"
                    else if (currentIndex == 1) format = "tar"
                    else if (currentIndex == 2) format = "tar.gz"
                    else if (currentIndex == 3) format = "tar.bz2"
                }
            }


        }


    }


}
