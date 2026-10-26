#include "FileManager.hpp"

#include "TextBuffer.hpp"

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

bool FileManager::load(const std::string& filename,
                       TextBuffer& buffer) {
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::error_code ec;
        const bool exists = std::filesystem::exists(filename, ec);

        if (!ec && !exists) {
            buffer.setLines({""});
            return true;
        }

        return false;
    }

    std::vector<std::string> lines;
    std::string line;

    while (std::getline(file, line)) {
        // Handle Windows CRLF files when running on Linux.
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        lines.push_back(line);
    }

    if (file.bad()) {
        return false;
    }

    buffer.setLines(lines);
    return true;
}

bool FileManager::save(const std::string& filename,
                       const TextBuffer& buffer) {
    // Saving will be implemented in a later stage.                    
    (void)filename;
    (void)buffer;
    return false;
}