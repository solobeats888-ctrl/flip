#include "I18n.h"

namespace flipper::i18n
{
using namespace juce;

namespace
{
std::atomic<int> currentLang { -1 };

PropertiesFile& settings()
{
    static PropertiesFile file = []
    {
        PropertiesFile::Options o;
        o.applicationName = "FLIPPER";
        o.folderName = "Viszeversza/FLIPPER";
        o.filenameSuffix = ".settings";
        o.osxLibrarySubFolder = "Application Support";
        return PropertiesFile (o);
    }();
    return file;
}

ChangeBroadcaster& broadcaster() { static ChangeBroadcaster b; return b; }
}

Lang language()
{
    if (currentLang.load() < 0)
    {
        // primera vez: el idioma del sistema (castellano si es "es", si no ingles)
        const bool sysSpanish = SystemStats::getUserLanguage().startsWithIgnoreCase ("es");
        currentLang = settings().getIntValue ("language", sysSpanish ? 0 : 1);
    }
    return (Lang) currentLang.load();
}

bool english() { return language() == Lang::en; }

void setLanguage (Lang l)
{
    if (language() == l) return;
    currentLang = (int) l;
    settings().setValue ("language", (int) l);
    settings().saveIfNeeded();
    broadcaster().sendSynchronousChangeMessage();
}

ChangeBroadcaster& changes() { return broadcaster(); }

bool tutorialSeen() { return settings().getBoolValue ("tutorialSeen", false); }
void setTutorialSeen (bool s) { settings().setValue ("tutorialSeen", s); settings().saveIfNeeded(); }

String tr (const String& s)
{
    if (! english() || s.isEmpty()) return s;
    const auto& d = dictionary();
    auto it = d.find (s);
    return it != d.end() ? it->second : s;
}
}
