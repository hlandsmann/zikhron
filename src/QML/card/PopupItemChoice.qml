import QtQuick 2.15
import QtQuick.Layouts 1.4
import QtQuick.Controls 2.15
import QtQuick.Window 2.2

Popup {
    id: popup
    property var positions: []
    property int spanX
    property int spanY
    property string text
    property bool openOnce: false
    topPadding:5
    bottomPadding:5
    leftPadding:10
    rightPadding:10
    height: textArea.contentHeight + bottomPadding + topPadding
    width:  buttons.width + textArea.width + leftPadding + rightPadding

    modal: false
    focus: false
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    onOpenedChanged: {
        if(opened) {
            if (textArea.paintedWidth < textArea.width) {
                textArea.width = textArea.paintedWidth
            }
            popup.width = buttons.width + textArea.width + popup.leftPadding + popup.rightPadding

            popup.x = Math.min(spanX, parent.width - popup.width)
            popup.y = Math.min(spanY, parent.height - popup.height)
            openOnce=true
        }
        else {
            textArea.width = 500
        }
    }

    Item{
        id: buttons
        x:0
        y:0
        width: 70

        Repeater { model: popup.positions.length
            RadioButton { height: 20
                y: textArea.positionToRectangle(popup.positions[index]).y + 3
                   + textArea.width - textArea.width // this evaluates to zero but forces update
                x: 0
                text : index
            }
        }
    }
    TextArea{
        id: textArea

        x: buttons.width
        y: 0
        width: 500

        // background: Rectangle {
        //     id:bg
        //     border.color:  "#21be2b"
        // }
        topPadding:0
        bottomPadding:0
        leftPadding:0
        rightPadding:0

        text: popupItemChoice.text
        wrapMode: TextArea.WordWrap
        textFormat: Text.RichText
        readOnly: true
        font.pointSize: 20
    }
}
