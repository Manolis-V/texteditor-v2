#include <iostream>

#include "Editor.hpp"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: ./texteditor <filename>\n";
        return 1;
    }

    Editor editor(argv[1]);
    editor.run();

    return 0;
}