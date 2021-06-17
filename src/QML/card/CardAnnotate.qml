import QtQuick 2.14
import QtQuick.Layouts 1.11
import QtQuick.Window 2.2
import QtQuick.Controls 2.15
import Qt.labs.settings 1.0
import QtQml.Models 2.2

import CardAnnotate 1.0
CardAnnotate {
    id: cardAnnotate
    objectName: "CardAnnotate"


    property string copiedText: ""
    SelectableTextArea {
        id: cardText
        function textPositionClicked(pos) {cardAnnotate.clickedTextPosition(pos)}
        function textPositionHovered(pos) {cardAnnotate.hoveredTextPosition(pos)}
        function xyClicked(x, y) { annotationChoice.x = x,
                                   annotationChoice.y = y,
                                   annotationChoice.visible = true
                                   annotationChoice.popup.open() }

        Layout.alignment: Qt.AlignTop
        Layout.preferredWidth: parent.width
        Layout.preferredHeight: parent.height
        width: parent.width
        height: parent.height
    }

    onTextUpdate: {
        if( copiedText === newText){
            return
        }

        cardText.text = newText
        copiedText = cardText.text
    }
    // ComboBox {
    //         // textFormat: Text.RichText
    // // font.pointSize: settingsCard.cardFontSize

    // model: ["上半场", "下半场", "上半场"]
    // }

    AnnotationChoice {
        id: annotationChoice
        editable: true
        visible: false

        model: ListModel {
            id: cbItems
            ListElement { text: "Banana" }
            ListElement { text: "Apple" }
            ListElement { text: "Coconut" }
        }
        onAccepted: {
            if (find(editText) === -1)
                model.append({text: editText})
            visible:false
            console.log("Hello world")
        }
        onCurrentIndexChanged: { console.debug(cbItems.get(currentIndex).text)
                                 console.log("index: ", currentIndex)
                                 annotationChoice.visible = false }
    }
}
