import QtQuick 2.14
import QtQuick.Layouts 1.4
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import Qt.labs.settings 1.0

ColumnLayout {
    CardDisplay{
        id: cardDisplay
        Layout.alignment: Qt.AlignTop
        Layout.fillWidth: true
        Layout.fillHeight: true

        visible: displayButton.checked
    }
    CardAnnotate{
        id: cardAnnotate
        Layout.alignment: Qt.AlignTop
        Layout.fillWidth: true
        Layout.fillHeight: true

        visible: annotateButton.checked
    }
    CardEdit{
        id: cardEdit
        Layout.alignment: Qt.AlignTop
        Layout.fillWidth: true
        Layout.fillHeight: true

        visible: editButton.checked
    }

    RowLayout{
        Layout.alignment: Qt.AlignBottom
        width: parent.width
        Layout.fillWidth: true

        Item{
            height: cardDisplayOption.implicitHeight
            width:  cardDisplayOption.implicitWidth
            RowLayout{
                id: cardDisplayOption
                Layout.alignment: Qt.AlignLeft

                Button{
                    checked:true
                    checkable: true
                    id: displayButton
                    text: "Display"
                    onClicked:{
                        checked = true
                        annotateButton.checked = false
                        editButton.checked = false
                    }
                }
                Button{
                    checked:false
                    checkable: true
                    id: annotateButton
                    text: "Annotate"
                    onClicked:{
                        checked = true
                        displayButton.checked = false
                        editButton.checked = false
                    }
                }
                Button{
                    checked:false
                    checkable: true
                    id: editButton
                    text: "Edit"
                    onClicked:{
                        checked = true
                        displayButton.checked = false
                        annotateButton.checked = false
                    }
                }
            }
        }

        Item{
            id: answerOption
            visible: displayButton.checked

            x: Math.max(cardDisplayOption.width + 5,
                        (cardDisplay.width / 2)  -
                                                ((showAnswer.visible ? showAnswer.width
                                                                     : difficulty.width)
                                                 / 2)
                       )

            function toggleVisibility() {
                if(showAnswer.visible) {
                    showAnswer.visible = false
                    difficulty.visible = true
                }
                else {
                    showAnswer.visible = true
                    difficulty.visible = false
                }
            }
            RowLayout{
                id: showAnswer
                Layout.alignment: Qt.AlignHCenter
                visible: true
                Button{
                    id: answerButton
                    text: "Show Answer"
                    onClicked:{ answerOption.toggleVisibility() }
                }
            }
            RowLayout{
                id: difficulty
                Layout.alignment: Qt.AlignHCenter
                visible: false

                Button{
                    id: againButton
                    text: "again"
                    onClicked:{ answerOption.toggleVisibility() }
                }
                Button{
                    id: hardButton
                    text: "hard"
                    onClicked:{ answerOption.toggleVisibility() }
                }
                Button{
                    id: goodButton
                    text: "good"
                    onClicked:{ answerOption.toggleVisibility() }
                }
                Button{
                    id: easyButton
                    text: "easy"
                    onClicked:{ answerOption.toggleVisibility() }
                }
            }
        }
    }

    Settings {
        category: "Card"
        id: settingsCard

        property int cardFontSize: 20
        property string cardFontColor: "#FFF"
    }

}
