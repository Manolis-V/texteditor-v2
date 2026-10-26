
CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -Iinclude
LDLIBS = -lncurses

TARGET = texteditor

SOURCES = \
	src/main.cpp \
	src/Editor.cpp \
	src/TextBuffer.cpp \
	src/FileManager.cpp \
	src/Screen.cpp \
	src/InputHandler.cpp \

OBJECTS = $(SOURCES:.cpp=.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LDLIBS)

src/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)
