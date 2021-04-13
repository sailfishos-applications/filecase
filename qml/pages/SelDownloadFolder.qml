import QtQuick 2.0
import Sailfish.Silica 1.0
import FileCase 1.0


Dialog {

    allowedOrientations: appWindow.pagesOrientations

    onAccepted: {
        if (currentLocation=="device")
            config.setConfig("StartupFolder", browser.path)
        else if (currentLocation=="extract")
            config.setConfig("ExtractFolder", browser.path)
        else
            config.setConfig(currentLocation + "/download_folder", browser.path)

        pageStack.pop()
    }

    Browser { id: browser }

    onStatusChanged: {
        if (status===PageStatus.Activating) {
            console.log("CURRENT LOCATION: " + currentLocation)

            if (currentLocation=="device")
                browser.setPath( config.readConfig("StartupFolder", "/home/nemo") )
            else if (currentLocation=="extract")
                browser.setPath( config.readConfig("ExtractFolder", "/home/nemo") )
            else
                browser.setPath( config.readConfig(currentLocation + "/download_folder", "/home/nemo/Downloads") )
        }
    }

    canAccept: !loadingFolder

    function checkRoot() {
        if (browser.path==="/home/nemo" || browser.path===sdfolder)
            upBtn.enabled = false
        else
            upBtn.enabled = true
    }

    Connections {
        target: browser

        onCountChanged: loadingFolder = false

        onDataChanged: loadingFolder = false

        onPathChanged: checkRoot()

    }

    DialogHeader {
        id: header
        acceptText: qsTr("Done")
        cancelText: qsTr("Cancel")
        spacing: 0
    }

    SilicaFlickable {
        anchors.fill: parent
        anchors.topMargin: header.height
        clip: true


        PullDownMenu {
            visible: sdfolder!==""
            MenuItem {
                text: browser.path.indexOf("/home/nemo")>-1? qsTr("SD Card") : qsTr("Phone memory")
                onClicked: {
                    if (browser.path.indexOf("/home/nemo")>-1)
                        browser.setPath(sdfolder)
                    else
                        browser.setPath("/home/nemo")
                }
            }
        }


        MyHeader {
            id: header2
            title: browser.path1.length>0 ? browser.path1 : "/"
            description: browser.path2
            anchors.top: parent.top
            anchors.left: upBtn.right
            //anchors.leftMargin: Theme.paddingMedium
            anchors.right: parent.right
        }

        IconButton {
            id: upBtn
            enabled: browser.path.indexOf("/home/nemo/")>-1
            anchors.verticalCenter: header2.verticalCenter
            anchors.left: parent.left
            anchors.leftMargin: Theme.paddingMedium
            icon.source: "image://theme/icon-m-up"
            onClicked: {
                console.log("Current path: " + browser.path)

                if (browser.path==="/home/nemo" || browser.path===sdfolder) {
                    return
                }
                else
                {
                    loadingFolder = true
                    browser.goUp()
                    //container.positionViewAtIndex(container.last, ListView.Center)
                    //fast.listViewChanged()
                }
            }
        }

        SilicaListView {
            id: container

            anchors.fill: parent
            anchors.topMargin: header2.height
            clip: true

            model: browser

            delegate: FileDelegate {

                enabled: model.exten==="folder"

                opacity: enabled? 1 : 0.5

                contentHeight:Theme.itemSizeMedium

                highlighted: down || model.type===Browser.FolderSel || model.type===Browser.FileSel //selectionmodeItems.indexOf(model.path)>-1

                imgsource: model.exten.indexOf("/")===-1? "/usr/share/filecase/" + iconTheme + "/" + model.exten + ".png" : model.path

                mime: model.exten.indexOf("/")>-1? model.exten : "image/png"

                title: model.name

                description: model.size.length>0? model.date + (model.date.length>0? " - ":"") + model.size : model.date

                onClicked: {
                    browser.setPath(model.path)
                }

            }


        }

        ViewPlaceholder {
            enabled: container.count===0 && !loadingFolder
            Behavior on opacity { FadeAnimation { duration: 10 } }
            text: qsTr("Folder is empty")
        }
    }

}
