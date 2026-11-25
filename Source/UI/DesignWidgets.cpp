#include "DesignWidgets.h"
#include "../Core/Parameters.h"
#include "../DSP/Engines.h"
#include "../Help/I18n.h"
#include "../Help/Help.h"

namespace flipper
{
using namespace juce;
static String u8 (const char* s) { return String::fromUTF8 (s); }
using L = VintageKnob::Look;

const Identity& identityFor (int e)
{
    static const std::array<Identity, numEngines> ids {{
        { Colour (0xff8fae3a), Colour (0xff181614), Colour (0xff0b0a09), L::moog,    true,  "3 OSC · FILTRO 24 DB",
          "EMPEZÁ POR EL FILTRO: CORTE Y CONTORNO HACEN CASI TODO EL SONIDO" },
        { Colour (0xffe9c45a), Colour (0xff15130f), Colour (0xff090807), L::prophet, true,  "2 OSC · POLY-MOD · 24 DB",
          "POLY-MOD (ENV FILTRO A FREC A) DA CAMPANAS Y SONIDOS METÁLICOS; UNÍSONO ENGORDA" },
        { Colour (0xffff8a2a), Colour (0xff2a2a2e), Colour (0xff141416), L::jupiter, false, "2 VCO · CROSS MOD · ARPEGIADOR",
          "MEZCLÁ LOS DOS VCO; CROSS MOD Y SYNC DAN LOS SONIDOS AGRESIVOS" },
        { Colour (0xffff4fa0), Colour (0xff1e1b21), Colour (0xff0f0e11), L::juno,    false, "1 DCO · SUB · CHORUS",
          "SIERRA + SUB + CHORUS = EL PAD CLÁSICO; BAJÁ EL CORTE PARA BAJOS" },
        { Colour (0xffb7e86a), Colour (0xff141410), Colour (0xff070706), L::cs80,    false, "2 CANALES · AFTERTOUCH · CINTA",
          "CANAL I Y II SON DOS SINTES EN CAPA; LA CINTA DOBLA EL TONO COMO EN LAS BANDAS DE SONIDO DE LOS 80" },
        { Colour (0xff6cc04a), Colour (0xff3a2a24), Colour (0xff1e1512), L::dx7,     false, "6 OPERADORES · 32 ALGORITMOS",
          "LOS PORTADORES (P) SUENAN; LOS MODULADORES DAN BRILLO: SUBÍ SU NIVEL DESPACIO" },
        { Colour (0xffff8a20), Colour (0xff202124), Colour (0xff111214), L::m1,      false, "2 FUENTES PCM PROPIAS · EFECTOS",
          "ELEGÍ DOS MUESTRAS Y MEZCLALAS: PIANO + PAD ES EL SONIDO DE LOS 90" },
        { Colour (0xffeadfc4), Colour (0xff3b4652), Colour (0xff252c33), L::arp,     false, "3 VCO · S&H · PATCH BAY",
          "CONECTÁ CABLES: S&H AL FILTRO DA EL SONIDO DE ROBOT CLÁSICO" },
        { Colour (0xffb26bff), Colour (0xff1a1620), Colour (0xff0d0b11), L::vcs3,    true,  "MATRIZ DE PINES · JOYSTICK · RESORTE",
          "CADA PIN CONECTA UNA FUENTE (FILA) CON UN DESTINO (COLUMNA); MOVÉ EL JOYSTICK" },
    }};
    return ids[(size_t) jlimit (0, numEngines - 1, e)];
}

//============================================================================== ChoiceButtons
ChoiceButtons::ChoiceButtons (RangedAudioParameter& p, String l, const Identity& i, UndoManager* um)
    : param (p), label (std::move (l)), id (i),
      attachment (p, [this] (float v) { index = roundToInt (v); repaint(); }, um)
{
    if (auto* c = dynamic_cast<AudioParameterChoice*> (&p)) options = c->choices;
    isSwitch = options.size() == 2 && options[0] == "NO";
    setMouseCursor (MouseCursor::PointingHandCursor);
    attachment.sendInitialUpdate();
}

float ChoiceButtons::preferredWeight (const RangedAudioParameter& p)
{
    if (auto* c = dynamic_cast<const AudioParameterChoice*> (&p))
        return (c->choices.size() == 2 && c->choices[0] == "NO") ? 0.9f : jlimit (1.0f, 2.8f, 0.55f * (float) c->choices.size());
    return 1.0f;
}

Rectangle<float> ChoiceButtons::buttonRect (int i) const
{
    const float w = (float) getWidth(), h = (float) getHeight();
    if (isSwitch) return Rectangle<float> (jmin (46.0f, w - 6.0f), 26.0f).withCentre ({ w * 0.5f, h * 0.55f });
    const int n = options.size();
    const float bw = (w - 4.0f) / (float) n;
    return { 2.0f + bw * (float) i + 1.5f, h * 0.55f - 13.0f, bw - 3.0f, 26.0f };
}

void ChoiceButtons::mouseDown (const MouseEvent& e)
{
    if (e.mods.isPopupMenu()) return;   // click derecho: menu del DAW (Link to controller) y modulaciones

    int target = -1;
    if (isSwitch) target = index == 0 ? 1 : 0;
    else for (int i = 0; i < options.size(); ++i) if (buttonRect (i).expanded (1.5f).contains (e.position)) target = i;
    if (target >= 0) attachment.setValueAsCompleteGesture ((float) target);
}

void ChoiceButtons::paint (Graphics& g)
{
    const float w = (float) getWidth();
    theme::drawPrintedText (g, label, w * 0.5f, 9.0f, theme::fitHeight (label, w - 2.0f, 12.0f), 0.0f, theme::cream, theme::black, 0.15f);
    const bool membrane = id.look == L::dx7;
    const int n = isSwitch ? 1 : options.size();
    for (int i = 0; i < n; ++i)
    {
        const auto r = buttonRect (i);
        const bool on = isSwitch ? index == 1 : index == i;
        g.setColour (Colours::black.withAlpha (0.5f));
        g.fillRoundedRectangle (r.translated (1, 2), 3.0f);
        if (on)
            g.setGradientFill (ColourGradient (id.accent.brighter (0.35f), r.getX(), r.getY(), id.accent.darker (0.3f), r.getX(), r.getBottom(), false));
        else
            g.setGradientFill (ColourGradient (membrane ? Colour (0xff3d5a30) : Colour (0xff35322e), r.getX(), r.getY(),
                                               membrane ? Colour (0xff22331b) : Colour (0xff151311), r.getX(), r.getBottom(), false));
        g.fillRoundedRectangle (r, membrane ? 1.5f : 3.0f);
        g.setColour (on ? id.accent.brighter (0.6f) : id.accent.withAlpha (0.35f));
        g.drawRoundedRectangle (r, membrane ? 1.5f : 3.0f, 1.0f);
        if (on && ! membrane)
        {
            g.setGradientFill (ColourGradient (id.accent.withAlpha (0.45f), r.getCentreX(), r.getCentreY(), id.accent.withAlpha (0.0f), r.getCentreX() + r.getWidth(), r.getCentreY(), true));
            g.fillEllipse (r.expanded (r.getWidth() * 0.3f, 8.0f));
        }
        const String t = isSwitch ? (on ? "ON" : "OFF") : options[i];
        theme::drawPrintedText (g, t, r.getCentreX(), r.getCentreY() + 0.5f, theme::fitHeight (t, r.getWidth() - 4.0f, 11.5f), 0.0f,
                                on ? Colour (0xff160d06) : theme::cream.withAlpha (0.8f), theme::black, 0.0f, false);
    }
}

//============================================================================== LcdSelector
LcdSelector::LcdSelector (RangedAudioParameter& p, String l, const Identity& i, UndoManager* um)
    : param (p), label (std::move (l)), id (i),
      attachment (p, [this] (float v) { index = roundToInt (v) - offset; repaint(); if (onChange) onChange(); }, um)
{
    if (auto* c = dynamic_cast<AudioParameterChoice*> (&p)) count = c->choices.size();
    else if (auto* n = dynamic_cast<AudioParameterInt*> (&p)) { offset = n->getRange().getStart(); count = n->getRange().getLength() + 1; }
    setMouseCursor (MouseCursor::PointingHandCursor);
    attachment.sendInitialUpdate();
}

void LcdSelector::step (int delta)
{
    const int target = (index + delta + count) % count;
    attachment.setValueAsCompleteGesture ((float) (target + offset));
}

void LcdSelector::mouseDown (const MouseEvent& e)            { if (! e.mods.isPopupMenu()) step (e.x < getWidth() / 2 ? -1 : 1); }
void LcdSelector::mouseWheelMove (const MouseEvent&, const MouseWheelDetails& w) { step (w.deltaY > 0 ? -1 : 1); }

void LcdSelector::paint (Graphics& g)
{
    const float w = (float) getWidth(), h = (float) getHeight();
    if (label.isNotEmpty())
        theme::drawPrintedText (g, label, w * 0.5f, 9.0f, theme::fitHeight (label, w - 2.0f, 12.0f), 0.0f, theme::cream, theme::black, 0.15f);
    const float top = label.isNotEmpty() ? 20.0f : 2.0f;
    auto r = Rectangle<float> (2.0f, top, w - 4.0f, jmin (34.0f, h - top - 4.0f));
    g.setColour (Colour (0xff0b0a08)); g.fillRoundedRectangle (r.expanded (2.0f), 4.0f);
    g.setGradientFill (ColourGradient (Colour (0xffa9bd66), r.getX(), r.getY(), Colour (0xff7f9446), r.getX(), r.getBottom(), false));
    g.fillRoundedRectangle (r, 3.0f);
    g.setColour (Colours::black.withAlpha (0.06f));
    for (float y = r.getY() + 2; y < r.getBottom(); y += 2.0f) g.drawHorizontalLine ((int) y, r.getX(), r.getRight());

    String text;
    if (auto* c = dynamic_cast<AudioParameterChoice*> (&param)) text = c->choices[jlimit (0, count - 1, index)];
    else text = String (index + offset);
    const Colour ink (0xff1c2810);
    g.setColour (ink);
    Path left, right;
    left.addTriangle (r.getX() + 6, r.getCentreY(), r.getX() + 13, r.getCentreY() - 5, r.getX() + 13, r.getCentreY() + 5);
    right.addTriangle (r.getRight() - 6, r.getCentreY(), r.getRight() - 13, r.getCentreY() - 5, r.getRight() - 13, r.getCentreY() + 5);
    g.fillPath (left); g.fillPath (right);
    theme::drawPrintedText (g, text, r.getCentreX(), r.getCentreY() + 0.5f, theme::fitHeight (text, r.getWidth() - 34.0f, 13.0f), 0.0f,
                            ink, ink, 0.0f, false);
}

//============================================================================== EnginePage
EnginePage::EnginePage (int e, AudioProcessorValueTreeState& s, MouseListener& h)
    : engine (e), id (identityFor (e)), state (s), history (h)
{
    setInterceptsMouseClicks (false, true);
}

Component& EnginePage::place (const String& paramId, const String& label, Rectangle<int> r, Colour cap)
{
    auto* prm = state.getParameter (paramId);
    jassert (prm != nullptr);
    std::unique_ptr<Component> comp;

    if (auto* ch = dynamic_cast<AudioParameterChoice*> (prm))
    {
        if (ch->choices.size() > 5) comp = std::make_unique<LcdSelector> (*prm, label, id, nullptr);
        else comp = std::make_unique<ChoiceButtons> (*prm, label, id, nullptr);
    }
    else if (paramId == "dx7_algorithm")
        comp = std::make_unique<LcdSelector> (*prm, label, id, nullptr);
    else
    {
        VintageKnob::Style st;
        st.look = id.look;
        st.accent = id.accent;
        st.radius = jlimit (14.0f, 19.0f, (float) r.getWidth() * 0.27f);
        st.scale = { "0", "10" };
        if (cap != Colour()) st.cap = cap;
        else if (id.look == L::juno) st.cap = Colour (0xffd9d6cf);
        else if (id.look == L::arp)  st.cap = Colour (0xfff0ebe0);
        auto k = std::make_unique<VintageKnob> (label, st);
        if (VintageKnob::isFader (id.look))
            k->setBounds (r);
        else
        {
            k->setBounds (r);
            k->centre = { (float) r.getWidth() * 0.5f, 50.0f };
        }
        attachments.push_back (std::make_unique<AudioProcessorValueTreeState::SliderAttachment> (state, paramId, *k));
        k->setDoubleClickReturnValue (true, prm->convertFrom0to1 (prm->getDefaultValue()));
        comp = std::move (k);
    }
    comp->setBounds (r);
    comp->getProperties().set ("help", paramId);
    comp->addMouseListener (&history, false);
    addAndMakeVisible (*comp);
    owned.push_back (std::move (comp));
    lastCreated.push_back (owned.back().get());
    return *owned.back();
}

float EnginePage::row (float y, std::vector<Group> groups, float height)
{
    const bool fader = VintageKnob::isFader (id.look);
    if (height <= 0.0f) height = 96.0f;
    const float x0 = 44.0f, x1 = (float) getWidth() - 44.0f, gap = 12.0f;
    static const Colour cs80Caps[] = { Colour (0xfff2efe6), Colour (0xffe8c23a), Colour (0xffc8322a), Colour (0xff3a6fc8), Colour (0xfff2efe6) };
    static const Colour vcsCaps[]  = { Colour (0xffc8322a), Colour (0xff3a6fc8), Colour (0xffe8c23a), Colour (0xff4f9a4a), Colour (0xffe0dcd2), Colour (0xff8a4fd0) };

    auto weightOf = [this] (const Item& it)
    {
        auto* prm = state.getParameter (it.id);
        if (auto* ch = dynamic_cast<AudioParameterChoice*> (prm)) return ch->choices.size() > 5 ? 1.9f : ChoiceButtons::preferredWeight (*prm);
        if (it.id == "dx7_algorithm") return 1.9f;
        return VintageKnob::isFader (id.look) ? 0.85f : 1.0f;
    };

    float total = 0.0f;
    for (auto& g : groups) for (auto& it : g.items) total += weightOf (it);
    const float avail = x1 - x0 - gap * (float) (groups.size() - 1) - 8.0f * (float) groups.size();
    const float unit = avail / total;

    lastCreated.clear();
    float x = x0;
    int gi = 0;
    for (auto& g : groups)
    {
        float gw = 8.0f;
        for (auto& it : g.items) gw += weightOf (it) * unit;
        bands.push_back ({ g.title, { x, y, gw, height + 16.0f } });
        float ix = x + 4.0f;
        Colour cap;
        if (id.look == L::cs80) cap = cs80Caps[capIndex++ % 5];
        for (auto& it : g.items)
        {
            const float w = weightOf (it) * unit;
            if (id.look == L::vcs3) cap = vcsCaps[capIndex++ % 6];
            place (it.id, it.label, Rectangle<float> (ix, y + 14.0f, w, height).toNearestInt(), cap);
            ix += w;
        }
        x += gw + gap;
        ++gi;
    }
    ignoreUnused (fader, gi);
    return height + 18.0f;
}

void EnginePage::paint (Graphics& g)
{
    // cara del panel con el material de la maquina
    auto face = Rectangle<float> (36, 100, (float) getWidth() - 72, (float) getHeight() - 140);
    if (id.wood)
    {
        Random r (engine * 31);
        for (auto side : { Rectangle<float> (12, 96, 20, face.getHeight() + 8), Rectangle<float> ((float) getWidth() - 32, 96, 20, face.getHeight() + 8) })
        {
            g.setGradientFill (ColourGradient (Colour (0xff4a2a16), side.getX(), 0, Colour (0xff2a160b), side.getRight(), 0, false));
            g.fillRoundedRectangle (side, 6.0f);
            for (int i = 0; i < 22; ++i)
            {
                const float gx = side.getX() + r.nextFloat() * side.getWidth();
                g.setColour ((r.nextBool() ? Colour (0xff6b4024) : Colour (0xff1a0d06)).withAlpha (0.3f));
                g.drawLine (gx, side.getY(), gx + r.nextFloat() * 3 - 1.5f, side.getBottom(), 0.8f + r.nextFloat());
            }
        }
    }
    g.setGradientFill (ColourGradient (id.panelTop, face.getX(), face.getY(), id.panelBottom, face.getX(), face.getBottom(), false));
    g.fillRect (face);

    Random r (engine * 977 + 5);
    if (id.look == L::jupiter) // franjas de color en la parte de arriba
    {
        const Colour stripes[] = { Colour (0xffc8322a), Colour (0xffff8a2a), Colour (0xffe8c23a), Colour (0xff3a6fc8) };
        for (int i = 0; i < 4; ++i) { g.setColour (stripes[i]); g.fillRect (face.getX(), face.getY() + i * 3.0f, face.getWidth(), 3.0f); }
    }
    if (id.look == L::dx7) // barras de color de los botones de membrana
    {
        const Colour bars[] = { Colour (0xff6cc04a), Colour (0xff3a8fc8), Colour (0xffd84a8a), Colour (0xffe8c23a) };
        for (int i = 0; i < 4; ++i) { g.setColour (bars[i].withAlpha (0.8f)); g.fillRect (face.getX() + face.getWidth() * i / 4.0f, face.getY(), face.getWidth() / 4.0f, 5.0f); }
    }
    if (id.look == L::arp) // tornillos del panel de metal
        for (auto p : { face.getTopLeft() + Point<float> (10, 14), face.getTopRight() + Point<float> (-10, 14),
                        face.getBottomLeft() + Point<float> (10, -10), face.getBottomRight() + Point<float> (-10, -10) })
        {
            g.setGradientFill (ColourGradient (Colour (0xffd8d4cc), p.x - 3, p.y - 3, Colour (0xff4a4f55), p.x + 3, p.y + 3, false));
            g.fillEllipse (Rectangle<float> (9, 9).withCentre (p));
            g.setColour (Colours::black.withAlpha (0.6f)); g.drawLine (p.x - 3, p.y, p.x + 3, p.y, 1.2f);
        }
    for (int i = 0; i < 500; ++i) // desgaste
    {
        g.setColour ((r.nextBool() ? theme::cream : Colours::black).withAlpha (0.03f + r.nextFloat() * 0.05f));
        const float s = 0.8f + r.nextFloat() * 2.0f;
        g.fillEllipse (face.getX() + r.nextFloat() * face.getWidth(), face.getY() + r.nextFloat() * face.getHeight(), s, s);
    }
    g.setColour (theme::gold.withAlpha (0.5f));
    g.drawRect (face, 1.2f);

    for (auto& b : bands)
    {
        g.setColour (Colours::white.withAlpha (0.03f));
        g.fillRoundedRectangle (b.area, 4.0f);
        g.setColour (id.accent.withAlpha (0.35f));
        g.drawRoundedRectangle (b.area, 4.0f, 1.0f);
        const float th = theme::fitHeight (b.title, b.area.getWidth() - 16.0f, 12.5f);
        const float tw = theme::textWidth (b.title, th);
        g.setColour (id.panelBottom);
        g.fillRect (Rectangle<float> (tw + 14, th + 3.0f).withCentre ({ b.area.getCentreX(), b.area.getY() }));
        theme::drawPrintedText (g, b.title, b.area.getCentreX(), b.area.getY(), th, 0.0f, id.accent.brighter (0.45f), theme::black, 0.15f, false);
    }

    theme::drawPrintedText (g, u8 (id.help), (float) getWidth() * 0.5f, face.getBottom() - 14.0f,
                            theme::fitHeight (u8 (id.help), face.getWidth() - 30.0f, 10.5f), 0.0f, id.accent.withAlpha (0.8f), theme::black, 0.0f);
}

//============================================================================== DX7: algoritmo y operadores
namespace
{
class AlgorithmDisplay : public Component
{
public:
    AlgorithmDisplay (RangedAudioParameter& p, const Identity& i)
        : id (i), attachment (p, [this] (float v) { alg = jlimit (1, 32, roundToInt (v)); repaint(); if (onChange) onChange(); }, nullptr)
    { attachment.sendInitialUpdate(); setMouseCursor (MouseCursor::PointingHandCursor); }

    std::function<void (int)> onSelectOp;
    std::function<void()> onChange;
    int selected = 0;
    int algorithm() const { return alg; }

    void mouseDown (const MouseEvent& e) override
    {
        for (int o = 0; o < 6; ++o)
            if (boxes[(size_t) o].contains (e.position)) { if (onSelectOp) onSelectOp (o); return; }
        if (e.y < 34) attachment.setValueAsCompleteGesture ((float) (e.x < getWidth() / 2 ? (alg + 30) % 32 + 1 : alg % 32 + 1));
    }

    void paint (Graphics& g) override
    {
        auto r = getLocalBounds().toFloat().reduced (2.0f);
        g.setColour (Colour (0xff0b0a08)); g.fillRoundedRectangle (r.expanded (2.0f), 5.0f);
        g.setGradientFill (ColourGradient (Colour (0xffa9bd66), r.getX(), r.getY(), Colour (0xff7f9446), r.getX(), r.getBottom(), false));
        g.fillRoundedRectangle (r, 4.0f);
        const Colour ink (0xff1c2810);
        Path left, right;
        left.addTriangle (r.getX() + 8, r.getY() + 17, r.getX() + 16, r.getY() + 11, r.getX() + 16, r.getY() + 23);
        right.addTriangle (r.getRight() - 8, r.getY() + 17, r.getRight() - 16, r.getY() + 11, r.getRight() - 16, r.getY() + 23);
        g.setColour (ink); g.fillPath (left); g.fillPath (right);
        theme::drawPrintedText (g, i18n::tr ("ALGORITMO") + " " + String (alg).paddedLeft ('0', 2), r.getCentreX(), r.getY() + 17, 14.0f, 0.0f, ink, ink, 0.0f, false);

        // diagrama: portadores abajo, cada modulador arriba de su destino
        const auto& a = dsp::DX7Engine::algorithms()[(size_t) alg - 1];
        std::array<int, 6> depth {}, target {};
        target.fill (-1);
        for (int o = 0; o < 6; ++o) for (int k = 0; k < 6; ++k) if ((a.mods[(size_t) k] >> o) & 1) { target[(size_t) o] = k; break; }
        for (int o = 0; o < 6; ++o) { int d = 0, t = o; while (target[(size_t) t] >= 0 && d < 6) { t = target[(size_t) t]; ++d; } depth[(size_t) o] = d; }
        std::array<int, 7> perRow {}; std::array<int, 6> slot {};
        for (int o = 0; o < 6; ++o) slot[(size_t) o] = perRow[(size_t) depth[(size_t) o]]++;
        const float bw = 30, bh = 22, area = r.getHeight() - 50;
        const int maxDepth = *std::max_element (depth.begin(), depth.end());
        for (int o = 0; o < 6; ++o)
        {
            const int d = depth[(size_t) o], n = perRow[(size_t) d];
            const float cx = r.getX() + r.getWidth() * (float) (slot[(size_t) o] + 1) / (float) (n + 1);
            const float cy = r.getBottom() - 22 - (maxDepth > 0 ? area * (float) d / (float) jmax (3, maxDepth) : 0.0f);
            boxes[(size_t) o] = Rectangle<float> (bw, bh).withCentre ({ cx, cy });
        }
        g.setColour (ink);
        for (int o = 0; o < 6; ++o)
        {
            const auto& b = boxes[(size_t) o];
            if (target[(size_t) o] >= 0) g.drawLine ({ b.getCentre().withY (b.getBottom()), boxes[(size_t) target[(size_t) o]].getCentre().withY (boxes[(size_t) target[(size_t) o]].getY()) }, 1.6f);
            for (int k = 0; k < 6; ++k) if (k != target[(size_t) o] && ((a.mods[(size_t) k] >> o) & 1))
                g.drawLine ({ b.getCentre().withY (b.getBottom()), boxes[(size_t) k].getCentre().withY (boxes[(size_t) k].getY()) }, 1.6f);
            if ((a.carriers >> o) & 1) g.drawLine (b.getCentreX(), b.getBottom(), b.getCentreX(), r.getBottom() - 4, 1.6f);
        }
        for (int o = 0; o < 6; ++o)
        {
            const auto& b = boxes[(size_t) o];
            const bool sel = o == selected;
            g.setColour (sel ? ink : Colour (0xffb9cc78));
            g.fillRect (b);
            g.setColour (ink); g.drawRect (b, 1.5f);
            if (o == a.feedbackOp) // lazo de feedback
            {
                Path loop; loop.startNewSubPath (b.getRight(), b.getCentreY()); loop.lineTo (b.getRight() + 7, b.getCentreY());
                loop.lineTo (b.getRight() + 7, b.getY() - 6); loop.lineTo (b.getCentreX(), b.getY() - 6); loop.lineTo (b.getCentreX(), b.getY());
                g.strokePath (loop, PathStrokeType (1.3f));
            }
            const String t = String (o + 1) + (((a.carriers >> o) & 1) ? (i18n::english() ? "C" : "P") : "");
            theme::drawPrintedText (g, t, b.getCentreX(), b.getCentreY() + 0.5f, 11.0f, 0.0f, sel ? Colour (0xffb9cc78) : ink, ink, 0.0f, false);
        }
        ignoreUnused (id);
    }

private:
    const Identity& id;
    int alg = 5;
    std::array<Rectangle<float>, 6> boxes;
    ParameterAttachment attachment;
};

class DX7Page : public EnginePage
{
public:
    DX7Page (AudioProcessorValueTreeState& s, MouseListener& h) : EnginePage (5, s, h)
    {
        setSize (632, 1064);
        display = std::make_unique<AlgorithmDisplay> (*s.getParameter ("dx7_algorithm"), id);
        display->setBounds (48, 116, 330, 220);
        display->getProperties().set ("help", "dx7_algorithm");
        display->addMouseListener (&history, false);
        display->onSelectOp = [this] (int o) { select (o); };
        display->onChange = [this] { repaint(); };
        addAndMakeVisible (*display);

        bands.push_back ({ "FEEDBACK / LFO", { 392, 112, 196, 228 } });
        const char* gid[] = { "dx7_feedback", "dx7_lfo_rate", "dx7_lfo_pitch", "dx7_lfo_amp" };
        const char* glb[] = { "FEEDBACK", "LFO VELOC.", "LFO TONO", "LFO VOLUMEN" };
        for (int i = 0; i < 4; ++i) place (gid[i], glb[i], { 398 + i * 47, 150, 46, 170 });

        for (int o = 0; o < 6; ++o)
        {
            auto b = std::make_unique<TextButton>();
            b->onClick = [this, o] { select (o); };
            b->setAlpha (0.0f);  // el boton de membrana se dibuja en paint
            b->setBounds (48 + o * 90, 362, 84, 40);
            b->getProperties().set ("help", "ui:dx_op");
            b->setMouseCursor (MouseCursor::PointingHandCursor);
            addAndMakeVisible (*b);
            opButtons.push_back (std::move (b));
        }
        for (int o = 0; o < 6; ++o)
        {
            const String n ("dx7_op" + String (o + 1) + "_");
            std::vector<Component*> set;
            bands.clear(); // las bandas de operador se dibujan una vez (son iguales para todos)
            row (442, { { "FRECUENCIA", { { n + "coarse", "RATIO" }, { n + "fine", "FINO" }, { n + "detune", u8 ("DESAFINACIÓN") } } },
                        { "SALIDA", { { n + "level", "NIVEL" }, { n + "vel", "VELOCIDAD" } } } });
            set.insert (set.end(), lastCreated.begin(), lastCreated.end());
            row (574, { { "ENVOLVENTE DEL OPERADOR", { { n + "attack", "ATAQUE" }, { n + "decay", u8 ("CAÍDA") }, { n + "sustain", u8 ("SOSTÉN") }, { n + "release", u8 ("LIBERACIÓN") } } } });
            set.insert (set.end(), lastCreated.begin(), lastCreated.end());
            opSets.push_back (set);
        }
        opBands = bands;
        bands.clear();
        bands.push_back ({ "FEEDBACK / LFO", { 392, 112, 196, 228 } });
        row (716, { { "NIVEL DE LOS 6 OPERADORES", { { "dx7_op1_level", "OP 1" }, { "dx7_op2_level", "OP 2" }, { "dx7_op3_level", "OP 3" },
                                                    { "dx7_op4_level", "OP 4" }, { "dx7_op5_level", "OP 5" }, { "dx7_op6_level", "OP 6" } } } }, 150);
        for (auto& b : opBands) bands.push_back (b);
        select (0);
    }

    void reveal (const String& paramId) override
    {
        if (paramId.startsWith ("dx7_op") && ! paramId.startsWith ("dx7_op1_level") && paramId.length() > 7)
        {
            // los niveles tambien estan en la vista rapida: no hace falta cambiar de operador
            static const StringArray quick { "dx7_op1_level", "dx7_op2_level", "dx7_op3_level", "dx7_op4_level", "dx7_op5_level", "dx7_op6_level" };
            if (quick.contains (paramId)) return;
            const int o = paramId.substring (6, 7).getIntValue() - 1;
            if (o >= 0 && o < 6 && o != selected) select (o);
        }
    }

    void select (int o)
    {
        selected = o;
        display->selected = o;
        display->repaint();
        for (int i = 0; i < 6; ++i) for (auto* c : opSets[(size_t) i]) c->setVisible (i == o);
        repaint();
    }

    void paint (Graphics& g) override
    {
        EnginePage::paint (g);
        const auto& a = dsp::DX7Engine::algorithms()[(size_t) display->algorithm() - 1];
        for (int o = 0; o < 6; ++o)
        {
            auto r = opButtons[(size_t) o]->getBounds().toFloat();
            const bool sel = o == selected, carrier = (a.carriers >> o) & 1;
            g.setColour (Colours::black.withAlpha (0.5f)); g.fillRect (r.translated (1, 2));
            g.setGradientFill (ColourGradient (sel ? id.accent.brighter (0.3f) : Colour (0xff3d5a30), r.getX(), r.getY(),
                                               sel ? id.accent.darker (0.3f) : Colour (0xff22331b), r.getX(), r.getBottom(), false));
            g.fillRect (r);
            g.setColour (id.accent.withAlpha (sel ? 1.0f : 0.4f)); g.drawRect (r, 1.0f);
            const String t = "OP " + String (o + 1) + (carrier ? (i18n::english() ? "  (C)" : "  (P)") : "  (M)");
            theme::drawPrintedText (g, t, r.getCentreX(), r.getCentreY(), 13.0f, 0.0f, sel ? Colour (0xff10180a) : theme::cream, theme::black, 0.0f, false);
        }
        theme::drawPrintedText (g, i18n::tr ("EDITANDO OPERADOR") + " " + String (selected + 1) + i18n::tr (u8 (" · P = PORTADOR (SUENA) · M = MODULADOR (DA BRILLO)")),
                                316, 418, 10.5f, 0.0f, theme::cream.withAlpha (0.7f), theme::black, 0.0f);
    }

private:
    std::unique_ptr<AlgorithmDisplay> display;
    std::vector<std::unique_ptr<TextButton>> opButtons;
    std::vector<std::vector<Component*>> opSets;
    std::vector<Band> opBands;
    int selected = 0;
};

//============================================================================== CS-80: canales y cinta
class Ribbon : public Component
{
public:
    Ribbon (RangedAudioParameter& p, const Identity& i)
        : param (p), id (i), attachment (p, [this] (float v) { value = v; repaint(); }, nullptr)
    { attachment.sendInitialUpdate(); setMouseCursor (MouseCursor::LeftRightResizeCursor); }

    void mouseDown (const MouseEvent& e) override { attachment.beginGesture(); startX = (float) e.x; mouseDrag (e); }
    void mouseDrag (const MouseEvent& e) override
    {
        // la cinta del CS-80 es relativa: donde apoyas el dedo es el tono actual, deslizar lo dobla
        const float v = jlimit (-1.0f, 1.0f, ((float) e.x - startX) / ((float) getWidth() * 0.5f));
        attachment.setValueAsPartOfGesture (v);
    }
    void mouseUp (const MouseEvent&) override { attachment.setValueAsPartOfGesture (0.0f); attachment.endGesture(); }

    void paint (Graphics& g) override
    {
        auto r = getLocalBounds().toFloat();
        theme::drawPrintedText (g, u8 ("CINTA (RIBBON): APOYÁ Y DESLIZÁ PARA DOBLAR EL TONO ±12 ST"), r.getCentreX(), 8.0f, 10.5f, 0.0f, theme::cream, theme::black, 0.1f);
        auto strip = r.withTrimmedTop (18).reduced (4, 4);
        g.setGradientFill (ColourGradient (Colour (0xff2a2a26), strip.getX(), strip.getY(), Colour (0xff0a0a09), strip.getX(), strip.getBottom(), false));
        g.fillRoundedRectangle (strip, 4.0f);
        g.setColour (Colour (0xffb9b4aa).withAlpha (0.5f));
        g.drawRoundedRectangle (strip, 4.0f, 1.2f);
        for (int i = 1; i < 24; ++i)
        {
            const float x = strip.getX() + strip.getWidth() * i / 24.0f;
            g.setColour (Colours::white.withAlpha (i % 12 == 0 ? 0.35f : 0.08f));
            g.drawVerticalLine ((int) x, strip.getY() + 4, strip.getBottom() - 4);
        }
        const float cx = strip.getCentreX() + value * strip.getWidth() * 0.5f;
        g.setGradientFill (ColourGradient (id.accent.withAlpha (0.7f), cx, strip.getCentreY(), id.accent.withAlpha (0.0f), cx + 40, strip.getCentreY(), true));
        g.fillEllipse (Rectangle<float> (80, strip.getHeight() + 10).withCentre ({ cx, strip.getCentreY() }));
        g.setColour (id.accent); g.fillRect (Rectangle<float> (3, strip.getHeight() - 6).withCentre ({ cx, strip.getCentreY() }));
        ignoreUnused (param);
    }

private:
    RangedAudioParameter& param;
    const Identity& id;
    float value = 0, startX = 0;
    ParameterAttachment attachment;
};

class CS80Page : public EnginePage
{
public:
    CS80Page (AudioProcessorValueTreeState& s, MouseListener& h) : EnginePage (4, s, h)
    {
        setSize (632, 1064);
        for (int L = 0; L < 2; ++L)
        {
            auto b = std::make_unique<TextButton>();
            b->setAlpha (0.0f);
            b->onClick = [this, L] { select (L); };
            b->setBounds (48 + L * 274, 112, 262, 38);
            b->getProperties().set ("help", "ui:cs_layer");
            b->setMouseCursor (MouseCursor::PointingHandCursor);
            addAndMakeVisible (*b);
            layerButtons.push_back (std::move (b));
        }
        std::vector<Band> keep;
        for (int L = 0; L < 2; ++L)
        {
            const String n ("cs80_l" + String (L + 1) + "_");
            std::vector<Component*> set;
            bands.clear(); capIndex = 0;
            row (170, { { "OSCILADOR", { { n + "feet", "PIES" }, { n + "saw", "SIERRA" }, { n + "pulse", "PULSO" }, { n + "pw", "ANCHO" }, { n + "noise", "RUIDO" } } },
                        { "FILTROS", { { n + "hpf", "PASAALTOS" }, { n + "lpf", "PASABAJOS" }, { n + "res", "RESON." }, { n + "fenv", "ENV" } } } });
            set.insert (set.end(), lastCreated.begin(), lastCreated.end());
            row (306, { { "ENVOLVENTE DEL CANAL", { { n + "attack", "ATAQUE" }, { n + "decay", u8 ("CAÍDA") }, { n + "sustain", u8 ("SOSTÉN") }, { n + "release", u8 ("LIBERACIÓN") } } } });
            set.insert (set.end(), lastCreated.begin(), lastCreated.end());
            layerSets.push_back (set);
            keep = bands;
        }
        bands = keep;
        row (450, { { "MEZCLA", { { "cs80_balance", "I / II" }, { "cs80_detune", "DESAF. II" } } },
                    { "ANILLO", { { "cs80_ring_depth", "PROFUND." }, { "cs80_ring_speed", "VELOC." } } },
                    { "AFTERTOUCH", { { "cs80_at_bright", "BRILLO" }, { "cs80_at_vib", "VIBRATO" } } } });
        row (586, { { "PORTAMENTO", { { "cs80_porta", "TIEMPO" }, { "cs80_gliss", "GLISSANDO" } } } });
        ribbon = std::make_unique<Ribbon> (*s.getParameter ("cs80_ribbon"), id);
        ribbon->setBounds (48, 740, 536, 80);
        ribbon->getProperties().set ("help", "cs80_ribbon");
        ribbon->addMouseListener (&history, false);
        addAndMakeVisible (*ribbon);
        select (0);
    }

    void reveal (const String& paramId) override
    {
        if (paramId.startsWith ("cs80_l1_") && layer != 0) select (0);
        if (paramId.startsWith ("cs80_l2_") && layer != 1) select (1);
    }

    void select (int L)
    {
        layer = L;
        for (int i = 0; i < 2; ++i) for (auto* c : layerSets[(size_t) i]) c->setVisible (i == L);
        repaint();
    }

    void paint (Graphics& g) override
    {
        EnginePage::paint (g);
        const Colour layerCol[] = { Colour (0xffc8322a), Colour (0xff3a6fc8) };
        for (int L = 0; L < 2; ++L)
        {
            auto r = layerButtons[(size_t) L]->getBounds().toFloat();
            const bool on = L == layer;
            g.setColour (on ? layerCol[L] : Colour (0xff1e1d1a)); g.fillRoundedRectangle (r, 4.0f);
            g.setColour (layerCol[L].brighter (0.3f)); g.drawRoundedRectangle (r, 4.0f, 1.4f);
            theme::drawPrintedText (g, L == 0 ? "CANAL I" : "CANAL II", r.getCentreX(), r.getCentreY(), 15.0f, 0.0f,
                                    on ? Colours::white : theme::cream.withAlpha (0.7f), theme::black, 0.0f);
        }
    }

private:
    std::vector<std::unique_ptr<TextButton>> layerButtons;
    std::vector<std::vector<Component*>> layerSets;
    std::unique_ptr<Ribbon> ribbon;
    int layer = 0;
};

//============================================================================== ARP 2600: patch bay
class PatchBay : public Component
{
public:
    PatchBay (AudioProcessorValueTreeState& s) : state (s)
    {
        setInterceptsMouseClicks (false, false);
        for (int c = 0; c < 4; ++c)
            for (auto suffix : { "_src", "_dst", "_amt" })
            {
                auto* p = s.getParameter ("arp2600_c" + String (c + 1) + suffix);
                atts.push_back (std::make_unique<ParameterAttachment> (*p, [this] (float) { repaint(); }, nullptr));
            }
    }

    static Point<float> jack (bool source, int index, Rectangle<float> r)
    {
        const int n = source ? dsp::Arp2600Engine::cableSources().size() : dsp::Arp2600Engine::cableDestinations().size();
        return { source ? r.getX() + 52 : r.getRight() - 72, r.getY() + 26 + (r.getHeight() - 40) * (float) index / (float) (n - 1) };
    }

    void paint (Graphics& g) override
    {
        auto r = getLocalBounds().toFloat();
        auto drawJacks = [&] (bool src)
        {
            const auto& names = src ? dsp::Arp2600Engine::cableSources() : dsp::Arp2600Engine::cableDestinations();
            for (int i = 0; i < names.size(); ++i)
            {
                const auto p = jack (src, i, r);
                g.setColour (Colour (0xff0d0f12)); g.fillEllipse (Rectangle<float> (16, 16).withCentre (p));
                g.setColour (Colour (0xffc9ccd0)); g.drawEllipse (Rectangle<float> (16, 16).withCentre (p), 2.0f);
                theme::drawPrintedText (g, names[i], src ? p.x - 30 : p.x + 36, p.y, 9.5f, 0.0f, theme::cream, theme::black, 0.0f, false);
            }
        };
        drawJacks (true); drawJacks (false);

        const Colour cols[] = { Colour (0xffd03a2a), Colour (0xffe8c23a), Colour (0xff3a7fd0), Colour (0xff3aa04a) };
        for (int c = 0; c < 4; ++c)
        {
            const int src = roundToInt (state.getParameter ("arp2600_c" + String (c + 1) + "_src")->convertFrom0to1 (state.getParameter ("arp2600_c" + String (c + 1) + "_src")->getValue()));
            const int dst = roundToInt (state.getParameter ("arp2600_c" + String (c + 1) + "_dst")->convertFrom0to1 (state.getParameter ("arp2600_c" + String (c + 1) + "_dst")->getValue()));
            const float amt = state.getParameter ("arp2600_c" + String (c + 1) + "_amt")->convertFrom0to1 (state.getParameter ("arp2600_c" + String (c + 1) + "_amt")->getValue());
            const auto a = jack (true, src, r), b = jack (false, dst, r);
            const bool plugged = std::abs (amt) > 0.005f;
            Path cable; cable.startNewSubPath (a);
            const float sag = 60.0f + 18.0f * c;
            cable.cubicTo (a.translated (60, sag), b.translated (-60, sag), b);
            g.setColour (Colours::black.withAlpha (plugged ? 0.45f : 0.15f));
            g.strokePath (cable, PathStrokeType (plugged ? 7.0f : 4.0f, PathStrokeType::curved, PathStrokeType::rounded), AffineTransform::translation (2, 3));
            g.setColour (cols[c].withAlpha (plugged ? 0.95f : 0.22f));
            g.strokePath (cable, PathStrokeType (plugged ? 2.5f + 3.0f * std::abs (amt) : 2.0f, PathStrokeType::curved, PathStrokeType::rounded));
            for (auto p : { a, b }) { g.setColour (cols[c].darker (0.3f).withAlpha (plugged ? 1.0f : 0.3f)); g.fillEllipse (Rectangle<float> (10, 10).withCentre (p)); }
        }
    }

private:
    AudioProcessorValueTreeState& state;
    std::vector<std::unique_ptr<ParameterAttachment>> atts;
};

class ArpPage : public EnginePage
{
public:
    ArpPage (AudioProcessorValueTreeState& s, MouseListener& h) : EnginePage (7, s, h)
    {
        setSize (632, 1064);
        row (112, { { "VCO 1", { { "arp2600_vco1_wave", "FORMA" }, { "arp2600_vco1_level", "NIVEL" } } },
                    { "VCO 2", { { "arp2600_vco2_wave", "FORMA" }, { "arp2600_vco2_tune", u8 ("AFINACIÓN") }, { "arp2600_vco2_pw", "ANCHO" }, { "arp2600_vco2_level", "NIVEL" } } } });
        row (240, { { "VCO 3", { { "arp2600_vco3_wave", "FORMA" }, { "arp2600_vco3_tune", u8 ("AFINACIÓN") }, { "arp2600_vco3_lf", "LFO" },
                                 { "arp2600_vco3_lfrate", "VELOC. LFO" }, { "arp2600_vco3_level", "NIVEL" } } },
                    { "RUIDO", { { "arp2600_noise", "NIVEL" }, { "arp2600_noise_color", "COLOR" } } },
                    { "ANILLO", { { "arp2600_ring", "NIVEL" } } } });
        row (368, { { "FILTRO", { { "arp2600_filter_mode", "MODO" }, { "arp2600_cutoff", "CORTE" }, { "arp2600_res", "RESON." },
                                  { "arp2600_fenv", "ADSR" }, { "arp2600_kbd", "TECLADO" } } },
                    { "PORTAMENTO", { { "arp2600_porta", "TIEMPO" } } } });
        row (496, { { "ADSR", { { "arp2600_attack", "ATAQUE" }, { "arp2600_decay", u8 ("CAÍDA") }, { "arp2600_sustain", u8 ("SOSTÉN") }, { "arp2600_release", "LIBERAC." } } },
                    { "AR", { { "arp2600_ar_attack", "ATAQUE" }, { "arp2600_ar_release", "LIBERAC." } } },
                    { "S&H", { { "arp2600_sh_rate", "RELOJ" } } } });

        bands.push_back ({ "PATCH BAY (4 CABLES)", { 44, 630, 544, 370 } });
        bay = std::make_unique<PatchBay> (s);
        bay->setBounds (44, 640, 544, 350);
        addAndMakeVisible (*bay);
        for (int c = 0; c < 4; ++c)
        {
            const String n ("arp2600_c" + String (c + 1) + "_");
            const int y = 650 + c * 84;
            place (n + "src", "CABLE " + String (c + 1), { 186, y, 120, 58 });
            place (n + "dst", "DESTINO", { 316, y, 120, 58 });
            place (n + "amt", "CANTIDAD", { 438, y - 6, 40, 84 });
        }
    }
private:
    std::unique_ptr<PatchBay> bay;
};

//============================================================================== VCS3: matriz de pines y joystick
class PinMatrix : public Component, public help::Provider
{
public:
    explicit PinMatrix (AudioProcessorValueTreeState& s) : state (s)
    {
        for (int r = 0; r < 8; ++r)
            for (int c = 0; c < 7; ++c)
            {
                auto* p = s.getParameter (dsp::VCS3Engine::pinId (r, c));
                atts.push_back (std::make_unique<ParameterAttachment> (*p, [this, r, c] (float v) { on[r][c] = v > 0.5f; repaint(); }, nullptr));
                atts.back()->sendInitialUpdate();
            }
        setMouseCursor (MouseCursor::PointingHandCursor);
    }

    String helpKeyAt (Point<int> p) override
    {
        for (int r = 0; r < 8; ++r) for (int c = 0; c < 7; ++c)
            if (cell (r, c).contains (p.toFloat())) return "pin:" + String (r) + ":" + String (c);
        return {};
    }

    Rectangle<float> cell (int r, int c) const
    {
        const float x0 = 100, y0 = 70, cw = ((float) getWidth() - x0 - 6) / 7.0f, ch = ((float) getHeight() - y0 - 6) / 8.0f;
        return { x0 + c * cw, y0 + r * ch, cw, ch };
    }

    void mouseDown (const MouseEvent& e) override
    {
        if (e.mods.isPopupMenu()) return;
        for (int r = 0; r < 8; ++r) for (int c = 0; c < 7; ++c)
            if (cell (r, c).contains (e.position)) { atts[(size_t) (r * 7 + c)]->setValueAsCompleteGesture (on[r][c] ? 0.0f : 1.0f); return; }
    }

    void paint (Graphics& g) override
    {
        auto full = getLocalBounds().toFloat();
        g.setColour (Colour (0xffe8e2d2)); g.fillRoundedRectangle (full.withTrimmedLeft (96).withTrimmedTop (66), 3.0f);
        for (int c = 0; c < 7; ++c)
        {
            const auto r = cell (0, c);
            theme::drawPrintedText (g, dsp::VCS3Engine::pinDestinations()[c], r.getCentreX(), 40, 9.5f, -60.0f, theme::cream, theme::black, 0.0f, false);
        }
        for (int r = 0; r < 8; ++r)
        {
            const auto c0 = cell (r, 0);
            theme::drawPrintedText (g, dsp::VCS3Engine::pinSources()[r], 50, c0.getCentreY(), 10.0f, 0.0f, theme::cream, theme::black, 0.0f, false);
            for (int c = 0; c < 7; ++c)
            {
                const auto ce = cell (r, c);
                g.setColour (Colours::black.withAlpha (0.12f)); g.drawRect (ce, 0.6f);
                const auto p = ce.getCentre();
                g.setColour (Colour (0xff2a2622)); g.fillEllipse (Rectangle<float> (7, 7).withCentre (p));
                if (on[r][c]) // pin puesto (blanco con cabeza de color)
                {
                    g.setColour (Colours::black.withAlpha (0.35f)); g.fillEllipse (Rectangle<float> (15, 15).withCentre (p.translated (1.5f, 2)));
                    g.setGradientFill (ColourGradient (Colour (0xffd7a6ff), p.x - 4, p.y - 4, Colour (0xff6a2fb0), p.x + 5, p.y + 5, false));
                    g.fillEllipse (Rectangle<float> (14, 14).withCentre (p));
                    g.setColour (Colours::white.withAlpha (0.7f)); g.fillEllipse (Rectangle<float> (4, 4).withCentre (p.translated (-2.5f, -2.5f)));
                }
            }
        }
    }

private:
    AudioProcessorValueTreeState& state;
    bool on[8][7] {};
    std::vector<std::unique_ptr<ParameterAttachment>> atts;
};

class Joystick : public Component
{
public:
    Joystick (AudioProcessorValueTreeState& s, const Identity& i)
        : id (i),
          ax (*s.getParameter ("vcs3_joy_x"), [this] (float v) { x = v; repaint(); }, nullptr),
          ay (*s.getParameter ("vcs3_joy_y"), [this] (float v) { y = v; repaint(); }, nullptr)
    { ax.sendInitialUpdate(); ay.sendInitialUpdate(); setMouseCursor (MouseCursor::CrosshairCursor); }

    Rectangle<float> pad() const { return getLocalBounds().toFloat().withTrimmedTop (18).reduced (6); }
    void mouseDown (const MouseEvent& e) override { ax.beginGesture(); ay.beginGesture(); mouseDrag (e); }
    void mouseDrag (const MouseEvent& e) override
    {
        const auto p = pad();
        ax.setValueAsPartOfGesture (jlimit (-1.0f, 1.0f, (e.position.x - p.getCentreX()) / (p.getWidth() * 0.5f)));
        ay.setValueAsPartOfGesture (jlimit (-1.0f, 1.0f, (p.getCentreY() - e.position.y) / (p.getHeight() * 0.5f)));
    }
    void mouseUp (const MouseEvent&) override { ax.endGesture(); ay.endGesture(); }

    void paint (Graphics& g) override
    {
        theme::drawPrintedText (g, "JOYSTICK", (float) getWidth() * 0.5f, 8, 11.0f, 0.0f, theme::cream, theme::black, 0.1f);
        const auto p = pad();
        g.setGradientFill (ColourGradient (Colour (0xff1c1820), p.getX(), p.getY(), Colour (0xff070609), p.getRight(), p.getBottom(), false));
        g.fillRoundedRectangle (p, 6.0f);
        g.setColour (id.accent.withAlpha (0.4f)); g.drawRoundedRectangle (p, 6.0f, 1.2f);
        g.setColour (Colours::white.withAlpha (0.1f));
        g.drawLine (p.getCentreX(), p.getY() + 4, p.getCentreX(), p.getBottom() - 4);
        g.drawLine (p.getX() + 4, p.getCentreY(), p.getRight() - 4, p.getCentreY());
        const Point<float> k (p.getCentreX() + x * p.getWidth() * 0.45f, p.getCentreY() - y * p.getHeight() * 0.45f);
        g.setColour (Colours::black.withAlpha (0.6f)); g.drawLine ({ p.getCentre(), k }, 6.0f);
        g.setColour (Colour (0xffb9b4aa)); g.drawLine ({ p.getCentre(), k }, 3.0f);
        g.setGradientFill (ColourGradient (id.accent.brighter (0.5f), k.x - 6, k.y - 6, id.accent.darker (0.6f), k.x + 8, k.y + 8, false));
        g.fillEllipse (Rectangle<float> (22, 22).withCentre (k));
        theme::drawPrintedText (g, "X " + String (x, 2) + "   Y " + String (y, 2), p.getCentreX(), p.getBottom() - 10, 9.5f, 0.0f,
                                theme::cream.withAlpha (0.7f), theme::black, 0.0f, false);
    }

private:
    const Identity& id;
    float x = 0, y = 0;
    ParameterAttachment ax, ay;
};

class VCS3Page : public EnginePage
{
public:
    VCS3Page (AudioProcessorValueTreeState& s, MouseListener& h) : EnginePage (8, s, h)
    {
        setSize (632, 1064);
        row (112, { { "OSCILADOR 1", { { "vcs3_osc1_tune", u8 ("AFINACIÓN") }, { "vcs3_osc1_shape", "FORMA" }, { "vcs3_osc1_sine", "SENO" }, { "vcs3_osc1_saw", "SIERRA" } } },
                    { "OSCILADOR 2", { { "vcs3_osc2_tune", u8 ("AFINACIÓN") }, { "vcs3_osc2_pw", "ANCHO" }, { "vcs3_osc2_square", "CUADRADA" }, { "vcs3_osc2_tri", u8 ("TRIÁNGULO") } } } });
        row (240, { { "OSCILADOR 3", { { "vcs3_osc3_lf", "LFO" }, { "vcs3_osc3_rate", "VELOC." }, { "vcs3_osc3_tune", u8 ("AFIN.") },
                                       { "vcs3_osc3_square", "CUADR." }, { "vcs3_osc3_tri", u8 ("TRIÁNG.") } } },
                    { "RUIDO", { { "vcs3_noise", "NIVEL" }, { "vcs3_noise_color", "COLOR" } } },
                    { "ANILLO", { { "vcs3_ring", "NIVEL" } } } });
        row (368, { { "FILTRO", { { "vcs3_cutoff", "CORTE" }, { "vcs3_res", "RESPUESTA" } } },
                    { "TRAPEZOIDE", { { "vcs3_env_attack", "ATAQUE" }, { "vcs3_env_on", "ENCENDIDO" }, { "vcs3_env_decay", u8 ("CAÍDA") },
                                      { "vcs3_env_off", "APAGADO" }, { "vcs3_env_trig", "DISPARO" } } } });
        row (496, { { "TRAPEZOIDE A", { { "vcs3_env_vca", "VCA" }, { "vcs3_env_filter", "FILTRO" } } },
                    { "PROFUNDIDAD DE LA MATRIZ", { { "vcs3_depth_pitch", "TONO" }, { "vcs3_depth_filter", "CORTE" }, { "vcs3_depth_vca", "VCA" } } },
                    { "RESORTE", { { "vcs3_reverb", "REVERB" } } } });

        bands.push_back ({ "MATRIZ DE PINES (FUENTE > DESTINO)", { 44, 630, 380, 370 } });
        matrix = std::make_unique<PinMatrix> (s);
        matrix->setBounds (50, 640, 368, 352);
        matrix->addMouseListener (&history, false);
        addAndMakeVisible (*matrix);
        bands.push_back ({ "CONTROL", { 436, 630, 152, 370 } });
        joy = std::make_unique<Joystick> (s, id);
        joy->setBounds (442, 700, 140, 160);
        joy->getProperties().set ("help", "ui:joystick");
        joy->addMouseListener (&history, false);
        addAndMakeVisible (*joy);
    }
private:
    std::unique_ptr<PinMatrix> matrix;
    std::unique_ptr<Joystick> joy;
};
}

//============================================================================== fabrica de paginas
std::unique_ptr<EnginePage> createEnginePage (int engine, AudioProcessorValueTreeState& s, MouseListener& h)
{
    std::unique_ptr<EnginePage> p;
    switch (engine)
    {
        case 1:
            p = std::make_unique<EnginePage> (1, s, h); p->setSize (632, 1064);
            p->row (112, { { "OSCILADOR A", { { "prophet5_a_saw", "SIERRA" }, { "prophet5_a_pulse", "PULSO" }, { "prophet5_a_pw", "ANCHO" } } },
                           { "MEZCLADOR", { { "prophet5_mix_a", "OSC A" }, { "prophet5_mix_b", "OSC B" }, { "prophet5_noise", "RUIDO" } } } });
            p->row (244, { { "OSCILADOR B", { { "prophet5_b_semi", "SEMITONOS" }, { "prophet5_b_fine", "FINO" }, { "prophet5_b_saw", "SIERRA" },
                                              { "prophet5_b_tri", u8 ("TRIÁNGULO") }, { "prophet5_b_pulse", "PULSO" }, { "prophet5_b_pw", "ANCHO" },
                                              { "prophet5_sync", "SYNC" } } } });
            p->row (376, { { "POLY-MOD", { { "prophet5_pm_env", "ENV FILTRO" }, { "prophet5_pm_oscb", "OSC B" }, { "prophet5_pm_freqa", "> FREC A" }, { "prophet5_pm_filter", "> FILTRO" } } },
                           { "LFO", { { "prophet5_lfo_rate", "VELOC." }, { "prophet5_lfo_freq", "> FREC" }, { "prophet5_lfo_pw", "> ANCHO" }, { "prophet5_lfo_filter", "> FILTRO" } } } });
            p->row (508, { { "FILTRO 24 DB", { { "prophet5_cutoff", "CORTE" }, { "prophet5_res", "RESONANCIA" }, { "prophet5_fenv", "ENV" }, { "prophet5_kbd", "TECLADO" } } },
                           { "VOCES", { { "prophet5_unison", u8 ("UNÍSONO") }, { "prophet5_uni_detune", "DESAFIN." }, { "prophet5_glide", "GLIDE" } } } });
            p->row (640, { { "ENVOLVENTE DEL FILTRO", { { "prophet5_f_attack", "ATAQUE" }, { "prophet5_f_decay", u8 ("CAÍDA") }, { "prophet5_f_sustain", u8 ("SOSTÉN") }, { "prophet5_f_release", "LIBERAC." } } },
                           { "ENVOLVENTE DE VOLUMEN", { { "prophet5_a_attack", "ATAQUE" }, { "prophet5_a_decay", u8 ("CAÍDA") }, { "prophet5_a_sustain", u8 ("SOSTÉN") }, { "prophet5_a_release", "LIBERAC." } } } });
            break;
        case 2:
            p = std::make_unique<EnginePage> (2, s, h); p->setSize (632, 1064);
            p->row (112, { { "VCO-1", { { "jupiter8_vco1_wave", "FORMA" }, { "jupiter8_vco1_range", "PIES" } } },
                           { "VCO-2", { { "jupiter8_vco2_wave", "FORMA" }, { "jupiter8_vco2_range", "PIES" }, { "jupiter8_vco2_fine", "FINO" } } } });
            p->row (234, { { "VCO MOD", { { "jupiter8_cross", "CROSS MOD" }, { "jupiter8_sync", "SYNC" }, { "jupiter8_pw", "ANCHO" }, { "jupiter8_lfo_pwm", "LFO PWM" }, { "jupiter8_mix", "VCO 1 / 2" } } },
                           { "LFO", { { "jupiter8_lfo_rate", "VELOC." }, { "jupiter8_lfo_vco", "> VCO" }, { "jupiter8_lfo_vcf", "> VCF" } } } });
            p->row (356, { { "FORMA DEL LFO", { { "jupiter8_lfo_wave", "ONDA" } } },
                           { "VCF", { { "jupiter8_hpf", "HPF" }, { "jupiter8_cutoff", "CORTE" }, { "jupiter8_res", "RESON." }, { "jupiter8_slope", "PENDIENTE" },
                                      { "jupiter8_env_amt", "ENV-1" }, { "jupiter8_kbd", "TECLADO" } } } });
            p->row (478, { { "ENV-1 (FILTRO)", { { "jupiter8_e1_attack", "A" }, { "jupiter8_e1_decay", "D" }, { "jupiter8_e1_sustain", "S" }, { "jupiter8_e1_release", "R" } } },
                           { "ENV-2 (VCA)", { { "jupiter8_e2_attack", "A" }, { "jupiter8_e2_decay", "D" }, { "jupiter8_e2_sustain", "S" }, { "jupiter8_e2_release", "R" } } } });
            p->row (600, { { "ARPEGIADOR (TODOS LOS MOTORES)", { { "arp_on", "ARP" }, { "arp_mode", "MODO" }, { "arp_rate", "VALOR" }, { "arp_oct", "OCTAVAS" }, { "arp_gate", "GATE" } } } });
            p->row (722, { { "TECLADO", { { "jupiter8_zone", "ZONA" }, { "jupiter8_split", "SPLIT" } } } });
            break;
        case 3:
            p = std::make_unique<EnginePage> (3, s, h); p->setSize (632, 1064);
            p->row (112, { { "LFO", { { "juno_lfo_rate", "VELOC." }, { "juno_lfo_delay", "RETARDO" } } },
                           { "DCO", { { "juno_lfo_dco", "LFO" }, { "juno_pw", "ANCHO" }, { "juno_pwm_src", "PWM" }, { "juno_range", "PIES" } } } });
            p->row (234, { { "FUENTES", { { "juno_saw", "SIERRA" }, { "juno_pulse", "PULSO" }, { "juno_sub", "SUB" }, { "juno_noise", "RUIDO" } } },
                           { "HPF", { { "juno_hpf", "PASAALTOS" } } } });
            p->row (356, { { "VCF", { { "juno_cutoff", "CORTE" }, { "juno_res", "RESON." }, { "juno_env_amt", "ENV" }, { "juno_lfo_vcf", "LFO" }, { "juno_kbd", "TECLADO" } } } });
            p->row (478, { { "VCA", { { "juno_vca_mode", "MODO" } } },
                           { "ENV", { { "juno_attack", "A" }, { "juno_decay", "D" }, { "juno_sustain", "S" }, { "juno_release", "R" } } } });
            p->row (600, { { "CHORUS", { { "juno_chorus", "CHORUS" } } } });
            p->row (722, { { "ARPEGIADOR (TODOS LOS MOTORES)", { { "arp_on", "ARP" }, { "arp_mode", "MODO" }, { "arp_rate", "VALOR" }, { "arp_oct", "OCTAVAS" }, { "arp_gate", "GATE" } } } });
            break;
        case 4: p = std::make_unique<CS80Page> (s, h); break;
        case 5: p = std::make_unique<DX7Page> (s, h); break;
        case 6:
            p = std::make_unique<EnginePage> (6, s, h); p->setSize (632, 1064);
            p->row (112, { { "FUENTE 1", { { "m1_s1_wave", "MUESTRA" }, { "m1_s1_level", "NIVEL" }, { "m1_s1_coarse", "SEMITONOS" }, { "m1_s1_fine", "CENTS" } } } });
            p->row (244, { { "FUENTE 2", { { "m1_s2_wave", "MUESTRA" }, { "m1_s2_level", "NIVEL" }, { "m1_s2_coarse", "SEMITONOS" }, { "m1_s2_fine", "CENTS" } } } });
            p->row (376, { { "FILTRO DIGITAL", { { "m1_cutoff", "CORTE" }, { "m1_fenv", "ENV" }, { "m1_vel_filter", "VELOCIDAD" } } },
                           { "LFO", { { "m1_lfo_rate", "VELOC." }, { "m1_lfo_pitch", "> TONO" }, { "m1_lfo_filter", "> FILTRO" } } } });
            p->row (508, { { "ENVOLVENTE DEL FILTRO", { { "m1_f_attack", "ATAQUE" }, { "m1_f_decay", u8 ("CAÍDA") }, { "m1_f_sustain", u8 ("SOSTÉN") }, { "m1_f_release", "LIBERAC." } } },
                           { "ENVOLVENTE DE VOLUMEN", { { "m1_a_attack", "ATAQUE" }, { "m1_a_decay", u8 ("CAÍDA") }, { "m1_a_sustain", u8 ("SOSTÉN") }, { "m1_a_release", "LIBERAC." } } } });
            p->row (640, { { "EFECTOS", { { "m1_fx_type", "TIPO" }, { "m1_fx_mix", "MEZCLA" }, { "m1_delay_time", "TIEMPO DELAY" } } } });
            break;
        case 7: p = std::make_unique<ArpPage> (s, h); break;
        case 8: p = std::make_unique<VCS3Page> (s, h); break;
        default: break;
    }
    return p;
}
}
