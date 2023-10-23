cmake_minimum_required(VERSION 3.5)

project(human_card VERSION 0.1 LANGUAGES CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(PROJECT_NAME human_card)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)



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

##
## SDL
##
find_package(SDL2 REQUIRED CONFIG)
message("SDL2 libs: ${SDL2_LIBRARIES}")
message("SDL2 include dir: ${SDL2_INCLUDE_DIR}")


set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/imgui)
file(GLOB IMGUI_HEADERS ${IMGUI_DIR}/*.h)
file(GLOB IMGUI_SOURCES ${IMGUI_DIR}/*.cpp)
list(APPEND IMGUI_HEADERS
        ${IMGUI_DIR}/backends/imgui_impl_opengl3.h
        ${IMGUI_DIR}/backends/imgui_impl_sdl2.h
)
list(APPEND IMGUI_SOURCES
        ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
        ${IMGUI_DIR}/backends/imgui_impl_sdl2.cpp
)

file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/video/*.cpp)
file(GLOB HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/video/*.h)


set(PROJECT_SOURCES
        main.cpp
        ${SOURCES}
        ${HEADERS}

        ${IMGUI_HEADERS}
        ${IMGUI_SOURCES}

)


add_executable(human_card ${PROJECT_SOURCES})


target_include_directories(human_card PRIVATE
        ${CMAKE_SOURCE_DIR}/3rdparty/cppchannel
        ${CMAKE_SOURCE_DIR}/3rdparty/imgui
        ${CMAKE_SOURCE_DIR}/3rdparty/imgui/backends
        ${CMAKE_SOURCE_DIR}/video
        ${FFMPEG_INCLUDE_DIRS}
        ${SDL2_INCLUDE_DIRS}

)


target_link_libraries(human_card PRIVATE
        openvino::runtime
        donde::DondeToolkits

        ${FFMPEG_LIBRARIES}
        ${SDL2_LIBRARIES}
        ${OpenCV_LIBS}

	"-framework OpenGL"
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
