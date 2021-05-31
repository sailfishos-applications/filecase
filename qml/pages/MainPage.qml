import QtQuick 2.0
import Sailfish.Silica 1.0
import FileCase 1.0

Page {
    id: page

    allowedOrientations: appWindow.pagesOrientations

    property string pageID: "mainPage"

    property bool selectionmode: false
    property string selectionmodeItems: ""

    property bool loaded: false

    property string startFolder
    property string nextFolder

    canNavigateForward: !clearRemorse.visible
    backNavigation: !clearRemorse.visible


    Browser {
        id: browser
    }


    onStatusChanged: {
        if (status===PageStatus.Activating)
        {
            if (!loaded)
                browser.clearItems()

            currentLocation = "device"
            currentDeviceFolder = startFolder
        }

        if (status===PageStatus.Active)
        {

            if (!loaded)
            {
                if (sdfolder==="-")
                    sdfolder = browser.getSDcard()

                //console.log("Loading broser. Folder = "+ startFolder + " - Next = " + nextFolder)

                if (!showRootFiles) {
                    console.log("Root files hidden. index = " + nextFolder.indexOf(sdfolder))
                    if (nextFolder.indexOf(sdfolder)===-1 && nextFolder.indexOf(config.getHome())===-1)
                        nextFolder = config.getHome()
                }

                //console.log("Destination Folder = "+nextFolder)

                if (nextFolder!==startFolder && !startPageLoaded) {
                    var nextpage = nextFolder.replace(startFolder, "")
                    var i = nextpage.indexOf("/")
                    if (i>0) nextpage = nextpage.substring(0, i)
                    if (i==0) {
                        var nexttemp = nextpage.replace("/","")
                        i = nexttemp.indexOf("/")
                        if (i>0) nextpage = "/" + nexttemp.substring(0, i)
                    }
                    pageStack.push("MainPage.qml", {"id": page, "startFolder": startFolder + nextpage,
                                                    "nextFolder": nextFolder}, PageStackAction.Immediate)
                } else {
                    startPageLoaded = true
                }

                browser.setPath(startFolder)
                loaded = true

                if (startPageLoaded)
                    pageStack.pushAttached(Qt.resolvedUrl("FoldersPanel.qml"))

            }


            else
            {
                pageStack.pushAttached(Qt.resolvedUrl("FoldersPanel.qml"))
                checkRoot()

            }
        }
    }

    function checkRoot() {
        if ((!showRootFiles && (browser.path==config.getHome() || browser.path==sdfolder)) || browser.path=="/")
            backNavigation = false
        else
            backNavigation = true
    }

    function renameFile(path, filename) {
        console.log("Renaming file to " + filename)
        browser.renameFile(path, filename)
    }

    function createFolder(filename) {
        browser.newFolder(filename)
        browser.reload()
    }

    function setClipboard(action) {
        utilities.checkClipboard(action, currentDeviceFolder, config.readConfig("AutoRemove", "false"))
    }

    function share(currentSel) {
        for (var i=0; i<browser.multi2; ++i) {
            var val = browser.get(i)

            console.log("Checking " + val.name + " - Cloud: " + currentSel)

            /*if (currentSel==="email") {
                currentDeviceFolder()
                shareToEmail(val.path)
            }*/
            if (currentSel==="Dropbox")
                utilities.transferFile("Dropbox", "upload", val.name, val.path, val.tsize, val.size)
            else if (currentSel==="Google Drive")
                utilities.transferFile("Google Drive", "upload", val.name, val.path, val.tsize, val.size)
            else if (currentSel==="OneDrive")
                utilities.transferFile("OneDrive", "upload", val.name, val.path, val.tsize, val.size)
            else if (currentSel==="Box")
                utilities.transferFile("Box", "upload", val.name, val.path, val.tsize, val.size)
            else
                utilities.transferFile("WebDav-"+currentSel, "upload", val.name, val.path, val.tsize, val.size)

        }
        controlPanel.open = false
        browser.clearSelection()
    }

    Connections {
        target: appWindow

        onClearSelectionItems: {
            controlPanel.open = false
            browser.clearSelection()
        }

        onReloadItems: {
            browser.reload()
        }

        onReloadFolder: {
            if (target===startFolder)
                browser.reload()
        }

    }

    Connections {
        target: browser

        onCountChanged: loaded = true

        onDataChanged: loaded = true

        onBannerChanged: showBanner(msg, ok)

        onPathChanged: checkRoot()

        onFileRenamed: itemRenamed(newname)

        /*onAskPassword: {
            passInput.text = ""
            passDialog.open()
        }*/

        /*onCountChanged: {
            fast.listViewChanged()
        }*/

        /*onCopyDialog: {
           sta = 1;
           copyFileDialog.open();
        }*/

    }

    SilicaFlickable {
        anchors.top: parent.top
        anchors.bottom: controlPanel.top
        width: parent.width

        //opacity: foldersPanel.open? 0.3 : 1
        //enabled: !foldersPanel.open

        PullDownMenu {
            enabled: !selectionmode
            MenuItem {
                text: qsTr("Settings")
                onClicked: pageStack.push("Settings.qml")
            }
            MenuItem {
                text: qsTr("Search")
                onClicked: {
                    search.cleanSearch()
                    pageStack.push("SearchPage.qml")
                }
            }
            MenuItem {
                text: qsTr("Clipboard")
                visible: !standardClipboard
                onClicked: pageStack.push("ClipboardPage.qml")
            }
            MenuItem {
                text: qsTr("New folder")
                onClicked: pageStack.push("NewFolder.qml")
            }
            MenuItem {
                text: qsTr("Paste here")
                visible: standardClipboard && (action=="singlecopy" || action=="singlemove")
                onClicked: {
                    working = true
                    if (action=="singlecopy")
                        browser.checkStandardClipboard("copy")
                    else if (action=="singlemove")
                        browser.checkStandardClipboard("move")
                }
            }

        }

        MyHeader {
            id: header
            title: browser.path1!==""? browser.path1 : "/"
            description: selectionmode? (browser.multi1==0? qsTr("No files selected") :
                         (browser.multi1==1? qsTr("1 file selected") :
                         qsTr("%1 files selected").arg(browser.multi1))) :
                         (browser.path1===""? "" : browser.path2)
            width: parent.width -(working? Theme.itemSizeSmall : 0) //-upBtn.width
            anchors.right: parent.right
        }

        /*IconButton {
            id: upBtn
            //enabled: browser.path!=="/" || (showRootFiles && browser.path==config.getHome())
            anchors.verticalCenter: header.verticalCenter
            anchors.left: parent.left
            anchors.leftMargin: Theme.paddingMedium
            icon.source: "image://theme/icon-m-up"
            onClicked: {
                console.log("Current path: " + browser.path)

                upBtn.enabled = false
                controlPanel.open = false
                browser.clearSelection()
                loadingFolder = true
                browser.goUp()
                searchText.text = ""
                container.positionViewAtIndex(container.last, ListView.Center)
                //fast.listViewChanged()
            }
        }*/

        BusyIndicator {
            anchors.verticalCenter: header.verticalCenter
            anchors.left: parent.left //upBtn.right
            anchors.leftMargin: Theme.paddingLarge*2
            visible: working || loadingFolder
            running: visible
            size: BusyIndicatorSize.Small
        }

        MouseArea {
            anchors.verticalCenter: header.verticalCenter
            height: header.height
            width: Theme.itemSizeSmall
            anchors.left: parent.left //upBtn.right
            anchors.leftMargin: Theme.paddingLarge*2
            onClicked: {
                if ( action == "search" )
                    search.stopSearch()
                else if ( action == "extract" )
                    utilities.stopExtracting()
                else if ( action == "compress" )
                    utilities.stopExtracting()
                else if ( action == "copy/move" )
                    browser.stopCopying()
                working = false
                action = ""
            }
        }



        Text {
            id: searchText
            visible: false

        }

        SilicaListView { //SilicaGridView {
            id: container

            anchors.fill: parent
            anchors.topMargin: header.height
            visible: loaded
            clip: true

            model: browser

            //cellWidth: appWindow._screenWidth>appWindow._screenHeight? appWindow._screenWidth/2 : appWindow._screenWidth
            //cellHeight: Theme.itemSizeMedium

            delegate: FileDelegate {
                id: fileDelegate
                property bool filtered: model.name.match(new RegExp(searchText.text,"i")) !== null
                contentHeight: Theme.itemSizeMedium //container.cellHeight
                //contentWidth: container.cellWidth

                highlighted: down || model.type===Browser.FolderSel || model.type===Browser.FileSel //selectionmodeItems.indexOf(model.path)>-1

                imgsource: model.exten.indexOf("/")===-1? "/usr/share/filecase/" + iconTheme + "/" + model.exten +
                           (model.exten==="folder" && model.link==="yes"? "-link" : "") + ".png" : model.path

                mime: model.exten.indexOf("/")>-1? model.exten : "image/png"

                title: searchText.text.length>0 ? replaceText(model.name, searchText.text) : model.name

                description: model.size.length>0? model.date + (model.date.length>0? " - ":"") + model.size : model.date

                showLink: model.link==="yes" && model.exten!=="folder"

                onClicked: {
                    if ( selectionmode ) //&& (extractcancel.visible==false) )
                    {
                        //var action = selectionmodeItems.indexOf(model.path)>-1? "remove" : "add"
                        //selectionmodeItems = browser.manageList(action, selectionmodeItems, model.path)
                        browser.selectItem(model.path)
                        //console.log(model.type)
                    }
                    else if (type==Browser.Folder || type==Browser.FolderSel)
                    {
                        browser.clearSelection()
                        //loadingFolder = true
                        //browser.setPath(model.path)
                        pageStack.push("MainPage.qml", {"id": page, "startFolder": model.path, "nextFolder": model.path})
                        //searchText.text=""
                        //hideSearchBar()
                        container.positionViewAtBeginning()
                        //fast.listViewChanged()
                    }
                    else if (type==Browser.File || type==Browser.FileSel)
                    {
                        if (exten=="tar")
                        {
                            action = "nothing"
                            //config.setConfig("CompressedFile", model.path)
                            //config.setConfig("CompressedFileName", model.name)
                            pageStack.push("Compressed.qml", {"filename": model.path, "name": model.name})
                        }
                        else {
                            //browser.openFile(model.path)
                            fileInfo.setFile(model.path)
                            pageStack.push("FileInfo.qml", {"cloudFile": false})
                        }
                    }
                }

                onPressAndHold: {
                    selectionmode = true
                    controlPanel.open = true
                    browser.selectItem(model.path)
                    //selectionmodeItems = browser.manageList("add", selectionmodeItems, model.path)
                }

            }


        }

        ViewPlaceholder {
            enabled: container.count===0 && loaded
            Behavior on opacity { FadeAnimation { duration: 10 } }
            text: qsTr("Folder is empty")
        }

        BusyIndicator {
            anchors.centerIn: parent
            visible: !loaded
            running: visible
            size: BusyIndicatorSize.Large
        }

    }

    DockedPanel {
        id: controlPanel
        width: parent.width
        height: Theme.itemSizeLarge
        enabled: !working
        dock: Dock.Bottom
        open: false
        opacity: open? 1 : 0
        Behavior on opacity { FadeAnimation { duration: 200 } }

        onOpenChanged: {
            if (!open) {
                selectionmode = false
                browser.clearSelection()
            }
        }

        Image {
            anchors.fill: parent
            fillMode: Image.PreserveAspectFit
            source: "image://theme/graphic-gradient-edge"
        }

        Row {
            anchors.centerIn: parent
            //width: parent.width
            //anchors.horizontalCenter: parent.horizontalCenter
            //spacing: container.isPortrait? Theme.paddingSmall: Theme.paddingLarge

            IconButton {
                icon.source: "image://theme/icon-m-clear"
                //anchors.verticalCenter: parent.verticalCenter
                width: favBtn.show || shareBtn.show? page.width/6 : page.width/5
                Behavior on width { NumberAnimation { duration: 200 } }
                onClicked: {
                    controlPanel.open = false
                    browser.clearSelection()
                }
            }

            IconButton {
                icon.source: "image://theme/icon-m-clipboard"
                enabled: browser.multi1>0
                width: favBtn.show || shareBtn.show? page.width/6 : page.width/5
                Behavior on width { NumberAnimation { duration: 200 } }
                onClicked: {
                    if (standardClipboard) {
                        config.setConfig("StandardClipboard", "")
                        action = "singlecopy"
                        browser.addToStandardClipboard("selectedfiles", true)
                    } else {
                        browser.addToClipboard("selectedfiles", config.readConfig("AutoSelect", "false"))
                    }
                    controlPanel.open = false
                    browser.clearSelection()
                }
            }

            /*IconButton {
                icon.source: "file:///usr/share/filecase/icons/toolbar-cut.png"
                enabled: browser.multi1>0
                visible: standardClipboard
                width: favBtn.show || shareBtn.show? page.width/6 : page.width/5
                onClicked: {
                    config.setConfig("StandardClipboard", "")
                    action = "singlemove"
                    browser.addToStandardClipboard("selectedfiles", true)
                    controlPanel.open = false
                    browser.clearSelection()
                }
            }*/

            IconButton {
                icon.source: "image://theme/icon-m-delete"
                enabled: browser.multi1>0
                width: favBtn.show || shareBtn.show? page.width/6 : page.width/5
                Behavior on width { NumberAnimation { duration: 200 } }
                onClicked: {
                    working = true
                    deleteMultipleItems()
                }
            }


            IconButton {
                icon.source: "image://theme/icon-m-levels"
                enabled: browser.multi1>0
                width: favBtn.show || shareBtn.show? page.width/6 : page.width/5
                Behavior on width { NumberAnimation { duration: 200 } }
                onClicked: {
                    var list = browser.getSelectedList()
                    controlPanel.open = false
                    browser.clearSelection()
                    pageStack.push("CompressDialog.qml", {"filelist": list})
                }
            }

            IconButton {
                id: favBtn
                property bool show: browser.multi1==1 && browser.multi3==1
                icon.source: "image://theme/icon-m-favorite-selected" //: "image://theme/icon-m-favorite"
                width: show? page.width/6 : 0
                Behavior on width { NumberAnimation { duration: 200 } }
                opacity: show? 1 : 0
                Behavior on opacity { FadeAnimation { duration: 200 } }
                onClicked: {
                    utilities.addToBookmarks(browser.getSelectedList())
                    controlPanel.open = false
                    browser.clearSelection()
                }
            }

            IconButton {
                id: shareBtn
                property bool show: browser.multi1>0 && browser.multi3==0
                icon.source: "image://theme/icon-m-share"
                //enabled: driveLoaded
                width: show? page.width/6 : 0
                Behavior on width { NumberAnimation { duration: 200 } }
                opacity: show? 1 : 0
                Behavior on opacity { FadeAnimation { duration: 200 } }
                onClicked: {
                    //browser.infoFiles("selectedfiles")
                    fileInfo.setFile(browser.getSelectedList())
                    pageStack.push("UploadFiles.qml")
                    //controlPanel.open = false
                    //browser.clearSelection()
                }
            }

            IconButton {
                icon.source: "file:///usr/share/filecase/icons/toolbar-info.png"
                enabled: browser.multi1>0
                width: favBtn.show || shareBtn.show? page.width/6 : page.width/5
                Behavior on width { NumberAnimation { duration: 200 } }
                onClicked: {
                    //commonTools.enabled = false
                    //if (browser.multi1==1)
                    //    browser.infoFiles(model.path)
                    //else
                    //browser.infoFiles("selectedfiles")
                    fileInfo.setFile(browser.getSelectedList())
                    pageStack.push("FileInfo.qml", { "cloudFile": false })
                    controlPanel.open = false
                    browser.clearSelection()
                }
            }
        }

    }

    function deleteMultipleItems()
    {

        clearRemorse.execute(qsTr("Removing"), function()
        {
            working = true
            browser.removeFile("selectedfiles", true)
            controlPanel.open = false
            browser.clearSelection()
        })
    }

    RemorsePopup {
        id: clearRemorse
        onCanceled: working = false
    }



    /*MouseArea {
        anchors.fill: parent
        enabled: foldersPanel.open // || bigPicturePanel.open
        onClicked: {
            foldersPanel.open = false
            //bigPicturePanel.open = false
        }
    }*/

    /*FoldersPanel {
        id: foldersPanel
        width: Math.min(parent.height, parent.width) * (mainPage.isPortrait? 0.8 : 0.9)
        height: parent.height
        dock: Dock.Right
        open: false
        opacity: open? 1 : 0

        Behavior on opacity { FadeAnimation {} }

        Behavior on x {
            NumberAnimation {
                id: horizontalMoving
                duration: 250; easing.type: Easing.InOutQuad
            }
        }

    }*/


}


