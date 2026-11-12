# Flashcards

A simple desktop flashcard app written in C++17 with Qt 6, shipped with a deck of the ~1000 most common Italian words.

## Features
- Flip cards between question and answer
- Mark "Got it" or "Missed": missed cards come back later in the session
- Add new cards from the app
- Import any deck from a JSON file
- Cards and scores saved automatically as JSON

## Download
Windows and macOS builds are on the [Releases](../../releases) page.

## Build
Requirements: Qt 6, CMake 3.16+, a C++17 compiler.

Open `CMakeLists.txt` in Qt Creator and press Run, or:

```
cmake -B build
cmake --build build
./build/flashcards
```

## Structure
- `card.h`: the Card data type
- `deck.h/.cpp`: deck logic (study queue, JSON load/save), no UI code
- `mainwindow.h/.cpp`: the Qt user interface

## Releases
GitHub Actions builds a Windows `.zip` and a macOS `.dmg` whenever a tag like `v1.0` is pushed (see `.github/workflows/release.yml`).

## Deck format
```json
[{"front": "casa", "back": "home, house", "correct": 0, "missed": 0}]
```
