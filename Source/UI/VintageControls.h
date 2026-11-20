#pragma once
#include "Theme.h"

namespace flipper
{
/**
    Perilla "de maquina" dibujada por codigo, imitando la forma de los controles del sintetizador
    original de cada motor (no la perillita generica), con la estetica gastada de FLIPPER.

    Puede ser perilla continua (con escala impresa alrededor) o selector por posiciones
    (con las posiciones rotuladas: rangos, formas de onda...). En los selectores, un click
    sobre el rotulo salta a esa posicion.
*/
class VintageKnob : public juce::Slider
{
public:
    /** Forma del control segun la maquina original. Las de fader son deslizadores verticales. */
    enum class Look { moog, prophet, m1, vcs3, jupiter, juno, cs80, dx7, arp };
    static bool isFader (Look l) { return l == Look::jupiter || l == Look::juno || l == Look::cs80 || l == Look::dx7 || l == Look::arp; }

    struct Style
    {
        Look look = Look::moog;
        float radius = 22.0f;
        juce::Colour accent { 0xff8fae3a };          // color del personaje
        juce::StringArray scale;                     // rotulos de la escala (continua)
        juce::StringArray positions;                 // rotulos de un selector
        bool waveIcons = false;                      // dibujar formas de onda en vez de texto
        juce::Colour cap { 0xffe8e2d4 };             // color de la tapa (faders, VCS3)
    };

    VintageKnob (juce::String label, Style);

    juce::Point<float> centre;
    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseEnter (const juce::MouseEvent&) override { hovering = true; repaint(); }
    void mouseExit (const juce::MouseEvent&) override  { hovering = false; repaint(); }

    /** Dibuja la forma de onda del Minimoog (0..5) en un rectangulo. */
    static void drawWave (juce::Graphics&, int wave, juce::Rectangle<float>, juce::Colour, float thickness);

private:
    float angleFor (float proportion) const;
    void paintMoog (juce::Graphics&, float angle);
    void paintRotary (juce::Graphics&, float angle);
    void paintFader (juce::Graphics&);
    void paintScale (juce::Graphics&);

    juce::String label;
    Style style;
    bool hovering = false;
};
}
