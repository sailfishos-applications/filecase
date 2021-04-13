import QtQuick 2.0
import Sailfish.Silica 1.0
import FileCase 1.0

Rectangle {
    id: popup
    x: mainPage.isPortrait? 0 : mainPage.height
    y: 0
    width: mainPage.width
    height: message.paintedHeight + (Theme.paddingSmall * 2)
    property alias title: message.text
    property alias timeout: hideTimer.interval
    visible: opacity > 0
    opacity: 0.0
    rotation: mainPage.isPortrait? 0 : 90
    transformOrigin: Item.TopLeft
    color: Theme.rgba(Theme.secondaryBackgroundColor, 0.7)

    Behavior on opacity {
        FadeAnimation {}
    }

    Item {
        id: bg
        anchors.fill: parent

        Image {
            id: icon
            height: 32
            width: 32
            smooth: true
            anchors.left: parent.left
            anchors.leftMargin: Theme.paddingMedium
            anchors.verticalCenter: parent.verticalCenter
        }

        Label {
            id: message
            anchors.top: parent.top
            anchors.topMargin: Theme.paddingSmall
            font.pixelSize: Theme.fontSizeSmall
            anchors.left: icon.right
            anchors.leftMargin: Theme.paddingMedium
            anchors.right: parent.right
            anchors.rightMargin: Theme.paddingMedium
            //horizontalAlignment: Text.AlignHCenter
            //elide: Text.ElideRight
            wrapMode: Text.Wrap
        }
    }

    Timer {
        id: hideTimer
        triggeredOnStart: false
        repeat: false
        interval: 5000
        onTriggered: popup.hide()
    }

    function hide() {
        if (hideTimer.running)
            hideTimer.stop()
        popup.opacity = 0.0
    }

    function show() {
        popup.opacity = 1.0
        hideTimer.restart()
    }

    function displayMessage(text, ok) {
        popup.title = text
        if (ok) icon.source = "file:///usr/share/filecase/icons/message.png"
        else icon.source = "file:///usr/share/filecase/icons/message-error.png"
        show()
    }

    MouseArea {
        anchors.fill: parent
        onClicked: hide()
    }
}
