#include "CharacterPanel.h"

namespace flipper
{
using namespace juce;

CharacterPanel::CharacterPanel()
    : knob ("knob_character_png", 30.0f, 39.0f)
{
    addAndMakeVisible (knob);
    getProperties().set ("help", "ui:teacher");
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

bool CharacterPanel::overPortrait (Point<float> p) const
{
    const float dx = (p.x - ovalCentre.x) / rx, dy = (p.y - ovalCentre.y) / ry;
    return dx * dx + dy * dy <= 1.0f || bubble.expanded (4.0f).contains (p);
}

void CharacterPanel::mouseMove (const MouseEvent& e)
{
    setMouseCursor (overPortrait (e.position) ? MouseCursor::PointingHandCursor : MouseCursor::NormalCursor);
}

void CharacterPanel::mouseDown (const MouseEvent& e)
{
    if (overPortrait (e.position) && onTeach) onTeach();
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

    // globo de dialogo: tocando el retrato, el personaje te ensena (MAESTRO)
    {
        Path b;
        b.addRoundedRectangle (bubble, 10.0f);
        b.addTriangle (bubble.getX() + 6.0f, bubble.getBottom() - 6.0f, bubble.getX() + 18.0f, bubble.getBottom() - 2.0f,
                       bubble.getX() - 4.0f, bubble.getBottom() + 9.0f);
        g.setColour (Colours::black.withAlpha (0.5f));
        g.fillPath (b, AffineTransform::translation (1.5f, 2.0f));
        g.setColour (teaching ? theme::mustardHi : theme::cream);
        g.fillPath (b);
        g.setColour (theme::ink);
        g.strokePath (b, PathStrokeType (1.6f));
        for (int i = 0; i < 3; ++i)
            g.fillEllipse (bubble.getCentreX() - 12.0f + (float) i * 10.0f, bubble.getCentreY() - 3.0f, 5.5f, 5.5f);
    }

    // CHARACTER: el valor se nombra arriba de la perilla
    const String desc = characterDescriptor ((float) knob.getValue());
    theme::drawPrintedText (g, desc, 79.0f, 273.0f, theme::fitHeight (desc, 72.0f, 25.0f), 0.0f,
                            theme::mustard, theme::black, 0.3f);
}
}
