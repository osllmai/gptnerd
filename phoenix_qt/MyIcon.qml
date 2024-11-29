import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Templates 2.1 as T
import QtQuick.Layouts
import Qt5Compat.GraphicalEffects

T.Button {
    id: control
    width: 30
    height: 30

    leftPadding: 4
    rightPadding: 4

    text: control.state
    property alias myLable: lableFoolTipId.text
    property var myIconId
    property var myFillIconId
    property bool havePupup: true

    property int heightSource: 16
    property int widthSource: 16

    property color normalColor
    property color hoverColor

    property int xPopup: 30
    property int yPopup: 23

    autoExclusive: false
    checkable: true

    // Define a custom tooltip using a Popup
    Popup {
        id: customToolTipId
        x: control.xPopup
        y: control.yPopup
        width: lableFoolTipId.width + 20
        height: 30
        visible: control.hovered && control.havePupup // Show when the button is hovered
        background:Rectangle {
            color: "#ffffff"
            radius: 4
            border.color: "#cbcbcb"
            anchors.left: parent.left
            Label {
                id: lableFoolTipId
                text: "Customized tooltip background"
                anchors.centerIn: parent
                color: "black"
            }
            layer.enabled: true
            layer.effect: Glow {
                 samples: 30
                 color: "#cbcbcb"
                 spread: 0.3
                 transparentBorder: true
             }
        }
    }

    background: Rectangle{
        id: backgroundId
        color: "#00ffffff"
        anchors.fill: parent
        radius: 0
        Rectangle{
            id: column
            color: "#00ffffff"
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.horizontalCenter: parent.horizontalCenter
            Image {
                id: iconId
                visible: true
                height: control.heightSource
                width: control.widthSource
                source: !control.pressed &&!control.hovered?control.myIconId: control.myFillIconId
                sourceSize.height: control.heightSource
                sourceSize.width: control.widthSource
                anchors.centerIn: parent
                fillMode: Image.PreserveAspectFit
                mipmap: true
            }
            ColorOverlay {
                id: colorOverlayIconId
                anchors.fill: iconId
                source: iconId
                color: control.hovered? hoverColor:normalColor
            }
        }
    }

}
