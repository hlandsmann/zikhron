import QtQuick 2.14
import QtQuick.Layouts 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Window 2.2
import CardDisplay 1.0
import QtQuick.Controls 2.15
CardDisplay {
    id: cardDisplay
    objectName: "CardDisplay"

    property string copiedText: ""
    property bool textUpdateReceived: false
    property int cardTextCursorPos: 0
    onHovered: {
        var pos = cardText.positionAt(x, y, TextInput.CursorOnCharacter)
        // var pos = cardText.positionAt(x, y, TextInput.CursorBetweenCharacters)
        cardDisplay.hoveredTextPosition(pos)
    }
    onDoubleClicked: {
        cardText.selectWord();
        window.getSelection();

    }
    onTextUpdate: {
        if( copiedText === newText){
            return
        }

        textUpdateReceived = true
        cardText.text = newText
        copiedText = cardText.text
        cardText.cursorPosition = cardTextCursorPos
    }

    onClicked: {
        var pos = cardText.positionAt(x, y, TextInput.CursorOnCharacter)
        cardDisplay.clickedTextPosition(pos)

    }
    onOpenPopup: {
        var posRect = cardText.positionToRectangle(pos)
        popupItemChoice.text = popupText
        cardText.cursorPosition = pos
        console.log("pos: ", pos)
        popupItemChoice.spanX = posRect.x
        popupItemChoice.spanY = posRect.y + posRect.height
        popupItemChoice.positions = popupPosList
        popupItemChoice.open()
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

        font.pointSize: 20
        color:"#FFF"
        // style: TextAreaStyle {
        //     textColor: "#FFF"
        //     selectionColor: "steelblue"
        //     selectedTextColor: "#eee"
        //     backgroundColor: "#222"
        // }
        onTextChanged: {
            if (cardDisplay.textUpdateReceived){
                cardDisplay.textUpdateReceived = false
            }
            else {
                console.log("Text editited")
            }
        }
        PopupItemChoice {
            id: popupItemChoice
        }

    }
}
