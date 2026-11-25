#include "PluginEditor.h"
#include "Core/SoundGenerator.h"
#include "Core/PresetManager.h"
#include "Help/I18n.h"
#include "Help/Help.h"

using namespace juce;
using namespace flipper;

namespace
{
constexpr int boardW = 1024, boardH = 1536;
constexpr int defaultWidth = 640;
constexpr int maxWidth = boardW * 3 / 2;   // en pantallas grandes se puede agrandar mas que el arte original
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
      teacher (p.state),
      modPanel (p.state, *p.mod, *this),
      modOverlay (*p.mod),
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
        static const char* modeHelp[] = { "ui:mode_play", "ui:mode_design", "ui:mode_lab" };
        modeButtons[(size_t) i]->getProperties().set ("help", modeHelp[i]);
        board.addAndMakeVisible (*modeButtons[(size_t) i]);
    }

    // mezcla de motores
    for (int e = 0; e < numEngines; ++e)
    {
        const float y = engines()[(size_t) e].mixRowY;
        leds[(size_t) e] = std::make_unique<EngineLed>();
        leds[(size_t) e]->setBounds (855 - 16, (int) y - 16, 32, 32);
        leds[(size_t) e]->getProperties().set ("help", ids::engineOn (e));
        board.addAndMakeVisible (*leds[(size_t) e]);
        buttonAtts.push_back (std::make_unique<ButtonAtt> (state, ids::engineOn (e), *leds[(size_t) e]));

        pills[(size_t) e] = std::make_unique<LevelPill>();
        pills[(size_t) e]->setBounds (949, (int) y - 13, 47, 26);
        pills[(size_t) e]->getProperties().set ("help", ids::engineLevel (e));
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
    characterPanel.knob.getProperties().set ("help", ids::character);

    // macros
    for (int m = 0; m < numMacros; ++m)
    {
        const auto& info = macros()[(size_t) m];
        auto res = "knob_macro_" + String (m + 1) + "_png";
        macroKnobs[(size_t) m] = std::make_unique<SpriteKnob> (res.toRawUTF8(), 27.0f, 33.5f, String::fromUTF8 (info.label));
        macroKnobs[(size_t) m]->setBounds ((int) info.x - 48, (int) macroY - 44, 96, 118);
        macroKnobs[(size_t) m]->centre = { 48.0f, 44.0f };
        macroKnobs[(size_t) m]->getProperties().set ("help", ids::macro (m));
        board.addAndMakeVisible (*macroKnobs[(size_t) m]);
        sliderAtts.push_back (std::make_unique<SliderAtt> (state, ids::macro (m), *macroKnobs[(size_t) m]));
        macroKnobs[(size_t) m]->setDoubleClickReturnValue (true, 0.5);
    }

    // botones
    generateButton.setBounds (812, 1335, 70, 116);
    variationsButton.setBounds (887, 1335, 56, 116);
    favoriteButton.setBounds (945, 1335, 57, 116);
    generateButton.onClick   = [this] { generate(); };
    generateButton.onRightClick = [this]
    {
        PopupMenu m;
        m.setLookAndFeel (&lnf);
        const String pos = proc.generatedCount() > 0 ? "  (" + String (proc.generatedIndex() + 1) + " / " + String (proc.generatedCount()) + ")" : String();
        m.addSectionHeader (i18n::tr ("GENERADO") + pos);
        m.addItem (1, i18n::pick ("< SONIDO GENERADO ANTERIOR", "< PREVIOUS GENERATED SOUND"), proc.generatedIndex() > 0);
        m.addItem (2, i18n::pick ("SONIDO GENERADO SIGUIENTE >", "NEXT GENERATED SOUND >"), proc.generatedIndex() < proc.generatedCount() - 1);
        m.addItem (3, i18n::pick ("GENERAR OTRO", "GENERATE ANOTHER"));
        SafePointer<FlipperEditor> safe (this);
        m.showMenuAsync (PopupMenu::Options().withTargetComponent (&generateButton), [safe] (int r)
        {
            if (safe == nullptr) return;
            if (r == 1) safe->proc.previousGenerated();
            if (r == 2) safe->proc.nextGenerated();
            if (r == 3) safe->generate();
            if (r > 0) { safe->generateButton.flash(); if (safe->labPanel.isVisible()) safe->labPanel.refresh(); }
        });
    };
    variationsButton.onClick = [this] { showVariations(); };
    favoriteButton.onClick   = [this] { saveFavorite(); };
    generateButton.getProperties().set ("help", "ui:generate");
    variationsButton.getProperties().set ("help", "ui:variations");
    favoriteButton.getProperties().set ("help", "ui:favorite");
    for (auto* b : { &generateButton, &variationsButton, &favoriteButton })
        board.addAndMakeVisible (*b);

    // ayuda: boton ?, recorrido guiado y ventana de ayuda al pasar el mouse
    helpButton.setBounds (948, 14, 64, 64);
    helpButton.onClick = [this] { startTour(); };
    board.addAndMakeVisible (helpButton);
    board.addChildComponent (tour);
    tour.setBounds (0, 0, boardW, boardH);
    tour.resolve = [this] (const String& t) { return resolveTarget (t); };
    // MAESTRO: tocando el retrato del panel PERSONAJE
    board.addChildComponent (teacher);
    teacher.setBounds (0, 0, boardW, boardH);
    teacher.resolve       = [this] (const String& t) { return resolveTarget (t); };
    teacher.prepare       = [this] (const teach::Lesson& l) { prepareLesson (l); };
    teacher.currentEngine = [this] { return playfield.getSelected(); };
    teacher.heldNotes     = [this] { return proc.getHeldNotes(); };
    teacher.onClose       = [this] { characterPanel.teaching = false; characterPanel.repaint(); };
    characterPanel.onTeach = [this] { openTeacher(); };

    tooltipWindow = std::make_unique<HelpTooltipWindow> (this);
    tooltipWindow->setLookAndFeel (&lnf);
    i18n::changes().addChangeListener (this);

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

    // MODULACION: panel en la columna izquierda (DISENO) y marcas encima de todo el tablero
    board.addChildComponent (modPanel);
    modPanel.setBounds (ModPanel::boardBounds());
    board.addAndMakeVisible (modOverlay);
    modOverlay.setBounds (0, 0, boardW, boardH);
    modOverlay.visibleControls = [this] { return modulatableControls(); };
    modOverlay.nameFor = [] (const String& id) { return help::tooltip (id).upToFirstOccurrenceOf ("\n", false, false); };
    for (auto& chip : modPanel.chips)
    {
        chip->onDrag = [this] (int s, Point<float> p)
        {
            const auto key = paramAt (p);
            modOverlay.dragUpdate (s, p, key, key.isNotEmpty() ? findControl (key, false) : Rectangle<int>());
        };
        chip->onDrop = [this] (int s, Point<float> p) { dropModulation (s, p); modOverlay.dragEnd(); };
    }
    proc.mod->onRoutesChanged = [safe = SafePointer<FlipperEditor> (this)]
    { if (safe != nullptr) { safe->modOverlay.refresh(); safe->modOverlay.repaint(); } };

    // Link to controller / menu del DAW con click derecho en cualquier perilla
    for (int i = 0; i < proc.getParameters().size(); ++i)
        if (auto* rp = dynamic_cast<RangedAudioParameter*> (proc.getParameters()[i]))
            paramIndex[rp->getParameterID()] = i;
    board.addMouseListener (&rightClick, true);

    // cartel de lectura (encima de todo, a tamano real de pantalla)
    addChildComponent (readout);

    // estado guardado
    selectEngine ((int) state.state.getProperty (ids::selectedEngine, 8));
    setMode ((UiMode) (int) state.state.getProperty (ids::uiMode, 0));
    refreshEngineStates (true);

    setResizable (true, true);
    setResizeLimits (boardW / 2, boardH / 2, maxWidth, boardH * 3 / 2);
    getConstrainer()->setFixedAspectRatio ((double) boardW / (double) boardH);
    // tamano inicial: si la pantalla es alta, abre mas grande (mas legible); nunca mas chico que antes
    int startWidth = defaultWidth;
    if (auto* display = Desktop::getInstance().getDisplays().getPrimaryDisplay())
        startWidth = jlimit (defaultWidth, boardW, roundToInt (display->userArea.getHeight() * 0.9 * boardW / boardH));
    const int w = jlimit (boardW / 2, maxWidth, (int) state.state.getProperty (ids::editorWidth, startWidth));
    setSize (w, roundToInt (w * (double) boardH / boardW));

    startTimerHz (30);

   #if ! FLIPPER_SNAPSHOT
    // la primera vez que se abre el plugin, el recorrido de JUGAR arranca solo
    if (! i18n::tutorialSeen())
    {
        i18n::setTutorialSeen (true);
        SafePointer<FlipperEditor> safe (this);
        Timer::callAfterDelay (700, [safe] { if (safe != nullptr) safe->startTour(); });
    }
   #endif
}

FlipperEditor::~FlipperEditor()
{
    proc.mod->onRoutesChanged = nullptr;
    board.removeMouseListener (&rightClick);
    i18n::changes().removeChangeListener (this);
    if (tooltipWindow) tooltipWindow->setLookAndFeel (nullptr);
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
    modPanel.setVisible (m == UiMode::design);
    modOverlay.refresh();
    labPanel.setVisible (m == UiMode::lab);
    if (m == UiMode::lab) labPanel.refresh();
}

void FlipperEditor::changeListenerCallback (ChangeBroadcaster*)
{
    board.invalidateCache();
    labPanel.languageChanged();
    board.repaint();
}

Rectangle<int> FlipperEditor::resolveTarget (const String& t)
{
    if (t.startsWith ("rect:"))
    {
        const auto v = StringArray::fromTokens (t.substring (5), ",", "");
        return { v[0].getIntValue(), v[1].getIntValue(), v[2].getIntValue(), v[3].getIntValue() };
    }
    if (t.startsWith ("band:"))
        return designPanel.bandRect (t.substring (5)) + designPanel.getPosition();
    if (t.isNotEmpty())
        return findControl (t);
    return {};
}

Rectangle<int> FlipperEditor::findControl (const String& key, bool reveal)
{
    if (reveal) designPanel.reveal (key);
    std::function<Component* (Component&)> search = [&] (Component& c) -> Component*
    {
        for (auto* child : c.getChildren())
        {
            if (child == &teacher || child == &tour || child == &modOverlay) continue;
            if (! child->isVisible()) continue;
            if (child->getProperties()["help"].toString() == key) return child;
            if (auto* found = search (*child)) return found;
        }
        return nullptr;
    };
    if (auto* c = search (board))
        return board.getLocalArea (c, c->getLocalBounds());
    return {};
}

//============================================================================== Link to controller / modulacion
String FlipperEditor::paramKeyOf (Component* c) const
{
    for (; c != nullptr && c != this; c = c->getParentComponent())
    {
        if (auto* p = dynamic_cast<help::Provider*> (c))
        {
            const auto k = p->helpKeyAt (c->getMouseXYRelative());
            if (paramIndex.count (k)) return k;
        }
        const auto k = c->getProperties()["help"].toString();
        if (k.isNotEmpty()) return paramIndex.count (k) ? k : String();
    }
    return {};
}

int FlipperEditor::getControlParameterIndex (Component& c)
{
    const auto key = paramKeyOf (&c);
    auto it = paramIndex.find (key);
    return it != paramIndex.end() ? it->second : -1;
}

String FlipperEditor::paramAt (Point<float> p)
{
    return paramKeyOf (board.getComponentAt (p.toInt()));
}

std::vector<std::pair<String, Rectangle<int>>> FlipperEditor::modulatableControls()
{
    std::vector<std::pair<String, Rectangle<int>>> out;
    std::function<void (Component&)> walk = [&] (Component& c)
    {
        for (auto* child : c.getChildren())
        {
            if (child == &teacher || child == &tour || child == &modOverlay || ! child->isVisible()) continue;
            const auto k = child->getProperties()["help"].toString();
            if (k.isNotEmpty() && proc.mod->canModulate (k))
                out.push_back ({ k, board.getLocalArea (child, child->getLocalBounds()) });
            walk (*child);
        }
    };
    walk (board);
    return out;
}

bool FlipperEditor::dropModulation (int source, Point<float> p)
{
    const auto key = paramAt (p);
    if (key.isEmpty() || ! proc.mod->canModulate (key)) return false;
    proc.pushHistory();
    const float existing = proc.mod->depthOf (source, key);
    const float depth = existing != 0.0f ? existing : (source < ModMatrix::lfo1 ? 0.5f : 0.25f);
    return proc.mod->setRoute (source, key, depth);
}

void FlipperEditor::RightClick::mouseDown (const MouseEvent& e)
{
    if (! e.mods.isPopupMenu()) return;
    const auto key = ed.paramKeyOf (e.eventComponent);
    if (key.isNotEmpty()) ed.showParamMenu (key, e.getScreenPosition());
}

void FlipperEditor::showParamMenu (const String& id, Point<int> screenPos)
{
    auto* param = state.getParameter (id);
    if (param == nullptr) return;

    PopupMenu menu;
    bool hostItems = false;
    if (auto* ctx = getHostContext())
        if (auto hm = ctx->getContextMenuForParameter (param))
        {
            menu = hm->getEquivalentPopupMenu();
            hostItems = menu.getNumItems() > 0;
        }
    if (! hostItems)
    {
        menu.addSectionHeader (i18n::pick ("LINK TO CONTROLLER", "LINK TO CONTROLLER"));
        menu.addItem (PopupMenu::Item (i18n::pick ("Mové la perilla y en FL: Tools > Last tweaked > Link to controller",
                                                    "Move the knob, then in FL: Tools > Last tweaked > Link to controller")).setEnabled (false));
    }

    enum { removeBase = 20000, addBase = 21000 };
    if (proc.mod->canModulate (id))
    {
        menu.addSeparator();
        menu.addSectionHeader (i18n::pick ("MODULACIÓN", "MODULATION"));
        const auto routes = proc.mod->routesFor (id);
        for (auto& r : routes)
            menu.addItem (removeBase + r.source, i18n::pick ("Quitar ", "Remove ") + ModMatrix::sourceName (r.source)
                                                   + "  (" + (r.depth >= 0 ? "+" : "") + String (roundToInt (r.depth * 100.0f)) + "%)");
        PopupMenu add;
        for (int s = 0; s < ModMatrix::numSources; ++s)
            add.addItem (addBase + s, ModMatrix::sourceName (s), proc.mod->depthOf (s, id) == 0.0f);
        menu.addSubMenu (i18n::pick ("Modular con", "Modulate with"), add);
    }

    SafePointer<FlipperEditor> safe (this);
    menu.setLookAndFeel (&lnf);
    menu.showMenuAsync (PopupMenu::Options().withTargetScreenArea ({ screenPos.x, screenPos.y, 1, 1 }), [safe, id] (int r)
    {
        if (safe == nullptr) return;
        if (r >= removeBase && r < removeBase + ModMatrix::numSources) { safe->proc.pushHistory(); safe->proc.mod->removeRoute (r - removeBase, id); }
        if (r >= addBase && r < addBase + ModMatrix::numSources)
        { safe->proc.pushHistory(); safe->proc.mod->setRoute (r - addBase, id, r - addBase < ModMatrix::lfo1 ? 0.5f : 0.25f); }
    });
}

void FlipperEditor::openTeacher()
{
    if (tour.isRunning()) tour.close();
    characterPanel.teaching = true;
    characterPanel.repaint();
    teacher.openAsk();
}

void FlipperEditor::prepareLesson (const teach::Lesson& l)
{
    // la leccion trabaja sobre TU sonido: no apaga otros motores ni vuelve nada a cero,
    // no toca macros, CHARACTER ni modulaciones. Solo prende el motor de la receta si estaba apagado.
    proc.pushHistory();   // por las dudas, todo queda en DESHACER

    if (auto* on = state.getParameter (ids::engineOn (l.engine)); on != nullptr && on->getValue() < 0.5f)
    {
        on->beginChangeGesture(); on->setValueNotifyingHost (1.0f); on->endChangeGesture();
        if (auto* lvl = state.getParameter (ids::engineLevel (l.engine)); lvl != nullptr && lvl->getValue() < 0.3f)
        { lvl->beginChangeGesture(); lvl->setValueNotifyingHost (0.7f); lvl->endChangeGesture(); }
    }

    selectEngine (l.engine);
    setMode (UiMode::design);
    refreshEngineStates (true);
}

void FlipperEditor::startTour()
{
    const auto mode = (UiMode) (int) state.state.getProperty (ids::uiMode, 0);
    const String screen = mode == UiMode::play ? String ("play") : mode == UiMode::lab ? String ("lab")
                                                                  : "design" + String (designPanel.getEngine());
    tour.start (help::tour (screen));
    tour.grabKeyboardFocus();
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
    updateReadout();
}

//============================================================================== cartel de lectura
void FlipperEditor::showReadoutFor (const String& key)
{
    const auto r = findControl (key, true);
    forcedReadout = key;
    forcedAnchor = getLocalArea (&board, r).getCentre();
    updateReadout();
}

void FlipperEditor::updateReadout()
{
    auto mouse = Desktop::getInstance().getMainMouseSource();
    auto* under = mouse.getComponentUnderMouse();
    const bool dragging = mouse.isDragging() || forcedReadout.isNotEmpty();
    // mientras se arrastra, JUCE sigue mandando los eventos al control donde empezo el arrastre

    String key;
    if (under != nullptr && (under == this || isParentOf (under)) && ! teacher.isOpen() && ! tour.isRunning())
        key = paramKeyOf (under);

    if (forcedReadout.isNotEmpty()) key = forcedReadout;
    if (key != readoutKey) { readoutKey = key; readoutSince = Time::getMillisecondCounter(); }
    const bool show = key.isNotEmpty() && (dragging || Time::getMillisecondCounter() - readoutSince > 250);
    if (! show) { if (readout.isVisible()) readout.setVisible (false); return; }

    auto* p = state.getParameter (key);
    if (p == nullptr) { readout.setVisible (false); return; }
    const String title = help::tooltip (key).upToFirstOccurrenceOf ("\n", false, false);
    const String value = i18n::tr (p->getText (p->getValue(), 32) + p->getLabel());
    String extra;
    Colour accent = theme::mustardHi;
    const auto routes = proc.mod->routesFor (key);
    if (! routes.empty())
    {
        StringArray s;
        for (auto& r : routes) s.add (ModMatrix::sourceName (r.source) + " " + (r.depth >= 0 ? "+" : "") + String (roundToInt (r.depth * 100.0f)) + "%");
        extra = i18n::pick ("MODULADO: ", "MODULATED: ") + s.joinIntoString ("  ·  ")
              + "   >  " + i18n::tr (p->getText (proc.mod->modulatedNormalised (key), 32) + p->getLabel());
        accent = ModMatrix::sourceColour (routes.front().source);
    }
    readout.show (title.isNotEmpty() ? title : p->getName (40).toUpperCase(), value, extra, accent, forcedReadout.isNotEmpty() ? forcedAnchor : getLocalPoint (nullptr, mouse.getScreenPosition().toInt()));
}

void ReadoutBubble::show (const String& t, const String& v, const String& x, Colour a, Point<int> anchor)
{
    if (t == title && v == value && x == extra && isVisible() && getBounds().contains (anchor.translated (0, -40)) ) return;
    title = t; value = v; extra = x; accent = a;
    auto width = [] (const String& t, float h) { GlyphArrangement ga; ga.addLineOfText (theme::font (h), t, 0.0f, 0.0f); return (int) std::ceil (ga.getBoundingBox (0, -1, true).getWidth()); };
    const int w = jmax (width (title, 15.0f), width (value, 22.0f), extra.isNotEmpty() ? width (extra, 13.0f) : 0) + 40;
    const int h = extra.isNotEmpty() ? 74 : 56;
    auto* parent = getParentComponent();
    Rectangle<int> r (anchor.x + 18, anchor.y - h - 14, w, h);
    if (parent != nullptr)
    {
        const auto area = parent->getLocalBounds().reduced (4);
        if (r.getRight() > area.getRight()) r.setX (anchor.x - w - 18);
        if (r.getY() < area.getY()) r.setY (anchor.y + 22);
        r = r.constrainedWithin (area);
    }
    setBounds (r);
    setVisible (true);
    toFront (false);
    repaint();
}

void ReadoutBubble::paint (Graphics& g)
{
    auto r = getLocalBounds().toFloat().reduced (1.5f);
    g.setColour (Colour (0xf2120c08)); g.fillRoundedRectangle (r, 7.0f);
    g.setColour (accent.withAlpha (0.9f)); g.drawRoundedRectangle (r, 7.0f, 1.6f);
    g.setFont (theme::font (15.0f));
    g.setColour (accent);
    g.drawText (title, r.reduced (14.0f, 0.0f).withHeight (24.0f).translated (0.0f, 6.0f), Justification::centredLeft, false);
    g.setFont (theme::font (22.0f));
    g.setColour (theme::cream);
    g.drawText (value, r.reduced (14.0f, 0.0f).withTrimmedTop (26.0f).withHeight (26.0f), Justification::centredLeft, false);
    if (extra.isNotEmpty())
    {
        g.setFont (theme::font (13.0f));
        g.setColour (accent.brighter (0.3f));
        g.drawText (extra, r.reduced (14.0f, 0.0f).withTrimmedTop (52.0f).withHeight (18.0f), Justification::centredLeft, false);
    }
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
    menu.addSectionHeader (i18n::tr ("VARIACIONES"));
    for (int i = 0; i < 4; ++i)
        menu.addItem (i + 1, i18n::tr (String::fromUTF8 ("VARIACIÓN")) + " " + String (i + 1));

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
