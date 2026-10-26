#pragma once

#include <string>

#include "EditorState.hpp"
#include "TextBuffer.hpp"
#include "FileManager.hpp"
#include "Screen.hpp"
#include "InputHandler.hpp"

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
};