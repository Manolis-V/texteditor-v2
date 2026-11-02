#include "Screen.hpp"

#include "TextBuffer.hpp"

#include <algorithm>
#include <ncurses.h>
#include <string>

Screen::~Screen() {
    shutdown();
}

bool Screen::initialize() {
    if (initialized) {
        return true;
    }

    if (initscr() == nullptr) {
        return false;
    }

    initialized = true;
    
    start_color();
    init_pair(1, COLOR_BLACK, COLOR_YELLOW);
    init_pair(2, COLOR_BLACK, COLOR_GREEN);

    // cbreak();  replaced by raw() so Ctrl + s works
    raw();
    noecho();
    keypad(stdscr, TRUE);
    set_escdelay(25);
    curs_set(1);

    refreshDimensions();

    return true;
}

void Screen::shutdown() {
    if (initialized) {
        endwin();
        initialized = false;
    }
}

void Screen::refreshDimensions() {
    if (!initialized) {
        return;
    }

    getmaxyx(stdscr, rows, cols);
}

int Screen::getKey() {
    if (!initialized) {
        return ERR;
    }

    int key = wgetch(stdscr);

    if (key == KEY_RESIZE) {
        refreshDimensions();
    }

    return key;
}

int Screen::getRows() const {
    return rows;
}

int Screen::getCols() const {
    return cols;
}

void Screen::draw(
    const TextBuffer& buffer,
    const EditorState& state,
    const std::string& filename,
    const std::string& searchQuery
) {
    if (!initialized || rows <= 0 || cols <= 0) {
        return;
    }

    erase();

    const auto& lines = buffer.getLines();

    // Reserve the last terminal row for the status bar.
    const int textRows = std::max(0, rows - 1);
    const int gutterWidth = 6;
    const int textWidth = std::max(0, cols - gutterWidth);

    for (int screenY = 0; screenY < textRows; ++screenY) {
        const std::size_t fileY =
            state.rowOffset + static_cast<std::size_t>(screenY);

        if (fileY >= lines.size()) {
            break;
        }

        // Draw line number.
        mvprintw(screenY, 0, "%4zu ", fileY + 1);

        // Draw visible part of the line.
        const std::string& line = lines[fileY];

        if (textWidth > 0 && state.colOffset < line.size()) {
            const std::string visible =
                line.substr(state.colOffset,
                            static_cast<std::size_t>(textWidth));

            if (searchQuery.empty()) {
                mvaddnstr(screenY, gutterWidth, visible.c_str(), textWidth);
            } else {
                std::size_t pos = 0;

                while (pos < visible.size()) {
                    std::size_t match = visible.find(searchQuery, pos);

                    if (match == std::string::npos) {
                        mvaddnstr(screenY, gutterWidth + pos,
                                visible.c_str() + pos,
                                visible.size() - pos);
                        break;
                    }

                    // Print normal text before the match
                    if (match > pos) {
                        mvaddnstr(screenY, gutterWidth + pos,
                                visible.c_str() + pos,
                                match - pos);
                    }

                    // Convert visible position to document position
                    std::size_t documentX = match + state.colOffset;

                    // Check whether this is the selected match
                    bool selected =
                        state.cursorY == state.rowOffset + screenY - 0 /* adjust if needed */ &&
                        state.cursorX == documentX;

                    attron(COLOR_PAIR(selected ? 2 : 1));

                    mvaddnstr(screenY, gutterWidth + match,
                            visible.c_str() + match,
                            searchQuery.size());

                    attroff(COLOR_PAIR(selected ? 2 : 1));

                    pos = match + searchQuery.size();
                }
            }
        }
    }

    // Draw message / status bar.
    attron(A_REVERSE);

    if (!state.message.empty()) {
        const int messageRow = rows - 2;
        move(messageRow, 0);
        clrtoeol();
        mvaddnstr(messageRow, 0, state.message.c_str(), cols);
    }

    std::string status =
        filename + 
        (state.modified ? " [+]" : "") +
        " | " +
        std::to_string(state.cursorY + 1) + ":" +
        std::to_string(state.cursorX + 1) + " | " +
        (state.mode == EditorMode::Insert ? "INSERT" : "NORMAL");

    status.resize(std::min(status.size(),
                           static_cast<std::size_t>(cols)));

    mvaddnstr(rows - 1, 0, status.c_str(), cols);

    attroff(A_REVERSE);

    // Position cursor relative to the visible viewport.
    const int screenX =
        static_cast<int>(state.cursorX - state.colOffset) + gutterWidth;
    const int screenY =
        static_cast<int>(state.cursorY - state.rowOffset);

    if (screenX >= gutterWidth && screenX < cols &&
        screenY >= 0 && screenY < textRows) {
        move(screenY, screenX);
    }

    refresh();
}