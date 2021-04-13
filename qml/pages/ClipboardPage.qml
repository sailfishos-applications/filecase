import QtQuick 2.0
import Sailfish.Silica 1.0
import FileCase 1.0
import org.nemomobile.thumbnailer 1.0

Page {
    id: clipboardPage

    allowedOrientations: appWindow.pagesOrientations

    onStatusChanged: {
        if (status === PageStatus.Activating) {
            clipboard.reload()
        }
    }

    SilicaFlickable {
        anchors.top: parent.top
        anchors.bottom: controlPanel.top
        width: parent.width

        PullDownMenu {
            //enabled: !selectionmode
            MenuItem {
                text: qsTr("Clear clipboard")
                onClicked: {
                    config.setConfig("Clipboard", "")
                    config.setConfig("ClipboardSelected", "")
                    clipboard.reload()
                }
            }
        }

        MyHeader {
            id: header
            title: qsTr("Clipboard")
        }

        SilicaListView {
            id: container
            anchors.fill: parent
            anchors.topMargin: header.height
            clip: true

            model: clipboard

            delegate: FileDelegate {
                width: parent.width
                contentHeight: Theme.itemSizeMedium

                highlighted: down || model.type===1 //selectionmodeItems.indexOf(model.path)>-1

                imgsource: model.exten.indexOf("/")===-1? "/usr/share/filecase/icons/" + exten + ".png" :
                            model.path

                mime: model.exten.indexOf("/")>-1? model.exten : "image/png"
                title: model.name
                description: model.date

                onClicked: {
                    clipboard.selectFile(model.path)
                }

            }

        }

        ViewPlaceholder {
            enabled: container.count===0 && !loadingFolder
            Behavior on opacity { FadeAnimation { duration: 10 } }
            text: qsTr("Clipboard is empty")
        }

    }

    DockedPanel {
        id: controlPanel
        width: parent.width
        height: Theme.itemSizeLarge
        dock: Dock.Bottom
        open: clipboard.multi1>0

        Image {
            anchors.fill: parent
            fillMode: Image.PreserveAspectFit
            source: "image://theme/graphic-gradient-edge"
        }

        Row {
            anchors.centerIn: parent
            //anchors.horizontalCenter: parent.horizontalCenter
            spacing: clipboardPage.isPortrait? Theme.paddingLarge*3 : Theme.paddingLarge*6

            IconButton {
                icon.source: "image://theme/icon-m-clipboard"
                enabled: clipboard.multi1>0
                onClicked: {
                    working = true
                    pageStack.pop()
                    action = "copy/move"
                    //browser.checkClipboard("copy", config.readConfig("AutoRemove", "false"))
                    var cpage = pageStack.find(function(page) { return page.startFolder===currentDeviceFolder; })
                    cpage.setClipboard("copy")
                }
            }

            IconButton {
                icon.source: "file:///usr/share/filecase/icons/toolbar-cut.png"
                enabled: clipboard.multi1>0
                onClicked: {
                    working = true
                    pageStack.pop()
                    action = "copy/move"
                    //browser.checkClipboard("move", config.readConfig("AutoRemove", "false"))
                    var cpage = pageStack.find(function(page) { return page.startFolder===currentDeviceFolder; })
                    cpage.setClipboard("move")
                }
            }

            IconButton {
                icon.source: "image://theme/icon-m-delete"
                enabled: clipboard.multi1>0
                onClicked: {
                    clipboard.removeSelected()
                }
            }


        }

    }


}
