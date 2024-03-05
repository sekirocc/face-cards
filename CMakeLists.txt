cmake_minimum_required(VERSION 3.16)

project(face-cards VERSION 0.1 LANGUAGES CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(PROJECT_NAME face-cards)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# set(CMAKE_AUTOUIC ON)
# set(CMAKE_AUTOMOC ON)
# set(CMAKE_AUTORCC ON)

##
## QT6
##


## set(CMAKE_PREFIX_PATH "/opt/Qt/6.5.3/gcc_64" ${CMAKE_PREFIX_PATH})
message("CMAKE_PREFIX_PATH: ${CMAKE_PREFIX_PATH}")

find_package(Qt6 6.4 REQUIRED COMPONENTS REQUIRED Quick OPTIONAL_COMPONENTS Multimedia)

qt_standard_project_setup()



##
## Find FFMPEG include directories and link libraries.
##
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/Modules ${CMAKE_MODULE_PATH})
find_package(FFMPEG REQUIRED)
message("FFMPEG_LIBRARIES ${FFMPEG_LIBRARIES}")
message("FFMPEG_INCLUDE_DIRS ${FFMPEG_INCLUDE_DIRS}")



##
## Find DondeToolkits
##
set(DONDE_TOOLKITS_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../cpp/donde-toolkits/install/)




##
## OpenVINO library
##

find_package(OpenVINO REQUIRED COMPONENTS Runtime)
message("OpenVINO_FOUND: ${OpenVINO_FOUND}")
message("OpenVINO_Runtime_FOUND: ${OpenVINO_Runtime_FOUND}")

##
## OpenCV
##

find_package(OpenCV REQUIRED CONFIG)
message("OpenCV_LIBS: ${OpenCV_LIBS}")




file(GLOB VIDEO_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/video/*.cpp)
file(GLOB VIDEO_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/video/*.h)


set(PROJECT_SOURCES
    main.cpp
    # mainwindow.cpp
    # mainwindow.h
    # mainwindow.ui
    # 3rdparty/qt-collapsible-section/Section.h
    # 3rdparty/qt-collapsible-section/Section.cpp
    # 3rdparty/flowlayout/flowlayout.h
    # 3rdparty/flowlayout/flowlayout.cpp
    ${VIDEO_SOURCES}
    ${VIDEO_HEADERS}
)

qt_add_resources(PROJECT_SOURCES
    assets.qrc
)

qt_add_executable(face-cards
    ${PROJECT_SOURCES}
)


qt_add_qml_module(face-cards
    URI face-cards
    VERSION 1.0
    QML_FILES
        "main.qml"
        "ExpandableMenus.qml"
        "Style.qml"
)

target_include_directories(face-cards PRIVATE
    video
    3rdparty/qt-collapsible-section
    ${DONDE_TOOLKITS_PATH}/include
    ${FFMPEG_INCLUDE_DIRS}
)

target_link_libraries(face-cards PRIVATE
    Qt6::Quick
    Qt6::Multimedia
    ${FFMPEG_LIBRARIES}
    ${OpenCV_LIBS}
    openvino::runtime
    ${DONDE_TOOLKITS_PATH}/libfeature_extract.a
    ${DONDE_TOOLKITS_PATH}/libfeature_search.a
    ${DONDE_TOOLKITS_PATH}/libvideo_process.a
)


add_subdirectory(3rdparty/cpp-channel)

target_include_directories(face-cards PRIVATE
    ${CMAKE_SOURCE_DIR}/3rdparty/cpp-channel/include
)


##
## Conan
##
find_package(fmt REQUIRED)
find_package(spdlog REQUIRED)
find_package(Poco REQUIRED)
find_package(SQLiteCpp REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(libyuv REQUIRED)
target_link_libraries(face-cards
    PRIVATE
    fmt::fmt
    spdlog::spdlog
    Poco::Poco
    SQLiteCpp
    libyuv::libyuv
    nlohmann_json::nlohmann_json
)

target_include_directories(face-cards
    BEFORE
    PRIVATE
    ${CMAKE_INCLUDE_PATH}
)



set_target_properties(face-cards PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

install(TARGETS face-cards
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(face-cards)
endif()
