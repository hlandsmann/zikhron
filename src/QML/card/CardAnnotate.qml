import QtQuick 2.14
import QtQuick.Layouts 1.11
import QtQuick.Window 2.2
import QtQuick.Controls 2.15
import Qt.labs.settings 1.0

import CardAnnotate 1.0
CardAnnotate {
    id: cardAnnotate
    objectName: "CardAnnotate"

    property string copiedText: ""
    property int cardTextCursorPos: 0
    onTextUpdate: {
        if( copiedText === newText){
            return
        }

        cardText.text = newText
        copiedText = cardText.text
        cardText.cursorPosition = cardTextCursorPos
        console.log("Annotate - text - update")
    }

    TextArea
    {
        id: cardText
        wrapMode: TextArea.WordWrap

        Layout.alignment: Qt.AlignTop
        width: parent.width
        height: parent.height
        readOnly: true

        textFormat: Text.RichText
        text: ""
        font.pointSize: settingsCard.cardFontSize
        color:          settingsCard.cardFontColor

        background: Rectangle {
            color: settingsCard.cardBackgroundColor
        }
    }

}
