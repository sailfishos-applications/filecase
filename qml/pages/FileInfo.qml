import QtQuick 2.0
import Sailfish.Silica 1.0
import FileCase 1.0
import org.nemomobile.thumbnailer 1.0
import QtMultimedia 5.0

Page {
    id: infoPage

    allowedOrientations: appWindow.pagesOrientations

    onStatusChanged: {
        console.log("LOCATION: " + currentLocation)
        if (status === PageStatus.Inactive) {
            if (!cloudFile) {
                fileInfo.stopInfoThread()
                if (player.playbackState == MediaPlayer.PlayingState)
                    player.stop()
                if (player.source !== "")
                    player.source = ""
            }
        }
    }

    property bool cloudFile: false
    property bool loadingLink: false

    property string fileId
    property string fileName
    property string fileLocation
    property string fileSize
    property string fileCreated
    property variant fileType
    property string fileModified
    property string fileImage

    property string htype : cloudFile? fileType.name : fileInfo.data.mime

    property bool isTextFile: !cloudFile && (htype.indexOf("x-shellscript")>-1
                                             || htype.indexOf("text/plain")>-1
                                             || htype.indexOf("text/x-rpm-spec")>-1
                                             || htype.indexOf("application/x-desktop")>-1)

    property bool isPackage: htype.indexOf("application/vnd.android.package-archive")>-1
                             || htype.indexOf("application/x-rpm")>-1

    Audio {
        id: player
    }

    /*Connections {
        target: browser

        onFileRenamed: textname.text = newname
    }*/

    Connections {
        target: appWindow

        onItemRenamed: textname.text = newname

        onItemLinkShared: {
            loadingLink = false
            if (fileurl=="error") {
                ibanner.displayMessage(qsTr("Error getting info. Try again later"), false)
            } else {
                linkText.text = fileurl
                linkText.visible = true
            }
        }
    }


    SilicaFlickable {
        anchors.fill: parent

        PullDownMenu {
            MenuItem {
                text: qsTr("Rename")
                //visible: !cloudFile
                onClicked: pageStack.push("RenameFile.qml", {
                                              "path": (currentLocation=="device"? fileInfo.data.path : fileId),
                                              "name": textname.text
                                          })
            }

            MenuItem {
                text: qsTr("Share")
                visible: cloudFile && fileImage!=="/usr/share/filecase/icons/folder.png" && currentLocation.indexOf("WebDav-")===-1
                onClicked: {
                    loadingLink = true
                    var cpage = pageStack.find(function(page) { return page.startFolder===currentDeviceFolder; })
                        cpage.shareFile(fileId)

                }
            }

            MenuItem {
                text: qsTr("View package files")
                visible: isPackage && !cloudFile && htype.indexOf("application/vnd.android.package-archive")>-1
                onClicked: {
                    action = "nothing"
                    pageStack.replace("Compressed.qml", {"filename": fileInfo.data.path + "/" + fileInfo.data.name, "name": fileInfo.data.name})
                }
            }


            MenuItem {
                text: isPackage && !cloudFile? qsTr("Install") : qsTr("Open")
                visible: !cloudFile && !isTextFile
                onClicked: utilities.openFile(fileInfo.data.path + "/" + fileInfo.data.name)
            }

            MenuItem {
                text: qsTr("Edit")
                visible: !cloudFile && isTextFile
                onClicked: pageStack.push("TextEditor.qml", {"fileName":fileInfo.data.path + "/" + fileInfo.data.name})
            }

            MenuItem {
                text: qsTr("View")
                visible: !cloudFile && isTextFile
                onClicked: pageStack.push("TextViewer.qml", {"fileName":fileInfo.data.path + "/" + fileInfo.data.name})
            }

        }

        contentHeight: header.height + dataPanel.height + Theme.paddingLarge

        MyHeader {
            id: header
            title: qsTr("Details")
            //description: browser.path2
            width: parent.width
        }

        Column {
            id: dataPanel
            spacing: Theme.paddingLarge
            anchors.top: header.bottom
            x: Theme.paddingLarge
            width: parent.width -Theme.paddingLarge*2


            Image {
                id: image
                visible: cloudFile || (fileInfo.data.icon!=="" && fileInfo.data.mime.indexOf("audio")===-1)

                source: cloudFile? fileImage : (fileInfo.data.icon.indexOf("/")===-1?
                        "file:///usr/share/filecase/" + iconTheme + "/" + fileInfo.data.icon + ".png" :
                        (fileInfo.data.icon!=="" ? fileInfo.getFilePreview(fileInfo.data.path + "/" + fileInfo.data.name) : "") )

                width: parent.width
                height: implicitHeight < 480 && implicitHeight != 0 ? implicitHeight : 480
                sourceSize.width: width
                sourceSize.height: height
                fillMode: Image.PreserveAspectFit
                anchors.horizontalCenter: parent.horizontalCenter
                asynchronous: true

            }

            IconButton {
                visible: htype.indexOf("audio")>-1 && !cloudFile
                //height: Theme.itemSizeMedium
                //width: height
                icon.source: player.playbackState !== MediaPlayer.PlayingState?
                             "image://theme/icon-l-play" : "image://theme/icon-l-pause"
                anchors.horizontalCenter: parent.horizontalCenter
                onClicked: {
                    if (player.playbackState !== MediaPlayer.PlayingState) {
                        player.source = fileInfo.data.path + "/" + fileInfo.data.name
                        player.play()
                    } else {
                        player.pause()
                    }
                }
            }


            Text {
                id: textname
                width: parent.width
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                color: Theme.primaryColor
                text: cloudFile? fileName : fileInfo.data.name
            }

            Column {
                width: parent.width
                spacing: Theme.paddingSmall

                ValueItem {
                    id: location
                    title: fileInfo.data.target==="" || cloudFile ? qsTr("Location") : qsTr("Target")
                    value: cloudFile? fileLocation : (fileInfo.data.target==="" ? fileInfo.data.path : fileInfo.data.target)
                }

                ValueItem {
                    id: filetype
                    visible: fileInfo.data.icon!==""
                    title: qsTr("Type")
                    value: cloudFile? fileType.comment + "\n(" + fileType.name + ")" :
                                      fileInfo.data.comment + "\n(" + fileInfo.data.mime + ")"
                }

                ValueItem {
                    id: filesize
                    title: fileInfo.data.icon==="folder"? qsTr("Content size") : qsTr("Size")
                    value: cloudFile? fileSize : fileInfo.data.size
                    visible: value!==""
                }

                ValueItem {
                    id: filecreated
                    visible: value!==""
                    title: qsTr("Created")
                    value: cloudFile? fileCreated : fileInfo.data.created
                }

                ValueItem {
                    id: filemodified
                    visible: value!==""
                    title: qsTr("Last modified")
                    value: cloudFile? fileModified : fileInfo.data.modified
                }

            }

            Column {
                width: parent.width
                spacing: Theme.paddingSmall

                Label {
                    visible: linkText.visible
                    font.pixelSize: Theme.fontSizeExtraSmall
                    text: qsTr("Public link")
                    color: Theme.secondaryColor
                    horizontalAlignment: Text.AlignHCenter
                    width: parent.width
                }

                Label {
                    id: linkText
                    visible: false
                    font.pixelSize: Theme.fontSizeSmall
                    color: Theme.highlightColor
                    horizontalAlignment: Text.AlignHCenter
                    width: parent.width
                    wrapMode: Text.WrapAnywhere
                }
            }

            Button {
                visible: linkText.visible
                anchors.horizontalCenter: parent.horizontalCenter
                text: qsTr("Copy link")
                onClicked: {
                    textHelper.text = linkText.text
                    textHelper.selectAll()
                    textHelper.copy()
                }
            }

            BusyIndicator {
                anchors.horizontalCenter: parent.horizontalCenter
                size: BusyIndicatorSize.Medium
                visible: loadingLink
                running: visible
            }

        }


    }

    TextArea {
        id: textHelper
        visible: false
    }

}
