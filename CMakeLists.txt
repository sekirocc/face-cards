cmake_minimum_required(VERSION 3.5)

project(human_card VERSION 0.1 LANGUAGES CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(PROJECT_NAME human_card)


set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_C_COMPILER "/usr/bin/clang")
set(CMAKE_CXX_COMPILER "/usr/bin/clang++")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

##
## QT6
##

set(CMAKE_PREFIX_PATH "/opt/Qt/6.5.3/gcc_64" ${CMAKE_PREFIX_PATH})
message("CMAKE_PREFIX_PATH: ${CMAKE_PREFIX_PATH}")
find_package(QT NAMES Qt6 Qt5
        REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR}
        REQUIRED COMPONENTS Widgets
        OPTIONAL_COMPONENTS Multimedia MultimediaWidgets)



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
set(CMAKE_PREFIX_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../cpp/donde-toolkits/install/cmake ${CMAKE_PREFIX_PATH})
message("CMAKE_MODULE_PATH: ${CMAKE_MODULE_PATH}")
find_package(DondeToolkits CONFIG REQUIRED)




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



set(VIDEO_SOURCES
        video/picture_factory.h
        video/video_picture.h
        video/media_controller.h
        video/media_controller.cpp
        3rdparty/cppchannel/channel.h
        3rdparty/cppchannel/channel.cpp
)


set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
        ${VIDEO_SOURCES}
)


if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(human_card
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET human_card APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(human_card SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(human_card
            ${PROJECT_SOURCES}
        )
    endif()
endif()




target_include_directories(human_card PRIVATE
        ${CMAKE_SOURCE_DIR}/3rdparty/cppchannel
        ${CMAKE_SOURCE_DIR}/video
        ${FFMPEG_INCLUDE_DIRS}
)


target_link_libraries(human_card PRIVATE
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Multimedia
        Qt${QT_VERSION_MAJOR}::MultimediaWidgets
        ${FFMPEG_LIBRARIES}
        ${OpenCV_LIBS}
        openvino::runtime
        donde::DondeToolkits
    )


##
## Conan
##
set(CMAKE_PREFIX_PATH "build" ${CMAKE_PREFIX_PATH})
find_package(fmt REQUIRED)
find_package(spdlog REQUIRED)
find_package(Poco REQUIRED)
find_package(SQLiteCpp REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(libyuv REQUIRED)
target_link_libraries(${PROJECT_NAME}
    PRIVATE
    fmt::fmt
    spdlog::spdlog
    Poco::Poco
    #### msgpack::msgpack
    SQLiteCpp
    libyuv::libyuv
    nlohmann_json::nlohmann_json
)
target_include_directories(${PROJECT_NAME}
    BEFORE
    PRIVATE
    ${CONAN_INCLUDE_DIRS}
)



set_target_properties(human_card PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

install(TARGETS human_card
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(human_card)
endif()
