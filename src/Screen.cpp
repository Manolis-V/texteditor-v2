#include "Screen.hpp"

#include <ncurses.h>

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
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    return true;
}

void Screen::shutdown() {
    if (initialized) {
        endwin();
        initialized = false;
    }
}

void Screen::draw(const std::string& message) {
    if (!initialized) {
        return;
    }

    erase();
    mvaddstr(0, 0, message.c_str());
    refresh();
}

int Screen::getKey() {
    if (!initialized) {
        return ERR;
    }

    return wgetch(stdscr);
}