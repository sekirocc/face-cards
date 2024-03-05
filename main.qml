import QtQuick 2.15
import QtQuick.Layouts
import QtQuick.Controls 2.15

ApplicationWindow {
    id: mainWindow
    visible: true
    width: 800
    height: 600
    title: "Main Page"
    color: "lightgray"

    FontLoader {
        id: fontAwesomeLoader
        source: "qrc:///resources/fonts/fa-solid-900.ttf"
    }

    Rectangle {
        id: menuPane
        width: mainWindow.width * 0.25
        height: mainWindow.height
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        ExpandableMenus {
            anchors.fill: parent

            menuWidth: width
        }

        Text {
            id: menuPaneIcon
            anchors.bottom: parent.bottom
            anchors.horizontalCenter: parent
            padding: 10
            font {
                family: fontAwesomeLoader.font.family
                pixelSize: 40
            }
            text: "\uf0fc"
            color: "red"
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
    }

    // Body Area
    Rectangle {
        id: bodyArea
        width: mainWindow.width * 0.75 // Adjust width as needed
        height: mainWindow.height
        color: "white"
        anchors.right: parent.right
        anchors.left: menuPane.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom

        // Content for body area
        // You can dynamically load different pages based on submenu clicks
        ColumnLayout {
            anchors.fill: parent
            ScrollView {
                padding: 10
                Layout.fillHeight: true
                Layout.fillWidth: true
                Rectangle {
                    color: "green"
                    anchors.fill: parent
                    ListView {
                        id: listView
                        anchors.fill: parent
                        model: ListModel {
                            id: listModel
                            ListElement { sender: "aaa"; message: "cccccccc" }
                            ListElement { sender: "bbb"; message: "cccccccc" }
                            ListElement { sender: "ccc"; message: "cccccccc" }
                            ListElement { sender: "ddd"; message: "cccccccc" }
                            ListElement { sender: "eee"; message: "cccccccc" }
                            ListElement { sender: "fff"; message: "cccccccc" }
                        }
                        delegate: Row {
                            Text {
                                text: model.sender + ": " + model.message
                            }
                        }
                    }
                }
            }
            RowLayout {
                Layout.fillWidth: true
                TextInput {
                    Layout.fillWidth: true
                    id: inputText
                    text: "input text"
                }
                Button {
                    id: sendBtn
                    text: "Send"
                    onClicked: inputText.text = ""
                }
            }
        }
    }
}
