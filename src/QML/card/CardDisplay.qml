import QtQuick 2.14
import QtQuick.Layouts 1.4
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4

import MyObserver 1.0

MyObserver {
    id: app
    objectName: "MyObserver"

    property string copiedText: ""
    property bool textUpdateReceived: false
    property int cardTextCursorPos: 0
    onHovered: {
        var pos = cardText.positionAt(x, y, TextInput.CursorOnCharacter)
        app.hoveredTextPosition(pos)
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
        cardTextCursorPos = cardText.positionAt(x, y)
        // app.hoveredTextPosition(pos)
        cardText.cursorPosition = cardTextCursorPos
        console.log("pos: ", pos)
    }

    TextArea
    {
        id: cardText
        Layout.alignment: Qt.AlignTop
        width: parent.width
        height: parent.height
        readOnly: false

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
            if (app.textUpdateReceived){
                console.log("TextChanged")
                app.textUpdateReceived = false
            }
            else {
                console.log("Text editited")
            }
        }
    }
}