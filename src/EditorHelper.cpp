#include "Editor.hpp"

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