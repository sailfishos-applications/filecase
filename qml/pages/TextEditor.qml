import QtQuick 2.0
import Sailfish.Silica 1.0
import FileCase 1.0

Page {
    id: infoPage

    allowedOrientations: appWindow.pagesOrientations

    property string fileName

    property bool loaded: false

    onStatusChanged: {
        if (status === PageStatus.Active)
            textData.text = utilities.getTextFile(fileName)
    }

    Connections {
        target: utilities

        onAddTextLines: textData.text = textData.text + lines

        onReadTextFinished: loaded = true

    }


    SilicaFlickable {
        anchors.fill: parent
        contentHeight: header.height + dataPanel.height + Theme.paddingMedium

        PullDownMenu {
            MenuItem {
                text: qsTr("Save")
                //visible: !cloudFile
                onClicked: {
                    utilities.saveTextFile(fileName, textData.text)
                }
            }
        }

        MyHeader {
            id: header
            title: getName(fileName)
            width: parent.width
        }

        BusyIndicator {
            size: BusyIndicatorSize.Small
            anchors.verticalCenter: header.verticalCenter
            anchors.left: parent.left
            anchors.leftMargin: Theme.paddingLarge
            visible: !loaded
            running: visible
        }

        Column {
            id: dataPanel
            width: parent.width
            anchors.top: header.bottom
            clip: true

            TextArea {
                id: textData
                anchors.top: header.bottom
                anchors.left: parent.left
                width: parent.width
                font.pixelSize: Theme.fontSizeExtraSmall
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                labelVisible: false
                background: Item { visible: false }
            }

        }

    }

}
