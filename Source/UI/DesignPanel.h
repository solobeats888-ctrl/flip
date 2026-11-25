#pragma once
#include "Controls.h"
#include "VintageControls.h"
#include "DesignWidgets.h"
#include "../PluginProcessor.h"

namespace flipper
{
/**
    Pantalla DISENO: la arquitectura del motor elegido en el tablero, con sus controles reales.
    Solo se muestran motores que ya tienen DSP; el resto lo dice (no hay perillas que no suenen).
*/
class DesignPanel : public juce::Component
{
public:
    static juce::Rectangle<int> boardBounds() { return { 188, 196, 632, 1064 }; }

    DesignPanel (FlipperProcessor&, juce::MouseListener& historyListener);
    void setEngine (int engine);
    int getEngine() const noexcept { return engine; }
    juce::Rectangle<int> bandRect (const juce::String& title) const;
    void reveal (const juce::String& paramId) { if (engine > 0) pages[(size_t) engine]->reveal (paramId); }
    void paint (juce::Graphics&) override;

private:
    struct Band { juce::String title; juce::Rectangle<float> area; };

    void buildMinimoog();
    void paintMinimoog (juce::Graphics&, juce::Rectangle<float> panel);
    VintageKnob& addKnob (const juce::String& paramId, const juce::String& label, float x, float y, VintageKnob::Style);

    FlipperProcessor& proc;
    juce::MouseListener& history;
    int engine = 0;
    std::vector<std::unique_ptr<VintageKnob>> knobs;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> attachments;
    std::vector<Band> bands;
    juce::Image portrait;
    juce::Component minimoogPage;
    std::array<std::unique_ptr<EnginePage>, numEngines> pages;
    std::array<juce::Image, numEngines> portraits;
};
}
