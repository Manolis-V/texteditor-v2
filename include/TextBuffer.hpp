#pragma once

#include <string>
#include <vector>

class TextBuffer {
public:
    TextBuffer();

    const std::vector<std::string>& getLines() const;
    std::size_t lineCount() const;

    void setLines(const std::vector<std::string>& newLines);

        void insertCharacter(
        std::size_t row,
        std::size_t col,
        char character
    );

    void insertNewline(std::size_t row, std::size_t col);

    void deleteCharacter(std::size_t row, std::size_t col);

    void backspace(std::size_t& row, std::size_t& col);

private:
    std::vector<std::string> lines;
};