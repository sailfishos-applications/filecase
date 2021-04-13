import QtQuick 2.0
import Sailfish.Silica 1.0
import FileCase 1.0
import QtMultimedia 5.0
import Sailfish.TransferEngine 1.0

Page {

    allowedOrientations: appWindow.pagesOrientations

    onStatusChanged: {
        if (status === PageStatus.Inactive) {
            fileInfo.stopInfoThread()
        }
    }

    function share(currentSel) {
        var cpage = pageStack.find(function(page) { return page.startFolder===currentDeviceFolder; })
        cpage.share(currentSel)
        pageStack.pop()
    }

    SailfishTransferMethodsModel {
        id: transferMethodsModel
        filter: fileInfo.data.mime
    }

    ListModel {
        id: emptyModel
    }

    SilicaFlickable {
        anchors.fill: parent
        //anchors.topMargin: header.height
        clip: true

        contentHeight: header2.height + dataPanel.height + Theme.paddingLarge*2

        Column {
            id: header2
            anchors.left: parent.left
            anchors.right: parent.right

            MyHeader {
                title: qsTr("Share")
                description: qsTr("Select service")
            }

            Repeater {
                model: cloudModel
                //anchors.top: header2.bottom
                delegate: FileDelegate {
                    //highlighted: down || currentSel===item.name
                    contentHeight: Theme.itemSizeSmall
                    enabled: model.loaded
                    opacity: enabled? 1 : 0.5
                    imgsource: "" //item.icon
                    title: model.name
                    onClicked: share(model.name) //currentSel = item.name
                }
            }

            Repeater {
                model: davModel
                //anchors.top: cloudModel.bottom
                delegate: FileDelegate {
                    contentHeight: Theme.itemSizeSmall
                    //opacity: enabled? 1 : 0.5
                    imgsource: "" //item.icon
                    title: model.service
                    onClicked: share(model.service) //currentSel = item.name
                }
            }

            Repeater {
                id: rootList
                property url source: fileInfo.data.path + "/" + fileInfo.data.name
                property variant content: ({})
                //property alias filter: transferMethodsModel.filter

                model: fileInfo.data.icon!==""? transferMethodsModel : emptyModel

                objectName: "menuList"
                //source: fileInfo.path + "/" + fileInfo.name
                //filter: fileInfo.ftype.name

                delegate: FileDelegate {
                    contentHeight: Theme.itemSizeSmall
                    imgsource: ""
                    title: qsTrId(displayName)
                    description: userName
                    onClicked: {
                        clearSelectionItems()
                        pageStack.replace(shareUIPath, {
                                           source: rootList.source,
                                           content: rootList.content,
                                           methodId: methodId,
                                           displayName: displayName,
                                           accountId: accountId,
                                           accountName: userName
                                       })

                    }

                }

            }

        }

        Column {
            id: dataPanel
            spacing: Theme.paddingLarge
            anchors.top: header2.bottom
            anchors.topMargin: Theme.paddingLarge
            x: Theme.paddingLarge
            width: parent.width -Theme.paddingLarge*2

            Separator {
                width: parent.width
                color: Theme.secondaryColor
            }

            Image {
                id: image
                visible: fileInfo.data.icon!==""

                source: fileInfo.data.icon.indexOf("/")===-1?
                        "file:///usr/share/filecase/icons/" + fileInfo.data.icon + ".png" :
                        (fileInfo.data.icon!=="" ? fileInfo.getFilePreview(fileInfo.data.path + "/" + fileInfo.data.name) : "")

                width: parent.width
                height: implicitHeight < 480 && implicitHeight != 0 ? implicitHeight : 480
                sourceSize.width: width
                sourceSize.height: height
                fillMode: Image.PreserveAspectFit
                anchors.horizontalCenter: parent.horizontalCenter
                asynchronous: true

            }

            Text {
                id: textname
                width: parent.width
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                color: Theme.primaryColor
                text: fileInfo.data.name
            }

            Column {
                width: parent.width
                spacing: Theme.paddingSmall

                ValueItem {
                    id: location
                    title: fileInfo.data.target==="" ? qsTr("Location") : qsTr("Target")
                    value: fileInfo.data.target==="" ? fileInfo.data.path : fileInfo.data.target
                }

                ValueItem {
                    id: filetype
                    visible: fileInfo.data.icon!==""
                    title: qsTr("Type")
                    value: fileInfo.data.comment + "\n(" + fileInfo.data.mime + ")"
                }

                ValueItem {
                    id: filesize
                    title: qsTr("Size")
                    value: fileInfo.data.size
                }

                ValueItem {
                    id: filecreated
                    visible: value!==""
                    title: qsTr("Created")
                    value: fileInfo.data.created
                }

                ValueItem {
                    id: filemodified
                    visible: value!==""
                    title: qsTr("Last modified")
                    value: fileInfo.data.modified
                }

            }

        }


    }


}
