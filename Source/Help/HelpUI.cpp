#include "HelpUI.h"
#include "../UI/Theme.h"

namespace flipper
{
using namespace juce;

//============================================================================== tooltip
String HelpTooltipWindow::getTipFor (Component& c)
{
    for (Component* comp = &c; comp != nullptr; comp = comp->getParentComponent())
    {
        if (auto* p = dynamic_cast<help::Provider*> (comp))
        {
            const auto key = p->helpKeyAt (comp->getMouseXYRelative());
            if (key.isNotEmpty()) return help::tooltip (key);
        }
        const auto key = comp->getProperties()["help"].toString();
        if (key.isNotEmpty()) return help::tooltip (key);
    }
    return {};
}

//============================================================================== boton ?
void HelpButton::paintButton (Graphics& g, bool highlighted, bool down)
{
    auto r = getLocalBounds().toFloat().reduced (3.0f);
    g.setColour (Colours::black.withAlpha (0.6f));
    g.fillEllipse (r.translated (1.5f, 2.5f));
    g.setGradientFill (ColourGradient (Colour (0xfff0d28a), r.getX(), r.getY(), Colour (0xff6b4a1e), r.getRight(), r.getBottom(), false));
    g.fillEllipse (r);
    auto inner = r.reduced (r.getWidth() * 0.14f);
    g.setGradientFill (ColourGradient (down ? Colour (0xff2a1a0c) : Colour (0xff3a2612), inner.getX(), inner.getY(),
                                       Colour (0xff120a05), inner.getX(), inner.getBottom(), false));
    g.fillEllipse (inner);
    if (highlighted)
    {
        g.setColour (theme::mustardHi.withAlpha (0.35f));
        g.drawEllipse (r.expanded (2.0f), 2.0f);
    }
    theme::drawPrintedText (g, "?", r.getCentreX(), r.getCentreY() + (down ? 1.0f : 0.0f), r.getHeight() * 0.42f, 0.0f,
                            theme::mustardHi, theme::black, 0.0f);
}

//============================================================================== recorrido
void TourOverlay::start (const std::vector<help::Step>& s)
{
    if (s.empty()) return;
    steps = &s;
    index = 0;
    setVisible (true);
    toFront (false);
    setWantsKeyboardFocus (true);
    layout();
    repaint();
}

void TourOverlay::close()
{
    steps = nullptr;
    setVisible (false);
    if (onClose) onClose();
}

void TourOverlay::layout()
{
    const auto& st = (*steps)[(size_t) index];
    hole = (resolve ? resolve (st.target) : Rectangle<int>()).toFloat();
    const bool whole = hole.getWidth() >= (float) getWidth() - 4.0f;
    if (whole) hole = {};

    const float cw = 600.0f, ch = 250.0f;
    const auto bounds = getLocalBounds().toFloat().reduced (16.0f);
    float y;
    if (hole.isEmpty()) y = bounds.getCentreY() - ch * 0.5f;
    else if (hole.getBottom() + 24.0f + ch < bounds.getBottom()) y = hole.getBottom() + 24.0f;
    else if (hole.getY() - 24.0f - ch > bounds.getY()) y = hole.getY() - 24.0f - ch;
    else y = bounds.getCentreY() - ch * 0.5f;
    const float cx = jlimit (bounds.getX() + cw * 0.5f, bounds.getRight() - cw * 0.5f, hole.isEmpty() ? bounds.getCentreX() : hole.getCentreX());
    card = { cx - cw * 0.5f, y, cw, ch };
    const float by = card.getBottom() - 54.0f;
    prevBtn = { card.getX() + 24.0f, by, 140.0f, 36.0f };
    nextBtn = { card.getRight() - 184.0f, by, 160.0f, 36.0f };
    exitBtn = { card.getCentreX() - 60.0f, by, 120.0f, 36.0f };
}

void TourOverlay::paint (Graphics& g)
{
    if (steps == nullptr) return;
    const auto& st = (*steps)[(size_t) index];

    // fondo oscuro con la zona iluminada
    Path shade;
    shade.addRectangle (getLocalBounds().toFloat());
    if (! hole.isEmpty()) shade.addRoundedRectangle (hole.expanded (6.0f), 12.0f);
    shade.setUsingNonZeroWinding (false);
    g.setColour (Colours::black.withAlpha (0.68f));
    g.fillPath (shade);
    if (! hole.isEmpty())
    {
        g.setColour (theme::mustardHi.withAlpha (0.25f));
        g.drawRoundedRectangle (hole.expanded (10.0f), 14.0f, 8.0f);
        g.setColour (theme::mustardHi);
        g.drawRoundedRectangle (hole.expanded (6.0f), 12.0f, 3.0f);
    }

    // cartel
    g.setColour (Colours::black.withAlpha (0.6f));
    g.fillRoundedRectangle (card.translated (5, 8), 14.0f);
    g.setGradientFill (ColourGradient (Colour (0xff2a1f17), card.getX(), card.getY(), Colour (0xff100b08), card.getX(), card.getBottom(), false));
    g.fillRoundedRectangle (card, 14.0f);
    g.setColour (theme::gold);
    g.drawRoundedRectangle (card.reduced (3.0f), 12.0f, 3.0f);
    g.setColour (Colour (0xff6b5638));
    g.drawRoundedRectangle (card.reduced (9.0f), 9.0f, 1.2f);
    for (auto p : { card.getTopLeft() + Point<float> (18, 18), card.getTopRight() + Point<float> (-18, 18) })
    {
        g.setGradientFill (ColourGradient (Colour (0xffd8c39a), p.x - 3, p.y - 3, Colour (0xff4a3a28), p.x + 4, p.y + 4, false));
        g.fillEllipse (p.x - 6, p.y - 6, 12, 12);
    }

    const String title = i18n::pick (st.titleEs, st.titleEn);
    theme::drawPrintedText (g, title, card.getCentreX(), card.getY() + 38.0f, theme::fitHeight (title, card.getWidth() - 90.0f, 26.0f), 0.0f,
                            theme::mustardHi, theme::black, 0.2f);
    theme::drawPrintedText (g, String (index + 1) + " / " + String ((int) steps->size()), card.getRight() - 46.0f, card.getY() + 38.0f, 13.0f, 0.0f,
                            theme::cream.withAlpha (0.6f), theme::black, 0.0f, false);

    AttributedString body;
    body.append (i18n::pick (st.textEs, st.textEn), theme::font (15.5f), theme::cream);
    body.setJustification (Justification::centred);
    body.setLineSpacing (3.0f);
    TextLayout tl;
    tl.createLayout (body, card.getWidth() - 70.0f);
    tl.draw (g, Rectangle<float> (card.getX() + 35.0f, card.getY() + 64.0f, card.getWidth() - 70.0f, card.getHeight() - 130.0f));

    auto button = [&] (Rectangle<float> r, const String& text, bool lit, bool enabled)
    {
        g.setColour (Colours::black.withAlpha (0.5f)); g.fillRoundedRectangle (r.translated (1, 2), 6.0f);
        g.setGradientFill (lit ? ColourGradient (theme::mustardHi, r.getX(), r.getY(), Colour (0xffc98314), r.getX(), r.getBottom(), false)
                               : ColourGradient (Colour (0xff3a2e24), r.getX(), r.getY(), Colour (0xff17110d), r.getX(), r.getBottom(), false));
        g.fillRoundedRectangle (r, 6.0f);
        g.setColour (theme::gold.withAlpha (enabled ? 0.9f : 0.3f)); g.drawRoundedRectangle (r, 6.0f, 1.3f);
        theme::drawPrintedText (g, text, r.getCentreX(), r.getCentreY(), 15.0f, 0.0f,
                                lit ? theme::ink : theme::cream.withAlpha (enabled ? 1.0f : 0.3f), theme::black, 0.0f, ! lit);
    };
    const bool last = index == (int) steps->size() - 1;
    button (prevBtn, "ANTERIOR", false, index > 0);
    button (exitBtn, "SALIR", false, true);
    button (nextBtn, last ? "TERMINAR" : "SIGUIENTE", true, true);
}

void TourOverlay::mouseDown (const MouseEvent& e)
{
    if (steps == nullptr) return;
    if (nextBtn.contains (e.position)) { next(); return; }
    else if (prevBtn.contains (e.position) && index > 0) --index;
    else if (exitBtn.contains (e.position)) { close(); return; }
    else return;
    layout();
    repaint();
}

void TourOverlay::next()
{
    if (index == (int) steps->size() - 1) { close(); return; }
    ++index;
    layout();
    repaint();
}

bool TourOverlay::keyPressed (const KeyPress& k)
{
    if (steps == nullptr) return false;
    if (k == KeyPress::escapeKey) { close(); return true; }
    if (k == KeyPress::rightKey || k == KeyPress::returnKey) { next(); return true; }
    if (k == KeyPress::leftKey && index > 0) { --index; layout(); repaint(); return true; }
    return false;
}
}
