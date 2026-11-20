#include "VintageControls.h"

namespace flipper
{
using namespace juce;

static constexpr float startA = -MathConstants<float>::pi * 0.75f;
static constexpr float sweepA =  MathConstants<float>::pi * 1.5f;

VintageKnob::VintageKnob (String l, Style s) : label (std::move (l)), style (std::move (s))
{
    setTextBoxStyle (NoTextBox, true, 0, 0);
    if (isFader (style.look))
    {
        setSliderStyle (LinearVertical);
        setSliderSnapsToMousePosition (false);
        setMouseCursor (MouseCursor::UpDownResizeCursor);
        return;
    }
    setSliderStyle (RotaryHorizontalVerticalDrag);
    setRotaryParameters (MathConstants<float>::pi * 1.25f, MathConstants<float>::pi * 2.75f, true);
    setMouseDragSensitivity (style.positions.isEmpty() ? 220 : 120);
    setMouseCursor (MouseCursor::UpDownLeftRightResizeCursor);
}

float VintageKnob::angleFor (float p) const
{
    if (style.positions.size() > 1)
    {
        // selector: el arco de posiciones es mas corto, como en el panel original
        const float span = sweepA * jmin (1.0f, 0.18f * (float) (style.positions.size() - 1));
        return -span * 0.5f + p * span;
    }
    return startA + p * sweepA;
}

void VintageKnob::mouseDown (const MouseEvent& e)
{
    const int n = style.positions.size();
    if (n > 1 && e.position.getDistanceFrom (centre) > style.radius + 4.0f)
    {
        // click sobre un rotulo: salta a esa posicion
        const float a = std::atan2 (e.position.x - centre.x, centre.y - e.position.y);
        int best = 0; float bestD = 10.0f;
        for (int i = 0; i < n; ++i)
        {
            const float d = std::abs (angleFor ((float) i / (float) (n - 1)) - a);
            if (d < bestD) { bestD = d; best = i; }
        }
        setValue (proportionOfLengthToValue ((double) best / (double) (n - 1)), sendNotificationSync);
        return;
    }
    Slider::mouseDown (e);
}

void VintageKnob::drawWave (Graphics& g, int wave, Rectangle<float> r, Colour c, float th)
{
    Path p;
    const float x0 = r.getX(), x1 = r.getRight(), y0 = r.getY(), y1 = r.getBottom(), ym = r.getCentreY(), w = r.getWidth();
    switch (wave)
    {
        case 0: p.startNewSubPath (x0, y1); p.lineTo (x0 + w * 0.25f, y0); p.lineTo (x0 + w * 0.75f, y1); p.lineTo (x1, ym); break;   // triangulo
        case 1: p.startNewSubPath (x0, y1); p.lineTo (x0 + w * 0.35f, y0); p.lineTo (x0 + w * 0.5f, y1);                                // tri-sierra
                p.lineTo (x0 + w * 0.85f, y0); p.lineTo (x1, y1); break;
        case 2: p.startNewSubPath (x0, y1); p.lineTo (x0 + w * 0.5f, y0); p.lineTo (x0 + w * 0.5f, y1); p.lineTo (x1, y0); break;     // sierra
        case 3: case 4: case 5:
        {
            const float duty = wave == 3 ? 0.5f : (wave == 4 ? 0.32f : 0.14f);
            p.startNewSubPath (x0, y1); p.lineTo (x0, y0); p.lineTo (x0 + w * duty, y0); p.lineTo (x0 + w * duty, y1);
            p.lineTo (x1, y1); p.lineTo (x1, y0);
            break;
        }
        default: break;
    }
    g.setColour (c);
    g.strokePath (p, PathStrokeType (th, PathStrokeType::mitered, PathStrokeType::square));
}

void VintageKnob::paintScale (Graphics& g)
{
    const auto c = centre;
    const float r = style.radius;
    const int n = style.positions.size();

    if (n > 1)
    {
        const int current = roundToInt (valueToProportionOfLength (getValue()) * (n - 1));
        for (int i = 0; i < n; ++i)
        {
            const float a = angleFor ((float) i / (float) (n - 1));
            const auto tick0 = c.getPointOnCircumference (r + 3.0f, a);
            const auto tick1 = c.getPointOnCircumference (r + 7.0f, a);
            const bool sel = i == current;
            g.setColour ((sel ? theme::mustardHi : theme::cream).withAlpha (sel ? 1.0f : 0.7f));
            g.drawLine ({ tick0, tick1 }, 1.5f);
            const auto lp = c.getPointOnCircumference (r + 16.0f, a);
            if (style.waveIcons)
                drawWave (g, i, Rectangle<float> (13.0f, 8.0f).withCentre (lp), sel ? theme::mustardHi : theme::cream.withAlpha (0.75f), sel ? 1.8f : 1.2f);
            else
                theme::drawPrintedText (g, style.positions[i], lp.x, lp.y, 9.0f, 0.0f,
                                        sel ? theme::mustardHi : theme::cream.withAlpha (0.75f), theme::black, 0.0f, false);
        }
        return;
    }

    // perilla continua: escala del panel (ticks + numeros)
    const int ticks = 11;
    for (int i = 0; i < ticks; ++i)
    {
        const float a = startA + sweepA * (float) i / (float) (ticks - 1);
        const bool major = i % 2 == 0;
        g.setColour (theme::cream.withAlpha (major ? 0.8f : 0.45f));
        g.drawLine ({ c.getPointOnCircumference (r + 3.0f, a), c.getPointOnCircumference (r + (major ? 7.5f : 5.5f), a) }, major ? 1.4f : 1.0f);
    }
    if (style.scale.size() > 1)
        for (int i = 0; i < style.scale.size(); ++i)
        {
            const float a = startA + sweepA * (float) i / (float) (style.scale.size() - 1);
            const auto lp = c.getPointOnCircumference (r + 15.0f, a);
            theme::drawPrintedText (g, style.scale[i], lp.x, lp.y, 8.0f, 0.0f, theme::cream.withAlpha (0.7f), theme::black, 0.0f, false);
        }

    // arco de valor fino en el color del personaje
    Path arc;
    arc.addCentredArc (c.x, c.y, r + 2.0f, r + 2.0f, 0.0f, startA, angleFor ((float) valueToProportionOfLength (getValue())), true);
    g.setColour (style.accent.withAlpha (0.9f));
    g.strokePath (arc, PathStrokeType (1.6f));
}

void VintageKnob::paintMoog (Graphics& g, float angle)
{
    const auto c = centre;
    const float r = style.radius;

    // sombra
    g.setColour (Colours::black.withAlpha (0.55f));
    g.fillEllipse (Rectangle<float> (r * 2.1f, r * 2.1f).withCentre (c.translated (1.5f, 3.0f)));

    // pollera estriada negra (borde con ranuras, como las perillas del Model D)
    Path skirt;
    const int flutes = 30;
    for (int i = 0; i <= flutes * 2; ++i)
    {
        const float a = angle + MathConstants<float>::twoPi * (float) i / (float) (flutes * 2);
        const float rr = i % 2 == 0 ? r : r * 0.94f;
        const auto p = c.getPointOnCircumference (rr, a);
        if (i == 0) skirt.startNewSubPath (p); else skirt.lineTo (p);
    }
    skirt.closeSubPath();
    g.setGradientFill (ColourGradient (Colour (0xff3a3632), c.x - r, c.y - r, Colour (0xff050505), c.x + r * 0.6f, c.y + r, false));
    g.fillPath (skirt);
    g.setColour (Colours::white.withAlpha (0.08f));
    g.strokePath (skirt, PathStrokeType (0.8f));

    // linea blanca de indice sobre la pollera
    g.setColour (Colour (0xfff2ead8));
    g.drawLine ({ c.getPointOnCircumference (r * 0.64f, angle), c.getPointOnCircumference (r * 0.97f, angle) }, 2.2f);

    // tapa metalica (aluminio cepillado)
    const float cr = r * 0.58f;
    g.setGradientFill (ColourGradient (Colour (0xfff4f1ea), c.x - cr, c.y - cr, Colour (0xff6d6a64), c.x + cr, c.y + cr, false));
    g.fillEllipse (Rectangle<float> (cr * 2, cr * 2).withCentre (c));
    for (float k : { 0.25f, 0.45f, 0.65f, 0.85f })
    {
        g.setColour (Colours::black.withAlpha (0.08f));
        g.drawEllipse (Rectangle<float> (cr * 2 * k, cr * 2 * k).withCentre (c), 0.7f);
    }
    // brillo y desgaste del metal
    g.setColour (Colours::white.withAlpha (0.35f));
    g.fillEllipse (Rectangle<float> (cr * 0.7f, cr * 0.35f).withCentre (c.translated (-cr * 0.3f, -cr * 0.45f)));
    Random wear ((int64) label.hashCode());
    for (int i = 0; i < 6; ++i)
    {
        g.setColour (Colour (0xff3a2a18).withAlpha (0.15f + wear.nextFloat() * 0.2f));
        const auto p = c.getPointOnCircumference (cr * wear.nextFloat() * 0.9f, wear.nextFloat() * 6.3f);
        g.fillEllipse (Rectangle<float> (1.2f + wear.nextFloat() * 2, 1.0f + wear.nextFloat()).withCentre (p));
    }
    g.setColour (Colours::black.withAlpha (0.5f));
    g.drawEllipse (Rectangle<float> (cr * 2, cr * 2).withCentre (c), 1.0f);

    if (hovering || isMouseButtonDown())
    {
        g.setColour (style.accent.withAlpha (0.5f));
        g.drawEllipse (Rectangle<float> (r * 2 + 3, r * 2 + 3).withCentre (c), 1.4f);
    }
}

void VintageKnob::paintRotary (Graphics& g, float angle)
{
    const auto c = centre;
    const float r = style.radius;
    g.setColour (Colours::black.withAlpha (0.55f));
    g.fillEllipse (Rectangle<float> (r * 2.1f, r * 2.1f).withCentre (c.translated (1.5f, 3.0f)));

    if (style.look == Look::prophet)
    {
        // perilla negra lisa con anillo de metal y linea blanca (Prophet-5)
        g.setGradientFill (ColourGradient (Colour (0xffb9b4aa), c.x - r, c.y - r, Colour (0xff3a3835), c.x + r, c.y + r, false));
        g.fillEllipse (Rectangle<float> (r * 2, r * 2).withCentre (c));
        g.setGradientFill (ColourGradient (Colour (0xff34312e), c.x - r, c.y - r, Colour (0xff050505), c.x + r * 0.5f, c.y + r, false));
        g.fillEllipse (Rectangle<float> (r * 1.8f, r * 1.8f).withCentre (c));
        g.setColour (Colours::white.withAlpha (0.1f));
        g.fillEllipse (Rectangle<float> (r * 0.9f, r * 0.4f).withCentre (c.translated (-r * 0.25f, -r * 0.45f)));
        g.setColour (Colour (0xfff4efe2));
        g.drawLine ({ c.getPointOnCircumference (r * 0.2f, angle), c.getPointOnCircumference (r * 0.86f, angle) }, 2.2f);
    }
    else if (style.look == Look::m1)
    {
        // perilla de goma gris con punto naranja (estilo de los controles digitales de fines de los 80)
        g.setGradientFill (ColourGradient (Colour (0xff6d7075), c.x - r, c.y - r, Colour (0xff26282b), c.x + r, c.y + r, false));
        g.fillEllipse (Rectangle<float> (r * 2, r * 2).withCentre (c));
        for (int i = 0; i < 24; ++i)
        {
            const float a = MathConstants<float>::twoPi * i / 24.0f;
            g.setColour (Colours::black.withAlpha (0.25f));
            g.drawLine ({ c.getPointOnCircumference (r * 0.86f, a), c.getPointOnCircumference (r, a) }, 1.0f);
        }
        g.setGradientFill (ColourGradient (Colour (0xff5a5d62), c.x, c.y - r * 0.6f, Colour (0xff35373a), c.x, c.y + r * 0.6f, false));
        g.fillEllipse (Rectangle<float> (r * 1.5f, r * 1.5f).withCentre (c));
        const auto dot = c.getPointOnCircumference (r * 0.55f, angle);
        g.setColour (style.accent.withAlpha (0.4f));
        g.fillEllipse (Rectangle<float> (9, 9).withCentre (dot));
        g.setColour (style.accent);
        g.fillEllipse (Rectangle<float> (5, 5).withCentre (dot));
    }
    else // vcs3: perilla chica negra con tapa de color
    {
        g.setGradientFill (ColourGradient (Colour (0xff2c2a28), c.x - r, c.y - r, Colour (0xff050505), c.x + r, c.y + r, false));
        g.fillEllipse (Rectangle<float> (r * 2, r * 2).withCentre (c));
        const float cr = r * 0.62f;
        g.setGradientFill (ColourGradient (style.cap.brighter (0.4f), c.x - cr, c.y - cr, style.cap.darker (0.5f), c.x + cr, c.y + cr, false));
        g.fillEllipse (Rectangle<float> (cr * 2, cr * 2).withCentre (c));
        g.setColour (Colours::white);
        g.drawLine ({ c.getPointOnCircumference (r * 0.25f, angle), c.getPointOnCircumference (r * 0.95f, angle) }, 2.0f);
    }
    if (hovering || isMouseButtonDown())
    {
        g.setColour (style.accent.withAlpha (0.5f));
        g.drawEllipse (Rectangle<float> (r * 2 + 3, r * 2 + 3).withCentre (c), 1.4f);
    }
}

void VintageKnob::paintFader (Graphics& g)
{
    const auto b = getLocalBounds().toFloat();
    const float top = 24.0f, bottom = b.getHeight() - 22.0f;
    const float cx = b.getCentreX();
    const float p = (float) valueToProportionOfLength (getValue());
    const float y = bottom - p * (bottom - top);

    // ranura y escala impresa
    g.setColour (Colours::black.withAlpha (0.85f));
    g.fillRoundedRectangle (Rectangle<float> (4.0f, bottom - top + 8.0f).withCentre ({ cx, (top + bottom) * 0.5f }), 2.0f);
    for (int i = 0; i <= 10; ++i)
    {
        const float ty = bottom - (bottom - top) * (float) i / 10.0f;
        const float len = i % 5 == 0 ? 7.0f : 4.0f;
        g.setColour (theme::cream.withAlpha (i % 5 == 0 ? 0.7f : 0.35f));
        g.drawHorizontalLine ((int) ty, cx - 9.0f - len, cx - 9.0f);
        g.drawHorizontalLine ((int) ty, cx + 9.0f, cx + 9.0f + len);
    }
    // valor en el color del personaje, a lo largo de la ranura
    g.setColour (style.accent.withAlpha (0.8f));
    g.fillRect (Rectangle<float> (2.0f, bottom - y).withPosition (cx - 1.0f, y));

    // tapa del fader segun la maquina
    const float w = style.look == Look::cs80 ? 26.0f : 22.0f;
    const float h = style.look == Look::cs80 ? 18.0f : 14.0f;
    auto cap = Rectangle<float> (w, h).withCentre ({ cx, y });
    g.setColour (Colours::black.withAlpha (0.5f));
    g.fillRoundedRectangle (cap.translated (1.0f, 2.0f), 2.0f);
    Colour capCol = style.cap;
    if (style.look == Look::jupiter) capCol = Colour (0xff1a1a1c);
    if (style.look == Look::dx7)     capCol = Colour (0xff2b201b);
    g.setGradientFill (ColourGradient (capCol.brighter (0.35f), cap.getX(), cap.getY(), capCol.darker (0.35f), cap.getX(), cap.getBottom(), false));
    g.fillRoundedRectangle (cap, style.look == Look::cs80 ? 4.0f : 2.0f);
    const bool lightCap = capCol.getPerceivedBrightness() > 0.5f;
    g.setColour (lightCap ? Colours::black.withAlpha (0.85f) : Colours::white.withAlpha (0.9f));
    g.drawHorizontalLine ((int) y, cap.getX() + 2.0f, cap.getRight() - 2.0f);
    if (hovering || isMouseButtonDown())
    {
        g.setColour (style.accent);
        g.drawRoundedRectangle (cap.expanded (2.0f), 3.0f, 1.2f);
    }

    const float lh = theme::fitHeight (label, b.getWidth() - 2.0f, 10.5f);
    theme::drawPrintedText (g, label, cx, 9.0f, lh, 0.0f, theme::cream, theme::black, 0.15f);
    const auto v = getTextFromValue (getValue());
    theme::drawPrintedText (g, v, cx, b.getHeight() - 8.0f, theme::fitHeight (v, b.getWidth() - 2.0f, 9.5f), 0.0f,
                            (hovering || isMouseButtonDown()) ? theme::mustardHi : style.accent.brighter (0.2f), theme::black, 0.0f, false);
}

void VintageKnob::paint (Graphics& g)
{
    if (isFader (style.look)) { paintFader (g); return; }
    const float p = (float) valueToProportionOfLength (getValue());
    paintScale (g);
    if (style.look == Look::moog) paintMoog (g, angleFor (p));
    else paintRotary (g, angleFor (p));

    // rotulo arriba (como en el panel) y valor abajo
    const float lh = theme::fitHeight (label, (float) getWidth() - 4.0f, 11.0f);
    theme::drawPrintedText (g, label, centre.x, centre.y - style.radius - (style.positions.size() > 1 ? 32.0f : 26.0f), lh, 0.0f,
                            theme::cream, theme::black, 0.2f);
    if (style.positions.isEmpty() || hovering || isMouseButtonDown())
    {
        const auto v = getTextFromValue (getValue());
        theme::drawPrintedText (g, v, centre.x, centre.y + style.radius + 24.0f, theme::fitHeight (v, (float) getWidth() - 6.0f, 10.0f), 0.0f,
                                (hovering || isMouseButtonDown()) ? theme::mustardHi : style.accent.brighter (0.3f), theme::black, 0.0f, false);
    }
}
}
