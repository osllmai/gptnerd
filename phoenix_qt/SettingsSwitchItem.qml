import QtQuick 2.15
import QtQuick.Controls 2.15

Item {
    id: root
    width: 250
    height: 60

    Constants{
        id: constantsId
    }

    property alias myTextName: textId.text

    property var fontFamily: constantsId.fontFamily
    property color textColor: "black"

    Rectangle{
        id: settingsSliderBox
        anchors.fill: parent
        Text{
            id:textId
            text: "Temperature"
            color:root.textColor
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.leftMargin: 0
            anchors.topMargin: 0
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            font.pointSize: 10
            font.styleName: "Bold"
            font.family: root.fontFamily
        }

        MySwitch{
            id:switchId
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: parent.right
            anchors.rightMargin: 5
        }
    }
}
