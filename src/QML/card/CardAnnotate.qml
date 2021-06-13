import QtQuick 2.14
import QtQuick.Layouts 1.11
import QtQuick.Window 2.2
import QtQuick.Controls 2.15
import Qt.labs.settings 1.0

import CardAnnotate 1.0
CardAnnotate {
    id: cardAnnotate
    objectName: "CardAnnotate"


    property string copiedText: ""
    SelectableTextArea {
        id: cardText
    }

    onTextUpdate: {
        if( copiedText === newText){
            return
        }

        cardText.text = newText
        copiedText = cardText.text
        console.log("Annotate - text - update")
    }
}
