#include "Editor.hpp"

#include <algorithm>
#include <cctype>
#include <ncurses.h>

Editor::Editor(const std::string& filename)
    : filename(filename) {}

void Editor::quit(){

    if (!state.modified) {
        state.running = false;
        return;
    }

    // Ask what to do with unsaved changes.
    // {
        const int statusRow = screen.getRows() - 1;

        state.setMessage("Unsaved changes: save? (y/n/Esc)");
        screen.draw(buffer, state, filename);

        const int answer = inputHandler.readKey();
        state.clearMessage();

        if (answer == 'y' || answer == 'Y') {
            if (fileManager.save(filename, buffer)) {
                state.modified = false;
                state.running = false;
            } else {
                move(statusRow, 0);
                clrtoeol();
                printw("Save failed! Press any key to continue.");
                refresh();
                inputHandler.readKey();
            }
        } else if (answer == 'n' || answer == 'N') {
            state.running = false;
        }
        // Esc or any other key cancels quitting.
    // }
    return;
}

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
        state.clearMessage();

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

            case KEY_HOME:
                state.cursorX = 0;
                break;

            case KEY_END:
                state.cursorX = buffer.getLines()[state.cursorY].length();
                break;

            case KEY_SHOME: // Shift + Home 
                state.cursorX = 0;
                state.cursorY = 0;
                break;

            case KEY_SEND: // Shift + End 
                state.cursorY = buffer.lineCount() - 1;
                state.cursorX = buffer.getLines()[state.cursorY].length();
                break;

            case KEY_PPAGE: // Page Up
                if (state.cursorY > visibleRows) {
                    state.cursorY -= visibleRows;
                } else {
                    state.cursorY = 0;
                }

                state.cursorX = std::min(
                    state.cursorX,
                    buffer.getLines()[state.cursorY].length()
                );
                break;

            case KEY_NPAGE: // Page Down
                state.cursorY = std::min(
                    state.cursorY + visibleRows,
                    buffer.lineCount() - 1
                );

                state.cursorX = std::min(
                    state.cursorX,
                    buffer.getLines()[state.cursorY].length()
                );
                break;

            case KEY_BACKSPACE:
            case 127:
            case 8:
                if (state.mode == EditorMode::Insert && (state.cursorX > 0 || state.cursorY > 0)) {
                    saveSnapshot();
                    buffer.backspace(state.cursorY, state.cursorX);
                    state.modified = true;
                }
                break;

            case KEY_DC:
                if (state.mode == EditorMode::Insert) {
                    saveSnapshot();
                    if (state.cursorX < lines[state.cursorY].size()) {
                        buffer.deleteCharacter(state.cursorY, state.cursorX);
                        state.modified = true;
                    } else if (state.cursorY + 1 < lines.size()) {
                        std::size_t nextRow = state.cursorY + 1;
                        std::size_t nextCol = 0;

                        buffer.backspace(nextRow, nextCol);
                        state.modified = true;
                    }
                }
                break;

            case '\n':
            case '\r':
            case KEY_ENTER:
                if (state.mode == EditorMode::Insert) {

                    saveSnapshot();
                    buffer.insertNewline(state.cursorY, state.cursorX);
                    ++state.cursorY;
                    state.cursorX = 0;
                    state.modified = true;
                }
                break;
            case 19: // Ctrl+S
                if (fileManager.save(filename, buffer)) {
                    state.modified = false;
                    state.setMessage("File saved successfully.");
                } else {
                    state.setMessage("Error: could not save file.");
                }
                break;
            case 3: // Ctrl+C
            case 'q':
            case 'Q':
                if (key != 3 && state.mode == EditorMode::Insert) {
                    saveSnapshot();
                    buffer.insertCharacter(state.cursorY, state.cursorX, static_cast<char>(key));
                    ++state.cursorX;
                    state.modified = true;
                    break;
                } else {
                    quit();
                }
                break;
            
            case 26:
                undo();
                break;
            
            case 25:
                redo();
                break;

            case 27: // Esc
                state.mode = EditorMode::Normal;
                break;

            case 'i':
                if (state.mode == EditorMode::Normal) {
                    state.mode = EditorMode::Insert;
                } else {
                    saveSnapshot();
                    buffer.insertCharacter(state.cursorY, state.cursorX, 'i');
                    ++state.cursorX;
                    state.modified = true;
                }
                break;
            
            case 'h':
                if (state.mode == EditorMode::Normal) {
                    if (state.cursorX > 0) {
                        --state.cursorX;
                    } else if (state.cursorY > 0) {
                        --state.cursorY;
                        state.cursorX = lines[state.cursorY].size();
                    }
                } else {
                    saveSnapshot();
                    buffer.insertCharacter(state.cursorY, state.cursorX, 'h');
                    ++state.cursorX;
                    state.modified = true;
                }
                break;

            case 'j':
                if (state.mode == EditorMode::Normal) {
                    if (state.cursorY + 1 < lines.size()) {
                        ++state.cursorY;
                        state.cursorX = std::min(
                            state.cursorX, lines[state.cursorY].size());
                    }
                } else {
                    saveSnapshot();
                    buffer.insertCharacter(state.cursorY, state.cursorX, 'j');
                    ++state.cursorX;
                    state.modified = true;
                }
                break;

            case 'k':
                if (state.mode == EditorMode::Normal) {
                    if (state.cursorY > 0) {
                        --state.cursorY;
                        state.cursorX = std::min(
                            state.cursorX, lines[state.cursorY].size());
                    }
                } else {
                    saveSnapshot();
                    buffer.insertCharacter(state.cursorY, state.cursorX, 'k');
                    ++state.cursorX;
                    state.modified = true;
                }
                break;

            case 'l':
                if (state.mode == EditorMode::Normal) {
                    if (state.cursorX < lines[state.cursorY].size()) {
                        ++state.cursorX;
                    } else if (state.cursorY + 1 < lines.size()) {
                        ++state.cursorY;
                        state.cursorX = 0;
                    }
                } else {
                    saveSnapshot();
                    buffer.insertCharacter(state.cursorY, state.cursorX, 'l');
                    ++state.cursorX;
                    state.modified = true;
                }
                break;

            case 'x':
                if (state.mode == EditorMode::Normal) {
                    if (state.cursorX < lines[state.cursorY].size()) {
                        saveSnapshot();
                        buffer.deleteCharacter(state.cursorY, state.cursorX);
                        state.modified = true;
                    }
                } else {
                    saveSnapshot();
                    buffer.insertCharacter(state.cursorY, state.cursorX, 'x');
                    ++state.cursorX;
                    state.modified = true;
                }
                break;

            default:
                if (state.mode == EditorMode::Insert &&
                    key >= 32 && key <= 126 &&
                    std::isprint(static_cast<unsigned char>(key))) {
                    
                    saveSnapshot();
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