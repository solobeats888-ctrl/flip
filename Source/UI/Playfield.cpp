#include "Playfield.h"
#include "Theme.h"

namespace flipper
{
using namespace juce;

// ancho de cada placa pintada (para luces y brillo)
static constexpr std::array<float, numEngines> plateFrameW { 122.f, 156.f, 142.f, 158.f, 112.f, 146.f, 164.f, 190.f, 156.f };

Playfield::Playfield()
{
    setRepaintsOnMouseActivity (false);
}

void Playfield::setSelected (int e)
{
    if (e == selected) return;
    selected = e;
    repaint();
}

void Playfield::setEngineState (int e, EngineState s)
{
    auto& cur = states[(size_t) e];
    if (cur.on == s.on && std::abs (cur.activity - s.activity) < 0.02f) { cur.level = s.level; return; }
    cur = s;
    repaint (engines()[(size_t) e].hotspot.getSmallestIntegerContainer().expanded (40));
}

int Playfield::engineAt (Point<float> p) const
{
    int best = -1; float bestD = 1.0f;
    for (int e = 0; e < numEngines; ++e)
    {
        const auto& h = engines()[(size_t) e].hotspot;
        const float dx = (p.x - h.getCentreX()) / (h.getWidth() * 0.5f);
        const float dy = (p.y - h.getCentreY()) / (h.getHeight() * 0.5f);
        const float d = dx * dx + dy * dy;
        if (d < bestD) { bestD = d; best = e; }
    }
    return best;
}

bool Playfield::hitTest (int x, int y) { return engineAt ({ (float) x, (float) y }) >= 0; }

void Playfield::mouseMove (const MouseEvent& e)
{
    const int h = engineAt (e.position);
    if (h != hovered) { hovered = h; repaint(); }
    setMouseCursor (h >= 0 ? MouseCursor::PointingHandCursor : MouseCursor::NormalCursor);
}

void Playfield::mouseExit (const MouseEvent&)
{
    if (hovered != -1) { hovered = -1; repaint(); }
}

void Playfield::mouseUp (const MouseEvent& e)
{
    const int h = engineAt (e.position);
    if (h >= 0 && e.mouseWasClicked())
    {
        setSelected (h);
        if (onSelect) onSelect (h);
    }
}

void Playfield::paint (Graphics& g)
{
    // primero las sombras de los apagados, despues los iluminados (que quedan arriba)
    for (int e = 0; e < numEngines; ++e)
        if (! states[(size_t) e].on) paintEngine (g, e);
    for (int e = 0; e < numEngines; ++e)
        if (states[(size_t) e].on) paintEngine (g, e);
}

void Playfield::paintEngine (Graphics& g, int e)
{
    const auto& info = engines()[(size_t) e];
    const auto& st = states[(size_t) e];
    const auto hs = info.hotspot;
    const bool isSel = e == selected;
    const bool isHover = e == hovered;
    const Point<float> plate (info.plateX, info.plateY);
    const float fw = plateFrameW[(size_t) e];

    if (! st.on)
    {
        // personaje en sombra (no desaparece)
        g.setGradientFill (ColourGradient (Colours::black.withAlpha (0.62f), hs.getCentreX(), hs.getCentreY(),
                                           Colours::black.withAlpha (0.0f), hs.getCentreX() + hs.getWidth() * 0.62f, hs.getCentreY(), true));
        g.fillEllipse (hs.expanded (hs.getWidth() * 0.12f, hs.getHeight() * 0.06f));
    }

    if (isSel)
    {
        // foco calido sobre el personaje elegido
        g.setGradientFill (ColourGradient (theme::mustardHi.withAlpha (st.on ? 0.20f : 0.10f), hs.getCentreX(), hs.getCentreY(),
                                           theme::mustardHi.withAlpha (0.0f), hs.getCentreX() + hs.getWidth() * 0.6f, hs.getCentreY(), true));
        g.fillEllipse (hs.expanded (hs.getWidth() * 0.1f, hs.getHeight() * 0.05f));
    }

    // brillo de la placa: seleccionado, hover o actividad
    float glow = isSel ? 0.75f : (isHover ? 0.35f : 0.0f);
    if (st.on) glow = jmax (glow, st.activity * 0.5f);
    if (glow > 0.0f)
    {
        g.setGradientFill (ColourGradient (theme::mustardHi.withAlpha (0.55f * glow), plate.x, plate.y,
                                           theme::mustardHi.withAlpha (0.0f), plate.x + fw * 0.7f, plate.y, true));
        g.fillEllipse (plate.x - fw * 0.7f, plate.y - 38.0f, fw * 1.4f, 76.0f);
    }

    // lamparitas en los extremos de la placa
    if (isSel)
    {
        const float a = degreesToRadians (info.plateAngle);
        for (float side : { -1.0f, 1.0f })
        {
            const Point<float> p (plate.x + std::cos (a) * side * (fw * 0.5f - 7.0f),
                                  plate.y + std::sin (a) * side * (fw * 0.5f - 7.0f));
            g.setGradientFill (ColourGradient (Colours::white.withAlpha (0.95f), p.x, p.y,
                                               Colour (0xffffb020).withAlpha (0.0f), p.x + 13.0f, p.y, true));
            g.fillEllipse (p.x - 13.0f, p.y - 13.0f, 26.0f, 26.0f);
        }
    }

    // la placa ya trae el nombre del sintetizador en el arte aprobado

    if (! st.on)
    {
        // placa apagada
        Graphics::ScopedSaveState s (g);
        g.addTransform (AffineTransform::rotation (degreesToRadians (info.plateAngle), plate.x, plate.y));
        g.setColour (Colours::black.withAlpha (0.38f));
        g.fillRoundedRectangle (plate.x - fw * 0.5f, plate.y - 19.0f, fw, 38.0f, 6.0f);
    }
}

//============================================================================== ModeNotice
void ModeNotice::setContent (UiMode m, int e)
{
    mode = m; engine = e;
    setVisible (m != UiMode::play);
    repaint();
}

void ModeNotice::paint (Graphics& g)
{
    auto b = getLocalBounds().toFloat().reduced (4.0f);
    g.setColour (Colours::black.withAlpha (0.55f));
    g.fillRoundedRectangle (b.translated (4, 6), 14.0f);
    g.setGradientFill (ColourGradient (Colour (0xf0221913), b.getX(), b.getY(), Colour (0xf00f0b08), b.getX(), b.getBottom(), false));
    g.fillRoundedRectangle (b, 14.0f);
    g.setColour (theme::gold.withAlpha (0.85f));
    g.drawRoundedRectangle (b.reduced (3.0f), 12.0f, 3.0f);
    g.setColour (Colour (0xff6b5638));
    g.drawRoundedRectangle (b.reduced (9.0f), 9.0f, 1.2f);

    for (auto p : { b.getTopLeft() + Point<float> (18, 18), b.getTopRight() + Point<float> (-18, 18),
                    b.getBottomLeft() + Point<float> (18, -18), b.getBottomRight() + Point<float> (-18, -18) })
    {
        g.setGradientFill (ColourGradient (Colour (0xffd8c39a), p.x - 3, p.y - 3, Colour (0xff4a3a28), p.x + 4, p.y + 4, false));
        g.fillEllipse (p.x - 6, p.y - 6, 12, 12);
    }

    const auto& info = engines()[(size_t) engine];
    const String title = mode == UiMode::design ? String::fromUTF8 ("DISEÑO") : String ("LAB");
    theme::drawPrintedText (g, title, b.getCentreX(), b.getY() + 52.0f, 36.0f, 0.0f, theme::mustardHi, theme::black, 0.3f);
    theme::drawPrintedText (g, String::fromUTF8 (info.character) + "  /  " + info.technical,
                            b.getCentreX(), b.getY() + 100.0f, 22.0f, 0.0f, theme::cream, theme::black, 0.2f);

    g.setColour (theme::cream.withAlpha (0.85f));
    g.setFont (theme::font (15.0f));
    const String body = mode == UiMode::design
        ? String::fromUTF8 ("Los controles de este motor aparecen acá cuando tenga su DSP (fase 3).\n"
                            "No se muestran perillas que todavía no suenan. Presets y generador: LAB.")
        : String::fromUTF8 ("Matriz de modulación, ruteo entre motores y patch bay:\n"
                            "llegan en la fase 4. Tu sonido se mantiene al cambiar de modo.");
    g.drawFittedText (body, b.reduced (36.0f, 0.0f).withTrimmedTop (132.0f).withHeight (70.0f).toNearestInt(),
                      Justification::centred, 3);
}
}
