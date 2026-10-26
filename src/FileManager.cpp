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
    std::ofstream file(filename, std::ios::binary | std::ios::trunc);

    if (!file.is_open()) {
        return false;
    }

    const auto& lines = buffer.getLines();

    for (std::size_t i = 0; i < lines.size(); ++i) {
        file.write(lines[i].data(),
                   static_cast<std::streamsize>(lines[i].size()));

        file.put('\n');

        if (!file) {
            return false;
        }
    }

    file.flush();

    if (!file) {
        return false;
    }

    file.close();
    return !file.fail();
}