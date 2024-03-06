import QtQuick 2.15
import QtQuick.Layouts

Item {
    id: root

    required property int menuWidth
    required property int menuSeperatorWidth
    required property string fontAwesomeFamily


    ListModel {
        id: menuModel
        ListElement {name: "创建任务"; type: "任务管理"; canShow: false}
        ListElement {name: "任务列表"; type: "任务管理"; canShow: false}
        ListElement {name: "图片列表"; type: "媒体库"; canShow: false}
        ListElement {name: "视频列表"; type: "媒体库"; canShow: false}
        ListElement {name: "用户列表"; type: "权限管理"; canShow: false}
        ListElement {name: "组列表"; type: "权限管理"; canShow: false}
    }

    ListView {
        id: menuListView
        height: parent.height
        width: root.menuWidth
        property string  expandedSection: ""

        model: menuModel
        delegate: menuItemDelegate

        section.property: "type"
        section.criteria: ViewSection.FullString
        section.delegate: sectionHeader
    }



    Component {
        id: sectionHeader
        Rectangle {
            id: sectionHeaderRect
            width: menuListView.width
            height: 50
            color: "white"

            property bool isExpanded
            property string currentExpandedSection: ListView.view.expandedSection

            onCurrentExpandedSectionChanged: {
                if (currentExpandedSection === section) {
                    isExpanded = true;
                } else {
                    isExpanded = false;
                }
                print("currentExpandedSection: " + currentExpandedSection)
            }
            onIsExpandedChanged: {
                if (isExpanded) {
                    bottomBorder.height = 2;
                    expandIndicator.text = "\u25BC"
                } else {
                    bottomBorder.height = 0;
                    expandIndicator.text = "\u25B6"
                }
                for (var i = 0; i < menuModel.count; i ++) {
                    var m = menuModel.get(i);
                    if (section === m.type) {
                        m.canShow = sectionHeaderRect.isExpanded;
                    }
                }
            }


            RowLayout {
                anchors.fill: parent
                Text {
                    id: sectionHeaderText
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    color: "red"
                    text: section
                }
                Text {
                    id: expandIndicator
                    Layout.fillHeight: true
                    verticalAlignment: Text.AlignVCenter
                    width: 40
                    color: "blue"
                    text: "\u25B6"
                    font {
                        family: root.fontAwesomeFamily
                        pixelSize: 20
                    }
                }
            }


            Rectangle {
                id: bottomBorder
                color: "black"
                height: root.menuSeperatorWidth
                width: sectionHeaderRect.width
                anchors.bottom: sectionHeaderRect.bottom

            }

            Rectangle {
                id: topBorder
                color: "black"
                height: root.menuSeperatorWidth
                width: sectionHeaderRect.width
                anchors.top: sectionHeaderRect.top
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    sectionHeaderRect.isExpanded = !sectionHeaderRect.isExpanded;
                }
            }
        }
    }

    Component {
        id: menuItemDelegate
        Rectangle {
            width: menuListView.width
            visible: canShow
            color: ListView.isCurrentItem ? "lightblue": "white"

            onVisibleChanged: {
                if (visible)
                    height = 50;
                else
                    height = 0;
            }

            Behavior on height {
                NumberAnimation {duration: 200}
            }

            Text {
                text: name
                anchors.centerIn: parent
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    menuListView.currentIndex = index;
                }
            }

        }
    }

}
