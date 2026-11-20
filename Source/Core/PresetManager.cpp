#include "PresetManager.h"

namespace flipper
{
using namespace juce;

const StringArray& PresetManager::categories()
{
    static const StringArray c { String::fromUTF8 ("ANALÓGICO"), "FM", "PCM", "MODULAR", String::fromUTF8 ("HÍBRIDO") };
    return c;
}

String PresetManager::categoryFor (const Snapshot& s)
{
    std::vector<int> active;
    for (int e = 0; e < numEngines; ++e)
        for (auto& [id, v] : s)
            if (id == ids::engineOn (e) && v > 0.5f) active.push_back (e);

    if (active.size() != 1) return categories()[4];
    switch (active.front())
    {
        case 5:  return categories()[1];              // DX7
        case 6:  return categories()[2];              // M1
        case 7: case 8: return categories()[3];       // ARP 2600, VCS3
        default: return categories()[0];              // analogicos
    }
}

File PresetManager::defaultFolder()
{
    return File::getSpecialLocation (File::userApplicationDataDirectory)
               .getChildFile ("Viszeversza").getChildFile ("FLIPPER").getChildFile ("Presets");
}

PresetManager::PresetManager (File f) : folder (std::move (f)) {}

bool PresetManager::read (const File& file, Preset& p)
{
    auto xml = parseXML (file);
    if (xml == nullptr || ! xml->hasTagName ("FLIPPER_PRESET")) return false;

    p.file = file;
    p.name = xml->getStringAttribute ("name", file.getFileNameWithoutExtension());
    p.category = xml->getStringAttribute ("category", categories()[4]);
    p.tags = StringArray::fromTokens (xml->getStringAttribute ("tags"), ",", "");
    p.tags.trim(); p.tags.removeEmptyStrings();
    p.favorite = xml->getBoolAttribute ("favorite");
    p.values.clear();
    for (auto* e : xml->getChildWithTagNameIterator ("PARAM"))
        p.values.push_back ({ e->getStringAttribute ("id"), (float) e->getDoubleAttribute ("value") });

    for (int e = 0; e < numEngines; ++e)
        for (auto& [id, v] : p.values)
            if (id == ids::engineOn (e)) p.engineOn[(size_t) e] = v > 0.5f;
    for (auto& [id, v] : p.values)
        if (id == ids::character) p.character = v;
    return true;
}

StringArray PresetManager::factoryFavourites() const
{
    StringArray s;
    s.addLines (folder.getChildFile ("favoritos_de_fabrica.txt").loadFileAsString());
    s.removeEmptyStrings();
    return s;
}

bool PresetManager::setFavorite (const Preset& p, bool fav)
{
    if (! p.factory) return setFavorite (p.file, fav);
    if (! folder.createDirectory()) return false;
    auto s = factoryFavourites();
    s.removeString (p.name);
    if (fav) s.add (p.name);
    return folder.getChildFile ("favoritos_de_fabrica.txt").replaceWithText (s.joinIntoString ("\n"));
}

void PresetManager::rescan()
{
    presets.clear();
    const auto favs = factoryFavourites();
    for (auto p : factory) { p.favorite = favs.contains (p.name); presets.push_back (std::move (p)); }
    const size_t firstUser = presets.size();
    for (const auto& entry : RangedDirectoryIterator (folder, false, "*.flipper"))
    {
        Preset p;
        if (read (entry.getFile(), p)) presets.push_back (std::move (p));
    }
    std::sort (presets.begin() + (long) firstUser, presets.end(),
               [] (const Preset& a, const Preset& b) { return a.name.compareNatural (b.name) < 0; });
}

File PresetManager::save (const String& name, const String& category, const String& tags,
                          bool favorite, const Snapshot& values)
{
    if (! folder.createDirectory()) return {};

    XmlElement xml ("FLIPPER_PRESET");
    xml.setAttribute ("version", 1);
    xml.setAttribute ("name", name);
    xml.setAttribute ("category", category);
    xml.setAttribute ("tags", tags);
    xml.setAttribute ("favorite", favorite);
    for (auto& [id, v] : values)
    {
        auto* e = xml.createNewChildElement ("PARAM");
        e->setAttribute ("id", id);
        e->setAttribute ("value", (double) v);
    }

    auto safe = File::createLegalFileName (name).trim();
    if (safe.isEmpty()) safe = "Preset";
    auto file = folder.getChildFile (safe + ".flipper");
    if (file.existsAsFile()) file = file.getNonexistentSibling (false);
    return xml.writeTo (file) ? file : File();
}

bool PresetManager::setFavorite (const File& file, bool fav)
{
    auto xml = parseXML (file);
    if (xml == nullptr) return false;
    xml->setAttribute ("favorite", fav);
    return xml->writeTo (file);
}
}
