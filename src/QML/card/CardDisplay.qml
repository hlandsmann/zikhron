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
    function displayVocables(show){
        vocables.visible = show
    }

    function selectEase(ease){
        cardDisplay.clickedEase(ease)
    }

    // onHovered: {
    //     var pos = cardText.positionAt(x, y, TextInput.CursorOnCharacter)
    //     // var pos = cardText.positionAt(x, y, TextInput.CursorBetweenCharacters)
    //     cardDisplay.hoveredTextPosition(pos)
    // }

    onTextUpdate: {
        if( copiedText === newText){
            return
        }

        textUpdateReceived = true
        cardText.text = newText
        copiedText = cardText.text
        cardText.cursorPosition = cardTextCursorPos
    }

    // onClicked: {
    //     var pos = cardText.positionAt(x, y, TextInput.CursorOnCharacter)
    //     cardDisplay.clickedTextPosition(pos)

    // }
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

    ColumnLayout{

        Layout.alignment: Qt.AlignTop
        width: parent.width
        height: parent.height
    // Rectangle {
    //     color: "green"
    //     // anchors.fill: parent
    //     Layout.alignment: Qt.AlignTop
    //     Layout.preferredWidth: parent.width
    //     Layout.preferredHeight: parent.height/2
    //     // width: parent.width
    //     anchors.margins: 20
    TextArea
    {
        id: cardText
        wrapMode: TextArea.WordWrap
        // Layout.alignment: Qt.AlignTop
        // width: parent.width
        //height: parent.height
        Layout.alignment: Qt.AlignTop
        Layout.preferredWidth: parent.width
        Layout.preferredHeight: parent.height/2

        readOnly: true

        textFormat: Text.RichText
        text: ""

        font.pointSize: settingsCard.cardFontSize
        color:          settingsCard.cardFontColor
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
            onClicked: {  var pos = getTextPosition(mouse.x, mouse.y)
                          cardDisplay.clickedTextPosition(pos) }
            onPositionChanged: {
                var pos = getTextPosition(mouseX, mouseY)
                // console.log("Posc:", cardText.positionAt(mouseX, mouseY), " : ", pos)
                cardDisplay.hoveredTextPosition(pos)
            }

        }
    }
    // }//rectangle
    // Rectangle {
    //     color: "blue"
    //     // anchors.fill: parent
    //     Layout.alignment: Qt.AlignBottom
    //     Layout.preferredWidth: parent.width
    //     Layout.preferredHeight: parent.height/2
    //             //  width: parent.width
//    anchors.margins: 20
    TextArea {
        id: vocables
        visible: false
        // Layout.alignment: Qt.AlignBottom
        // width: parent.width
        //height: parent.height
        Layout.alignment: Qt.AlignBottom
        Layout.preferredWidth: parent.width
        Layout.preferredHeight: parent.height/2
        readOnly: true

        textFormat: Text.RichText
        text: "Hello World"

        font.pointSize: 20
        color:          settingsCard.cardFontColor
    }
    // } // rectangle
    } // columnlayout

}
