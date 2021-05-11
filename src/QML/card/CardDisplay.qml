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
    property string copiedVocables: ""
    property bool textUpdateReceived: false
    property int cardTextCursorPos: 0
    property var vocPositions: []
    property var easeList: []

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

    onVocableUpdate: {
        if( copiedVocables === newVocables){
            return
        }
        vocables.text = newVocables
        copiedVocables = newVocables
        vocPositions = vocablePosList
        easeList = vocableEaseList
        console.log("Setup ease: ", easeList)
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
            Layout.preferredHeight: cardText.paintedHeight //parent.height/2

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
            topPadding: 40
            Layout.alignment: Qt.AlignBottom
            Layout.preferredWidth: parent.width / 2
            Layout.preferredHeight: parent.height - cardText.paintedHeight
            readOnly: true

            textFormat: Text.RichText
            text: "Hello World"
            wrapMode: TextArea.WordWrap

            font.pointSize: 20
            color:          settingsCard.cardFontColor

        }
        Item {
            id: test
            visible: vocables.visible
            y: vocables.y// + vocables.topPadding
            x: vocables.paintedWidth + vocables.leftPadding + vocables.rightPadding
            Repeater { model: cardDisplay.vocPositions.length
                RowLayout{
                    property var rect: vocables.positionToRectangle(cardDisplay.vocPositions[index])
                    property var names: ["again", "hard", "good", "easy"]
                    property int indexVocable : index
                    id: ease
                    Layout.alignment: Qt.AlignHCenter
                    visible: vocables.visible
                    y: rect.y
                    x: 0
                    height: rect.height

                    Repeater {
                        model: ease.names.length
                        id: line
                        Button{
                            Layout.preferredHeight : rect.height
                            text: ease.names[index]
                            checkable: true
                            checked: cardDisplay.easeList[indexVocable] == index
                            onClicked:{
                                        for(var i=0; i<=3; i++)
                                            line.itemAt(i).checked = false
                                        checked = true
                                        cardDisplay.easeList[indexVocable] = index
                                        console.log("Ease : ", cardDisplay.easeList)
                            }
                        }
                    }
                }
            }
        }
    } // columnlayout

}
