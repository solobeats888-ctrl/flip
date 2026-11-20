#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

/**
    Ayuda de FLIPPER (castellano e ingles).

    Nivel 1, ayuda al pasar el mouse: cada control lleva una clave en sus propiedades
    ("help" = id del parametro o "ui:...") y la ventana de ayuda busca el texto.
    Nivel 2, recorridos guiados: pasos que iluminan una zona del tablero y la explican.
*/
namespace flipper::help
{
/** Texto de ayuda: primera linea = titulo, el resto = explicacion. Vacio si no hay. */
juce::String tooltip (const juce::String& key);

/** Componentes cuya ayuda depende de donde esta el mouse (personajes, pines). */
struct Provider { virtual ~Provider() = default; virtual juce::String helpKeyAt (juce::Point<int> local) = 0; };

struct Step
{
    juce::String target;          // "rect:x,y,w,h" (tablero 1024x1536) o "band:TITULO" (pantalla DISENO)
    const char* titleEs; const char* textEs;
    const char* titleEn; const char* textEn;
};

/** Recorrido de una pantalla: "play", "lab", "design0" .. "design8". */
const std::vector<Step>& tour (const juce::String& screen);
}
