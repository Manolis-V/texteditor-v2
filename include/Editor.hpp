#pragma once

#include <string>
#include <vector>

#include "EditorState.hpp"
#include "TextBuffer.hpp"
#include "FileManager.hpp"
#include "Screen.hpp"
#include "InputHandler.hpp"

struct EditorSnapshot {
    std::vector<std::string> lines;
    std::size_t cursorX;
    std::size_t cursorY;
};

class Editor {
public:
    explicit Editor(const std::string& filename);

    void run();

private:
    std::string filename;

    EditorState state;
    TextBuffer buffer;
    FileManager fileManager;
    Screen screen;
    InputHandler inputHandler;

    void saveSnapshot();
    void undo();
    void redo();
    void quit();
    std::vector<EditorSnapshot> undoHistory;
    std::vector<EditorSnapshot> redoHistory;
};