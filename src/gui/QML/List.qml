import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

ListView {
        id: root
        // anchors.fill: parent
            // width: parent.width
            implicitHeight: contentHeight
            implicitWidth: contentWidth

    // height: parent.height
        model: []
        // clip: true
        signal clicked(int row) // <---
        // property var columnWidths: [0.5, 0.5]   // as fractions of parent width
        //                                         // preferably overwrite this when using
        // columnWidthProvider: function (column) { return Math.max(parent.width * columnWidths[column], 1) }

        delegate: Rectangle {
            implicitHeight: text.implicitHeight
            implicitWidth: text.implicitWidth
            border.color: "#000000"
             color: "#dddddd"
            Text {
                id: text
                text: vocableEntry
                width: parent.width
                wrapMode: Text.Wrap
                padding: 5
            }
            MouseArea{
                anchors.fill: parent
                onClicked: root.clicked(model.row) // <---
            }
        }
    }
// ListView {
//     id: root
//     width: 180; height: 200

//     contentWidth: 320
//     flickableDirection: Flickable.AutoFlickDirection

//     model: ContactModel {}
//     delegate: Row {
//         Text { text: '<b>Name:</b> ' + name; width: 160 }
//         Text { text: '<b>Number:</b> ' + number; width: 160 }
//     }
// }
