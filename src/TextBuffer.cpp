#include "TextBuffer.hpp"

TextBuffer::TextBuffer()
    : lines{""} {}

const std::vector<std::string>& TextBuffer::getLines() const {
    return lines;
}

std::size_t TextBuffer::lineCount() const {
    return lines.size();
}

void TextBuffer::setLines(const std::vector<std::string>& newLines) {
    lines = newLines;

    if (lines.empty()) {
        lines.emplace_back();
    }
}

void TextBuffer::insertCharacter(
    std::size_t row,
    std::size_t col,
    char character
) {
    if (row >= lines.size()) {
        return;
    }

    col = std::min(col, lines[row].size());
    lines[row].insert(lines[row].begin() + col, character);
}

void TextBuffer::insertNewline(std::size_t row, std::size_t col) {
    if (row >= lines.size()) {
        return;
    }

    col = std::min(col, lines[row].size());

    std::string remaining = lines[row].substr(col);
    lines[row].erase(col);

    lines.insert(
        lines.begin() + row + 1,
        remaining
    );
}

void TextBuffer::deleteCharacter(std::size_t row, std::size_t col) {
    if (row >= lines.size() || col >= lines[row].size()) {
        return;
    }

    lines[row].erase(col, 1);
}

void TextBuffer::backspace(std::size_t& row, std::size_t& col) {
    if (row >= lines.size()) {
        return;
    }

    if (col > 0) {
        lines[row].erase(col - 1, 1);
        --col;
        return;
    }

    if (row > 0) {
        const std::size_t previousLength = lines[row - 1].size();

        lines[row - 1] += lines[row];
        lines.erase(lines.begin() + row);

        --row;
        col = previousLength;
    }
}