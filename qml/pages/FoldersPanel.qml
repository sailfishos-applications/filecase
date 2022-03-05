import QtQuick 2.0
import Sailfish.Silica 1.0
import FileCase 1.0

Page {
    id: foldersPage

    allowedOrientations: appWindow.pagesOrientations

    property string pageID: "foldersPage"

    function getName(text) {
        var res = text.split('/')
        res = res[res.length-1]
        if (res==="") res = "/"
        return res;
    }

    Rectangle {
        anchors.fill: parent
        color: Qt.darker(Theme.highlightColor, 2.5)
        opacity: 0.5
    }


    property var deviceModel: [
        { name: qsTr("Phone memory"), folder: config.getHome(), icon: "image://theme/icon-m-phone" } ,
        { name: qsTr("SD Card"), "/media/sdcard/", icon: "file:///usr/share/filecase/icons/toolbar-sdcard.png" }
    ]

    property var placesModel: [
        { name: qsTr("Documents"), folder: config.getHome() + "/Documents", icon: "image://theme/icon-m-document" },
        { name: qsTr("Downloads"), folder: config.getHome() + "/Downloads", icon: "image://theme/icon-m-cloud-download" },
        { name: qsTr("Music"), folder: config.getHome() + "/Music", icon: "image://theme/icon-m-sounds" },
        { name: qsTr("Videos"), folder: config.getHome() + "/Videos", icon: "image://theme/icon-m-media" },
        { name: qsTr("Pictures"), folder: config.getHome() + "/Pictures", icon: "image://theme/icon-m-image" }
    ]


    property bool showSectionTransfers: config.readConfig("showSectionTransfers", "true")==="true"
    property bool showSectionDevice: config.readConfig("showSectionDevice", "true")==="true"
    property bool showSectionPlaces: config.readConfig("showSectionPlaces", "true")==="true"
    property bool showSectionFavorites: config.readConfig("showSectionFavorites", "true")==="true"
    property bool showSectionCloud: config.readConfig("showSectionCloud", "true")==="true"
    property bool showSectionDav: config.readConfig("showSectionDav", "true")==="true"

    SilicaFlickable {
        id: list
        anchors.fill: parent
        contentHeight: column.height + Theme.paddingLarge

        PullDownMenu {
            MenuItem {
                text: qsTr("Add service")
                onClicked: pageStack.push("AddAccount.qml")
            }

        }

        Column {
            id: column
            width: parent.width

            MyHeader {
                id: header
                title: "FileCase"
                description: appVersion
            }

            Item {
                id: secTransfers
                clip: true
                width: parent.width
                height: Theme.itemSizeExtraSmall + (showSectionTransfers? Theme.itemSizeSmall : 0)

                Behavior on height { NumberAnimation { duration: 150 } }

                ButtonHeader {
                    id: header1
                    small: true
                    title: qsTr("Transfers")
                    imgsource: showSectionTransfers? "image://theme/icon-m-up" : "image://theme/icon-m-down"
                    width: parent.width - Theme.paddingMedium
                    onButtonClicked: {
                        showSectionTransfers = !showSectionTransfers
                        config.setConfig("showSectionTransfers", showSectionTransfers? "true" : "false")
                    }
                }

                FolderDelegate {
                    anchors.top: header1.bottom
                    icon: "image://theme/icon-m-cloud-download"
                    text: totalTransfers==0? qsTr("No pending transfers") :
                          (totalTransfers==1? qsTr("1 pending transfer") : qsTr("%1 pending transfers").arg(totalTransfers))
                    onClicked: {
                        //pageStack.replaceAbove(pageStack.find(function(page) { return page._depth===0; }), transfersPage)
                        pageStack.replaceAbove(pageStack.find(function(page) { return page.startFolder===currentDeviceFolder; }), transfersPage)
                    }
                }
            }

            Item {
                id: secDevice
                width: parent.width
                height: Theme.itemSizeExtraSmall + (showSectionDevice? deviceList.height : 0)
                clip: true

                Behavior on height { NumberAnimation { duration: 150 } }

                ButtonHeader {
                    id: header2
                    small: true
                    title: qsTr("Device")
                    imgsource: showSectionDevice? "image://theme/icon-m-up" : "image://theme/icon-m-down"
                    width: parent.width - Theme.paddingMedium
                    onButtonClicked: {
                        showSectionDevice = !showSectionDevice
                        config.setConfig("showSectionDevice", showSectionDevice? "true" : "false")
                    }
                }

                SilicaListView {
                    id: deviceList
                    anchors.top: header2.bottom
                    interactive: false
                    height: count*Theme.itemSizeSmall
                    width: parent.width
                    model: deviceModel
                    delegate: FolderDelegate {
                        property var item: model.modelData ? model.modelData : model
                        enabled: item.folder!==""
                        opacity: enabled? 1 : 0.5
                        icon: item.icon
                        text: item.name
                        onClicked: {
                            //console.log("Current: " + currentDeviceFolder + " - New: " + item.folder)
                            if (currentLocation=="device" && currentDeviceFolder===item.folder) {
                                pageStack.navigateBack()
                            } else {
                                //loadingFolder = true
                                startPageLoaded = false
                                pageStack.replaceAbove(null, "MainPage.qml", {"startFolder":"/", "nextFolder": item.folder, "loaded": false})
                            }
                        }
                    }
                }
            }

            Item {
                id: secPlaces
                clip: true
                width: parent.width
                height: Theme.itemSizeExtraSmall + (showSectionPlaces? placesList.height : 0)

                Behavior on height { NumberAnimation { duration: 150 } }

                ButtonHeader {
                    id: header3
                    small: true
                    title: qsTr("Places")
                    imgsource: showSectionPlaces? "image://theme/icon-m-up" : "image://theme/icon-m-down"
                    width: parent.width - Theme.paddingMedium
                    onButtonClicked: {
                        showSectionPlaces = !showSectionPlaces
                        config.setConfig("showSectionPlaces", showSectionPlaces? "true" : "false")
                    }
                }

                SilicaListView {
                    id: placesList
                    anchors.top: header3.bottom
                    height: count*Theme.itemSizeSmall
                    width: parent.width
                    model: placesModel
                    delegate: FolderDelegate {
                        property var item: model.modelData ? model.modelData : model
                        icon: item.icon
                        text: item.name
                        onClicked: {
                            console.log("Current: " + currentDeviceFolder + " - New: " + item.folder)
                            if (currentLocation=="device" && currentDeviceFolder===item.folder) {
                                pageStack.navigateBack()
                            } else {
                                //loadingFolder = true
                                startPageLoaded = false
                                pageStack.replaceAbove(null, "MainPage.qml", {"startFolder":"/", "nextFolder": item.folder, "loaded": false})
                            }
                        }
                    }
                }
            }

            Item {
                id: secFavorites
                clip: true
                width: parent.width
                height: bookModel.count===0? 0 : (Theme.itemSizeExtraSmall + (showSectionFavorites? favList.height : 0))

                Behavior on height { NumberAnimation { duration: 150 } }

                ButtonHeader {
                    id: header4
                    small: true
                    title: qsTr("Favorites")
                    imgsource: showSectionFavorites? "image://theme/icon-m-up" : "image://theme/icon-m-down"
                    width: parent.width - Theme.paddingMedium
                    onButtonClicked: {
                        showSectionFavorites = !showSectionFavorites
                        config.setConfig("showSectionFavorites", showSectionFavorites? "true" : "false")
                    }
                }

                SilicaListView {
                    id: favList
                    anchors.top: header4.bottom
                    height: count*Theme.itemSizeSmall
                    width: parent.width
                    model: bookModel
                    delegate: FolderDelegate {
                        icon: "image://theme/icon-m-folder"
                        text: getName(model.folder)
                        showButton: true
                        onClicked: {
                            console.log("Current: " + currentDeviceFolder + " - New: " + model.folder)
                            if (currentLocation=="device" && currentDeviceFolder===model.folder) {
                                pageStack.navigateBack()
                            } else {
                                //loadingFolder = true
                                startPageLoaded = false
                                pageStack.replaceAbove(null, "MainPage.qml", {"startFolder":"/", "nextFolder": model.folder, "loaded": false})
                            }
                        }
                        onButtonPressed: utilities.removeFromBookmarks(model.folder)
                    }
                }
            }

            Item {
                id: secCloud
                clip: true
                width: parent.width
                height: Theme.itemSizeExtraSmall + (showSectionCloud? cloudList.height : 0)

                Behavior on height { NumberAnimation { duration: 150 } }

                ButtonHeader {
                    id: header5
                    small: true
                    title: qsTr("Cloud services")
                    imgsource: showSectionCloud? "image://theme/icon-m-up" : "image://theme/icon-m-down"
                    width: parent.width - Theme.paddingMedium
                    onButtonClicked: {
                        showSectionCloud = !showSectionCloud
                        config.setConfig("showSectionCloud", showSectionCloud? "true" : "false")
                    }
                }

                SilicaListView {
                    id: cloudList
                    anchors.top: header5.bottom
                    height: count*Theme.itemSizeSmall
                    width: parent.width
                    model: cloudModel
                    delegate: FolderDelegate {
                        property var item: model.modelData ? model.modelData : model
                        icon: item.icon
                        text: item.name
                        onClicked: {
                            if (currentLocation===item.name)
                                pageStack.navigateBack()
                            else {
                                currentLocation = item.name
                                pageStack.replaceAbove(null, item.qml, {"startFolder": "start"})
                            }
                        }
                    }
                }
            }

            Item {
                id: secWebDav
                clip: true
                width: parent.width
                height: davModel.count===0? 0 : (Theme.itemSizeExtraSmall + (showSectionDav? davList.height : 0))

                Behavior on height { NumberAnimation { duration: 150 } }

                ButtonHeader {
                    id: header6
                    small: true
                    title: qsTr("WebDAV services")
                    imgsource: showSectionDav? "image://theme/icon-m-up" : "image://theme/icon-m-down"
                    width: parent.width - Theme.paddingMedium
                    onButtonClicked: {
                        showSectionDav = !showSectionDav
                        config.setConfig("showSectionDav", showSectionDav? "true" : "false")
                    }
                }

                SilicaListView {
                    id: davList
                    anchors.top: header6.bottom
                    height: count*Theme.itemSizeSmall
                    width: parent.width
                    model: davModel
                    delegate: FolderDelegate {
                        icon: "file:///usr/share/filecase/icons/webdav.png"
                        text: model.service
                        onClicked: {
                            if (currentLocation==="WebDav-"+model.service)
                                pageStack.navigateBack()
                            else {
                                currentLocation = "WebDav-"+model.service
                                webdavAccount = []
                                pageStack.replaceAbove(null, "WebDavPage.qml", {"service": model.service,
                                                           "startFolder": "start", "loaded": false})
                            }
                        }
                    }
                }
            }


        }



    }


}
