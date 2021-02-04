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
        popupTextArea.text = popupText
        cardText.cursorPosition = pos
        console.log("pos: ", pos)
        popup.spanX = posRect.x
        popup.spanY = posRect.y + posRect.height
        popup.open()
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
        Popup {
            id: popup
            property int spanX
            property int spanY

            height: popupTextArea.height + 30
            modal: false
            focus: false
            closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
            width: 500
            onOpenedChanged: {
                console.log("popup is now open - ", opened)
                if(opened) {
                    popupTextArea.width = popup.width
                    popup.width  = popupTextArea.paintedWidth + 30
                    popup.x = Math.min(spanX, cardText.width - popup.width)
                    popup.y = spanY
                }
                else {
                popup.width = 500
                popupTextArea.width = 500
                }
            }

            TextArea{
                id: popupTextArea
                wrapMode: TextArea.WordWrap
                Layout.alignment: Qt.AlignTopCenter
                width: paintedWidth

                textFormat: Text.RichText
                readOnly: true
                font.pointSize: 20
            }
        }
    }
}
