import QtQuick 2.14
import QtQuick.Layouts 1.4
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Window 2.2
import CardDisplay 1.0
import QtQuick.Controls 2.15 as QQC2
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
        // console.log(pos)
        // cardText.select(pos, pos+1)
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
        popupTextArea.text=popupText
        // app.hoveredTextPosition(pos)
        cardText.cursorPosition = pos
        console.log("pos: ", pos)
        popup.x = Math.min(posRect.x, cardText.width - popup.width)
        popup.y = posRect.y + posRect.height
        popup.open()
    }


    TextArea
    {
        id: cardText
        Layout.alignment: Qt.AlignTop
        width: parent.width
        height: parent.height
        readOnly: true

        textFormat: Text.RichText
        text: ""
            // "<div><table border='1'><caption><h4>Test stats</h4>"+
            // "</caption><tr bgcolor='#9acd32'><th/><th>Number1</th><th>Number2</th></tr> <tr><th>Line1</th>"+
            // "<td> 0 </td> <td> 1 </td> </tr> <tr><th>Line2</th> <td> 0 </td> <td> 1 </td> </tr>"+
            // "<tr><th>Line3</th> <td> 0 </td> <td> 0 </td> </tr> <tr><th>Line4</th> <td> 1 </td> <td> 0 </td> </tr>"+
            // "<tr><th>Line5</th> <td> 1 </td> <td> 1 </td> </tr> <tr><th>Line6</th> <td> 1 </td> <td> 1 </td> </tr> </div>"

        font.pointSize: 20

        style: TextAreaStyle {
            textColor: "#FFF"
            selectionColor: "steelblue"
            selectedTextColor: "#eee"
            backgroundColor: "#222"
        }
        onTextChanged: {
            if (cardDisplay.textUpdateReceived){
                console.log("TextChanged")
                cardDisplay.textUpdateReceived = false
            }
            else {
                console.log("Text editited")
            }
        }
    QQC2.Popup {
        id: popup
        // x: Math.min(x, cardText.width - width)
        // x: 100
        //y: 100
        // width: 200
        // height: 300
        modal: false
        focus: false
        closePolicy: QQC2.Popup.CloseOnEscape | QQC2.Popup.CloseOnPressOutside

        QQC2.TextArea{
            id: popupTextArea
            width: parent.width
            height: parent.height
            readOnly: true
            font.pointSize: 20
            textFormat: Text.RichText

        }

    }
    }
}
