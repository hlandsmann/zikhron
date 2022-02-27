import QtQuick 2.12
import QtQuick.Controls 2.12

ComboBox {
    id: control
    property var implicitX : 0
    property var startHighlighted : -1
    function menuClosed() {}
    x: (implicitX + width > parent.width ? parent.width - width : implicitX)
    model: []
    rightPadding: 0
    leftPadding: 0
    topPadding: 0
    bottomPadding: 0
    spacing: 0
    font.pointSize: settingsCard.cardFontSize
    delegate: ItemDelegate {
        width: control.width
        topPadding: 0
        bottomPadding: 0
        leftPadding: 0
        rightPadding: 0
        background: Rectangle {
            color: highlighted ? settingsWidgets.selectedColor : settingsWidgets.selectableColor
        }
        contentItem: Text {
            text: highlighted ? textMarked : textUnmarked
            font: control.font
            verticalAlignment: Text.AlignVCenter
            textFormat: Text.RichText
        }
        highlighted: startHighlighted > -1 ? control.startHighlighted === index
                                           : control.highlightedIndex === index
    }
    onHighlightedIndexChanged: { startHighlighted = -1 }

    indicator: Canvas {
        id: canvas
        x: control.width - width - control.rightPadding
        y: control.topPadding + (control.availableHeight - height) / 2
        width: 12
        height: 8
        contextType: "2d"

        Connections {
            target: control
            function onPressedChanged() { canvas.requestPaint(); }
        }

        onPaint: {
            context.reset();
            context.moveTo(0, 0);
            context.lineTo(width, 0);
            context.lineTo(width / 2, height);
            context.closePath();
            context.fillStyle = control.pressed ? settingsWidgets.selectedColor : "#BBB";
            context.fill();
        }
    }

    contentItem: Text {
        rightPadding: control.indicator.width + control.spacing
        topPadding: 0
        bottomPadding: 0
        text: control.displayText
        font: control.font
        verticalAlignment: Text.AlignVCenter
        textFormat: Text.RichText

    }

    background: Rectangle {
        border.width: 0
        radius: 0
        color: settingsWidgets.selectableColor
        border.color: settingsCard.cardBackgroundColor
    }

    popup: Popup {
        y: control.height
        width: control.width
        implicitHeight: contentItem.implicitHeight + 2
        padding: 1

        contentItem: ListView {
            clip: true
            implicitHeight: contentHeight
            model: control.popup.visible ? control.delegateModel : null
            currentIndex: control.highlightedIndex

            ScrollIndicator.vertical: ScrollIndicator { }
        }

        onOpenedChanged: {
            if (!opened)
                control.menuClosed()
        }


        background: Rectangle {
            border.width: 0
            color: settingsWidgets.selectableColor
            radius: 2
        }
    }
}
