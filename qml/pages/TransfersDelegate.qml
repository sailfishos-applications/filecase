import QtQuick 2.0
import Sailfish.Silica 1.0
import FileCase 1.0
import org.nemomobile.thumbnailer 1.0

ListItem
{
    id: itemcontainer

    contentHeight: Theme.itemSizeMedium

    signal removed

    property alias name: titleText.text
    property alias mime: image.mimeType
    property alias preview: image.source
    property string path
    property string mode
    property string filesize
    property string transfered
    property real progress
    property string service

    property string lname: name.toLowerCase()
    property string lprev: image.source

    property bool showVideo: lprev.indexOf("/usr/share/filecase")==-1 &&
                             (lname.indexOf(".mp4")>-1 || lname.indexOf(".avi")>-1 || lname.indexOf(".3gp")>-1
                              || lname.indexOf(".wmv")>-1 || lname.indexOf(".mpeg")>-1)


    width: parent.width
    clip: true


    Connections {
        target: utilities
        onProgressChanged: {
            //console.log("TRANSFERING: " + filename + " " + val)
            if (path == link) {
				progress = val;
				transfered = curprog;
			}
        }
    }

    Thumbnail {
        id: image

        height: parent.Theme.itemSizeMedium -Theme.paddingMedium
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
        visible: showVideo
    }

    Image {
        anchors.centerIn: rec
        height: rec.height - Theme.paddingSmall
        width: height
        smooth: true
        source: "image://theme/icon-m-video"
        visible: showVideo
    }


    Column {
        anchors.left: image.right
        anchors.leftMargin: Theme.paddingMedium
        anchors.right: cancelBtn.left
        anchors.rightMargin: Theme.paddingMedium
        spacing: Theme.paddingSmall
        y: progress>0 ? 0 : Theme.paddingMedium

        Behavior on y { NumberAnimation { duration: 100 } }

        Label
        {
            id: titleText
            width: parent.width
            truncationMode: TruncationMode.Fade
            color: Theme.primaryColor
        }

        Row {
            spacing: Theme.paddingSmall
            width: parent.width

            Image
            {
                anchors.top: parent.top
                width: height
                height: descText.height
                smooth: true
                source: "image://theme/icon-m-cloud-" + mode
            }

            Label
            {
                id: descText
                //width: parent.width
                font.pixelSize: Theme.fontSizeExtraSmall
                truncationMode: TruncationMode.Fade
                color: Theme.secondaryColor
                text: progress==0? service + " - " + filesize  : (transfered + " / " + filesize)
                visible: text!==""
            }

        }



    }

    ProgressBar
    {
        id: progressBar
        anchors.left: image.right
        anchors.leftMargin: Theme.paddingMedium
        anchors.right: cancelBtn.left
        anchors.rightMargin: Theme.paddingMedium
        anchors.bottom: parent.bottom
        anchors.bottomMargin: -Theme.fontSizeSmall
        minimumValue: 0
        maximumValue: 100
        value: progress
        //indeterminate: progress<=0
        _hasValueLabel: false
        height: descText.height
        leftMargin: 0
        rightMargin: 0
        opacity: progress>0?  1 : 0
        //visible: progress>0
        Behavior on opacity { FadeAnimation { duration: 250 } }
    }

    IconButton {
        id: cancelBtn
        anchors.right: parent.right
        anchors.verticalCenter: image.verticalCenter
        anchors.rightMargin: Theme.paddingMedium
        smooth: true
        icon.source: "image://theme/icon-m-clear"
        onClicked: removed()
    }
	


}
