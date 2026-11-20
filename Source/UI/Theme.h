#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

namespace flipper::theme
{
// Paleta tomada de la imagen aprobada
inline const juce::Colour black    { 0xff120d0a };
inline const juce::Colour ink      { 0xff1c120b };  // tinta de las placas
inline const juce::Colour red      { 0xff8e1414 };
inline const juce::Colour mustard  { 0xfff0b02a };
inline const juce::Colour mustardHi{ 0xffffd25a };
inline const juce::Colour cream    { 0xffeedcb6 };
inline const juce::Colour gold     { 0xffc89a4c };
inline const juce::Colour blueMetal{ 0xff7b9db1 };

/** Bebas Neue (OFL), embebida. */
juce::Font font (float height);

/** Imagen desde BinaryData (cacheada por JUCE). */
juce::Image image (const char* resourceName);

/** Ancho que ocupa un texto con la tipografia del plugin. */
float textWidth (const juce::String& text, float height);

/** Alto de letra para que el texto entre en maxWidth (sin pasarse de maxHeight). */
float fitHeight (const juce::String& text, float maxWidth, float maxHeight);

/**
    Texto "impreso": centrado en (cx, cy), rotado, con una sombra leve y un desgaste
    determinista (motas del color de fondo) para que no se vea como texto digital limpio.
*/
void drawPrintedText (juce::Graphics&, const juce::String& text, float cx, float cy,
                      float height, float angleDegrees, juce::Colour colour,
                      juce::Colour wearColour, float wear = 0.35f, bool shadow = true);
}
