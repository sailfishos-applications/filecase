import QtQuick 2.0
import Sailfish.Silica 1.0
import FileCase 1.0

Page {

    id: dialog

    allowedOrientations: appWindow.pagesOrientations

    /*onStatusChanged: {
        if (status === PageStatus.Activating) {
            dropboxLoaded = config.readConfig("Dropbox/access_token", "")!==""
            driveLoaded = config.readConfig("Google Drive/access_token", "")!==""
            skyLoaded = config.readConfig("OneDrive/access_token", "")!==""
            boxLoaded = config.readConfig("Box/access_token", "")!==""
        }
    }*/


    SilicaFlickable {
        anchors.top: parent.top
        //anchors.topMargin: header.height + header2.height
        width: parent.width
        height: parent.height
        contentHeight: col1.height
        //color: "transparent"

        Column {
            id: col1
            //spacing: Theme.paddingLarge
            width: parent.width

            PageHeader {
                id: header2
                title: qsTr("Add service")
                width: parent.width
            }

            FolderDelegate {
                icon: "file:///usr/share/filecase/icons/box.png"
                enabled: config.readConfig("Box/access_token", "")===""
                opacity: enabled? 1 : 0.5
                text: "Box"
                onClicked: {
                    pageStack.replace("Box.qml", {"newAccount":true})
                }
            }

            //FolderDelegate {
                //icon: "file:///usr/share/filecase/icons/dropbox.png"
                //enabled: config.readConfig("Dropbox/access_token", "")===""
                //opacity: enabled? 1 : 0.5
                //text: "Dropbox"
                //onClicked: {
                //    pageStack.replace("Dropbox.qml", {"newAccount":true})
                //}
            //}

            FolderDelegate {
                icon: "file:///usr/share/filecase/icons/gdrive.png"
                enabled: config.readConfig("Google Drive/access_token", "")===""
                opacity: enabled? 1 : 0.5
                text: "Google Drive"
                onClicked: {
                    pageStack.replace("Drive.qml", {"newAccount":true})
                }
            }

            FolderDelegate {
                icon: "file:///usr/share/filecase/icons/sky.png"
                enabled: config.readConfig("OneDrive/access_token", "")===""
                opacity: enabled? 1 : 0.5
                text: "OneDrive"
                onClicked: {
                    pageStack.replace("SkyDrive.qml", {"newAccount":true})
                }
            }

            //FolderDelegate {
                //icon: "file:///usr/share/filecase/icons/webdav.png"
                //text: "WebDAV"
                //onClicked: {
                //    pageStack.replace("AddWebDavAccount.qml")
                //}
            //}


        }

    }


}
