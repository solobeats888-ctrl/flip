#include "DesignPanel.h"
#include "../DSP/MinimoogEngine.h"
#include "../DSP/SynthCore.h"
#include "DesignWidgets.h"

namespace flipper
{
using namespace juce;
static String u8 (const char* s) { return String::fromUTF8 (s); }

// Paleta de KROKO: verde reptil + dorado
static const Colour krokoGreen { 0xff8fae3a };
static const Colour krokoDark  { 0xff3d4f17 };

DesignPanel::DesignPanel (FlipperProcessor& p, MouseListener& hl) : proc (p), history (hl)
{
    addChildComponent (minimoogPage);
    minimoogPage.setInterceptsMouseClicks (false, true);
    buildMinimoog();
    for (int e = 1; e < numEngines; ++e)
    {
        pages[(size_t) e] = createEnginePage (e, proc.state, history);
        pages[(size_t) e]->setTopLeftPosition (0, 0);
        addChildComponent (*pages[(size_t) e]);
    }
    for (int e = 0; e < numEngines; ++e) portraits[(size_t) e] = theme::image (engines()[(size_t) e].portrait);
    setEngine (0);
}

VintageKnob& DesignPanel::addKnob (const String& paramId, const String& label, float x, float y, VintageKnob::Style st)
{
    auto k = std::make_unique<VintageKnob> (label, st);
    const int w = 112, h = 130;
    k->setBounds ((int) x - w / 2, (int) y - 62, w, h);
    k->centre = { (float) w / 2, 62.0f };
    k->addMouseListener (&history, false);
    minimoogPage.addAndMakeVisible (*k);
    attachments.push_back (std::make_unique<AudioProcessorValueTreeState::SliderAttachment> (proc.state, paramId, *k));
    if (auto* prm = proc.state.getParameter (paramId))
        k->setDoubleClickReturnValue (true, prm->convertFrom0to1 (prm->getDefaultValue()));
    knobs.push_back (std::move (k));
    return *knobs.back();
}

void DesignPanel::buildMinimoog()
{
    using MM = dsp::MinimoogEngine;
    minimoogPage.setBounds (0, 0, boardBounds().getWidth(), boardBounds().getHeight());
    portrait = theme::image ("minimoog_character_png");

    VintageKnob::Style knob;   knob.accent = krokoGreen; knob.radius = 20.0f;
    knob.scale = { "0", "2", "4", "6", "8", "10" };
    VintageKnob::Style big = knob; big.radius = 27.0f;
    VintageKnob::Style tune = knob; tune.scale = { "-7", "0", "+7" };
    VintageKnob::Style range = knob; range.scale = {}; range.positions = { "32'", "16'", "8'", "4'", "2'" };
    VintageKnob::Style wave = range; wave.positions = { "1", "2", "3", "4", "5", "6" }; wave.waveIcons = true;

    // ---- BANCO DE OSCILADORES / MEZCLADOR (filas como en el panel del Model D)
    const float rowY[] = { 196, 302, 408 };
    for (int o = 0; o < 3; ++o)
    {
        const String n (o + 1);
        addKnob (MM::pid (("osc" + n + "_range").toRawUTF8()), "RANGO", 132, rowY[o], range);
        if (o > 0) addKnob (MM::pid (("osc" + n + "_detune").toRawUTF8()), "FRECUENCIA", 242, rowY[o], tune);
        addKnob (MM::pid (("osc" + n + "_wave").toRawUTF8()), "FORMA DE ONDA", 352, rowY[o], wave);
        addKnob (MM::pid (("mix" + n).toRawUTF8()), "OSC " + n, 510, rowY[o], knob);
    }
    addKnob (MM::pid ("glide"), "GLIDE", 132, 522, knob);
    addKnob (MM::pid ("noise"), "RUIDO", 510, 512, knob);

    bands.push_back ({ "CONTROLES", { 76, 462, 112, 112 } });
    bands.push_back ({ "BANCO DE OSCILADORES", { 76, 128, 336, 326 } });
    bands.push_back ({ "MEZCLADOR", { 446, 128, 128, 438 } });

    // ---- MODIFICADORES
    addKnob (MM::pid ("cutoff"), "FRECUENCIA DE CORTE", 112, 664, big);
    addKnob (MM::pid ("emphasis"), u8 ("ÉNFASIS"), 226, 664, knob);
    addKnob (MM::pid ("contour"), "CONTORNO", 330, 664, knob);
    addKnob (MM::pid ("kbd"), "TECLADO", 430, 664, knob);
    addKnob (MM::pid ("drive"), u8 ("SATURACIÓN"), 526, 664, knob);

    const char* adsr[] = { "TIEMPO DE ATAQUE", "TIEMPO DE CAÍDA", "NIVEL DE SOSTÉN", "LIBERACIÓN" };
    const char* fIds[] = { "f_attack", "f_decay", "f_sustain", "f_release" };
    const char* aIds[] = { "a_attack", "a_decay", "a_sustain", "a_release" };
    for (int i = 0; i < 4; ++i) addKnob (MM::pid (fIds[i]), u8 (adsr[i]), 134.0f + i * 120.0f, 790, knob);
    for (int i = 0; i < 4; ++i) addKnob (MM::pid (aIds[i]), u8 (adsr[i]), 134.0f + i * 120.0f, 916, knob);

    bands.push_back ({ "FILTRO", { 52, 590, 528, 124 } });
    bands.push_back ({ "CONTORNO DEL FILTRO", { 76, 726, 480, 116 } });
    bands.push_back ({ "CONTORNO DE VOLUMEN", { 76, 852, 480, 116 } });
}

void DesignPanel::setEngine (int e)
{
    engine = e;
    minimoogPage.setVisible (e == 0);
    for (int i = 1; i < numEngines; ++i) pages[(size_t) i]->setVisible (i == e);
    repaint();
}

//============================================================================== dibujo
static void paintFrame (Graphics& g, Rectangle<float> b)
{
    g.setColour (Colours::black.withAlpha (0.55f));
    g.fillRoundedRectangle (b.translated (4, 6), 14.0f);
    g.setGradientFill (ColourGradient (Colour (0xf6231a14), b.getX(), b.getY(), Colour (0xf60e0a08), b.getX(), b.getBottom(), false));
    g.fillRoundedRectangle (b, 14.0f);
    g.setColour (theme::gold.withAlpha (0.85f));
    g.drawRoundedRectangle (b.reduced (3.0f), 12.0f, 3.0f);
}

static void paintWood (Graphics& g, Rectangle<float> r, int seed)
{
    // costado de madera (nogal gastado), como los laterales del Model D
    g.setGradientFill (ColourGradient (Colour (0xff4a2a16), r.getX(), r.getY(), Colour (0xff2a160b), r.getRight(), r.getY(), false));
    g.fillRoundedRectangle (r, 6.0f);
    Random rnd (seed);
    for (int i = 0; i < 26; ++i)
    {
        Path grain;
        float x = r.getX() + rnd.nextFloat() * r.getWidth();
        grain.startNewSubPath (x, r.getY());
        for (float y = r.getY(); y < r.getBottom(); y += 40.0f)
        {
            x += rnd.nextFloat() * 2.4f - 1.2f;
            grain.lineTo (jlimit (r.getX(), r.getRight(), x), y + 40.0f);
        }
        g.setColour ((rnd.nextBool() ? Colour (0xff6b4024) : Colour (0xff1a0d06)).withAlpha (0.25f + rnd.nextFloat() * 0.3f));
        g.strokePath (grain, PathStrokeType (0.6f + rnd.nextFloat() * 1.4f));
    }
    g.setColour (Colours::black.withAlpha (0.45f));
    g.drawRoundedRectangle (r, 6.0f, 1.5f);
}

void DesignPanel::paintMinimoog (Graphics& g, Rectangle<float> panel)
{
    // panel negro de aluminio gastado entre los dos costados de madera
    paintWood (g, { panel.getX() + 8, panel.getY() + 96, 20, panel.getHeight() - 130 }, 11);
    paintWood (g, { panel.getRight() - 28, panel.getY() + 96, 20, panel.getHeight() - 130 }, 23);

    auto face = Rectangle<float> (panel.getX() + 32, panel.getY() + 100, panel.getWidth() - 64, panel.getHeight() - 140);
    g.setGradientFill (ColourGradient (Colour (0xff181614), face.getX(), face.getY(), Colour (0xff0b0a09), face.getX(), face.getBottom(), false));
    g.fillRect (face);
    Random r (1970);
    for (int i = 0; i < 260; ++i) // cepillado horizontal
    {
        g.setColour (Colours::white.withAlpha (0.012f + r.nextFloat() * 0.02f));
        const float y = face.getY() + r.nextFloat() * face.getHeight();
        const float x = face.getX() + r.nextFloat() * face.getWidth();
        g.drawHorizontalLine ((int) y, x, jmin (face.getRight(), x + 30 + r.nextFloat() * 160));
    }
    for (int i = 0; i < 500; ++i) // desgaste
    {
        g.setColour ((r.nextBool() ? theme::cream : Colours::black).withAlpha (0.03f + r.nextFloat() * 0.05f));
        const float s = 0.8f + r.nextFloat() * 2.0f;
        g.fillEllipse (face.getX() + r.nextFloat() * face.getWidth(), face.getY() + r.nextFloat() * face.getHeight(), s, s);
    }
    g.setColour (theme::gold.withAlpha (0.5f));
    g.drawRect (face, 1.2f);

    // bandas de seccion: rotulo abajo, en el verde de KROKO, como las leyendas del panel original
    for (auto& b : bands)
    {
        g.setColour (Colours::white.withAlpha (0.025f));
        g.fillRoundedRectangle (b.area, 4.0f);
        g.setColour (krokoGreen.withAlpha (0.35f));
        g.drawRoundedRectangle (b.area, 4.0f, 1.0f);
        const float ty = b.area.getY();
        const float tw = theme::textWidth (b.title, 11.0f);
        g.setColour (Colour (0xff0d0c0b));
        g.fillRect (Rectangle<float> (tw + 16, 14).withCentre ({ b.area.getCentreX(), ty }));
        theme::drawPrintedText (g, b.title, b.area.getCentreX(), ty, 11.0f, 0.0f, krokoGreen.brighter (0.4f), theme::black, 0.15f, false);
    }

    // numeros de oscilador al costado de cada fila
    const float rowY[] = { 196, 302, 408 };
    for (int o = 0; o < 3; ++o)
        theme::drawPrintedText (g, "OSC " + String (o + 1), 52.0f, rowY[o] + 2.0f, 12.0f, -90.0f, theme::mustard, theme::black, 0.2f);
    // divisor del grupo MODIFICADORES
    theme::drawPrintedText (g, "MODIFICADORES", 40.0f, 778.0f, 13.0f, -90.0f, theme::mustard, theme::black, 0.2f);
}

void DesignPanel::paint (Graphics& g)
{
    auto b = getLocalBounds().toFloat().reduced (4.0f);
    paintFrame (g, b);
    const float W = (float) getWidth();
    const auto& info = engines()[(size_t) engine];

    if (engine == 0) paintMinimoog (g, b);

    // cabecera comun: medallon del personaje + nombre en su color + motor
    const auto& idt = identityFor (engine);
    const Rectangle<float> medal (40, 18, 64, 78);
    {
        Graphics::ScopedSaveState s (g);
        Path clip; clip.addEllipse (medal);
        g.reduceClipRegion (clip);
        g.drawImage (portraits[(size_t) engine], medal, RectanglePlacement::centred | RectanglePlacement::fillDestination);
    }
    g.setColour (idt.accent); g.drawEllipse (medal, 2.5f);
    g.setColour (theme::gold);  g.drawEllipse (medal.expanded (3), 1.2f);
    const String name = u8 (info.character);
    const float nh = theme::fitHeight (name, 210.0f, 28.0f);
    theme::drawPrintedText (g, name, 118.0f + theme::textWidth (name, nh) * 0.5f, 44.0f, nh, 0.0f, idt.accent.brighter (0.25f), theme::black, 0.3f);
    const String motor = "MOTOR " + String (info.technical);
    theme::drawPrintedText (g, motor, 118.0f + theme::textWidth (motor, 13.0f) * 0.5f, 76.0f, 13.0f, 0.0f, theme::cream, theme::black, 0.1f);
    theme::drawPrintedText (g, u8 ("DISEÑO"), W - 110.0f, 44.0f, 26.0f, 0.0f, theme::mustardHi, theme::black, 0.3f);
    const String sub = u8 (idt.subtitle);
    theme::drawPrintedText (g, sub, W - 110.0f, 76.0f, theme::fitHeight (sub, 190.0f, 11.0f), 0.0f, theme::cream.withAlpha (0.7f), theme::black, 0.0f);
    if (engine == 0)
        theme::drawPrintedText (g, u8 ("LAS MACROS DE JUGAR SE SUMAN A ESTOS VALORES  ·  DOBLE CLICK = VALOR ORIGINAL"),
                                W * 0.5f, b.getBottom() - 22.0f, 10.0f, 0.0f, theme::cream.withAlpha (0.5f), theme::black, 0.0f);
}
}
