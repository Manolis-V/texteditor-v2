#pragma once

#include <cstddef>

enum class EditorMode {
    Normal,
    Insert
};

struct EditorState {
    std::size_t cursorX = 0;
    std::size_t cursorY = 0;
    std::size_t rowOffset = 0;
    std::size_t colOffset = 0;

    EditorMode mode = EditorMode::Normal;

    bool running = true;
    bool modified = false;

    std::string message;

    void setMessage(const std::string& text) {
        message = text;
    }

    void clearMessage() {
        message.clear();
    }
};