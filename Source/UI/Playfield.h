#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "../Core/Parameters.h"
#include <functional>
#include "../Help/Help.h"

namespace flipper
{
/**
    Capa interactiva del playfield (ocupa todo el tablero de 1024 x 1536).
    Los personajes son parte del arte; esta capa agrega:
      - zonas clickeables (elipses) para seleccionar cada motor
      - las placas (con el nombre del sintetizador, del arte aprobado)
      - estados: seleccionado (placa iluminada), apagado (personaje en sombra), hover, actividad
    Solo recibe clicks dentro de las elipses; el resto pasa a los controles de abajo.
*/
class Playfield : public juce::Component, public help::Provider
{
public:
    struct EngineState { bool on = true; float level = 0.7f; float activity = 0.0f; };

    Playfield();

    void setSelected (int engine);
    int  getSelected() const noexcept { return selected; }
    void setEngineState (int engine, EngineState);

    std::function<void (int)> onSelect;

    void paint (juce::Graphics&) override;
    bool hitTest (int x, int y) override;
    juce::String helpKeyAt (juce::Point<int> p) override { const int e = engineAt (p.toFloat()); return e >= 0 ? "ui:character_" + juce::String (e) : juce::String(); }
    void mouseMove (const juce::MouseEvent&) override;
    void mouseExit (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override;

private:
    int engineAt (juce::Point<float>) const;
    void paintEngine (juce::Graphics&, int engine);

    std::array<EngineState, numEngines> states {};
    int selected = 8;
    int hovered = -1;
};

/** Cartel de los modos DISENO y LAB mientras esas pantallas no estan construidas. */
class ModeNotice : public juce::Component
{
public:
    void setContent (UiMode, int engine);
    void paint (juce::Graphics&) override;

private:
    UiMode mode = UiMode::play;
    int engine = 0;
};
}
