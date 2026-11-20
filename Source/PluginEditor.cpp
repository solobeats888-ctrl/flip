#include "PluginEditor.h"
#include "Core/SoundGenerator.h"
#include "Core/PresetManager.h"

using namespace juce;
using namespace flipper;

namespace
{
constexpr int boardW = 1024, boardH = 1536;
constexpr int defaultWidth = 640;
constexpr float macroY = 1376.0f;
}

//============================================================================== Board
FlipperBoard::FlipperBoard()
    : background (theme::image ("main_playfield_jpg"))
{
    setOpaque (true);
    setSize (boardW, boardH);
}

void FlipperBoard::paint (Graphics& g)
{
    // el fondo y los textos fijos se dibujan una vez al tamano real de pantalla
    const float sf = jmax (0.1f, g.getInternalContext().getPhysicalPixelScaleFactor());
    if (! cacheValid || std::abs (sf - cacheScale) > 0.001f)
    {
        cache = Image (Image::RGB, roundToInt (boardW * sf), roundToInt (boardH * sf), false);
        Graphics cg (cache);
        cg.addTransform (AffineTransform::scale (sf));
        paintContent (cg);
        cacheScale = sf;
        cacheValid = true;
    }
    g.setImageResamplingQuality (Graphics::lowResamplingQuality);
    g.drawImageTransformed (cache, AffineTransform::scale (1.0f / sf));
}

void FlipperBoard::paintContent (Graphics& g)
{
    g.setImageResamplingQuality (Graphics::highResamplingQuality);
    g.drawImageAt (background, 0, 0);

    // subtitulo
    const char* lines[] = { "MOTOR DE", "SÍNTESIS", "VINTAGE" };
    const float ys[] = { 37.0f, 71.0f, 104.0f };
    for (int i = 0; i < 3; ++i)
    {
        const String t = String::fromUTF8 (lines[i]);
        theme::drawPrintedText (g, t, 870.0f, ys[i], theme::fitHeight (t, 112.0f, 27.0f), -7.0f,
                                theme::mustard, theme::black, 0.35f);
    }

    // MEZCLA DE MOTORES
    theme::drawPrintedText (g, "MEZCLA DE", 912.0f, 265.0f, 19.0f, 0.0f, theme::cream, theme::black, 0.3f);
    theme::drawPrintedText (g, "MOTORES",   912.0f, 288.0f, 19.0f, 0.0f, theme::cream, theme::black, 0.3f);

    // nombres tecnicos de los motores
    for (int e = 0; e < numEngines; ++e)
    {
        const auto& info = engines()[(size_t) e];
        const String t (info.technical);
        const float h = theme::fitHeight (t, 68.0f, 15.5f);
        const float w = theme::textWidth (t, h);
        const auto col = engineOn[(size_t) e] ? theme::cream : theme::cream.withAlpha (0.4f);
        theme::drawPrintedText (g, t, 879.0f + w * 0.5f, info.mixRowY + 1.0f, h, 0.0f, col, theme::black, 0.2f);
    }
}

//============================================================================== Editor
FlipperEditor::FlipperEditor (FlipperProcessor& p)
    : AudioProcessorEditor (p), proc (p), state (p.state),
      labPanel (p),
      designPanel (p, *this),
      generateButton ("GENERAR", ArcadeButton::Style::lit, { 4.0f, 78.0f, 62.0f, 20.0f }),
      variationsButton ("VARIACIONES", ArcadeButton::Style::dark, { 3.0f, 80.0f, 50.0f, 16.0f }),
      favoriteButton ("FAVORITO", ArcadeButton::Style::dark, { 3.0f, 80.0f, 51.0f, 16.0f })
{
    setLookAndFeel (&lnf);

    addAndMakeVisible (board);

    board.addAndMakeVisible (playfield);
    playfield.setBounds (0, 0, boardW, boardH);
    playfield.onSelect = [this] (int e) { selectEngine (e); animLayer.trigger (e); };

    // animaciones de los personajes (encima del arte, debajo de los paneles)
    board.addAndMakeVisible (animLayer);
    animLayer.setBounds (0, 0, boardW, boardH);
    animLayer.onSfx = [this] (anim::Sfx s, int e) { proc.sfx.trigger ((int) s, e); };
    animLayer.setEnabled ((bool) state.state.getProperty (ids::animOn, true));
    proc.sfx.enabled.store ((bool) state.state.getProperty (ids::sfxOn, false));
    labPanel.onAnimationsChanged = [this] (bool on) { animLayer.setEnabled (on); };

    // modos
    const String modeNames[] = { "JUGAR", String::fromUTF8 ("DISEÑO"), "LAB" };
    const Rectangle<int> modeBounds[] = { { 24, 266, 150, 92 }, { 24, 362, 150, 88 }, { 24, 452, 150, 88 } };
    const Point<float> ledC[] = { { 42.0f, 55.0f }, { 37.0f, 50.0f }, { 37.0f, 43.0f } };
    const float ledR[] = { 22.0f, 20.0f, 20.0f };
    const Rectangle<float> textA[] = { { 68, 36, 80, 38 }, { 62, 33, 84, 31 }, { 62, 26, 84, 31 } };
    for (int i = 0; i < 3; ++i)
    {
        modeButtons[(size_t) i] = std::make_unique<ModeButton> (modeNames[i], ledC[i], ledR[i], textA[i]);
        modeButtons[(size_t) i]->setBounds (modeBounds[i]);
        modeButtons[(size_t) i]->onClick = [this, i] { setMode ((UiMode) i); };
        board.addAndMakeVisible (*modeButtons[(size_t) i]);
    }

    // mezcla de motores
    for (int e = 0; e < numEngines; ++e)
    {
        const float y = engines()[(size_t) e].mixRowY;
        leds[(size_t) e] = std::make_unique<EngineLed>();
        leds[(size_t) e]->setBounds (855 - 16, (int) y - 16, 32, 32);
        leds[(size_t) e]->setTooltip ("Activar / desactivar");
        board.addAndMakeVisible (*leds[(size_t) e]);
        buttonAtts.push_back (std::make_unique<ButtonAtt> (state, ids::engineOn (e), *leds[(size_t) e]));

        pills[(size_t) e] = std::make_unique<LevelPill>();
        pills[(size_t) e]->setBounds (949, (int) y - 13, 47, 26);
        board.addAndMakeVisible (*pills[(size_t) e]);
        sliderAtts.push_back (std::make_unique<SliderAtt> (state, ids::engineLevel (e), *pills[(size_t) e]));
        pills[(size_t) e]->setDoubleClickReturnValue (true, 0.7);

        onParams[(size_t) e]    = state.getParameter (ids::engineOn (e));
        levelParams[(size_t) e] = state.getParameter (ids::engineLevel (e));
    }

    // panel PERSONAJE
    board.addAndMakeVisible (characterPanel);
    characterPanel.setBounds (CharacterPanel::boardBounds());
    sliderAtts.push_back (std::make_unique<SliderAtt> (state, ids::character, characterPanel.knob));
    characterPanel.knob.setDoubleClickReturnValue (true, 0.6);

    // macros
    for (int m = 0; m < numMacros; ++m)
    {
        const auto& info = macros()[(size_t) m];
        auto res = "knob_macro_" + String (m + 1) + "_png";
        macroKnobs[(size_t) m] = std::make_unique<SpriteKnob> (res.toRawUTF8(), 27.0f, 33.5f, String::fromUTF8 (info.label));
        macroKnobs[(size_t) m]->setBounds ((int) info.x - 48, (int) macroY - 44, 96, 118);
        macroKnobs[(size_t) m]->centre = { 48.0f, 44.0f };
        board.addAndMakeVisible (*macroKnobs[(size_t) m]);
        sliderAtts.push_back (std::make_unique<SliderAtt> (state, ids::macro (m), *macroKnobs[(size_t) m]));
        macroKnobs[(size_t) m]->setDoubleClickReturnValue (true, 0.5);
    }

    // botones
    generateButton.setBounds (812, 1335, 70, 116);
    variationsButton.setBounds (887, 1335, 56, 116);
    favoriteButton.setBounds (945, 1335, 57, 116);
    generateButton.onClick   = [this] { generate(); };
    variationsButton.onClick = [this] { showVariations(); };
    favoriteButton.onClick   = [this] { saveFavorite(); };
    for (auto* b : { &generateButton, &variationsButton, &favoriteButton })
        board.addAndMakeVisible (*b);

    // pantalla DISENO (controles del motor elegido)
    board.addChildComponent (designPanel);
    designPanel.setBounds (DesignPanel::boardBounds());

    // pantalla LAB
    board.addChildComponent (labPanel);
    labPanel.setBounds (LabPanel::boardBounds());

    // historial: foto antes de tocar cualquier control de sonido
    for (auto& k : macroKnobs) k->addMouseListener (this, false);
    for (auto& l : leds) l->addMouseListener (this, false);
    for (auto& pl : pills) pl->addMouseListener (this, false);
    characterPanel.knob.addMouseListener (this, false);

    // estado guardado
    selectEngine ((int) state.state.getProperty (ids::selectedEngine, 8));
    setMode ((UiMode) (int) state.state.getProperty (ids::uiMode, 0));
    refreshEngineStates (true);

    setResizable (true, true);
    setResizeLimits (boardW / 2, boardH / 2, boardW, boardH);
    getConstrainer()->setFixedAspectRatio ((double) boardW / (double) boardH);
    const int w = jlimit (boardW / 2, boardW, (int) state.state.getProperty (ids::editorWidth, defaultWidth));
    setSize (w, roundToInt (w * (double) boardH / boardW));

    startTimerHz (30);
}

FlipperEditor::~FlipperEditor()
{
    stopTimer();
    setLookAndFeel (nullptr);
}

void FlipperEditor::resized()
{
    const float s = (float) getWidth() / (float) boardW;
    board.setTransform (AffineTransform::scale (s));
    state.state.setProperty (ids::editorWidth, getWidth(), nullptr);
}

void FlipperEditor::selectEngine (int e)
{
    e = jlimit (0, numEngines - 1, e);
    state.state.setProperty (ids::selectedEngine, e, nullptr);
    playfield.setSelected (e);
    characterPanel.setEngine (e);
    animLayer.setIdleEngine (e);
    designPanel.setEngine (e);
}

void FlipperEditor::setMode (UiMode m)
{
    state.state.setProperty (ids::uiMode, (int) m, nullptr);
    for (int i = 0; i < 3; ++i)
        modeButtons[(size_t) i]->setToggleState (i == (int) m, dontSendNotification);
    designPanel.setVisible (m == UiMode::design);
    labPanel.setVisible (m == UiMode::lab);
    if (m == UiMode::lab) labPanel.refresh();
}

void FlipperEditor::mouseDown (const MouseEvent&)
{
    proc.pushHistory();
}

void FlipperEditor::refreshEngineStates (bool force)
{
    const bool notes = proc.getHeldNotes() > 0;
    bool anyActive = false;
    bool namesChanged = false;

    for (int e = 0; e < numEngines; ++e)
    {
        const bool on = onParams[(size_t) e]->getValue() > 0.5f;
        const float level = levelParams[(size_t) e]->getValue();
        const float act = (notes && on && level > 0.001f)
                              ? (0.55f + 0.45f * std::sin (activityPhase + (float) e * 0.7f)) * (0.4f + 0.6f * level)
                              : 0.0f;
        anyActive |= act > 0.0f;

        auto& led = *leds[(size_t) e];
        if (force || std::abs (led.activity - act) > 0.02f) { led.activity = act; led.repaint(); }

        auto& pill = *pills[(size_t) e];
        if (force || pill.enabledLook != on) { pill.enabledLook = on; pill.repaint(); }

        if (board.engineOn[(size_t) e] != on) { board.engineOn[(size_t) e] = on; namesChanged = true; }

        playfield.setEngineState (e, { on, level, act });
    }

    if (namesChanged || force)
    {
        board.invalidateCache();
        board.repaint (870, 320, 80, 380);
    }

    lastActive = anyActive;
}

void FlipperEditor::timerCallback()
{
    activityPhase += 0.35f;
    refreshEngineStates (false);
}

void FlipperEditor::generate()
{
    proc.generateSound();   // usa el modo, estilo y bloqueos de LAB
    generateButton.flash();
    if (labPanel.isVisible()) labPanel.refresh();
}

void FlipperEditor::showVariations()
{
    const auto base = (uint32) Random::getSystemRandom().nextInt (999990) + 1;
    auto variations = std::make_shared<std::array<Snapshot, 4>>();
    for (int i = 0; i < 4; ++i)
        (*variations)[(size_t) i] = proc.variation (base + (uint32) i);

    PopupMenu menu;
    menu.setLookAndFeel (&lnf);
    menu.addSectionHeader ("VARIACIONES");
    for (int i = 0; i < 4; ++i)
        menu.addItem (i + 1, String::fromUTF8 ("VARIACIÓN ") + String (i + 1));

    SafePointer<FlipperEditor> safe (this);
    menu.showMenuAsync (PopupMenu::Options().withTargetComponent (&variationsButton),
                        [safe, variations] (int result)
                        {
                            if (safe == nullptr || result <= 0) return;
                            safe->proc.applyWithHistory ((*variations)[(size_t) result - 1]);
                            safe->variationsButton.flash();
                        });
}

void FlipperEditor::saveFavorite()
{
    const auto snapshot = SoundGenerator::capture (state);
    const String name = String::fromUTF8 (engines()[(size_t) playfield.getSelected()].character)
                      + " " + Time::getCurrentTime().formatted ("%d-%m %H.%M");
    if (proc.presets.save (name, PresetManager::categoryFor (snapshot), "favorito", true, snapshot).existsAsFile())
        favoriteButton.flash();
    if (labPanel.isVisible()) labPanel.refresh();
}
