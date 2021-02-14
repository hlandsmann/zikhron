import QtQuick 2.14
import QtQuick.Layouts 1.4
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import Qt.labs.settings 1.0
ApplicationWindow {
    id: window
    objectName: "window"

    visible: true
    width: 800
    height: 600
    color: "#222"

    Settings {
        category: "Window"
        id: settings
        property alias x: window.x
        property alias y: window.y
        property alias width: window.width
        property alias height: window.height
    }
    RowLayout {
        anchors.fill: parent
        ColumnLayout{
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillHeight: true

            Button {
                id: cardsButton
                checkable: true
                checked: true
                text: "Cards"
                onClicked: {
                    checked = true
                    textButton.checked = false
                    card.visible = true
                }
            }
            Button {
                id: textButton
                checkable: true
                checked: false
                text: "Browse"
                onClicked: {
                    checked = true
                    cardsButton.checked = false
                    card.visible = false
                }
            }
        }
        Card{
            Layout.alignment: Qt.AlignTop
            Layout.fillWidth: true
            Layout.fillHeight: true
            id: card
        }
    }
}
