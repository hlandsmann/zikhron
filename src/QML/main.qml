import QtQuick 2.14
import QtQuick.Layouts 1.4
// import QtQuick.Controls 1.4
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4

import MyObserver 1.0

ApplicationWindow {
    id: root
    objectName: "window"

    visible: true
    width: 800
    height: 600


    color: "#222"
    MyObserver {
        id: app
        objectName: "MyObserver"

        anchors.fill: parent
        onHovered: {
            var pos = cardText.positionAt(x, y)
            app.hoveredTextPosition(pos)
            // console.log(pos)
            // cardText.select(pos, pos+1)
        }
        onDoubleClicked: {
            cardText.selectWord();
            window.getSelection();

        }
        onTextUpdate: {
            cardText.text = newText
            // console.log(newText)
        }

        ColumnLayout {
            anchors.fill: parent
            TextArea
            {
                id: cardText
                Layout.alignment: Qt.AlignTop
                Layout.fillWidth: true
                Layout.fillHeight: true
                readOnly: true

                textFormat: Text.RichText
                text:""
                    // "<div><table border='1'><caption><h4>Test stats</h4>"+
                    // "</caption><tr bgcolor='#9acd32'><th/><th>Number1</th><th>Number2</th></tr> <tr><th>Line1</th>"+
                    // "<td> 0 </td> <td> 1 </td> </tr> <tr><th>Line2</th> <td> 0 </td> <td> 1 </td> </tr>"+
                    // "<tr><th>Line3</th> <td> 0 </td> <td> 0 </td> </tr> <tr><th>Line4</th> <td> 1 </td> <td> 0 </td> </tr>"+
                    // "<tr><th>Line5</th> <td> 1 </td> <td> 1 </td> </tr> <tr><th>Line6</th> <td> 1 </td> <td> 1 </td> </tr> </div>"

                font.pointSize: 20

                style: TextAreaStyle {
                    textColor: "#FFF"
                    selectionColor: "steelblue"
                    selectedTextColor: "#eee"
                    backgroundColor: "#222"
                }
            }

            RowLayout {
                id: set1
                Layout.alignment: Qt.AlignCenter

                Button {
                    text: "Ok"
                    onClicked: { set1.visible = false
                                set2.visible = true
                                }
                }
                Button {
                    text: "Cancel"
                    onClicked: {
                        Qt.quit();
                    }
                }
            }
            RowLayout {
                id: set2
                visible: false
                Layout.alignment: Qt.AlignCenter

                Button {
                    text: "1"
                    onClicked: {}
                }
                Button {
                    text: "2"
                    onClicked: {  }
                }
                Button {
                    text: "3"
                    onClicked: { set2.visible = false
                                 set1.visible = true
                    }
                }
            }

        }
    }


}
