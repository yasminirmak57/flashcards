cmake_minimum_required(VERSION 3.16)
project(Flashcards VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

# App icon: CI creates icon/flashcards.icns (macOS) and icon/flashcards.ico (Windows) from icon/icon.png
set(APP_ICON "")
if(APPLE AND EXISTS ${CMAKE_SOURCE_DIR}/icon/flashcards.icns)
    set(APP_ICON ${CMAKE_SOURCE_DIR}/icon/flashcards.icns)
    set_source_files_properties(${APP_ICON} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
endif()
if(WIN32 AND EXISTS ${CMAKE_SOURCE_DIR}/icon/flashcards.ico)
    file(WRITE ${CMAKE_BINARY_DIR}/app.rc "IDI_ICON1 ICON \"${CMAKE_SOURCE_DIR}/icon/flashcards.ico\"\n")
    set(APP_ICON ${CMAKE_BINARY_DIR}/app.rc)
endif()

add_executable(flashcards
    ${APP_ICON}
    src/main.cpp
    src/card.h
    src/deck.h
    src/deck.cpp
    src/mainwindow.h
    src/mainwindow.cpp
    resources.qrc
)

target_link_libraries(flashcards PRIVATE Qt6::Widgets)

# Make it a proper app bundle on macOS and a GUI app (no console) on Windows
set_target_properties(flashcards PROPERTIES
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_ICON_FILE flashcards.icns
    MACOSX_BUNDLE_BUNDLE_NAME Flashcards
    WIN32_EXECUTABLE TRUE
)
