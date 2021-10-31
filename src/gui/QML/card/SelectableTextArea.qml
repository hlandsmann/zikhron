import QtQuick 2.14
import QtQuick.Layouts 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Window 2.2
import CardDisplay 1.0
import QtQuick.Controls 2.15

TextArea
{
    id: cardText
    wrapMode: TextArea.WordWrap

    // // Layout.alignment: Qt.AlignTop
    // width: parent.width
    // height: parent.height
    readOnly: true

    textFormat: Text.RichText
    text: ""
    font.pointSize: settingsCard.cardFontSize
    color:          settingsCard.cardFontColor

    background: Rectangle {
        color: settingsCard.cardBackgroundColor
    }

    function textPositionClicked(pos) {}
    function textPositionHovered(pos) {}
    function xyClicked(x, y) {}
    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        function indexInsideRect(x, y, index){
            var rect = cardText.positionToRectangle(index)
            var x2 = cardText.positionToRectangle(index+1).x
            if (rect.x <= x && (x2 >= x || x2<rect.x)
                && rect.y <= y && rect.y+rect.height >= y){
                    return true
                }
            return false
        }
        function getTextPosition(x, y){
            var pos = cardText.positionAt(x, y, TextInput.CursorOnCharacter)
            for (var i=pos; i<cardText.length; i++) {
                if (indexInsideRect(x, y, i)){
                    return i
                }
            }
            for (var i=pos-1; i>=0; i--) {
                if (indexInsideRect(x, y, i)){
                    return i
                }
            }
            return -1
        }
        onClicked: { var pos = getTextPosition(mouse.x, mouse.y)
                     var rect = cardText.positionToRectangle(pos)
                     textPositionClicked(pos)
                     xyClicked(rect.x, rect.y) }
        onPositionChanged: { textPositionHovered(getTextPosition(mouseX, mouseY)) }
    }
}
