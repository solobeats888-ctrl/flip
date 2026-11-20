#include "CharacterPanel.h"

namespace flipper
{
using namespace juce;

CharacterPanel::CharacterPanel()
    : knob ("knob_character_png", 30.0f, 39.0f)
{
    addAndMakeVisible (knob);
    knob.onValueChange = [this] { repaint (0, 255, getWidth(), 36); };
    setEngine (8);
}

void CharacterPanel::setEngine (int e)
{
    engine = e;
    portrait = theme::image (engines()[(size_t) e].portrait);
    repaint();
}

void CharacterPanel::resized()
{
    knob.setBounds (75 - 46, 353 - 46, 92, 92);
    knob.centre = { 46.0f, 46.0f };
}

void CharacterPanel::paint (Graphics& g)
{
    const auto& info = engines()[(size_t) engine];

    theme::drawPrintedText (g, "PERSONAJE", 76.0f, 23.0f, theme::fitHeight ("PERSONAJE", 108.0f, 24.0f), 0.0f,
                            theme::cream, theme::black, 0.3f);

    // retrato dentro del ovalo pintado
    const Rectangle<float> oval (ovalCentre.x - rx, ovalCentre.y - ry, rx * 2, ry * 2);
    {
        Graphics::ScopedSaveState s (g);
        Path clip; clip.addEllipse (oval);
        g.reduceClipRegion (clip);
        g.setColour (theme::black);
        g.fillEllipse (oval);
        if (portrait.isValid())
        {
            g.setImageResamplingQuality (Graphics::highResamplingQuality);
            g.drawImage (portrait, oval, RectanglePlacement::centred | RectanglePlacement::fillDestination);
        }
        // sombra interna para que asiente en el marco
        g.setGradientFill (ColourGradient (Colours::transparentBlack, ovalCentre.x, ovalCentre.y,
                                           Colours::black.withAlpha (0.4f), ovalCentre.x + rx * 1.1f, ovalCentre.y, true));
        g.fillEllipse (oval);
    }
    g.setColour (Colour (0xff2a1d10));
    g.drawEllipse (oval, 2.0f);

    // placa con el nombre del personaje
    const String name = String::fromUTF8 (info.character);
    const Rectangle<float> plate (ovalCentre.x - 52.0f, ovalCentre.y + ry - 30.0f, 104.0f, 22.0f);
    g.setColour (Colours::black.withAlpha (0.5f));
    g.fillRoundedRectangle (plate.translated (1.5f, 2.0f), 4.0f);
    g.setGradientFill (ColourGradient (theme::mustardHi, plate.getX(), plate.getY(), Colour (0xffc98314), plate.getX(), plate.getBottom(), false));
    g.fillRoundedRectangle (plate, 4.0f);
    g.setColour (theme::ink);
    g.drawRoundedRectangle (plate, 4.0f, 1.3f);
    theme::drawPrintedText (g, name, plate.getCentreX(), plate.getCentreY() + 1.0f,
                            theme::fitHeight (name, plate.getWidth() - 12.0f, 13.0f), 0.0f, theme::ink, theme::mustard, 0.35f, false);

    // CHARACTER: el valor se nombra arriba de la perilla
    const String desc = characterDescriptor ((float) knob.getValue());
    theme::drawPrintedText (g, desc, 79.0f, 273.0f, theme::fitHeight (desc, 72.0f, 25.0f), 0.0f,
                            theme::mustard, theme::black, 0.3f);
}
}
