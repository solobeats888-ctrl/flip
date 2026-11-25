#pragma once
#include "Controls.h"
#include "../Core/Parameters.h"

namespace flipper
{
/** Panel PERSONAJE: retrato del motor elegido, su nombre y el control CHARACTER. */
class CharacterPanel : public juce::Component
{
public:
    static juce::Rectangle<int> boardBounds() { return { 858, 740, 152, 435 }; }

    CharacterPanel();
    void setEngine (int engine);
    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseMove (const juce::MouseEvent&) override;

    /** Click en el retrato: abre el MAESTRO. */
    std::function<void()> onTeach;
    bool teaching = false;    // el globo se ilumina mientras el maestro esta abierto

    SpriteKnob knob;

private:
    int engine = 8;
    juce::Image portrait;
    const juce::Point<float> ovalCentre { 76.0f, 149.0f };
    const float rx = 58.0f, ry = 74.0f;
    juce::Rectangle<float> bubble { 104.0f, 66.0f, 42.0f, 32.0f };
    bool overPortrait (juce::Point<float>) const;
};
}
