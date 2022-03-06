import QtQuick 2.0
import Sailfish.Silica 1.0
import FileCase 1.0
import "pages"

ApplicationWindow
{
    id: appWindow

    property string appVersion: "0.4.1"

    property string savedorientation: config.readConfig("Orientation", "auto")

    property int pagesOrientations: savedorientation==="auto"? (Orientation.Portrait | Orientation.Landscape) :
                                    (savedorientation==="landscape"? Orientation.Landscape : Orientation.Portrait)

    initialPage: MainPage { id: mainPage; startFolder: "/"; nextFolder: config.readConfig("StartupFolder", config.getHome()) }
    cover: coverPage

    property bool loadingFolder: false
    property bool working: false
    property string action: ""

    property bool startPageLoaded: false

    property string currentLocation: "device"
    property string currentDeviceFolder


    CoverPage { id: coverPage }
    //MainPage { id: mainPage; startFolder: "/"; nextFolder: config.readConfig("StartupFolder", config.getHome()) }
    Config { id: config }
    //Browser { id: browser }
    Utilities { id: utilities }
    FilesInfo { id: fileInfo }
    Clipboard { id: clipboard }
    Search { id: search }

    TransfersPage { id: transfersPage }


    Component.onCompleted: {
        utilities.loadBookmarks()
        utilities.loadWebDavs()
        utilities.loadClouds()
        utilities.loadTransfers()
        //savedorientation = config.getConfig("Orientation", "auto")
    }

    property bool standardClipboard: config.readConfig("UseStandardClipboard", "false")==="true"
    property bool showRootFiles: config.readConfig("ShowRootFiles", "false")==="true"
    property string iconTheme: config.readConfig("IconTheme", "icons")

    function replaceText(text,str) {
        var ltext = text.toLowerCase()
        var lstr = str.toLowerCase()
        var ind = ltext.indexOf(lstr)
        var txt = text.substring(0,ind)
        text = txt + "<u><font color="+fileInfo.selColor(currentThemeColor)+">" +
               text.slice(ind,ind+str.length)  + "</font></u>" +
               text.slice(ind+str.length,text.length);
        return text;
    }

    function showBanner(msg, ok) {
        working = false
        action = ""
        ibanner.displayMessage(msg, ok)
    }

    Banner { id: ibanner }

    signal setLanguage(string langval)

    signal reloadItems()
    signal reloadFolder(string target)

    signal clearSelectionItems()

    signal itemRenamed(string newname)
    signal itemLinkShared(string fileurl)

    function getName(text) {
        var res = text.split('/')
        res = res[res.length-1]
        if (res==="") res = "/"
        return res;
    }


    Connections {
        target: utilities

        onBannerChanged: showBanner(msg, ok)

        onAppendBookmark: {
            console.log("Loading book: " + folder)
            bookModel.append({"folder": folder})
        }

        onBookmarkAdded: {
            console.log("Appending book: " + folder)
            bookModel.append({"folder": folder})
        }

        onBookmarkRemoved: {
            console.log("Removing book: " + folder)
            for (var i=0; i<bookModel.count; ++i) {
                if (bookModel.get(i).folder===folder) {
                    bookModel.remove(i)
                    break
                }
            }
        }

        onUtilWorking: {
            loadingFolder = work
            console.log("Working done in folder: " + target)
            if (target!=="") reloadFolder(target)
        }

        onAppendWebDav: {
            console.log("Loading webdav: " + service)
            davModel.append({"service": service})
        }

        onAppendCloud: {
            console.log("Loading cloud: " + service + "::" + locate)
            cloudModel.append({"name": service, "qml": qmlfile,
                               "icon": "file:///usr/share/filecase/icons/"+locate+".png",
                               "locate": locate })
        }

        onAddTransferFile: {
            var added = false
            for (var i=0; i<transfersModel.count; ++i) {
                if (transfersModel.get(i).completed && !completed) {
                    transfersModel.insert(i, {"service":service.replace("WebDav-",""), "mode":mode, "name":name, "link":link,
                                            "exten":exten, "filesize":filesize, "realsize":realsize, "progress":0,
                                            "status":(completed? qsTr("Completed"):qsTr("Queued")),
                                            "completed":completed})
                    added = true;
                    break;
                }
            }
            if (!added) {
                transfersModel.append({"service":service.replace("WebDav-",""), "mode":mode, "name":name, "link":link,
                                       "exten":exten, "filesize":filesize, "realsize":realsize, "progress":0,
                                       "status":(completed? qsTr("Completed"):qsTr("Queued")),
                                       "completed":completed})
            }
        }

        onRemoveTransferFile: {
            for (var i=0; i<transfersModel.count; ++i) {
                if (transfersModel.get(i).link===link) {
                    //&& transfersModel.get(i).completed===completed) {
                    transfersModel.remove(i);
                    break;
                }
            }
        }

        onClearTransfers: transfersModel.clear()

        onLoadTransfersCompleted: {
            console.log("Loaded transfers: " + queued)
            totalTransfers = queued
        }

    }

    property string sdfolder: "-"

    ListModel {
        id: bookModel
    }

    ListModel {
        id: davModel
    }

    ListModel {
        id: transfersModel
    }

    ListModel {
        id: cloudModel
        /*: [
        { name: "Dropbox", qml: "Dropbox.qml", icon: "file:///usr/share/filecase/icons/dropbox.png", loaded: dropboxLoaded, locate: "dropbox" },
        { name: "Google Drive", qml: "Drive.qml", icon: "file:///usr/share/filecase/icons/gdrive.png", loaded: driveLoaded, locate: "gdrive" },
        { name: "OneDrive", qml: "SkyDrive.qml", icon: "file:///usr/share/filecase/icons/skydrive.png", loaded: skyLoaded, locate: "sky" }
        ]*/
    }


    property int totalTransfers: 0 //transfersModel.count


    //WebDav { id: webdav }
    //DriveClient { id: driveclient }
    //DropboxClient { id: dropboxclient }
    //SkyClient { id: skyclient }
    //BoxClient { id: boxclient }



    property variant boxAccount: []
    property string boxAction
    property string boxActionPath
    property string boxActionPathType
    property string boxActionPathFull

    property variant driveAccount: []
    property string driveAction
    property string driveActionPath
    property string driveActionPathType
    property string driveActionPathFull

    property variant dropboxAccount: []
    property string dropboxAction
    property string dropboxActionPath
    property string dropboxActionPathType
    property string dropboxActionPathFull

    property variant skyAccount: []
    property string skyAction
    property string skyActionPath
    property string skyActionPathType
    property string skyActionPathFull


    property variant webdavAccount: []
    property string webdavAction
    property string webdavActionPath
    property string webdavActionPathType
    property string webdavActionPathFull


    function getFolderName(folder) {
        var res = folder

        if (res==="/")
            return res

        res = res.split('/')
        res = res[res.length-1]
        if (res==="") res = "/"
        return res
    }


}


