#pragma once

#include <string>

#include "EditorState.hpp"

class TextBuffer;

class Screen {
public:
    Screen() = default;
    ~Screen();

    bool initialize();
    void shutdown();

    void refreshDimensions();

    int getKey();

    void draw(
        const TextBuffer& buffer,
        const EditorState& state,
        const std::string& filename
    );

    int getRows() const;
    int getCols() const;

private:
    bool initialized = false;

    int rows = 0;
    int cols = 0;
};