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