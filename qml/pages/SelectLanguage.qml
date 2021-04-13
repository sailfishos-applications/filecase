import QtQuick 2.0
import Sailfish.Silica 1.0
import FileCase 1.0

Dialog {
    id: dialog

    allowedOrientations: appWindow.pagesOrientations

    onStatusChanged: {
        if (status === PageStatus.Activating) {
            selValue = config.readConfig("Language", "en")
        }

    }

    property string prevValue
    property string selValue: "en" //mySettings.getSetting("Language", "en")
    property string selValueName: "English"

    onAccepted: {
        config.setConfig("Language", selValue)
        config.setConfig("LanguageName", selValueName)
        setLanguage(selValueName)
   }

    onRejected: {


    }

    DialogHeader {
        id: header
        acceptText: qsTr("Done")
        cancelText: qsTr("Cancel")
        spacing: 0
    }

    SilicaListView {
        anchors.fill: parent
        anchors.topMargin: header.height
        model: langModel
        delegate: myDelegate
        clip: true
    }

    Component {
        id: myDelegate

        ListItem {
            contentHeight: Theme.itemSizeSmall
            width: parent.width

            highlighted: down || model.value===selValue

            Label {
                x: Theme.paddingLarge
                width: parent.width -Theme.paddingLarge*2
                anchors.verticalCenter: parent.verticalCenter
                text: model.name
                //font.pointSize: 18
                elide: Text.ElideRight
                //color: theme.inverted? "white" : "black"
            }

            onClicked: {
                selValue = model.value
                selValueName = model.name
            }

        }
    }

    ListModel {
        id: langModel

        //ListElement { name: "Albanian"; value: "sq" }
        //ListElement { name: "Arabic"; value: "ar" }
        //ListElement { name: "Basque"; value: "eu" }
        //ListElement { name: "Bulgarian"; value: "bg" }
        ListElement { name: "Catalan"; value: "ca" }
        ListElement { name: "Chinese"; value: "zh" }
        //ListElement { name: "Chinese (China)"; value: "zh_CN" }
        ListElement { name: "Chinese (Hong Kong)"; value: "zh_HK" }
        ListElement { name: "Chinese (Taiwan)"; value: "zh_TW" }
        //ListElement { name: "Croatian"; value: "hr" }
        //ListElement { name: "Czech"; value: "cs" }
        ListElement { name: "Dutch"; value: "nl" }
        ListElement { name: "English"; value: "en" }
        //ListElement { name: "English (United Kingdom)"; value: "en_GB" }
        //ListElement { name: "English (United States)"; value: "en_US" }
        ListElement { name: "Finnish"; value: "fi" }
        ListElement { name: "French"; value: "fr" }
        //ListElement { name: "French (Switzerland)"; value: "fr_CH" }
        //ListElement { name: "Friulian"; value: "fur" }
        ListElement { name: "German"; value: "de" }
        //ListElement { name: "German (Switzerland)"; value: "de_CH" }
        //ListElement { name: "Greek"; value: "el" }
        //ListElement { name: "Hebrew"; value: "he" }
        //ListElement { name: "Hindi"; value: "hi" }
        //ListElement { name: "Hungarian"; value: "hu" }
        ListElement { name: "Italian"; value: "it" }
        //ListElement { name: "Macedonian"; value: "mk" }
        //ListElement { name: "Malay"; value: "ms" }
        ListElement { name: "Persian"; value: "fa" }
        //ListElement { name: "Polish"; value: "pl" }
        //ListElement { name: "Portuguese (Brazil)"; value: "pt" }
        //ListElement { name: "Portuguese (Portugal)"; value: "pt_PT" }
        //ListElement { name: "Romanian"; value: "ro" }
        ListElement { name: "Russian"; value: "ru" }
        ListElement { name: "Spanish"; value: "es" }
        //ListElement { name: "Spanish (Argentina)"; value: "es_AR" }
        //ListElement { name: "Spanish (Mexico)"; value: "es_MX" }
        //ListElement { name: "Swedish (Finland)"; value: "sv_FI" }
        ListElement { name: "Swedish"; value: "sv" }
        //ListElement { name: "Thai"; value: "th" }
        //ListElement { name: "Turkish"; value: "tr" }
        //ListElement { name: "Ukrainian"; value: "uk_UA" }
        //ListElement { name: "Urdu"; value: "ur" }
        //ListElement { name: "Vietnamese"; value: "vi" }
    }

}
