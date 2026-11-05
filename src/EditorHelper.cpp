#include "Editor.hpp"
#include <ncurses.h>

void Editor::saveSnapshot() {
    EditorSnapshot snapshot;

    snapshot.lines = buffer.getLines();
    snapshot.cursorX = state.cursorX;
    snapshot.cursorY = state.cursorY;

    undoHistory.push_back(snapshot);

    // A new edit invalidates the redo history.
    redoHistory.clear();
}

void Editor::redo() {
    if (redoHistory.empty()) {
        state.setMessage("Nothing to redo.");
        return;
    }

    // Save the current state for future Undo.
    EditorSnapshot currentSnapshot;
    currentSnapshot.lines = buffer.getLines();
    currentSnapshot.cursorX = state.cursorX;
    currentSnapshot.cursorY = state.cursorY;

    undoHistory.push_back(currentSnapshot);

    // Restore the next state.
    EditorSnapshot nextSnapshot = redoHistory.back();
    redoHistory.pop_back();

    buffer.setLines(nextSnapshot.lines);

    state.cursorX = nextSnapshot.cursorX;
    state.cursorY = nextSnapshot.cursorY;

    state.modified = true;
    state.setMessage("Redo successful.");
}

void Editor::undo() {
    if (undoHistory.empty()) {
        state.setMessage("Nothing to undo.");
        return;
    }

    // Save the current state for future Redo.
    EditorSnapshot currentSnapshot;
    currentSnapshot.lines = buffer.getLines();
    currentSnapshot.cursorX = state.cursorX;
    currentSnapshot.cursorY = state.cursorY;

    redoHistory.push_back(currentSnapshot);

    // Restore the previous state.
    EditorSnapshot previousSnapshot = undoHistory.back();
    undoHistory.pop_back();

    buffer.setLines(previousSnapshot.lines);

    state.cursorX = previousSnapshot.cursorX;
    state.cursorY = previousSnapshot.cursorY;

    state.modified = true;
    state.setMessage("Undo successful.");
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