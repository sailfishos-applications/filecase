import QtQuick 2.0
import Sailfish.Silica 1.0
import FileCase 1.0

Page {

    id: dialog

    allowedOrientations: appWindow.pagesOrientations

    WebDav {
        id: webdav
    }

    property bool working: false

    Connections {
        target: webdav

        onRequestDone: working = false

        onAccountCreated: {
            davModel.clear()
            utilities.loadWebDavs()
            pageStack.pop()
        }

        onAccountError: {
            working = false
            ibanner.displayMessage(message, true)
        }

    }

    /*DialogHeader {
        id: header
        acceptText: qsTr("Done")
        cancelText: qsTr("Cancel")
        spacing: 0
    }*/


    SilicaFlickable {
        anchors.top: parent.top
        //anchors.topMargin: header.height + header2.height
        width: parent.width
        height: parent.height
        contentHeight: col1.height+Theme.paddingLarge
        //color: "transparent"

        Column {
            id: col1
            spacing: Theme.paddingLarge
            width: parent.width

            PageHeader {
                id: header2
                title: qsTr("WebDAV account")
                anchors.top: header.bottom
                width: parent.width
            }

            TextField {
                id: name
                width: parent.width
                //placeholderText: qsTr("Service name")
                label: qsTr("Service name")
                _labelItem.opacity: 1
                enabled: !working
                focus: true
                EnterKey.enabled: name.text.length>0
                EnterKey.onClicked: host.forceActiveFocus()
            }

            TextField {
                id: host
                width: parent.width
                placeholderText: "webdav.service.com"
                inputMethodHints: Qt.ImhNoAutoUppercase
                label: qsTr("Server")
                _labelItem.opacity: 1
                enabled: !working
                EnterKey.enabled: host.text.length>0
                EnterKey.onClicked: path.forceActiveFocus()
            }

            TextField {
                id: path
                width: parent.width
                placeholderText: "/"
                text: "/"
                label: qsTr("Path")
                _labelItem.opacity: 1
                enabled: !working
                EnterKey.enabled: path.text.length>0
                EnterKey.onClicked: port.forceActiveFocus()
            }

            TextField {
                id: port
                width: parent.width
                placeholderText: host.text.indexOf("http://")>-1? "80" : "443"
                text: host.text.indexOf("http://")>-1? "80" : "443"
                label: qsTr("Port")
                _labelItem.opacity: 1
                inputMethodHints: Qt.ImhDigitsOnly
                enabled: !working
                EnterKey.enabled: port.text.length>0
                EnterKey.onClicked: user.forceActiveFocus()
            }

            TextField {
                id: user
                width: parent.width
                //placeholderText: qsTr("User")
                label: qsTr("User")
                _labelItem.opacity: 1
                inputMethodHints: Qt.ImhNoAutoUppercase
                enabled: !working
                EnterKey.enabled: user.text.length>0
                EnterKey.onClicked: pass.forceActiveFocus()
            }

            TextField {
                id: pass
                width: parent.width
                //placeholderText: qsTr("Password")
                label: qsTr("Password")
                _labelItem.opacity: 1
                enabled: !working
                echoMode: TextInput.Password
                inputMethodHints: Qt.ImhNoAutoUppercase
                EnterKey.enabled: pass.text.length>0
                EnterKey.iconSource: "image://theme/icon-m-enter-accept"
                EnterKey.onClicked: {
                    button.forceActiveFocus()
                    working = true
                    webdav.createAccount(name.text, host.text, path.text, user.text, pass.text, port.text)
                }
            }

            Button {
                id: button
                text: qsTr("Done")
                anchors.horizontalCenter: parent.horizontalCenter
                enabled: !working && name.text.length>0 && host.text.length>0 && path.text.length>0 && user.text.length>0 && pass.text.length>0
                onClicked: {
                    working = true
                    webdav.createAccount(name.text, host.text, path.text, user.text, pass.text)
                }
            }

            BusyIndicator {
                size: BusyIndicatorSize.Medium
                anchors.horizontalCenter: parent.horizontalCenter
                visible: working
                running: visible
            }

        }

    }


}
