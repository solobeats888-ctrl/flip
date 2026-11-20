#pragma once
#include "SoundGenerator.h"
#include <optional>

namespace flipper
{
/** Historial de deshacer / rehacer con fotos completas del sonido (hilo de mensajes). */
class History
{
public:
    void push (const Snapshot& s)
    {
        if (! undoStack.empty() && SoundGenerator::equal (undoStack.back(), s)) return;
        undoStack.push_back (s);
        if (undoStack.size() > maxSize) undoStack.erase (undoStack.begin());
        redoStack.clear();
    }

    /** Devuelve la foto a aplicar, o nada si no hay historial. */
    std::optional<Snapshot> undo (const Snapshot& current)
    {
        while (! undoStack.empty() && SoundGenerator::equal (undoStack.back(), current))
            undoStack.pop_back();
        if (undoStack.empty()) return std::nullopt;
        auto target = undoStack.back();
        undoStack.pop_back();
        redoStack.push_back (current);
        return target;
    }

    std::optional<Snapshot> redo (const Snapshot& current)
    {
        if (redoStack.empty()) return std::nullopt;
        auto target = redoStack.back();
        redoStack.pop_back();
        undoStack.push_back (current);
        return target;
    }

    bool canUndo() const noexcept { return ! undoStack.empty(); }
    bool canRedo() const noexcept { return ! redoStack.empty(); }

private:
    static constexpr size_t maxSize = 100;
    std::vector<Snapshot> undoStack, redoStack;
};
}
