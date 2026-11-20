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

    SpriteKnob knob;

private:
    int engine = 8;
    juce::Image portrait;
    const juce::Point<float> ovalCentre { 76.0f, 149.0f };
    const float rx = 58.0f, ry = 74.0f;
};
}
