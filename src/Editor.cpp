#include "Editor.hpp"

Editor::Editor(const std::string& filename)
    : filename(filename) {}

void Editor::run() {
    if (!fileManager.load(filename, buffer)) {
        // Report the error before entering the terminal interface.
        return;
    }

    if (!screen.initialize()) {
        return;
    }

    std::string message = "Text Editor v2\n";
    message += "File: " + filename + "\n";
    message += "------------------------------\n";

    const auto& lines = buffer.getLines();

    for (const auto& line : lines) {
        message += line;
        message += '\n';
    }

    message += "\nPress any key to exit.";

    screen.draw(message);
    screen.getKey();

    state.running = false;
    screen.shutdown();
}