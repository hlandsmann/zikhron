import QtQuick 2.15
import QtQuick.Layouts 1.4
import QtQuick.Controls 2.15
import QtQuick.Window 2.2

Popup {
    id: popup
    property int spanX: 0
    property int spanY: 0
    property string defaultStr: "Dummy text to make initialization not create to short strings"
    property string dicEntryText: defaultStr
    x: Math.min(spanX, parent.width - popup.width)
    y: Math.min(spanY, parent.height - popup.height)
    topPadding:5
    bottomPadding:5
    leftPadding:10
    rightPadding:10

    modal: false
    focus: false
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    height: columnLayout.heigth + topPadding + bottomPadding
    width: columnLayout.width + leftPadding + rightPadding

    function openPopup(chosenDicEntry, dicEntries) {
        dicEntry.width = 512
            // dicEntry.widthReduced = false
        dicEntry.text = chosenDicEntry
        popup.open()
    }

    onOpenedChanged: {
        if(opened) {

        }
        else {
        dicEntry.width = 512
        dicEntryText = defaultStr
        list.visible = false
        expandBtn.expanded = false
        }
    }

    // ListModel{
    // id: dataModel
    // ListElement{ vocableEntry: "Day" }
    // ListElement{ vocableEntry: "Week" }
    // ListElement{ vocableEntry: "Month" }
    // ListElement{ vocableEntry: "Year" }
    // }
    property ListModel dataModel: ListModel {}

    ColumnLayout{
        id: columnLayout
        width: rowLayout.width
        height: rowLayout.height
        RowLayout{
            id: rowLayout
            visible: true
            Layout.alignment: Qt.AlignTop
            width: expandBtn.width + dicEntry.width + 20
            height: dicEntry.height
            Button {
                id: expandBtn
                property bool expanded: false
                Text {
                    id: expandBtnTxt
                    anchors {centerIn: parent }
                    text: expandBtn.expanded ? "v" : ">"
                }
                Layout.preferredWidth: expandBtnTxt.width + 40

                onClicked: {
                    if(dataModel.count > 0) {
                    expandBtn.expanded = !expandBtn.expanded
                    list.visible = true
                    rowLayout.visible = false}
                }
            }
            TextArea {
                id: dicEntry
                property bool widthReduced: false
                width: 512
                // Layout.preferredWidth: 512
                // platformMaxImplicitWidth: 512
                // width: Math.min(dicEntry.paintedWidth, 512);
                onWidthChanged: {
                    console.log("width changed")
                // popup.x = Math.min(spanX, popup.parent.width - popup.width)
                // popup.y = Math.min(spanY, popup.parent.height - popup.height)
                }


                text: popup.dicEntryText
                wrapMode: TextArea.WordWrap
                textFormat: Text.RichText
                readOnly: true
                font.pointSize: 20

            }
        }
        List {
            id: list
            width: popup.width
            height: 50
            // Layout.alignment: Qt.AlignBottom
            visible: false
            model: dataModel
            onClicked: {
                console.log(row)
                dataModel.clear()
            }
        }

    }

}
