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

    function rowEaseRefresh() {
        buttonTable.names = []
        buttonTable.names = ["again", "hard", "good", "easy"]
    }

    onTextUpdate: {
        if( copiedText === newText){
            return
        }

        textUpdateReceived = true
        cardText.text = newText
        copiedText = cardText.text
    }

    onVocableUpdate: {
        if( copiedVocables === newVocables){
            return
        }
        cardDisplay.easeList = []
        cardDisplay.easeList = vocableEaseList
        copiedVocables = newVocables
        vocables.text = newVocables
        vocPositions = vocablePosList
        console.log("Setup ease: ", easeList)

        rowEaseRefresh()
    }

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

        SelectableTextArea {
            id: cardText
            function textPositionClicked(pos) {cardDisplay.clickedTextPosition(pos)}
            function textPositionHovered(pos) {cardDisplay.hoveredTextPosition(pos)}
            Layout.alignment: Qt.AlignTop
            Layout.preferredWidth: parent.width
            Layout.preferredHeight: cardText.paintedHeight + 29
        }

        TextArea {
            id: vocables
            visible: false
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
            id: buttonTable
            property var names: []

            visible: vocables.visible
            y: vocables.y
            x: vocables.paintedWidth + vocables.leftPadding + vocables.rightPadding
            Repeater { model: cardDisplay.vocPositions.length
                RowLayout{
                    property var rect: vocables.positionToRectangle(cardDisplay.vocPositions[index])
                    property int indexVocable : index
                    id: rowEase
                    Layout.alignment: Qt.AlignHCenter
                    visible: vocables.visible
                    y: rect.y
                    x: 0
                    height: rect.height

                    Repeater {
                        model: buttonTable.names.length
                        id: line
                        Button{
                            Layout.preferredHeight : rect.height
                            text: buttonTable.names[index]
                            checkable: true
                            checked: cardDisplay.easeList[indexVocable] == index
                            onClicked:{
                                        for(var i=0; i<buttonTable.names.length; i++){
                                            if (i != index ) { line.itemAt(i).checked = false }
                                        }
                                        checked = true
                                        cardDisplay.easeList[indexVocable] = index
                                        console.log("Ease : ", cardDisplay.easeList)
                            }
                            onDoubleClicked: { checked = true }
                            onPressed: { checked = true}
                            onReleased: { checked = true}
                        }
                    }
                }
            }
        } // Item
    } // columnlayout

}
