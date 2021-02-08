import QtQuick 2.15
import QtQuick.Layouts 1.4
import QtQuick.Controls 2.15
import QtQuick.Window 2.2
// import PopupItemChoice 1.0

Popup {
    id: popup
    property var positions: []
    property int spanX
    property int spanY
    property string text
    property bool openOnce: false

    height: popupTextArea.height + 30
    width:  buttons.width + popupTextArea.width + 30

    modal: false
    focus: false
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    onOpenedChanged: {
        console.log("popup is now open - ", opened)
        console.log(positions)
        if(opened) {
            if (popupTextArea.paintedWidth < popupTextArea.width) {
                popupTextArea.width = popupTextArea.paintedWidth + 30
            }
            // popupTextArea.width = popup.width
            // popup.width  = popupTextArea.paintedWidth +  buttons.width
            popup.x = Math.min(spanX, parent.width - popup.width)
            popup.y = Math.min(spanY, parent.height - popup.height)
            openOnce=true
        }
        else {
        // popup.width = 500
        popupTextArea.width = 600
        }
    }
    TextArea{
        id: popupTextArea
        Layout.alignment: Qt.AlignRight | Qt.AlignTop
        x: 30
        text: popupItemChoice.text
        wrapMode: TextArea.WordWrap

        width: 600
        // implicitWidth
        // paintedWidth ? paintedWidth : 500

        textFormat: Text.RichText
        readOnly: true
        font.pointSize: 20
        // MouseArea {
        //     anchors.fill: popupTextArea
        //     onClicked: {
        //         var pos = popupTextArea.positionAt(mouseX, mouseY, TextInput.CursorOnCharacter)
        //         console.log("popuppos: ", pos, " x: ", mouseX, " y: " , mouseY)
        //      }

        // }
        states: [
            State {
                name: "wide text"
                when: popupTextArea.paintedWidth > 500 && openOnce
                PropertyChanges {
                    target: popup
                    openOnce: false
                }
                PropertyChanges {
                    target: popupTextArea
                    width: 500
                    // script: console.log("entering first state")
                    // height: text_field.paintedHeight
                }

                StateChangeScript {
                    name: "firstScript"
                    script: console.log("entering first state---------------------", popupTextArea.width)
                }
                // PropertyChanges {
                //     target: buttons
                //     buttons.model:  popup.positions.length
                //    // height: text_field.paintedHeight
                // }
            }
            // ,
            // State {
            //     name: "not wide text"
            //     when: containing_rect.text.length <= 20
            //     PropertyChanges {
            //         target: containing_rect
            //         width: dummy_text.paintedWidth
            //         height: text_field.paintedHeight
            //     }
            // }
        ]
    }
    Item{
        id: buttons
        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
        width: 30
        // height: 100
        // height: popupTextArea.height
        Repeater { model: popup.positions.length
            RadioButton { width: 20; height: 20
                y: popupTextArea.positionToRectangle(popup.positions[index]).y + 3
                   + popupTextArea.width - popupTextArea.width // this evaluates to zero but forces update
                x: 0
            }
        }
    }
}
