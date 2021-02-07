import QtQuick 2.14
import QtQuick.Layouts 1.4
import QtQuick.Controls 2.15
import QtQuick.Window 2.2
// import PopupItemChoice 1.0

Popup {
    id: popup
    property var positions: [0, 20, 60, 80]
    property int spanX
    property int spanY
    property string text
    height: popupTextArea.height + 30
    modal: false
    focus: false
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    width: 500
    onOpenedChanged: {
        console.log("popup is now open - ", opened)
        console.log(positions)
        if(opened) {
            popupTextArea.width = popup.width
            popup.width  = popupTextArea.paintedWidth + 30 + buttons.width
            popup.x = Math.min(spanX, parent.width - popup.width)
            popup.y = Math.min(spanY, parent.height - popup.height)
        }
        else {
        popup.width = 500
        popupTextArea.width = 500
        }
    }
    TextArea{
        id: popupTextArea
        Layout.alignment: Qt.AlignRight | Qt.AlignTop
        x: 30
        text: popupItemChoice.text
        wrapMode: TextArea.WordWrap

        width: paintedWidth

        textFormat: Text.RichText
        readOnly: true
        font.pointSize: 20
        MouseArea {
            anchors.fill: popupTextArea
            onClicked: {
                var pos = popupTextArea.positionAt(mouseX, mouseY, TextInput.CursorOnCharacter)
                console.log("popuppos: ", pos, " x: ", mouseX, " y: " , mouseY)
        }

    }

        Repeater { model: popup.positions.length
            Rectangle {
                width:   5
                height:  popupTextArea.positionToRectangle(popup.positions[index]).height
                y: popupTextArea.positionToRectangle(popup.positions[index]).y
                x: popupTextArea.positionToRectangle(popup.positions[index]).x
                color: "black"
                            //    font.pointSize: 30
            }
        }
    }
    Item{
        id: buttons
        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
        width: 30
        height: 100
        // height: popupTextArea.height
        Repeater { model: popup.positions.length
            RadioButton { width: 20; height: 20
                y: popupTextArea.positionToRectangle(popup.positions[index]).y

                            //text: index
                            x: 0
                            // y: posRect.y
                            //    font.pointSize: 30
            }
        }


    }



}
