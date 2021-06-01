import QtQuick 2.0
import Sailfish.Silica 1.0
import FileCase 1.0


Page {
    id: settingsPage

    allowedOrientations: appWindow.pagesOrientations

    property bool loaded: false

    onStatusChanged: {
        if (status === PageStatus.Activating) {
            loaded = false
            orientationMenu.currentIndex = savedorientation==="auto"? 0 : (savedorientation==="portrait"? 1 : 2)
            startupFolder.value = getName(config.readConfig("StartupFolder",config.getHome()))
            hiddenFilesMenu.checked = config.readConfig("ShowHiddenFiles", "false")==="true"
            thumbnailsMenu.checked = config.readConfig("ShowThumbnails", "false")==="true"
            dateFormatMenu.currentIndex = config.readConfig("DateFormat","dd/mm/yyyy")==="dd/mm/yyyy"? 0 : 1
            sortByMenu.currentIndex = parseInt(config.readConfig("SortIndex","0"))
            sortOrderMenu.currentIndex = parseInt(config.readConfig("SortOrder","0"))
            folderFirstMenu.checked = config.readConfig("FoldersFirst", "1")==="1"
            loaded = true
        }
    }

    Connections {
        target: appWindow


        onSetLanguage: {
            langValue.value = langval
        }

    }


    SilicaFlickable {
        anchors.fill: parent
        contentWidth: settingsPage.width
        contentHeight: settingsColumn.height + Theme.paddingMedium
        flickableDirection: Flickable.VerticalFlick

        PullDownMenu {
            MenuItem {
                text: qsTr("About")
                onClicked: pageStack.push("AboutPage.qml")
            }
        }


        Column {
            id: settingsColumn
            spacing: Theme.paddingMedium
            anchors.left: parent.left
            //anchors.leftMargin: Theme.paddingMedium
            width: parent.width //-Theme.paddingMedium*2

            MyHeader {
                title: qsTr("Settings")
            }

            SectionHeader {
                text: qsTr("General")
            }

            ValueButton {
                id: langValue
                label: qsTr("Language")
                value: config.readConfig("LanguageName", "English")
                onClicked: {
                    pageStack.push("SelectLanguage.qml")
                }
            }


            Label {
                id: subTitle
                verticalAlignment: Text.AlignVCenter
                text: qsTr("*Restart to apply the new language")
                anchors.left: parent.left
                anchors.leftMargin: Theme.paddingLarge
                width: parent.width
                wrapMode: Text.WordWrap
                color: Theme.secondaryColor
                font.pixelSize: Theme.fontSizeExtraSmall
            }

            ComboBox {
                id: orientationMenu
                label: qsTr("Orientation")
                menu: ContextMenu {
                    MenuItem { text: qsTr("Automatic") }
                    MenuItem { text: qsTr("Portrait") }
                    MenuItem { text: qsTr("Landscape") }
                }
                onCurrentItemChanged: {
                    if (!loaded) return
                    if (currentIndex == 0) {
                        config.setConfig("Orientation", "auto")
                        savedorientation = "auto"
                    }
                    else if (currentIndex == 1) {
                        config.setConfig("Orientation", "portrait")
                        savedorientation = "portrait"
                    }
                    else {
                        config.setConfig("Orientation", "landscape")
                        savedorientation = "landscape"
                    }
                }
            }

            /*ComboBox {
                label: qsTr("Icon Theme")
                menu: ContextMenu {
                    MenuItem { text: qsTr("Default") }
                    MenuItem { text: qsTr("Color") }
                }
                currentIndex: iconTheme=="icons"? 0 : 1
                onCurrentItemChanged: {
                    if (currentIndex == 0) {
                        config.setConfig("IconTheme", "icons")
                        iconTheme = "icons"
                    }
                    else if (currentIndex == 1) {
                        config.setConfig("IconTheme", "color")
                        iconTheme = "color"
                    }
                }
            }*/


            SectionHeader {
                text: qsTr("File Manager")
            }

            ValueButton {
                id: startupFolder
                label: qsTr("Startup folder")
                value: config.readConfig("StartupFolder", config.getHome())
                onClicked: {
                    pageStack.push("SelDownloadFolder.qml")
                }
            }

            TextSwitch {
                text: qsTr("Show root filesystem")
                checked: showRootFiles
                onClicked: {
                    config.setConfig("ShowRootFiles", checked? "true" : "false")
                    showRootFiles = checked
                    reloadItems()
                }
            }

            TextSwitch {
                id: hiddenFilesMenu
                text: qsTr("Show hidden files")
                onClicked: {
                    config.setConfig("ShowHiddenFiles", checked? "true" : "false")
                    reloadItems()
                }
            }

            TextSwitch {
                id: thumbnailsMenu
                text: qsTr("Show thumbnails")
                onClicked: {
                    config.setConfig("ShowThumbnails", checked? "true" : "false")
                    reloadItems()
                }
            }

            ComboBox {
                id: dateFormatMenu
                label: qsTr("Date format")
                menu: ContextMenu {
                    MenuItem { text: qsTr("dd/mm/yyyy") }
                    MenuItem { text: qsTr("mm/dd/yyyy") }
                }
                onCurrentItemChanged: {
                    if (!loaded) return
                    if (currentIndex == 0) {
                        config.setConfig("DateFormat", "dd/mm/yyyy")
                        reloadItems()
                    }
                    else if (currentIndex == 1) {
                        config.setConfig("DateFormat", "mm/dd/yyyy")
                        reloadItems()
                    }
                }
            }

            ComboBox {
                id: sortByMenu
                label: qsTr("Sort items by")
                menu: ContextMenu {
                    MenuItem { text: qsTr("Name") }
                    MenuItem { text: qsTr("Size") }
                    MenuItem { text: qsTr("Date") }
                }
                onCurrentItemChanged: {
                    if (!loaded) return
                    config.setConfig("SortIndex", currentIndex)
                    reloadItems()
                }
            }

            ComboBox {
                id: sortOrderMenu
                label: qsTr("Sort order")
                menu: ContextMenu {
                    MenuItem { text: qsTr("Ascending") }
                    MenuItem { text: qsTr("Descending") }
                }
                onCurrentItemChanged: {
                    if (!loaded) return
                    config.setConfig("SortOrder", currentIndex)
                    reloadItems()
                }
            }

            TextSwitch {
                id: folderFirstMenu
                text: qsTr("Show folders first")
                onClicked: {
                    config.setConfig("FoldersFirst", checked? "1" : "0")
                    reloadItems()
                }
            }


            SectionHeader {
                text: qsTr("Clipboard")
            }

            /*TextSwitch {
                id: sclip
                text: qsTr("Use standard clipboard")
                checked: config.readConfig("UseStandardClipboard", "false")==="true"
                onClicked: {
                    config.setConfig("UseStandardClipboard", checked? "true" : "false")
                    standardClipboard = checked
                    //sclip.check = checked
                }
            }*/

            TextSwitch {
                text: qsTr("Autoselect files to copy")
                checked: config.readConfig("AutoSelect", "false")==="true"
                //enabled: !sclip.check
                opacity: enabled? 1 : 0.4
                onClicked: {
                    config.setConfig("AutoSelect", checked? "true" : "false")
                }
            }

            TextSwitch {
                text: qsTr("Autoremove files after action")
                checked: config.readConfig("AutoRemove","false")==="true"
                //enabled: !sclip.check
                opacity: enabled? 1 : 0.4
                onClicked: {
                    config.setConfig("AutoRemove", checked? "true" : "false")
                }
            }


        }
    }


}
