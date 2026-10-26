#pragma once

#include <string>

class Screen {
public:
    Screen() = default;
    ~Screen();

    bool initialize();
    void shutdown();

    void draw(const std::string& message);
    int getKey();

private:
    bool initialized = false;
};