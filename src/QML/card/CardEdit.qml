import QtQuick 2.14
import QtQuick.Layouts 1.4
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4

import CardEdit 1.0

CardEdit {
    id: cardEdit
    objectName: "CardEdit"
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
            if (cardDisplay.textUpdateReceived){
                console.log("TextChanged")
                cardDisplay.textUpdateReceived = false
            }
            else {
                console.log("Text editited")
            }
        }
    }
}
