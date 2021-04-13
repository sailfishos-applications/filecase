import QtQuick 2.0
import Sailfish.Silica 1.0
import FileCase 1.0

Dialog {

    allowedOrientations: appWindow.pagesOrientations

    id: dialog

    property string path: ""
    property string name: ""

    //allowedOrientations: appWindow.pagesOrientations


    onAccepted: {
        if (currentLocation=="device") {
            var page = pageStack.find(function(page) { return page.startFolder===currentDeviceFolder; })
            page.renameFile(path + "/" + name, ren.text)
        }
        else {
            var cpage = pageStack.find(function(page) { return page.startFolder===currentDeviceFolder; })
            cpage.renameFile(path, ren.text)
        }

        /*if (currentLocation=="device")
            browser.renameFile(path + "/" + name, ren.text)
        else if (currentLocation=="Google Drive")
            driveclient.renameFile(path, ren.text)
        else if (currentLocation=="Dropbox")
            dropboxclient.renameFile(path, ren.text)
        else if (currentLocation=="OneDrive")
            skyclient.renameFile(path, ren.text)
        else if (currentLocation=="Box")
            boxclient.renameFile(path, ren.text)
        else
            webdav.renameFile(source, dest)*/



    }

    canAccept: ren.text.length>0

    DialogHeader {
        id: header
        acceptText: qsTr("Done")
        cancelText: qsTr("Cancel")
        spacing: 0
    }

    PageHeader {
        id: header2
        title: qsTr("Rename")
        anchors.top: header.bottom
        width: parent.width
    }

    SilicaFlickable {
        anchors.top: parent.top
        anchors.topMargin: header.height + header2.height
        width: parent.width
        height: parent.height
        contentHeight: col1.height+Theme.paddingLarge
        //color: "transparent"

        Column {
            id: col1
            spacing: Theme.paddingLarge
            width: parent.width

            Label {
                anchors.left: parent.left
                anchors.leftMargin: 20
                text: qsTr("Enter new name")
            }

            TextField {
                id: ren
                width: parent.width
                placeholderText: qsTr("New name")
                label: qsTr("New name")
                text: name
                focus: true
                EnterKey.enabled: ren.text.length>0
                EnterKey.iconSource: "image://theme/icon-m-enter-accept"
                EnterKey.onClicked: dialog.accept()
            }


        }

    }


    /*ToolBarLayout {
        height: appWindow.inPortrait? 72 : 56
        id:statusTool
        ToolIcon{
            platformStyle: ToolItemStyle{ pressedBackground: useSailfish? "": "image://theme/meegotouch-button-navigationbar-button"+__invertedString+"-background-pressed" }
            iconSource: imagePath + (useSailfish? "/sailfish":"") + "/toolbar-back" + (appWindow.mirror?"-mirror":"") + (theme.inverted && !useSailfish? "-white":"") + ".png"
            anchors.left: parent.left
            onClicked: pageStack.pop()
        }

    }*/

}
