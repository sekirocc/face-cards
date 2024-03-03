import QtQuick 2.15

Item {
    id: root

    required property int menuWidth

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
            color: "gray"

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
                    color = "blue";
                } else {
                    color = "gray";
                }
                for (var i = 0; i < menuModel.count; i ++) {
                    var m = menuModel.get(i);
                    if (section === m.type) {
                        m.canShow = sectionHeaderRect.isExpanded;
                    }
                }
            }

            Text {
                id: sectionHeaderText
                text: section
                anchors.centerIn: parent
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
