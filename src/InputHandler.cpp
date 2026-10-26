#include "InputHandler.hpp"

#include <ncurses.h>

int InputHandler::readKey() {
    return getch();
}