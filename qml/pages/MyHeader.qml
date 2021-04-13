import QtQuick 2.0
import Sailfish.Silica 1.0

Item {
    id: header

    property alias title: headerText.text
    property alias description: descText.text

    height: appWindow._screenWidth<appWindow._screenHeight? Theme.itemSizeLarge : Theme.itemSizeSmall
    width: parent.width

    Column {
        width: parent.width
        anchors.verticalCenter: parent.verticalCenter

        Label {
            id: headerText
            width: parent.width -Theme.paddingLarge*2
            truncationMode: TruncationMode.Fade
            horizontalAlignment: Text.AlignRight
            color: Theme.highlightColor
            //anchors.top: parent.top
            //anchors.topMargin: descText.text==="" ? Theme.paddingLarge*1.5 : Theme.paddingMedium
            anchors.right: parent.right
            anchors.rightMargin: Theme.paddingLarge
            font {
                pixelSize: Theme.fontSizeLarge
                family: Theme.fontFamilyHeading
            }
        }

        Label {
            id: descText
            //anchors.top: headerText.bottom
            color: Theme.secondaryColor
            width: parent.width -Theme.paddingLarge*2
            horizontalAlignment: Text.AlignRight
            font.pixelSize: Theme.fontSizeExtraSmall
            anchors.right: parent.right
            anchors.rightMargin: Theme.paddingLarge
            truncationMode: TruncationMode.Fade
            visible: text!=""
        }

    }

}
