import QtQuick 2.0
import Sailfish.Silica 1.0

Row {
    spacing: Theme.paddingMedium
    width: parent.width

    property alias title: titleLabel.text
    property alias value: valueLabel.text

    property real fontSize: Theme.fontSizeExtraSmall

    Label {
        id: titleLabel
        color: Theme.secondaryColor
        width: parent.width/2 -Theme.paddingMedium
        horizontalAlignment: Text.AlignRight
        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
        font.pixelSize: fontSize
    }
    Label {
        id: valueLabel
        color: Theme.highlightColor
        width: parent.width/2
        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
        font.pixelSize: fontSize
    }
}
