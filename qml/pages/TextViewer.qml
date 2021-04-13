import QtQuick 2.0
import Sailfish.Silica 1.0
import FileCase 1.0

Page {
    id: infoPage

    allowedOrientations: appWindow.pagesOrientations

    property string fileName

    property bool loaded: false

    onStatusChanged: {
        if (status === PageStatus.Active) {
            console.log("Opening text file: " + fileName)
            textData.text = utilities.getTextFile(fileName)
        }
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
                text: qsTr("Copy text")
                onClicked: {
                    textHelper.text = textData.text
                    textHelper.copy()
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

            Text {
                id: textData
                anchors.top: header.bottom
                x: Theme.paddingLarge
                width: parent.width -Theme.paddingLarge*2
                font.pixelSize: Theme.fontSizeExtraSmall
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                color: Theme.secondaryColor
            }

        }

    }

    TextArea {
        id: textHelper
        visible: false
    }

}
