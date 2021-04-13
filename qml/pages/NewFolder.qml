import QtQuick 2.0
import Sailfish.Silica 1.0
import FileCase 1.0

Dialog {

    allowedOrientations: appWindow.pagesOrientations

    id: dialog

    property string path: ""
    property string name: ""


    onAccepted: {
        var cpage = pageStack.find(function(page) { return page.startFolder===currentDeviceFolder; })
        cpage.createFolder(ren.text)

        /*if (currentLocation=="device") {
            browser.newFolder(ren.text)
            browser.reload()
        } else if (currentLocation=="Google Drive") {
            driveclient.newFolder(ren.text)
        } else if (currentLocation=="Dropbox") {
            dropboxclient.newFolder(ren.text)
        } else if (currentLocation=="OneDrive") {
            skyclient.newFolder(ren.text)
        } else if (currentLocation=="Box") {
            boxclient.newFolder(ren.text)
        } else
            webdav.newFolder(ren.text)*/
    }

    canAccept: ren.text.length>0

    DialogHeader {
        id: header
        acceptText: qsTr("Done")
        cancelText: qsTr("Cancel")
        spacing: 0
    }


    SilicaFlickable {
        anchors.top: parent.top
        anchors.topMargin: header.height
        width: parent.width
        height: parent.height
        contentHeight: col1.height+Theme.paddingLarge
        //color: "transparent"

        Column {
            id: col1
            width: parent.width
            spacing: Theme.paddingMedium

            PageHeader {
                id: header2
                title: qsTr("New Folder")
                anchors.top: header.bottom
                width: parent.width
            }

            TextField {
                id: ren
                width: parent.width
                placeholderText: qsTr("New name")
                label: qsTr("New name")
                focus: true
                EnterKey.enabled: ren.text.length>0
                EnterKey.iconSource: "image://theme/icon-m-enter-accept"
                EnterKey.onClicked: dialog.accept()
            }

        }


    }

}
