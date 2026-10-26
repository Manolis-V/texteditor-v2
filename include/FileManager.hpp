#pragma once

#include <string>

class TextBuffer;

class FileManager {
public:
    bool load(const std::string& filename, TextBuffer& buffer);
    bool save(const std::string& filename,
              const TextBuffer& buffer);
};