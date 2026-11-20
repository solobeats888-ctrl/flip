#include "Controls.h"

namespace flipper
{
using namespace juce;

static constexpr float startAngle = -MathConstants<float>::pi * 0.75f;
static constexpr float sweep      =  MathConstants<float>::pi * 1.5f;

//============================================================================== SpriteKnob
SpriteKnob::SpriteKnob (const char* res, float fr, float ar, String l)
    : sprite (theme::image (res)), faceRadius (fr), arcRadius (ar), label (std::move (l))
{
    setSliderStyle (RotaryHorizontalVerticalDrag);
    setTextBoxStyle (NoTextBox, true, 0, 0);
    setRotaryParameters (MathConstants<float>::pi * 1.25f, MathConstants<float>::pi * 2.75f, true);
    setMouseDragSensitivity (220);
    setMouseCursor (MouseCursor::UpDownLeftRightResizeCursor);
}

void SpriteKnob::mouseEnter (const MouseEvent& e) { hovering = true;  repaint(); Slider::mouseEnter (e); }
void SpriteKnob::mouseExit  (const MouseEvent& e) { hovering = false; repaint(); Slider::mouseExit (e); }

void SpriteKnob::paint (Graphics& g)
{
    const float v = (float) valueToProportionOfLength (getValue());
    const float angle = startAngle + v * sweep;
    const auto c = centre;

    // arco de valor (sobre el anillo pintado)
    {
        Path bg; bg.addCentredArc (c.x, c.y, arcRadius, arcRadius, 0.0f, startAngle, startAngle + sweep, true);
        g.setColour (Colours::black.withAlpha (0.55f));
        g.strokePath (bg, PathStrokeType (3.2f, PathStrokeType::curved, PathStrokeType::rounded));

        Path arc; arc.addCentredArc (c.x, c.y, arcRadius, arcRadius, 0.0f, startAngle, angle, true);
        g.setColour (theme::mustard.withAlpha (0.25f));
        g.strokePath (arc, PathStrokeType (6.0f, PathStrokeType::curved, PathStrokeType::rounded));
        g.setColour (theme::mustardHi);
        g.strokePath (arc, PathStrokeType (2.2f, PathStrokeType::curved, PathStrokeType::rounded));
    }

    // textura metalica girando con el valor
    if (sprite.isValid())
    {
        const float s = faceRadius * 2.0f / (float) sprite.getWidth();
        g.setImageResamplingQuality (Graphics::highResamplingQuality);
        g.drawImageTransformed (sprite,
                                AffineTransform::translation (-sprite.getWidth() * 0.5f, -sprite.getHeight() * 0.5f)
                                    .scaled (s)
                                    .rotated (angle)
                                    .translated (c.x, c.y));
    }

    // indicador
    {
        const auto p1 = c.getPointOnCircumference (faceRadius * 0.38f, angle);
        const auto p2 = c.getPointOnCircumference (faceRadius * 0.92f, angle);
        g.setColour (Colours::black.withAlpha (0.7f));
        g.drawLine ({ p1, p2 }, 4.0f);
        g.setColour (theme::cream);
        g.drawLine ({ p1, p2 }, 2.0f);
    }

    if (label.isNotEmpty())
    {
        const bool showValue = hovering || isMouseButtonDown();
        const String txt = showValue ? getTextFromValue (getValue()) : label;
        const float cy = c.y + labelOffset;
        // misma altura para todas las macros (la que entra en la etiqueta mas larga)
        static const float labelH = theme::fitHeight (String::fromUTF8 ("LIBERACIÓN"), 90.0f, 19.0f);
        float h = labelCap > 0.0f ? labelCap : (showValue ? 19.0f : labelH);
        h = theme::fitHeight (txt, (float) getWidth() - 2.0f, h);
        if (valueLine)
        {
            theme::drawPrintedText (g, label, c.x, cy, theme::fitHeight (label, (float) getWidth() - 2.0f, h), 0.0f,
                                    theme::mustard, theme::black, 0.3f);
            const auto v = getTextFromValue (getValue());
            theme::drawPrintedText (g, v, c.x, cy + h + 7.0f, theme::fitHeight (v, (float) getWidth() - 4.0f, h * 0.8f), 0.0f,
                                    showValue ? theme::mustardHi : theme::cream.withAlpha (0.75f), theme::black, 0.0f, false);
        }
        else
            theme::drawPrintedText (g, txt, c.x, cy, h, 0.0f,
                                    showValue ? theme::mustardHi : theme::mustard, theme::black, showValue ? 0.0f : 0.3f);
    }
}

//============================================================================== EngineLed
void EngineLed::paintButton (Graphics& g, bool highlighted, bool)
{
    const auto c = getLocalBounds().toFloat().getCentre();
    const float r = 13.5f;

    if (! getToggleState())
    {
        // vidrio apagado encima del LED pintado
        ColourGradient grad (Colour (0xff3a2e26), c.x - r * 0.3f, c.y - r * 0.4f,
                             Colour (0xff0d0907), c.x + r * 0.5f, c.y + r * 0.7f, true);
        g.setGradientFill (grad);
        g.fillEllipse (c.x - r, c.y - r, r * 2, r * 2);
        g.setColour (Colours::white.withAlpha (0.18f));
        g.fillEllipse (c.x - r * 0.45f, c.y - r * 0.6f, r * 0.55f, r * 0.35f);
        g.setColour (Colour (0xff8a7a66).withAlpha (0.7f));
        g.drawEllipse (c.x - r, c.y - r, r * 2, r * 2, 1.4f);
    }
    else if (activity > 0.01f)
    {
        const float gr = r * (1.4f + activity * 0.5f);
        g.setGradientFill (ColourGradient (Colours::white.withAlpha (0.45f * activity), c.x, c.y,
                                           Colours::transparentWhite, c.x + gr, c.y, true));
        g.fillEllipse (c.x - gr, c.y - gr, gr * 2, gr * 2);
    }

    if (highlighted)
    {
        g.setColour (theme::cream.withAlpha (0.35f));
        g.drawEllipse (c.x - r - 2, c.y - r - 2, r * 2 + 4, r * 2 + 4, 1.2f);
    }
}

//============================================================================== LevelPill
LevelPill::LevelPill()
{
    setSliderStyle (LinearHorizontal);
    setTextBoxStyle (NoTextBox, true, 0, 0);
    setSliderSnapsToMousePosition (false);
    setMouseCursor (MouseCursor::LeftRightResizeCursor);
}

void LevelPill::paint (Graphics& g)
{
    auto b = getLocalBounds().toFloat().reduced (1.0f);
    const float rad = b.getHeight() * 0.5f;
    const float v = (float) valueToProportionOfLength (getValue());

    // carcasa
    g.setColour (Colour (0xff0c0907));
    g.fillRoundedRectangle (b, rad);
    ColourGradient rim (Colour (0xffb9a383), b.getX(), b.getY(), Colour (0xff3b3026), b.getX(), b.getBottom(), false);
    g.setGradientFill (rim);
    g.drawRoundedRectangle (b, rad, 1.6f);

    auto inner = b.reduced (3.0f);
    const float ir = inner.getHeight() * 0.5f;

    // relleno de nivel
    if (v > 0.001f)
    {
        auto fill = inner.withWidth (jmax (inner.getHeight(), inner.getWidth() * v));
        const auto top = enabledLook ? theme::mustardHi : Colour (0xff6d5a38);
        const auto bottom = enabledLook ? Colour (0xffc98314) : Colour (0xff3e3220);
        g.setGradientFill (ColourGradient (top, fill.getX(), fill.getY(), bottom, fill.getX(), fill.getBottom(), false));
        g.fillRoundedRectangle (fill, ir);

        Graphics::ScopedSaveState s (g);
        Path clip; clip.addRoundedRectangle (fill, ir);
        g.reduceClipRegion (clip);
        g.setColour (Colours::black.withAlpha (0.22f));
        for (float x = fill.getX() + 3.0f; x < fill.getRight(); x += 3.0f)
            g.drawVerticalLine ((int) x, fill.getY(), fill.getBottom());
    }

    // brillo del vidrio
    g.setColour (Colours::white.withAlpha (0.12f));
    g.fillRoundedRectangle (inner.withHeight (inner.getHeight() * 0.4f).reduced (2.0f, 0.0f), ir * 0.6f);
}

//============================================================================== ArcadeButton
ArcadeButton::ArcadeButton (const String& t, Style s, Rectangle<float> area)
    : Button (t), text (t), style (s), textArea (area)
{
    setMouseCursor (MouseCursor::PointingHandCursor);
}

void ArcadeButton::flash()
{
    flashStart = Time::getMillisecondCounter();
    startTimerHz (30);
    repaint();
}

void ArcadeButton::timerCallback()
{
    repaint();
    if (Time::getMillisecondCounter() - flashStart > 750) stopTimer();
}

void ArcadeButton::paintButton (Graphics& g, bool highlighted, bool down)
{
    const auto b = getLocalBounds().toFloat();

    if (down)
    {
        g.setColour (Colours::black.withAlpha (0.35f));
        g.fillRoundedRectangle (b.reduced (2.0f), 6.0f);
    }
    else if (highlighted)
    {
        g.setColour ((style == Style::lit ? Colours::white : theme::mustard).withAlpha (0.10f));
        g.fillRoundedRectangle (b.reduced (2.0f), 6.0f);
    }

    const float since = (float) (Time::getMillisecondCounter() - flashStart);
    if (flashStart != 0 && since < 700.0f)
    {
        const float a = 1.0f - since / 700.0f;
        g.setColour (theme::mustardHi.withAlpha (0.35f * a));
        g.fillRoundedRectangle (b.reduced (2.0f), 6.0f);
    }

    const float h = theme::fitHeight (text, textArea.getWidth(), textArea.getHeight());
    const auto col = style == Style::lit ? theme::ink : theme::cream;
    theme::drawPrintedText (g, text, textArea.getCentreX(), textArea.getCentreY() + (down ? 1.0f : 0.0f), h, 0.0f, col,
                            style == Style::lit ? theme::mustard : theme::black, 0.25f, style != Style::lit);
}

//============================================================================== ModeButton
ModeButton::ModeButton (const String& t, Point<float> lc, float lr, Rectangle<float> ta)
    : Button (t), text (t), ledCentre (lc), ledRadius (lr), textArea (ta), ledOn (theme::image ("mode_led_on_png"))
{
    setClickingTogglesState (false);
    setMouseCursor (MouseCursor::PointingHandCursor);
}

void ModeButton::paintButton (Graphics& g, bool highlighted, bool down)
{
    const bool on = getToggleState();
    if (on && ledOn.isValid())
    {
        const float r = ledRadius * 1.25f;
        // halo
        g.setGradientFill (ColourGradient (Colour (0xffff3a2a).withAlpha (0.45f), ledCentre.x, ledCentre.y,
                                           Colours::transparentBlack, ledCentre.x + r * 1.9f, ledCentre.y, true));
        g.fillEllipse (ledCentre.x - r * 1.9f, ledCentre.y - r * 1.9f, r * 3.8f, r * 3.8f);
        g.setImageResamplingQuality (Graphics::highResamplingQuality);
        g.drawImage (ledOn, Rectangle<float> (ledCentre.x - r, ledCentre.y - r, r * 2, r * 2));
    }
    else if (highlighted)
    {
        g.setColour (theme::mustard.withAlpha (0.25f));
        g.drawEllipse (ledCentre.x - ledRadius - 2, ledCentre.y - ledRadius - 2, ledRadius * 2 + 4, ledRadius * 2 + 4, 1.5f);
    }

    const float h = theme::fitHeight (text, textArea.getWidth(), textArea.getHeight());
    theme::drawPrintedText (g, text, textArea.getCentreX(), textArea.getCentreY() + (down ? 1.0f : 0.0f), h, 0.0f,
                            on ? theme::mustardHi : theme::cream, theme::black, 0.3f);
}

//============================================================================== LabButton
static void drawPlate (Graphics& g, Rectangle<float> b, bool lit, bool highlighted, bool down)
{
    g.setColour (Colours::black.withAlpha (0.5f));
    g.fillRoundedRectangle (b.translated (1.0f, 2.0f), 5.0f);
    if (lit)
        g.setGradientFill (ColourGradient (theme::mustardHi, b.getX(), b.getY(), Colour (0xffc98314), b.getX(), b.getBottom(), false));
    else
        g.setGradientFill (ColourGradient (Colour (0xff2c221a), b.getX(), b.getY(), Colour (0xff120d0a), b.getX(), b.getBottom(), false));
    g.fillRoundedRectangle (b, 5.0f);
    g.setGradientFill (ColourGradient (Colour (0xffd2bb8e), b.getX(), b.getY(), Colour (0xff4b3a28), b.getX(), b.getBottom(), false));
    g.drawRoundedRectangle (b.reduced (0.6f), 5.0f, 1.4f);
    if (highlighted && ! down)
    {
        g.setColour ((lit ? Colours::white : theme::mustard).withAlpha (0.12f));
        g.fillRoundedRectangle (b, 5.0f);
    }
    if (down)
    {
        g.setColour (Colours::black.withAlpha (0.3f));
        g.fillRoundedRectangle (b, 5.0f);
    }
}

LabButton::LabButton (const String& t, bool l) : Button (t), lit (l)
{
    setMouseCursor (MouseCursor::PointingHandCursor);
}

void LabButton::paintButton (Graphics& g, bool highlighted, bool down)
{
    const bool on = lit || getToggleState();
    auto b = getLocalBounds().toFloat().reduced (1.5f);
    drawPlate (g, b, on, highlighted, down);
    const auto t = getButtonText();
    const float h = theme::fitHeight (t, b.getWidth() - 12.0f, jmin (17.0f, b.getHeight() * 0.48f));
    theme::drawPrintedText (g, t, b.getCentreX(), b.getCentreY() + (down ? 1.0f : 0.0f), h, 0.0f,
                            on ? theme::ink : (isEnabled() ? theme::cream : theme::cream.withAlpha (0.35f)),
                            on ? theme::mustard : theme::black, 0.2f, ! on);
}

//============================================================================== LabToggle
LabToggle::LabToggle (const String& t) : ToggleButton (t)
{
    setMouseCursor (MouseCursor::PointingHandCursor);
}

void LabToggle::paintButton (Graphics& g, bool highlighted, bool)
{
    const auto b = getLocalBounds().toFloat();
    const float r = jmin (9.0f, b.getHeight() * 0.32f);
    const Point<float> c (b.getX() + r + 4.0f, b.getCentreY());
    const bool on = getToggleState();

    if (on)
    {
        g.setGradientFill (ColourGradient (theme::mustardHi.withAlpha (0.5f), c.x, c.y, Colours::transparentBlack, c.x + r * 2.4f, c.y, true));
        g.fillEllipse (c.x - r * 2.4f, c.y - r * 2.4f, r * 4.8f, r * 4.8f);
    }
    g.setGradientFill (ColourGradient (on ? Colour (0xfffff0b0) : Colour (0xff3a2e26), c.x - r * 0.3f, c.y - r * 0.4f,
                                       on ? Colour (0xffd88a10) : Colour (0xff0d0907), c.x + r * 0.6f, c.y + r * 0.7f, true));
    g.fillEllipse (c.x - r, c.y - r, r * 2, r * 2);
    g.setColour (Colour (0xffb9a383).withAlpha (highlighted ? 1.0f : 0.7f));
    g.drawEllipse (c.x - r, c.y - r, r * 2, r * 2, 1.3f);

    const auto t = getButtonText();
    const float h = theme::fitHeight (t, b.getWidth() - r * 2 - 14.0f, jmin (15.0f, b.getHeight() * 0.5f));
    const float w = theme::textWidth (t, h);
    theme::drawPrintedText (g, t, c.x + r + 8.0f + w * 0.5f, c.y, h, 0.0f,
                            on ? theme::mustardHi : theme::cream, theme::black, 0.15f);
}

//============================================================================== LookAndFeel
void FlipperLookAndFeel::drawComboBox (Graphics& g, int width, int height, bool, int, int, int, int, ComboBox& box)
{
    auto b = Rectangle<float> (0, 0, (float) width, (float) height).reduced (1.0f);
    g.setColour (Colour (0xff0e0a08));
    g.fillRoundedRectangle (b, 4.0f);
    g.setColour (box.hasKeyboardFocus (true) ? theme::mustard : Colour (0xff7d6646));
    g.drawRoundedRectangle (b.reduced (0.5f), 4.0f, 1.3f);

    const float ax = b.getRight() - 16.0f, ay = b.getCentreY();
    Path arrow; arrow.addTriangle (ax - 5.0f, ay - 3.0f, ax + 5.0f, ay - 3.0f, ax, ay + 4.0f);
    g.setColour (theme::mustard);
    g.fillPath (arrow);
}

void FlipperLookAndFeel::positionComboBoxText (ComboBox& box, Label& label)
{
    label.setBounds (6, 1, box.getWidth() - 30, box.getHeight() - 2);
    label.setFont (getComboBoxFont (box));
}

void FlipperLookAndFeel::fillTextEditorBackground (Graphics& g, int width, int height, TextEditor&)
{
    g.setColour (Colour (0xff0e0a08));
    g.fillRoundedRectangle (Rectangle<float> (0, 0, (float) width, (float) height).reduced (1.0f), 4.0f);
}

void FlipperLookAndFeel::drawTextEditorOutline (Graphics& g, int width, int height, TextEditor& ed)
{
    g.setColour (ed.hasKeyboardFocus (true) ? theme::mustard : Colour (0xff7d6646));
    g.drawRoundedRectangle (Rectangle<float> (0, 0, (float) width, (float) height).reduced (1.5f), 4.0f, 1.3f);
}

TextLayout FlipperLookAndFeel::tooltipLayout (const String& text, float width)
{
    AttributedString s;
    s.append (text.upToFirstOccurrenceOf ("\n", false, false) + "\n", theme::font (13.0f), theme::mustardHi);
    s.append (text.fromFirstOccurrenceOf ("\n", false, false), theme::font (10.5f), theme::cream);
    s.setLineSpacing (2.0f);
    TextLayout tl;
    tl.createLayout (s, width);
    return tl;
}

Rectangle<int> FlipperLookAndFeel::getTooltipBounds (const String& text, Point<int> pos, Rectangle<int> area)
{
    const auto tl = tooltipLayout (text, 250.0f);
    const int w = (int) std::ceil (jmin (250.0f, tl.getWidth())) + 26, h = (int) std::ceil (tl.getHeight()) + 20;
    return Rectangle<int> (pos.x > area.getCentreX() ? pos.x - (w + 12) : pos.x + 18,
                           pos.y > area.getCentreY() ? pos.y - (h + 8) : pos.y + 20, w, h).constrainedWithin (area);
}

void FlipperLookAndFeel::drawTooltip (Graphics& g, const String& text, int w, int h)
{
    auto r = Rectangle<float> (0, 0, (float) w, (float) h);
    g.setGradientFill (ColourGradient (Colour (0xf52a1f17), 0, 0, Colour (0xf5100b08), 0, (float) h, false));
    g.fillRoundedRectangle (r, 6.0f);
    g.setColour (theme::gold);
    g.drawRoundedRectangle (r.reduced (1.0f), 6.0f, 1.5f);
    tooltipLayout (text, 250.0f).draw (g, r.reduced (13.0f, 10.0f));
}

FlipperLookAndFeel::FlipperLookAndFeel()
{
    setColour (ComboBox::textColourId, theme::cream);
    setColour (ComboBox::backgroundColourId, Colour (0xff0e0a08));
    setColour (TextEditor::textColourId, theme::cream);
    setColour (TextEditor::highlightColourId, theme::mustard.withAlpha (0.4f));
    setColour (TextEditor::highlightedTextColourId, theme::cream);
    setColour (CaretComponent::caretColourId, theme::mustardHi);
    setColour (ListBox::backgroundColourId, Colours::transparentBlack);
    setColour (ScrollBar::thumbColourId, theme::gold.withAlpha (0.6f));
    setColour (PopupMenu::backgroundColourId, Colour (0xff17110d));
    setColour (PopupMenu::textColourId, theme::cream);
    setColour (PopupMenu::highlightedBackgroundColourId, theme::mustard);
    setColour (PopupMenu::highlightedTextColourId, theme::ink);
    setColour (PopupMenu::headerTextColourId, theme::mustard);
}
}
