#pragma once
#include "VintageControls.h"
#include "../Core/Modulation.h"
#include <juce_audio_processors/juce_audio_processors.h>

namespace flipper
{
/** Etiqueta de una fuente (MOD A, LFO 1...) que se arrastra hasta una perilla. */
class ModSourceChip : public juce::Component, private juce::Timer
{
public:
    ModSourceChip (ModMatrix&, int source);
    ~ModSourceChip() override;
    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override;

    std::function<void (int source, juce::Point<float> where)> onDrag;   // en coordenadas del padre de arriba (tablero)
    std::function<void (int source, juce::Point<float> where)> onDrop;
    const int source;

private:
    void timerCallback() override { repaint(); }
    juce::Point<float> toBoard (const juce::MouseEvent&) const;
    ModMatrix& matrix;
    bool dragging = false;
};

/** Selector de opciones de texto (forma y tempo del LFO): click = siguiente, rueda = mover. */
class ParamCycler : public juce::Component
{
public:
    ParamCycler (juce::RangedAudioParameter&, juce::Colour accent);
    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails&) override;

private:
    void step (int d);
    juce::RangedAudioParameter& param;
    juce::Colour accent;
    int index = 0;
    juce::ParameterAttachment attachment;
};

/** Panel MODULACION (columna izquierda, pantalla DISENO): MOD A-D y LFO 1-2. */
class ModPanel : public juce::Component
{
public:
    static juce::Rectangle<int> boardBounds() { return { 14, 552, 172, 762 }; }

    ModPanel (juce::AudioProcessorValueTreeState&, ModMatrix&, juce::MouseListener& history);
    void paint (juce::Graphics&) override;

    std::array<std::unique_ptr<ModSourceChip>, ModMatrix::numSources> chips;

private:
    ModMatrix& matrix;
    std::vector<std::unique_ptr<VintageKnob>> knobs;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> atts;
    std::vector<std::unique_ptr<ParamCycler>> cyclers;
    std::array<juce::Rectangle<float>, 2> scopes;
    struct Scope : juce::Component, juce::Timer
    {
        Scope (ModMatrix& m, int s) : matrix (m), source (s) { setInterceptsMouseClicks (false, false); startTimerHz (30); }
        void timerCallback() override { history[(size_t) pos] = matrix.sourceValue (source); pos = (pos + 1) % (int) history.size(); repaint(); }
        void paint (juce::Graphics&) override;
        ModMatrix& matrix; int source; std::array<float, 64> history {}; int pos = 0;
    };
    std::array<std::unique_ptr<Scope>, 2> scopeViews;
};

/**
    Capa encima del tablero: marca las perillas moduladas (punto del color de la fuente +
    barra con el valor modulado) y dibuja el arrastre. Solo recibe el mouse sobre las marcas:
    arrastrar una marca cambia la cantidad, doble click o click derecho la quita.
*/
class ModOverlay : public juce::Component, private juce::Timer
{
public:
    explicit ModOverlay (ModMatrix&);
    ~ModOverlay() override;

    /** Controles visibles que se pueden modular: clave (id) > zona en el tablero. */
    std::function<std::vector<std::pair<juce::String, juce::Rectangle<int>>>()> visibleControls;
    std::function<juce::String (const juce::String& paramId)> nameFor;

    void dragUpdate (int source, juce::Point<float> where, const juce::String& targetId, juce::Rectangle<int> targetRect);
    void dragEnd();
    void refresh();

    void paint (juce::Graphics&) override;
    bool hitTest (int x, int y) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override;
    void mouseDoubleClick (const juce::MouseEvent&) override;

private:
    void timerCallback() override;
    struct Badge { int source; juce::String param; juce::Rectangle<int> control; juce::Rectangle<float> dot; float depth; };
    int badgeAt (juce::Point<float>) const;

    ModMatrix& matrix;
    std::vector<Badge> badges;
    std::vector<std::pair<juce::String, juce::Rectangle<int>>> controls;
    int dragSource = -1;
    juce::Point<float> dragPos;
    juce::String dragTarget;
    juce::Rectangle<int> dragTargetRect;
    int editing = -1;
    float editStartDepth = 0.0f;
    int tick = 0;
};
}
