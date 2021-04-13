import QtQuick 2.0
import Sailfish.Silica 1.0
import FileCase 1.0

Page {
    id: transferspage

    allowedOrientations: appWindow.pagesOrientations

    property bool loaded: false

    SilicaListView {
        anchors.fill: parent


        PullDownMenu {
            visible: transfersModel.count>0

            MenuItem {
                text: qsTr("Clean completed transfers")
                enabled: transfersModel.count > totalTransfers
                onClicked: {
                    utilities.cleanCompleted()
                }

            }

            MenuItem {
                text: qsTr("Stop transfers")
                visible: utilities.isWorking
                onClicked: {
                    utilities.stopTransfers()
                }
            }

            MenuItem {
                text: qsTr("Start transfers")
                visible: !utilities.isWorking
                onClicked: {
                    utilities.startTransfers()
                }
            }

        }


        header: MyHeader {
            id: header
            title: qsTr("Transfers")
        }

        model: transfersModel

        section.property: "status"
        section.criteria: ViewSection.FullString
        section.delegate: SectionHeader {
            text: section
            height: Theme.itemSizeExtraSmall
        }

        delegate: TransfersDelegate
        {
            name: model.name
            preview: model.mode==="download"? (model.exten.indexOf("/home")>-1? model.exten : "/usr/share/filecase/" + iconTheme + "/" + model.exten + ".png") :
                     model.exten.indexOf("/")===-1? "/usr/share/filecase/" + iconTheme + "/" + model.exten + ".png" : model.link
            mime: "image/png"
            mode: model.mode
            path: model.link
            progress: model.progress
            filesize: model.filesize
            service: model.service

            onRemoved: {
                utilities.removeTransfer(model.link, model.completed)
            }

            Component.onCompleted: {
                console.log("ADD TRANSFER: " + model.name + " - " + model.link + " - " + model.exten)
            }

        }


    }

    ViewPlaceholder {
        enabled: transfersModel.count===0
        Behavior on opacity { FadeAnimation { duration: 10 } }
        text: qsTr("No pending transfers")
    }

}
