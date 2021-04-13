import QtQuick 2.0
import Sailfish.Silica 1.0
import FileCase 1.0
import org.nemomobile.thumbnailer 1.0

Page {
    id: page

    allowedOrientations: appWindow.pagesOrientations

    property bool selectionmode: false
    property bool searchDone: false
    property bool searching: false

    Connections {
        target: search

        onBannerChanged: {
            searching = false
            searchDone = true
            searchInput.text = ""
            searchInput.focus = false
        }
    }

    canNavigateForward: !clearRemorse.visible
    backNavigation: !clearRemorse.visible

    SilicaFlickable {
        anchors.top: parent.top
        anchors.bottom: controlPanel.top
        width: parent.width

        MyHeader {
            id: header
            title: qsTr("Search")
        }

        SearchField {
            id: searchInput
            anchors.top: header.bottom
            width: parent.width
            placeholderText: qsTr("New search")
            inputMethodHints: Qt.ImhNoPredictiveText
            enabled: !searching
            focus: true

            EnterKey.enabled: searchInput.text.length>0
            EnterKey.iconSource: "image://theme/icon-m-enter-accept"
            EnterKey.onClicked: {
                searching = true
                if (container.count>0)
                    search.cleanSearch()
                search.searchFiles(currentDeviceFolder, searchInput.text)
            }

        }

        SilicaListView {
            id: container
            anchors.fill: parent
            anchors.topMargin: header.height + searchInput.height
            clip: true

            model: search

            delegate: FileDelegate {
                width: parent.width
                contentHeight: Theme.itemSizeMedium

                highlighted: down || model.type===Search.FolderSel || model.type===Search.FileSel

                imgsource: model.exten.indexOf("/")===-1? "/usr/share/filecase/" + iconTheme + "/" + exten + ".png" :
                            model.fullpath

                mime: model.exten.indexOf("/")>-1? model.exten : "image/png"
                title: model.name
                description: model.path

                onClicked: {
                    if ( selectionmode ) //&& (extractcancel.visible==false) )
                    {
                        search.selectItem(model.fullpath)
                    }
                    else
                    {
                        fileInfo.setFile(model.fullpath)
                        pageStack.push("FileInfo.qml", { "cloudFile": false })
                    }
                }

                onPressAndHold: {
                    selectionmode = true
                    controlPanel.open = true
                    search.selectItem(model.fullpath)
                }

            }

        }

        ViewPlaceholder {
            enabled: container.count===0 && !searching && searchDone
            Behavior on opacity { FadeAnimation { duration: 10 } }
            text: container.count==0? qsTr("No files found") : qsTr("Clipboard is empty")
        }

        BusyIndicator {
            visible: searching
            running: visible
            anchors.centerIn: parent
            size: BusyIndicatorSize.Large
        }

    }

    DockedPanel {
        id: controlPanel
        width: parent.width
        height: Theme.itemSizeLarge
        enabled: !searching
        dock: Dock.Bottom
        open: false

        onOpenChanged: {
            if (!open) {
                selectionmode = false
                search.clearSelection()
            }
        }

        Image {
            anchors.fill: parent
            fillMode: Image.PreserveAspectFit
            source: "image://theme/graphic-gradient-edge"
        }

        Row {
            anchors.centerIn: parent
            anchors.horizontalCenter: parent.horizontalCenter

            IconButton {
                icon.source: "image://theme/icon-m-clear"
                width: standardClipboard? page.width/6 : page.width/5
                onClicked: {
                    controlPanel.open = false
                    search.clearSelection()
                }
            }

            IconButton {
                icon.source: "image://theme/icon-m-clipboard"
                enabled: search.multi1>0
                width: standardClipboard? page.width/6 : page.width/5
                onClicked: {
                    if (standardClipboard) {
                        config.setConfig("StandardClipboard", "")
                        action = "singlecopy"
                        search.addToStandardClipboard("selectedfiles", true)
                        controlPanel.open = false
                        search.clearSelection()
                    } else {
                        search.addToClipboard("selectedfiles", config.readConfig("AutoSelect", "false"))
                        controlPanel.open = false
                    }
                }
            }

            IconButton {
                icon.source: "file:///usr/share/filecase/icons/toolbar-cut.png"
                enabled: search.multi1>0
                visible: standardClipboard
                width: standardClipboard? page.width/6 : 0
                onClicked: {
                    config.setConfig("StandardClipboard", "")
                    action = "singlemove"
                    search.addToStandardClipboard("selectedfiles", true)
                    controlPanel.open = false
                    search.clearSelection()
                }
            }

            IconButton {
                icon.source: "image://theme/icon-m-delete"
                enabled: search.multi1>0
                width: standardClipboard? page.width/6 : page.width/5
                onClicked: {
                    working = true
                    deleteMultipleItems()
                }
            }


            IconButton {
                icon.source: "image://theme/icon-m-favorite-selected" //: "image://theme/icon-m-favorite"
                enabled: search.multi1>0 && search.multi1===1 && search.multi3===1
                width: standardClipboard? page.width/6 : page.width/5
                onClicked: {
                    utilities.addToBookmarks(search.getSelectedList())
                    controlPanel.open = false
                    search.clearSelection()
                }
            }

            IconButton {
                icon.source: "file:///usr/share/filecase/icons/toolbar-info.png"
                enabled: search.multi1>0
                width: standardClipboard? page.width/6 : page.width/5
                onClicked: {
                    fileInfo.setFile(search.getSelectedList())
                    pageStack.push("FileInfo.qml", { "cloudFile": false })
                    controlPanel.open = false
                    search.clearSelection()
                }
            }
        }

    }

    function deleteMultipleItems()
    {

        clearRemorse.execute(qsTr("Removing"), function()
        {
            working = true
            search.removeFile("selectedfiles", true)
            controlPanel.open = false
            //selectionmodeItems = ""
        })
    }

    RemorsePopup {
        id: clearRemorse
        onCanceled: working = false
    }

}
