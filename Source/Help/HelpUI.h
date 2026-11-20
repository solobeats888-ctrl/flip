#pragma once
#include "Help.h"
#include "I18n.h"

namespace flipper
{
/** Ventana de ayuda al pasar el mouse: busca la clave "help" del control (o de sus padres). */
class HelpTooltipWindow : public juce::TooltipWindow
{
public:
    explicit HelpTooltipWindow (juce::Component* parent) : juce::TooltipWindow (parent, 650) {}
    juce::String getTipFor (juce::Component&) override;
};

/** Boton "?" redondo de bronce. */
class HelpButton : public juce::Button
{
public:
    HelpButton() : juce::Button ("?") { setMouseCursor (juce::MouseCursor::PointingHandCursor); getProperties().set ("help", "ui:help"); }
    void paintButton (juce::Graphics&, bool highlighted, bool down) override;
};

/**
    Recorrido guiado: oscurece el tablero, ilumina una zona y la explica en un cartel
    con ANTERIOR / SIGUIENTE / SALIR. Se dibuja en el espacio del tablero (1024 x 1536).
*/
class TourOverlay : public juce::Component
{
public:
    std::function<juce::Rectangle<int> (const juce::String& target)> resolve;
    std::function<void()> onClose;

    void start (const std::vector<help::Step>&);
    bool isRunning() const noexcept { return steps != nullptr; }
    void close();

    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    bool keyPressed (const juce::KeyPress&) override;

    void next();

private:
    void layout();
    const std::vector<help::Step>* steps = nullptr;
    int index = 0;
    juce::Rectangle<float> hole, card, prevBtn, nextBtn, exitBtn;
};
}
