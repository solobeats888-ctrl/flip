#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

/**
    Idioma de la interfaz (castellano / ingles).
    Los textos se escriben en castellano en el codigo; tr() los traduce con el diccionario
    (Translations.cpp). Lo que no esta en el diccionario (nombres, valores) queda igual.
    El idioma y "ya vio el tutorial" se guardan por usuario, no por proyecto.
*/
namespace flipper::i18n
{
enum class Lang { es = 0, en = 1 };

Lang language();
void setLanguage (Lang);
bool english();

/** Traduce un texto de la interfaz (castellano -> idioma actual). */
juce::String tr (const juce::String& spanish);
/** Elige entre dos textos ya escritos. */
inline juce::String pick (const char* es, const char* en) { return juce::String::fromUTF8 (english() ? en : es); }

/** Avisos de cambio de idioma (para refrescar la interfaz). */
juce::ChangeBroadcaster& changes();

/** Preferencias del usuario (idioma, tutorial visto). */
bool tutorialSeen();
void setTutorialSeen (bool);

/** Todas las claves del diccionario (para pruebas). */
const std::map<juce::String, juce::String>& dictionary();
}
