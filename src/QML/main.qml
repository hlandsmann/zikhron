import QtQuick 2.14
import QtQuick.Layouts 1.4
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4

ApplicationWindow {
    id: root
    objectName: "window"

    visible: true
    width: 800
    height: 600
    color: "#222"

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
