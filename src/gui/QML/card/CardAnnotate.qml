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

    onAnnotationPossibilities: {
        annotationChoice.model.clear()
        for(var i=0; i<marked.length; i++) {
            annotationChoice.model.append({"textMarked": marked[i], "textUnmarked": unmarked[i]})
        }
        annotationChoice.displayText = unmarked[0]
        annotationChoice.startHighlighted = 0
        var rect = cardText.positionToRectangle(pos)
        annotationChoice.implicitX = rect.x
        annotationChoice.y = rect.y
        annotationChoice.visible = true
        annotationChoice.popup.open()
    }

    AnnotationChoice {
        id: annotationChoice
        editable: false
        visible: false
        model : ListModel {}
        function menuClosed() { visible = false }

        onAccepted: {
            if (find(editText) === -1)
                model.append({text: editText})
            visible:false
            console.log("Hello world")
        }
        onCurrentIndexChanged: { /* console.debug(cbItems.get(currentIndex).text) */
                                 console.log("index: ", currentIndex)
                                 if ( currentIndex != -1)
                                    cardAnnotate.chosenAnnotation(currentIndex)
                                 annotationChoice.visible = false }
    }
}
