import QtQuick
import QtQuick.Controls

Item {
    id: root
    width: 700
    height: 300

    signal themeRequested2(var myTheme)
    signal fontSizeRequested2(var myFontSize)
    signal fontFamilyRequested2(var myFontFamily)

    property color backgroundPageColor
    property color backgroungColor
    property color glowColor
    property color boxColor
    property color headerColor
    property color normalButtonColor
    property color selectButtonColor
    property color hoverButtonColor
    property color fillIconColor
    property color iconColor

    property color titleTextColor
    property color informationTextColor
    property color selectTextColor

    property var theme
    property var fontFamily

    Rectangle{
        id: applicationPageId
        x: 0
        y: 0
        height: parent.height
        width: parent.width
        color: "#00ffffff"

        MyComboBox {
            id: themeList
            height: 35
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.rightMargin: 10
            anchors.topMargin: 10
            width: 200

            backgroundPageColor: root.backgroundPageColor
            backgroungColor: root.backgroungColor
            glowColor: root.glowColor
            boxColor: root.boxColor
            headerColor: root.headerColor
            normalButtonColor: root.normalButtonColor
            selectButtonColor: root.selectButtonColor
            hoverButtonColor: root.hoverButtonColor
            fillIconColor: root.fillIconColor
            iconColor: root.iconColor

            titleTextColor: root.titleTextColor
            informationTextColor: root.informationTextColor
            selectTextColor: root.selectTextColor

            font.family: root.fontFamily
            model: ListModel {
                ListElement { text: "Dark" }
                ListElement { text: "light" }
            }
            onActivated: {
                root.themeRequested2(themeList.currentText)
            }
        }

        MyComboBox {
            id: fontSizeList
            height: 35
            anchors.right: parent.right
            anchors.top: themeList.bottom
            anchors.rightMargin: 10
            anchors.topMargin: 10
            width: 200

            backgroundPageColor: root.backgroundPageColor
            backgroungColor: root.backgroungColor
            glowColor: root.glowColor
            boxColor: root.boxColor
            headerColor: root.headerColor
            normalButtonColor: root.normalButtonColor
            selectButtonColor: root.selectButtonColor
            hoverButtonColor: root.hoverButtonColor
            fillIconColor: root.fillIconColor
            iconColor: root.iconColor

            titleTextColor: root.titleTextColor
            informationTextColor: root.informationTextColor
            selectTextColor: root.selectTextColor

            font.family: root.fontFamily
            model: ListModel {
                ListElement { name: qsTr("Small") }
                ListElement { name: qsTr("Medium") }
                ListElement { name: qsTr("Large") }
            }
            onActivated: {
                root.fontSizeRequested2(fontSizeList.currentText)
            }
        }

        MyComboBox {
            id: fontFamilyList
            height: 35
            anchors.right: parent.right
            anchors.top: fontSizeList.bottom
            anchors.rightMargin: 10
            anchors.topMargin: 10
            width: 200

            backgroundPageColor: root.backgroundPageColor
            backgroungColor: root.backgroungColor
            glowColor: root.glowColor
            boxColor: root.boxColor
            headerColor: root.headerColor
            normalButtonColor: root.normalButtonColor
            selectButtonColor: root.selectButtonColor
            hoverButtonColor: root.hoverButtonColor
            fillIconColor: root.fillIconColor
            iconColor: root.iconColor

            titleTextColor: root.titleTextColor
            informationTextColor: root.informationTextColor
            selectTextColor: root.selectTextColor

            font.family: root.fontFamily
            model: ListModel {
                ListElement { text: "Times New Roman" }
                ListElement { text: "Arial" }
                ListElement { text: "Courier" }
                ListElement { text: "DM Sans 9pt" }
                ListElement { text: "Tahoma" }
                ListElement { text: "Verdana" }
                ListElement { text: "./fontFamily/DMSans-VariableFont.ttf" }
            }
            onActivated: {
                root.fontFamilyRequested2(fontFamilyList.currentText)
            }
        }

        MyComboBox {
            id: deviceList
            height: 35
            anchors.right: parent.right
            anchors.top: fontFamilyList.bottom
            anchors.rightMargin: 10
            anchors.topMargin: 10
            width: 200

            backgroundPageColor: root.backgroundPageColor
            backgroungColor: root.backgroungColor
            glowColor: root.glowColor
            boxColor: root.boxColor
            headerColor: root.headerColor
            normalButtonColor: root.normalButtonColor
            selectButtonColor: root.selectButtonColor
            hoverButtonColor: root.hoverButtonColor
            fillIconColor: root.fillIconColor
            iconColor: root.iconColor

            titleTextColor: root.titleTextColor
            informationTextColor: root.informationTextColor
            selectTextColor: root.selectTextColor

            font.family: root.fontFamily
            model: ListModel {
                ListElement { text: "cpu" }
                 ListElement { text: "gpu" }
            }
            // Accessible.name: fontLabel.text
            // Accessible.description: fontLabel.helpText
        }

        Rectangle {
            id: cpuThreadsList
            height: 35
            color: "#00ffffff"
            anchors.right: parent.right
            anchors.top: deviceList.bottom
            anchors.rightMargin: 10
            anchors.topMargin: 10
            width: 200
            MySlider{
                id:sliderId
                anchors.left: parent.left
                anchors.right: valueSliderBoxId.left
                anchors.leftMargin: 0
                anchors.rightMargin: 0
                value: 8
                anchors.verticalCenter: parent.verticalCenter
                from: 1
                to: 10
                stepSize: 1

            }
            Rectangle{
                id:valueSliderBoxId
                width: 40
                height: 30
                radius: 2
                border.color: "#e0dede"
                anchors.verticalCenter: sliderId.verticalCenter
                anchors.right: parent.right
                anchors.rightMargin: 2
                Text {
                    id: valueSliderId
                    text: sliderId.value
                    anchors.fill: parent
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font.pointSize: 10
                    font.family: root.fontFamily
                }
            }
            Text {
                id: minValueSliderId
                text: sliderId.from
                anchors.left: parent.left
                anchors.top: sliderId.bottom
                anchors.leftMargin: 5
                anchors.topMargin: 2
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                font.pointSize: 10
                font.family: root.fontFamily
            }
            Text {
                id: maxValueSliderId
                text: sliderId.to
                anchors.right: valueSliderBoxId.left
                anchors.top: sliderId.bottom
                anchors.rightMargin: 5
                anchors.topMargin: 2
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                font.pointSize: 10
                font.family: root.fontFamily
            }


        }

        Rectangle{
            id: themeBox
            height: 35
            color: "#00ffffff"
            anchors.verticalCenter: themeList.verticalCenter
            anchors.left: parent.left
            anchors.leftMargin: 10
            width: 200

            Text {
                id: themeText
                text: qsTr("Theme")
                color: root.titleTextColor
                anchors.top: parent.top
                anchors.topMargin: 0
                font.pixelSize: 12
                font.bold: true
                font.family: root.fontFamily
            }
            Text {
                id: themeInformationText
                text: qsTr("The application color scheme")
                color: root.informationTextColor
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 7
                font.pixelSize: 12
                font.family: root.fontFamily
            }
        }

        Rectangle{
            id: fontSizeBox
            color: "#00ffffff"
            anchors.verticalCenter: fontSizeList.verticalCenter
            anchors.left: parent.left
            anchors.leftMargin: 10
            height: 35
            width: 200
            Text {
                id: fontSizeText
                text: qsTr("Font Size")
                color: root.titleTextColor
                anchors.top: parent.top
                anchors.topMargin: 0
                font.pixelSize: 12
                font.bold: true
                font.family: root.fontFamily
            }
            Text {
                id: fontSizeInformationText
                text: qsTr("The size of text in the application")
                color: root.informationTextColor
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 7
                font.pixelSize: 12
                font.family: root.fontFamily
            }
        }
        Rectangle{
            id: fontFamilyBox
            color: "#00ffffff"
            anchors.verticalCenter: fontFamilyList.verticalCenter
            anchors.left: parent.left
            anchors.leftMargin: 10
            height: 35
            width: 200
            Text {
                id: fontFamilyText
                text: qsTr("Font Family")
                color: root.titleTextColor
                anchors.top: parent.top
                anchors.topMargin: 0
                font.pixelSize: 12
                font.bold: true
                font.family: root.fontFamily
            }
            Text {
                id: fontFamilyInformationText
                text: qsTr("The fontFamily of text in the application")
                color: root.informationTextColor
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 7
                font.pixelSize: 12
                font.family: root.fontFamily
            }
        }
        Rectangle{
            id: deviceBox
            color: "#00ffffff"
            anchors.verticalCenter: deviceList.verticalCenter
            anchors.left: parent.left
            anchors.leftMargin: 10
            height: 35
            width: 200
            Text {
                id: deviceText
                text: qsTr("Device")
                color: root.titleTextColor
                anchors.top: parent.top
                anchors.topMargin: 0
                font.pixelSize: 12
                font.bold: true
                 font.family: root.fontFamily
            }
            Text {
                id: deviceInformationText
                text: qsTr("The computer device used for text generation")
                color: root.informationTextColor
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 7
                font.pixelSize: 12
                font.family: root.fontFamily
            }
        }
        Rectangle{
            id: cpuThreadsBox
            color: "#00ffffff"
            anchors.verticalCenter: cpuThreadsList.verticalCenter
            anchors.left: parent.left
            anchors.leftMargin: 10
            height: 35
            width: 200
            Text {
                id: cpuThreadsText
                text: qsTr("CPU Threads")
                color: root.titleTextColor
                anchors.top: parent.top
                anchors.topMargin: 0
                font.pixelSize: 12
                font.bold: true
                font.family: root.fontFamily
            }
            Text {
                id: cpuThreadsInformationText
                text: qsTr("The number of CPU threads used for inference and embedding.")
                color: root.informationTextColor
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 7
                font.pixelSize: 12
                font.family: root.fontFamily
            }
        }
    }
}
