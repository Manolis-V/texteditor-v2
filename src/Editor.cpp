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

    const int statusRow = screen.getRows() - 1;

    state.setMessage("Unsaved changes: save? (y/n/Esc)");
    screen.draw(buffer, state, filename, searchQuery);

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

    return;
}

void Editor::insertKey(int key){
    // if (!insertGroupActive) {
    //     saveSnapshot();
    //     insertGroupActive = true;
    // }
    // if (key == 32){
    //     insertGroupActive = false;
    // }

    if (key == 32) {
        insertGroupActive = false;
        saveSnapshot();
    }
    else {
        if (!insertGroupActive) {
            saveSnapshot();
            insertGroupActive = true;
        }
    }    


    buffer.insertCharacter(
    state.cursorY,
    state.cursorX,
    static_cast<char>(key)
    );

    ++state.cursorX;
    state.modified = true;
}

void Editor::search() {
    bool searching = true;
    escape_searching = false;
    searchQuery.clear();
    state.setMessage("Search: ");
    screen.draw(buffer, state, filename, searchQuery);

    while (searching) {
        int ch = inputHandler.readKey();

        if (ch == 27) { // Escape
            searching = false;
            escape_searching = true;
            searchQuery.clear();
            state.setMessage("Exiting search");
            screen.draw(buffer, state, filename, searchQuery);
            return;
        } else if (ch == '\n' || ch == KEY_ENTER) {
            searching = false;
            state.clearMessage();

            if (!searchQuery.empty()) {
                findNext();
            }
            return;
        }
        else if (ch == KEY_BACKSPACE || ch == 127 || ch == 8) {
            if (!searchQuery.empty()) {
                searchQuery.pop_back();
            }
        }
        else if (ch >= 32 && ch <= 126) {
            searchQuery += static_cast<char>(ch);
        }

        state.setMessage("Search: " + searchQuery);
        screen.draw(buffer, state, filename, searchQuery);
    }
}

void Editor::findNext() {
    const auto& lines = buffer.getLines();

    for (std::size_t y = state.cursorY; y < lines.size(); ++y) {
        std::size_t startX = (y == state.cursorY)
                                 ? state.cursorX + 1
                                 : 0;

        std::size_t pos = lines[y].find(searchQuery, startX);

        if (pos != std::string::npos) {
            state.cursorY = y;
            state.cursorX = pos;

            int visibleRows = screen.getRows() - 2;
            if (state.cursorY < state.rowOffset) {
                state.rowOffset = state.cursorY;
            }
            if (state.cursorY >= state.rowOffset + visibleRows) {
                state.rowOffset = state.cursorY - visibleRows + 1;
            }
            state.setMessage("Match found.");
            return;
        }
    }

    // Wrap around: search from the beginning
    for (std::size_t y = 0; y <= state.cursorY; ++y) {
        std::size_t pos = lines[y].find(searchQuery);

        if (pos != std::string::npos) {
            state.cursorY = y;
            state.cursorX = pos;
            int visibleRows = screen.getRows() - 2;
            if (state.cursorY < state.rowOffset) {
                state.rowOffset = state.cursorY;
            }
            if (state.cursorY >= state.rowOffset + visibleRows) {
                state.rowOffset = state.cursorY - visibleRows + 1;
            }
            state.setMessage("Search wrapped.");
            screen.draw(buffer, state, filename, searchQuery);
            return;
        }
    }

    state.setMessage("Pattern not found.");
}
void Editor::findPrevious() {
    const auto& lines = buffer.getLines();

    for (int y = static_cast<int>(state.cursorY); y >= 0; --y) {
        const std::string& line = lines[y];

        std::size_t startX;

        if (static_cast<std::size_t>(y) == state.cursorY) {
            if (state.cursorX == 0) {
                continue;
            }
            startX = state.cursorX - 1;
        } else {
            startX = line.size();
        }

        std::size_t pos = line.rfind(searchQuery, startX);

        if (pos != std::string::npos) {
            state.cursorY = static_cast<std::size_t>(y);
            state.cursorX = pos;

            int visibleRows = screen.getRows() - 2;
            if (state.cursorY < state.rowOffset) {
                state.rowOffset = state.cursorY;
            }
            if (state.cursorY >= state.rowOffset + visibleRows) {
                state.rowOffset = state.cursorY - visibleRows + 1;
            }
            state.setMessage("Match found.");
            return;
        }
    }

    // Wrap around: search backwards from the end
    for (int y = static_cast<int>(lines.size()) - 1;
         y >= static_cast<int>(state.cursorY); --y) {

        std::size_t pos = lines[y].rfind(searchQuery);

        if (pos != std::string::npos) {
            state.cursorY = static_cast<std::size_t>(y);
            state.cursorX = pos;
            int visibleRows = screen.getRows() - 2;
            if (state.cursorY < state.rowOffset) {
                state.rowOffset = state.cursorY;
            }
            if (state.cursorY >= state.rowOffset + visibleRows) {
                state.rowOffset = state.cursorY - visibleRows + 1;
            }
            state.setMessage("Search wrapped.");
            screen.draw(buffer, state, filename, searchQuery);
            return;
        }
    }

    state.setMessage("Pattern not found.");
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

        screen.draw(buffer, state, filename, searchQuery);

        const int key = inputHandler.readKey();
        state.clearMessage();

        switch (key) {
            case KEY_RESIZE:
                screen.refreshDimensions();
                break;

            case KEY_LEFT:
                insertGroupActive = false;
                if (state.cursorX > 0) {
                    --state.cursorX;
                } else if (state.cursorY > 0) {
                    --state.cursorY;
                    state.cursorX = lines[state.cursorY].size();
                }
                break;

            case KEY_RIGHT:
                insertGroupActive = false;
                if (state.cursorX < lines[state.cursorY].size()) {
                    ++state.cursorX;
                } else if (state.cursorY + 1 < lines.size()) {
                    ++state.cursorY;
                    state.cursorX = 0;
                }
                break;

            case KEY_UP:
                insertGroupActive = false;
                if (state.cursorY > 0) {
                    --state.cursorY;
                    state.cursorX = std::min(
                        state.cursorX,
                        lines[state.cursorY].size()
                    );
                }
                break;

            case KEY_DOWN:
                insertGroupActive = false;
                if (state.cursorY + 1 < lines.size()) {
                    ++state.cursorY;
                    state.cursorX = std::min(
                        state.cursorX,
                        lines[state.cursorY].size()
                    );
                }
                break;

            case KEY_HOME:
                insertGroupActive = false;
                state.cursorX = 0;
                break;

            case KEY_END:
                insertGroupActive = false;
                state.cursorX = buffer.getLines()[state.cursorY].length();
                break;

            case KEY_SHOME: // Shift + Home 
                insertGroupActive = false;
                state.cursorX = 0;
                state.cursorY = 0;
                break;

            case KEY_SEND: // Shift + End
                insertGroupActive = false;
                state.cursorY = buffer.lineCount() - 1;
                state.cursorX = buffer.getLines()[state.cursorY].length();
                break;

            case KEY_PPAGE: // Page Up
                insertGroupActive = false;
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
                insertGroupActive = false;
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
                    insertGroupActive = false;
                    saveSnapshot();
                    buffer.backspace(state.cursorY, state.cursorX);
                    state.modified = true;
                }
                break;

            case KEY_DC:
                if (state.mode == EditorMode::Insert) {
                    insertGroupActive = false;
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
                    insertGroupActive = false;
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
                    insertKey(key);
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

            case 6: // Ctrl+f
                if (state.mode == EditorMode::Normal) {
                    insertGroupActive = false;
                    search();
                } else {
                    state.setMessage("Exit Insert mode first.");
                }
                break;

            case 'm':
                if (state.mode == EditorMode::Normal && !searchQuery.empty()) {
                    insertGroupActive = false;
                    findNext();
                } else if (state.mode == EditorMode::Insert){
                    insertKey(key);
                }
                break;

            case 'n': // Find previous
                if (state.mode == EditorMode::Normal && !searchQuery.empty()) {
                    insertGroupActive = false;
                    findPrevious();
                } else if (state.mode == EditorMode::Insert){
                    insertKey(key);
                }
                break;

            case 27: // Esc
                insertGroupActive = false;
                state.mode = EditorMode::Normal;
                if(!escape_searching){
                    escape_searching = true;
                    searchQuery.clear();
                    screen.draw(buffer, state, filename, searchQuery);
                }
                break;

            case 'i':
                if (state.mode == EditorMode::Normal) {
                    state.mode = EditorMode::Insert;
                } else {
                    insertKey(key);
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
                    insertKey(key);
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
                    insertKey(key);
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
                    insertKey(key);
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
                    insertKey(key);
                }
                break;

            case 'x':
                if (state.mode == EditorMode::Normal) {
                    if (state.cursorX < lines[state.cursorY].size()) {
                        buffer.deleteCharacter(state.cursorY, state.cursorX);
                        state.modified = true;
                    }
                } else {
                    insertKey(key);
                }
                break;

            default:
                if (state.mode == EditorMode::Insert &&
                    key >= 32 && key <= 126 &&
                    std::isprint(static_cast<unsigned char>(key))) {
                    insertKey(key);
                }
                break;
        }
    }

    screen.shutdown();
}