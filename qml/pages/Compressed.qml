import QtQuick 2.0
import Sailfish.Silica 1.0
import FileCase 1.0

Page {
    id: compressed

    allowedOrientations: appWindow.pagesOrientations

    property string filename: ""
    property string name: ""

    property bool unrarError: false

    CompressedFiles {
        id: cbrowser
    }

    onStatusChanged: {
        if (status===PageStatus.Active && filename!=="")
        {
            unrarError = false
            cbrowser.loadZip(filename)
        }
    }

    Connections {
        target: cbrowser

        onCountChanged: {
            busyIndicator.visible = false
        }

        onUnrarError: {
            busyIndicator.visible = false
            unrarError = true
        }
    }


    SilicaFlickable {
        anchors.fill: parent

        PullDownMenu {
            enabled: !unrarError && !busyIndicator.visible
            MenuItem {
                text: qsTr("Extract files")
                onClicked: {
                    pageStack.replace("ExtractDialog.qml", {"zipfile": filename})
                }
            }
        }

        MyHeader {
            id: header
            title: name
        }

        Item {
            id: listado
            anchors.fill: parent
            anchors.topMargin: header.height
            clip: true

            BusyIndicator {
                id: busyIndicator
                anchors.centerIn: listado
                visible: true
                running: visible
                size: BusyIndicatorSize.Large
            }

            SilicaListView {
                id: container
                visible: ! busyIndicator.visible
                anchors.fill: listado

                model: cbrowser

                delegate: FileDelegate
                {
                    enabled: false
                    title: model.name
                    description: model.date
                    imgsource: "/usr/share/filecase/" + iconTheme + "/" + model.exten + ".png"
                    mime: "image/png"
                }

            }

            ViewPlaceholder {
                enabled: container.count===0 && !busyIndicator.visible && !unrarError
                text: qsTr("No files")
            }

            ViewPlaceholder {
                id: errorHolder
                enabled: unrarError
                text: qsTr("You need to install unrar to use this function")
            }


        }

    }


    /*ToolBarLayout {
        id: commonTools
        visible: true

        ToolIcon
        {
            iconSource: currentTheme==="sailfish"? "qrc:/icons/sailfish/toolbar-back.png" : "image://theme/icon-m-toolbar-back"+(theme.inverted?"-white":"")
            onClicked: {
                cbrowser.cancelReading()
                config.setConfig("CompressedFile", "")
                pageStack.pop()
            }
        }

        ToolButton
        {
            text: qsTr("Extract files")
            anchors.right: parent.right
            anchors.rightMargin: 14
            enabled: ! busyIndicator.visible
            onClicked: {
                action = "extract"
                pageStack.pop()
            }
        }

    }*/


}
