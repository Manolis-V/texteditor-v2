#pragma once

#include <string>
#include <vector>

class TextBuffer {
public:
    TextBuffer();

    const std::vector<std::string>& getLines() const;
    std::size_t lineCount() const;

    void setLines(const std::vector<std::string>& newLines);

private:
    std::vector<std::string> lines;
};