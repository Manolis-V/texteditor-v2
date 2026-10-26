#include "Editor.hpp"

#include <algorithm>
#include <cctype>
#include <ncurses.h>

Editor::Editor(const std::string& filename)
    : filename(filename) {}

void Editor::run() {
    if (!fileManager.load(filename, buffer)) {
        return;
    }

    if (!screen.initialize()) {
        return;
    }

    while (state.running) {
        screen.refreshDimensions();

        const int rows = screen.getRows();
        const int cols = screen.getCols();

        const auto& lines = buffer.getLines();

        state.cursorY = std::min(
            state.cursorY,
            lines.size() - 1
        );

        state.cursorX = std::min(
            state.cursorX,
            lines[state.cursorY].size()
        );

        const std::size_t visibleRows =
            static_cast<std::size_t>(std::max(1, rows - 1));

        if (state.cursorY < state.rowOffset) {
            state.rowOffset = state.cursorY;
        } else if (state.cursorY >= state.rowOffset + visibleRows) {
            state.rowOffset = state.cursorY - visibleRows + 1;
        }

        const std::size_t textWidth =
            static_cast<std::size_t>(std::max(1, cols - 6));

        if (state.cursorX < state.colOffset) {
            state.colOffset = state.cursorX;
        } else if (state.cursorX >= state.colOffset + textWidth) {
            state.colOffset = state.cursorX - textWidth + 1;
        }

        screen.draw(buffer, state, filename);

        const int key = inputHandler.readKey();

        switch (key) {
            case KEY_RESIZE:
                screen.refreshDimensions();
                break;

            case KEY_LEFT:
                if (state.cursorX > 0) {
                    --state.cursorX;
                } else if (state.cursorY > 0) {
                    --state.cursorY;
                    state.cursorX = lines[state.cursorY].size();
                }
                break;

            case KEY_RIGHT:
                if (state.cursorX < lines[state.cursorY].size()) {
                    ++state.cursorX;
                } else if (state.cursorY + 1 < lines.size()) {
                    ++state.cursorY;
                    state.cursorX = 0;
                }
                break;

            case KEY_UP:
                if (state.cursorY > 0) {
                    --state.cursorY;
                    state.cursorX = std::min(
                        state.cursorX,
                        lines[state.cursorY].size()
                    );
                }
                break;

            case KEY_DOWN:
                if (state.cursorY + 1 < lines.size()) {
                    ++state.cursorY;
                    state.cursorX = std::min(
                        state.cursorX,
                        lines[state.cursorY].size()
                    );
                }
                break;

            case KEY_BACKSPACE:
            case 127:
            case 8:
                if (state.cursorX > 0 || state.cursorY > 0) {
                    buffer.backspace(state.cursorY, state.cursorX);
                    state.modified = true;
                }
                break;

            case KEY_DC:
                if (state.cursorX < lines[state.cursorY].size()) {
                    buffer.deleteCharacter(
                        state.cursorY,
                        state.cursorX
                    );
                    state.modified = true;
                } else if (state.cursorY + 1 < lines.size()) {
                    std::size_t nextRow = state.cursorY + 1;
                    std::size_t nextCol = 0;

                    buffer.backspace(nextRow, nextCol);
                    state.modified = true;
                }
                break;

            case '\n':
            case '\r':
            case KEY_ENTER:
                buffer.insertNewline(
                    state.cursorY,
                    state.cursorX
                );

                ++state.cursorY;
                state.cursorX = 0;
                state.modified = true;
                break;

            case 'q':
            case 'Q':
                state.running = false;
                break;

            default:
                if (key >= 32 && key <= 126 &&
                    std::isprint(static_cast<unsigned char>(key))) {
                    buffer.insertCharacter(
                        state.cursorY,
                        state.cursorX,
                        static_cast<char>(key)
                    );

                    ++state.cursorX;
                    state.modified = true;
                }
                break;
        }
    }

    screen.shutdown();
}