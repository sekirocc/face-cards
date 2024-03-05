import QtQuick 2.15

Item {

    property alias fontAwesome: fontAwesomeLoader.font

    FontLoader {
        id: fontAwesomeLoader
        source: "qrc:/resources/fonts/fa-solid-900.ttf"
    }

}
