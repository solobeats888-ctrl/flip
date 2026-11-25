#include "ModUI.h"
#include "../Help/I18n.h"

namespace flipper
{
using namespace juce;

static String pk (const char* es, const char* en) { return i18n::pick (es, en); }

//============================================================================== fuente arrastrable
ModSourceChip::ModSourceChip (ModMatrix& m, int s) : source (s), matrix (m)
{
    setMouseCursor (MouseCursor::DraggingHandCursor);
    getProperties().set ("help", "ui:mod_source");
    startTimerHz (20);
}

ModSourceChip::~ModSourceChip() { stopTimer(); }

Point<float> ModSourceChip::toBoard (const MouseEvent& e) const
{
    // la etiqueta vive en el panel MODULACION, que vive en el tablero
    if (auto* panel = getParentComponent())
        if (auto* board = panel->getParentComponent())
            return board->getLocalPoint (this, e.position);
    return e.position;
}

void ModSourceChip::paint (Graphics& g)
{
    auto r = getLocalBounds().toFloat().reduced (1.5f);
    const auto col = ModMatrix::sourceColour (source);
    g.setColour (Colours::black.withAlpha (0.5f));
    g.fillRoundedRectangle (r.translated (1.0f, 2.0f), r.getHeight() * 0.5f);
    g.setGradientFill (ColourGradient (col.brighter (0.2f), r.getX(), r.getY(), col.darker (0.45f), r.getX(), r.getBottom(), false));
    g.fillRoundedRectangle (r, r.getHeight() * 0.5f);
    g.setColour (theme::ink);
    g.drawRoundedRectangle (r, r.getHeight() * 0.5f, 1.4f);

    // agarradera (cruz de flechas) + nombre + valor actual de la fuente
    const float cy = r.getCentreY();
    auto grip = Rectangle<float> (r.getX() + 6.0f, cy - 5.0f, 10.0f, 10.0f);
    g.setColour (theme::ink.withAlpha (0.85f));
    g.drawLine (grip.getCentreX(), grip.getY(), grip.getCentreX(), grip.getBottom(), 1.6f);
    g.drawLine (grip.getX(), cy, grip.getRight(), cy, 1.6f);

    const String name = ModMatrix::sourceName (source);
    const float h = theme::fitHeight (name, r.getWidth() - 48.0f, 15.0f);
    theme::drawPrintedText (g, name, r.getX() + 23.0f + theme::textWidth (name, h) * 0.5f, cy + 1.0f, h, 0.0f,
                            theme::ink, col.brighter (0.6f), 0.0f, false);

    // luz que late con la fuente
    const float v = matrix.sourceValue (source);
    const float lit = source < ModMatrix::lfo1 ? v : (v * 0.5f + 0.5f);
    const auto led = Rectangle<float> (r.getRight() - 15.0f, cy - 4.0f, 8.0f, 8.0f);
    g.setColour (theme::ink); g.fillEllipse (led.expanded (1.5f));
    g.setColour (col.brighter (0.8f).withAlpha (0.15f + 0.85f * jlimit (0.0f, 1.0f, lit))); g.fillEllipse (led);
}

void ModSourceChip::mouseDown (const MouseEvent&) { dragging = false; }

void ModSourceChip::mouseDrag (const MouseEvent& e)
{
    if (! dragging && e.getDistanceFromDragStart() < 4) return;
    dragging = true;
    if (onDrag) onDrag (source, toBoard (e));
}

void ModSourceChip::mouseUp (const MouseEvent& e)
{
    if (dragging && onDrop) onDrop (source, toBoard (e));
    dragging = false;
}

//============================================================================== selector de texto
ParamCycler::ParamCycler (RangedAudioParameter& p, Colour a)
    : param (p), accent (a), attachment (p, [this] (float v) { index = roundToInt (v); repaint(); }, nullptr)
{
    attachment.sendInitialUpdate();
    setMouseCursor (MouseCursor::PointingHandCursor);
    getProperties().set ("help", p.getParameterID());
}

void ParamCycler::step (int d)
{
    const int n = jmax (1, param.getNumSteps());
    attachment.setValueAsCompleteGesture ((float) ((index + d + n) % n));
}

void ParamCycler::mouseDown (const MouseEvent& e)
{
    if (e.mods.isPopupMenu()) return;   // click derecho: menu del DAW
    step (e.x < getWidth() / 4 ? -1 : 1);
}

void ParamCycler::mouseWheelMove (const MouseEvent&, const MouseWheelDetails& w) { step (w.deltaY > 0 ? -1 : 1); }

void ParamCycler::paint (Graphics& g)
{
    auto r = getLocalBounds().toFloat().reduced (1.0f);
    g.setColour (Colour (0xff0d0a07)); g.fillRoundedRectangle (r, 4.0f);
    g.setColour (accent.withAlpha (0.7f)); g.drawRoundedRectangle (r, 4.0f, 1.2f);
    const String t = i18n::tr (param.getText (param.getValue(), 24));
    theme::drawPrintedText (g, t, r.getCentreX(), r.getCentreY() + 1.0f, theme::fitHeight (t, r.getWidth() - 18.0f, 14.0f), 0.0f,
                            accent.brighter (0.5f), theme::black, 0.0f, false);
    g.setColour (accent.withAlpha (0.6f));
    Path arrow; arrow.addTriangle (r.getRight() - 9.0f, r.getCentreY() - 4.0f, r.getRight() - 9.0f, r.getCentreY() + 4.0f, r.getRight() - 4.0f, r.getCentreY());
    g.fillPath (arrow);
}

//============================================================================== panel
void ModPanel::Scope::paint (Graphics& g)
{
    auto r = getLocalBounds().toFloat();
    g.setColour (Colour (0xff070605)); g.fillRoundedRectangle (r, 4.0f);
    g.setColour (theme::gold.withAlpha (0.3f)); g.drawRoundedRectangle (r, 4.0f, 1.0f);
    Path p;
    const int n = (int) history.size();
    for (int i = 0; i < n; ++i)
    {
        const float v = history[(size_t) ((pos + i) % n)];
        const float x = r.getX() + 3.0f + (r.getWidth() - 6.0f) * (float) i / (float) (n - 1);
        const float y = r.getCentreY() - v * (r.getHeight() * 0.5f - 4.0f);
        if (i == 0) p.startNewSubPath (x, y); else p.lineTo (x, y);
    }
    g.setColour (ModMatrix::sourceColour (source));
    g.strokePath (p, PathStrokeType (1.8f));
}

ModPanel::ModPanel (AudioProcessorValueTreeState& state, ModMatrix& m, MouseListener& history) : matrix (m)
{
    setInterceptsMouseClicks (false, true);
    auto addKnob = [&] (const String& id, const String& label, float cx, float cy, Colour accent)
    {
        VintageKnob::Style st;
        st.look = VintageKnob::Look::moog;
        st.radius = 18.0f;
        st.accent = accent;
        auto k = std::make_unique<VintageKnob> (label, st);
        k->setBounds ((int) cx - 40, (int) cy - 30, 80, 76);
        k->centre = { 40.0f, 30.0f };
        k->getProperties().set ("help", id);
        k->addMouseListener (&history, false);
        addAndMakeVisible (*k);
        atts.push_back (std::make_unique<AudioProcessorValueTreeState::SliderAttachment> (state, id, *k));
        if (auto* p = state.getParameter (id)) k->setDoubleClickReturnValue (true, p->convertFrom0to1 (p->getDefaultValue()));
        knobs.push_back (std::move (k));
    };
    auto addChip = [&] (int s, Rectangle<int> b)
    {
        chips[(size_t) s] = std::make_unique<ModSourceChip> (m, s);
        chips[(size_t) s]->setBounds (b);
        addAndMakeVisible (*chips[(size_t) s]);
    };

    // MOD A-D
    for (int i = 0; i < 4; ++i)
    {
        const int col = i % 2, row = i / 2;
        const int x = 8 + col * 80, y = 56 + row * 124;
        addChip (i, { x, y, 76, 26 });
        addKnob (ModMatrix::paramFor (i), pk ("CANTIDAD", "AMOUNT"), (float) x + 38.0f, (float) y + 64.0f, ModMatrix::sourceColour (i));
    }

    // LFO 1 y 2
    for (int l = 0; l < 2; ++l)
    {
        const int s = ModMatrix::lfo1 + l;
        const int y0 = 316 + l * 222;
        const String n (l + 1);
        addChip (s, { 8, y0, 156, 26 });
        addKnob ("lfo" + n + "_rate", pk ("VELOC.", "RATE"), 48.0f, (float) y0 + 70.0f, ModMatrix::sourceColour (s));
        for (int c = 0; c < 2; ++c)
        {
            auto* prm = state.getParameter ("lfo" + n + (c == 0 ? "_shape" : "_sync"));
            auto cy = std::make_unique<ParamCycler> (*prm, ModMatrix::sourceColour (s));
            cy->setBounds (90, y0 + 46 + c * 46, 74, 28);
            addAndMakeVisible (*cy);
            cyclers.push_back (std::move (cy));
        }
        scopeViews[(size_t) l] = std::make_unique<Scope> (m, s);
        scopeViews[(size_t) l]->setBounds (8, y0 + 128, 156, 52);
        addAndMakeVisible (*scopeViews[(size_t) l]);
    }
}

void ModPanel::paint (Graphics& g)
{
    auto b = getLocalBounds().toFloat().reduced (2.0f);
    g.setColour (Colours::black.withAlpha (0.55f));
    g.fillRoundedRectangle (b.translated (3, 5), 12.0f);
    g.setGradientFill (ColourGradient (Colour (0xf6231a14), b.getX(), b.getY(), Colour (0xf60e0a08), b.getX(), b.getBottom(), false));
    g.fillRoundedRectangle (b, 12.0f);
    g.setColour (theme::gold.withAlpha (0.85f));
    g.drawRoundedRectangle (b.reduced (2.0f), 10.0f, 2.5f);

    theme::drawPrintedText (g, pk ("MODULACIÓN", "MODULATION"), b.getCentreX(), 24.0f, theme::fitHeight (pk ("MODULACIÓN", "MODULATION"), 150.0f, 21.0f), 0.0f,
                            theme::mustardHi, theme::black, 0.25f);
    theme::drawPrintedText (g, pk ("ARRASTRÁ A UNA PERILLA", "DRAG ONTO A KNOB"), b.getCentreX(), 44.0f, 10.5f, 0.0f,
                            theme::cream.withAlpha (0.6f), theme::black, 0.0f, false);

    // separadores de seccion
    for (int l = 0; l < 2; ++l)
    {
        const float y = 304.0f + (float) l * 222.0f;
        g.setColour (theme::gold.withAlpha (0.35f));
        g.drawHorizontalLine ((int) y, b.getX() + 10.0f, b.getRight() - 10.0f);
        theme::drawPrintedText (g, pk ("FORMA", "SHAPE"), 127.0f, y + 50.0f, 9.5f, 0.0f, theme::cream.withAlpha (0.55f), theme::black, 0.0f, false);
        theme::drawPrintedText (g, "TEMPO", 127.0f, y + 96.0f, 9.5f, 0.0f, theme::cream.withAlpha (0.55f), theme::black, 0.0f, false);
    }
}

//============================================================================== marcas sobre las perillas
ModOverlay::ModOverlay (ModMatrix& m) : matrix (m)
{
    setInterceptsMouseClicks (true, false);
    startTimerHz (20);
}

ModOverlay::~ModOverlay() { stopTimer(); }

void ModOverlay::refresh()
{
    controls = visibleControls ? visibleControls() : decltype (controls) {};
    badges.clear();
    const auto routes = matrix.routes();
    for (auto& [id, rect] : controls)
    {
        int k = 0;
        for (auto& r : routes)
        {
            if (r.param != id) continue;
            const float d = 15.0f;
            Rectangle<float> dot ((float) rect.getRight() - d - 2.0f - (float) k * (d + 3.0f), (float) rect.getY() + 2.0f, d, d);
            badges.push_back ({ r.source, id, rect, dot, r.depth });
            ++k;
        }
    }
}

void ModOverlay::timerCallback()
{
    if (! isShowing()) return;
    if (editing < 0 && ++tick % 4 == 0) refresh();   // posiciones (cambio de motor, pestana...)
    else for (auto& b : badges) b.depth = matrix.depthOf (b.source, b.param);
    if (! badges.empty() || dragSource >= 0) repaint();
}

int ModOverlay::badgeAt (Point<float> p) const
{
    for (int i = (int) badges.size() - 1; i >= 0; --i)
        if (badges[(size_t) i].dot.expanded (3.0f).contains (p)) return i;
    return -1;
}

bool ModOverlay::hitTest (int x, int y) { return badgeAt ({ (float) x, (float) y }) >= 0; }

void ModOverlay::paint (Graphics& g)
{
    // perillas moduladas: barra con el valor modulado y el punto de cada fuente
    std::map<String, bool> barDrawn;
    for (auto& b : badges)
    {
        const auto col = ModMatrix::sourceColour (b.source);
        const auto r = b.control.toFloat();
        if (! barDrawn[b.param])
        {
            barDrawn[b.param] = true;
            const float v = matrix.modulatedNormalised (b.param);
            auto bar = Rectangle<float> (r.getX() + 6.0f, r.getBottom() - 4.0f, r.getWidth() - 12.0f, 3.0f);
            g.setColour (Colours::black.withAlpha (0.6f)); g.fillRoundedRectangle (bar.expanded (1.0f), 2.0f);
            g.setColour (col); g.fillRoundedRectangle (bar.withWidth (bar.getWidth() * jlimit (0.0f, 1.0f, v)), 1.5f);
        }
        g.setColour (Colours::black.withAlpha (0.6f)); g.fillEllipse (b.dot.translated (1, 1.5f));
        g.setColour (col); g.fillEllipse (b.dot);
        g.setColour (theme::ink); g.drawEllipse (b.dot, 1.2f);
        // la cantidad: arco alrededor del punto (hacia la derecha = positiva)
        Path arc;
        const float a = b.depth * MathConstants<float>::pi;
        arc.addCentredArc (b.dot.getCentreX(), b.dot.getCentreY(), b.dot.getWidth() * 0.5f + 3.0f, b.dot.getWidth() * 0.5f + 3.0f, 0.0f,
                           0.0f, a, true);
        g.setColour (col.brighter (0.5f)); g.strokePath (arc, PathStrokeType (2.0f));
        const String letter = b.source < ModMatrix::lfo1 ? String::charToString ((juce_wchar) ('A' + b.source)) : String (b.source - ModMatrix::lfo1 + 1);
        theme::drawPrintedText (g, letter, b.dot.getCentreX(), b.dot.getCentreY() + 0.5f, 11.0f, 0.0f, theme::ink, col, 0.0f, false);
    }

    // cantidad mientras se edita
    if (editing >= 0 && editing < (int) badges.size())
    {
        auto& b = badges[(size_t) editing];
        const String t = (b.depth >= 0 ? "+" : "") + String (roundToInt (b.depth * 100.0f)) + "%";
        auto box = Rectangle<float> (theme::textWidth (t, 14.0f) + 14.0f, 22.0f).withCentre ({ b.dot.getCentreX(), b.dot.getY() - 16.0f });
        g.setColour (Colour (0xf0100b08)); g.fillRoundedRectangle (box, 5.0f);
        g.setColour (ModMatrix::sourceColour (b.source)); g.drawRoundedRectangle (box, 5.0f, 1.2f);
        theme::drawPrintedText (g, t, box.getCentreX(), box.getCentreY() + 1.0f, 14.0f, 0.0f, theme::cream, theme::black, 0.0f, false);
    }

    // arrastre desde una fuente
    if (dragSource >= 0)
    {
        const auto col = ModMatrix::sourceColour (dragSource);
        for (auto& [id, rect] : controls)
        {
            g.setColour (col.withAlpha (0.18f));
            g.drawRoundedRectangle (rect.toFloat().reduced (2.0f), 6.0f, 1.0f);
        }
        if (! dragTargetRect.isEmpty())
        {
            g.setColour (col.withAlpha (0.25f)); g.fillRoundedRectangle (dragTargetRect.toFloat(), 8.0f);
            g.setColour (col); g.drawRoundedRectangle (dragTargetRect.toFloat(), 8.0f, 3.0f);
        }
        auto tag = Rectangle<float> (dragPos.x + 12.0f, dragPos.y + 10.0f, 0.0f, 26.0f);
        String text = ModMatrix::sourceName (dragSource);
        if (dragTarget.isNotEmpty()) text << "  >  " << (nameFor ? nameFor (dragTarget) : dragTarget);
        tag.setWidth (theme::textWidth (text, 15.0f) + 20.0f);
        g.setColour (Colour (0xf0100b08)); g.fillRoundedRectangle (tag, 6.0f);
        g.setColour (col); g.drawRoundedRectangle (tag, 6.0f, 1.5f);
        theme::drawPrintedText (g, text, tag.getCentreX(), tag.getCentreY() + 1.0f, 15.0f, 0.0f, theme::cream, theme::black, 0.0f, false);
        g.setColour (col); g.fillEllipse (dragPos.x - 7.0f, dragPos.y - 7.0f, 14.0f, 14.0f);
    }
}

void ModOverlay::dragUpdate (int source, Point<float> where, const String& targetId, Rectangle<int> targetRect)
{
    if (dragSource < 0) refresh();
    dragSource = source; dragPos = where; dragTarget = targetId; dragTargetRect = targetRect;
    repaint();
}

void ModOverlay::dragEnd()
{
    dragSource = -1; dragTarget = {}; dragTargetRect = {};
    refresh();
    repaint();
}

void ModOverlay::mouseDown (const MouseEvent& e)
{
    const int i = badgeAt (e.position);
    if (i < 0) return;
    auto b = badges[(size_t) i];
    if (e.mods.isPopupMenu())
    {
        PopupMenu m;
        m.addSectionHeader (ModMatrix::sourceName (b.source) + "  >  " + (nameFor ? nameFor (b.param) : b.param));
        m.addItem (1, pk ("INVERTIR", "INVERT"));
        m.addItem (2, pk ("QUITAR MODULACIÓN", "REMOVE MODULATION"));
        SafePointer<ModOverlay> safe (this);
        m.showMenuAsync (PopupMenu::Options().withTargetScreenArea (localAreaToGlobal (b.dot.toNearestInt())), [safe, b] (int r)
        {
            if (safe == nullptr) return;
            if (r == 1) safe->matrix.setRoute (b.source, b.param, -b.depth);
            if (r == 2) safe->matrix.removeRoute (b.source, b.param);
            safe->refresh(); safe->repaint();
        });
        return;
    }
    editing = i;
    editStartDepth = b.depth;
    setMouseCursor (MouseCursor::UpDownResizeCursor);
}

void ModOverlay::mouseDrag (const MouseEvent& e)
{
    if (editing < 0 || editing >= (int) badges.size()) return;
    auto& b = badges[(size_t) editing];
    const float speed = e.mods.isShiftDown() ? 0.001f : 0.005f;
    b.depth = jlimit (-1.0f, 1.0f, editStartDepth - (float) e.getDistanceFromDragStartY() * speed);
    matrix.setRoute (b.source, b.param, b.depth);
    repaint();
}

void ModOverlay::mouseUp (const MouseEvent&)
{
    editing = -1;
    setMouseCursor (MouseCursor::NormalCursor);
    refresh();
    repaint();
}

void ModOverlay::mouseDoubleClick (const MouseEvent& e)
{
    const int i = badgeAt (e.position);
    if (i < 0) return;
    matrix.removeRoute (badges[(size_t) i].source, badges[(size_t) i].param);
    editing = -1;
    refresh();
    repaint();
}
}
