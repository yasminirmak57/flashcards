cmake_minimum_required(VERSION 3.16)
project(Flashcards VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(flashcards
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
    WIN32_EXECUTABLE TRUE
)
