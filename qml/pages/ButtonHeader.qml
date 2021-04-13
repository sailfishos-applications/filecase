import QtQuick 2.0
import Sailfish.Silica 1.0

Item {

    property string title
    property string imgsource

    property bool small: false

    signal buttonClicked()

    width: parent.width
    height: small? Theme.itemSizeExtraSmall : Theme.itemSizeMedium

    IconButton {
        id: btn
        anchors.right: parent.right
        //anchors.rightMargin: Theme.paddingMedium
        icon.source: imgsource
        icon.height: small? parent.height/2 : undefined
        icon.width: icon.height
        height: parent.height
        width: height - (small? Theme.paddingMedium : 0)
        onClicked: buttonClicked()
    }

    Label {
        id: header
        text: title
        font.pixelSize: small? Theme.fontSizeSmall : Theme.fontSizeMedium
        horizontalAlignment: Text.AlignRight
        color: Theme.highlightColor
        anchors.left: parent.left
        anchors.right: btn.left
        anchors.rightMargin: Theme.paddingMedium
        anchors.verticalCenter: parent.verticalCenter
    }

    MouseArea {
        anchors.fill: parent
        enabled: small
        onClicked: buttonClicked()
    }

}
