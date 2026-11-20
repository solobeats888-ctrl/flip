#pragma once
#include "SoundGenerator.h"

namespace flipper
{
/**
    Presets del usuario en ~/Library/Application Support/Viszeversza/FLIPPER/Presets (Mac)
    o %APPDATA%\Viszeversza\FLIPPER\Presets (Windows). Un archivo .flipper por preset (XML).
    Solo se toca desde el hilo de mensajes (nunca desde el audio).
*/
class PresetManager
{
public:
    struct Preset
    {
        juce::File file;
        juce::String name, category;
        juce::StringArray tags;
        bool favorite = false;
        bool factory = false;
        Snapshot values;
        std::array<bool, numEngines> engineOn {};
        float character = 0.5f;
    };

    static const juce::StringArray& categories();   // ANALÓGICO, FM, PCM, MODULAR, HÍBRIDO
    static juce::String categoryFor (const Snapshot&);

    explicit PresetManager (juce::File folder = defaultFolder());
    static juce::File defaultFolder();

    void rescan();
    void setFactory (std::vector<Preset> list) { factory = std::move (list); }
    const std::vector<Preset>& all() const noexcept { return presets; }

    juce::File save (const juce::String& name, const juce::String& category,
                     const juce::String& tags, bool favorite, const Snapshot&);
    bool setFavorite (const juce::File&, bool);
    bool setFavorite (const Preset&, bool);

private:
    static bool read (const juce::File&, Preset&);
    juce::File folder;
    std::vector<Preset> presets, factory;
    juce::StringArray factoryFavourites() const;
};
}
