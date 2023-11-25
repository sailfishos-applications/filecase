import QtQuick 2.0
import Sailfish.Silica 1.0
import FileCase 1.0
import Nemo.Thumbnailer 1.0

ListItem {

    contentHeight: Theme.itemSizeMedium

    //property real contentYOffset: 0.0

    //y: contentYOffset

    property string imgsource: ""
    property alias mime: image.mimeType
    property alias title: titleText.text
    property alias description: descText.text

    property string lname: title.toLowerCase()
    //property string lprev: image.source

    property bool showLink: false

    property bool showVideo: imgsource.indexOf("/usr/share/filecase")==-1 &&
                             (lname.indexOf(".mp4")>-1 || lname.indexOf(".avi")>-1 || lname.indexOf(".3gp")>-1
                              || lname.indexOf(".wmv")>-1 || lname.indexOf(".mpeg")>-1)




    Thumbnail {
        id: image

        source: imgsource
        height: Theme.itemSizeMedium -Theme.paddingMedium
        width: height
        sourceSize.width: width
        sourceSize.height: height
        priority: Thumbnail.NormalPriority

        anchors.left: parent.left
        anchors.leftMargin: Theme.paddingMedium
        anchors.top: parent.top
        anchors.topMargin: Theme.paddingMedium/2

    }

    Rectangle {
        id: rec
        color: Qt.darker(Theme.highlightColor, 2.0)
        height: Theme.paddingLarge
        anchors.bottom: image.bottom
        anchors.left: image.left
        width: image.width
        opacity: 0.7
        visible: showVideo || (showLink && imgsource.indexOf("/usr/share/filecase/icons")==-1)
    }

    Image {
        anchors.centerIn: rec
        height: rec.height - Theme.paddingSmall
        width: height
        smooth: true
        source: "image://theme/icon-m-video"
        visible: showVideo
    }

    Image {
        visible: showLink
        height: rec.height - Theme.paddingSmall
        width: height
        anchors.centerIn: rec
        smooth: true
        source: "image://theme/icon-status-call-forwarding"
    }


    Column {
        anchors.left: imgsource!=="" ? image.right : parent.left
        anchors.leftMargin: imgsource!=="" ? Theme.paddingMedium : Theme.paddingLarge
        anchors.verticalCenter: image.verticalCenter
        anchors.right: parent.right
        anchors.rightMargin: Theme.paddingLarge
        spacing: Theme.paddingSmall

        Label
        {
            id: titleText
            width: parent.width
            truncationMode: TruncationMode.Fade
            color: Theme.primaryColor
        }

        Label
        {
            id: descText
            width: parent.width
            font.pixelSize: Theme.fontSizeExtraSmall
            truncationMode: TruncationMode.Fade
            color: Theme.secondaryColor
            visible: text!==""
        }


    }
}
