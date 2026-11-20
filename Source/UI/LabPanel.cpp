#include "LabPanel.h"
#include "../Help/I18n.h"
using flipper::i18n::tr;

namespace flipper
{
using namespace juce;
using Gen = SoundGenerator;

static String u8 (const char* s) { return String::fromUTF8 (s); }

static void setupField (TextEditor& t, const String& hint)
{
    t.setFont (theme::font (14.0f));
    t.getProperties().set ("hint", hint);
    t.setTextToShowWhenEmpty (tr (hint), theme::cream.withAlpha (0.35f));
    t.setIndents (8, 7);
    t.setJustification (Justification::centredLeft);
}

LabPanel::LabPanel (FlipperProcessor& p) : proc (p)
{
    // ---------------------------------------------------------------- presets
    setupField (search, "BUSCAR NOMBRE O TAG");
    search.onTextChange = [this] { applyFilters(); };

    fillCombos();
    for (auto* c : { &categoryFilter, &engineFilter, &characterFilter })
    {
        c->setSelectedId (1, dontSendNotification);
        c->onChange = [this] { applyFilters(); };
    }
    favOnly.onClick = [this] { applyFilters(); };

    slotA.setClickingTogglesState (false);
    slotB.setClickingTogglesState (false);
    slotA.onClick = [this] { proc.switchAB (0); refresh(); };
    slotB.onClick = [this] { proc.switchAB (1); refresh(); };
    copyAB.onClick = [this]
    {
        proc.copyABToOther();
        status = tr (proc.getABSlot() == 0 ? u8 ("A COPIADO EN B") : u8 ("B COPIADO EN A"));
        repaint();
    };

    list.setModel (this);
    list.setRowHeight (30);
    list.setOutlineThickness (0);

    setupField (nameField, "NOMBRE DEL PRESET");
    setupField (tagsField, "TAGS (COMA)");
    saveButton.onClick = [this] { savePreset(); };

    // ---------------------------------------------------------------- generador
    for (int i = 0; i < 3; ++i)
    {
        modeButtons[(size_t) i] = std::make_unique<LabButton> (Gen::modeNames()[i]);
        modeButtons[(size_t) i]->onClick = [this, i] { proc.setGeneratorMode ((Gen::Mode) i); refreshGenerator(); };
        addAndMakeVisible (*modeButtons[(size_t) i]);
    }
    styleBox.onChange = [this] { proc.setGeneratorStyle (styleBox.getSelectedId() - 1); };

    setupField (seedField, "SEMILLA");
    seedField.setInputRestrictions (7, "0123456789");
    useSeed.onClick = [this]
    {
        const auto seed = (uint32) seedField.getText().getLargeIntValue();
        if (seed == 0) return;
        proc.generateSound (seed);
        status = tr ("SEMILLA") + " " + String (seed) + " " + tr ("APLICADA");
        refreshGenerator();
    };

    mutateAmount.setRange (0.05, 1.0);
    mutateAmount.onValueChange = [this] { proc.setMutateAmount ((float) mutateAmount.getValue()); };

    generate.onClick = [this] { proc.generateSound(); status = {}; refreshGenerator(); };
    mutate.onClick   = [this] { proc.mutateSound(); status = tr ("MUTADO"); refreshGenerator(); };
    surprise.onClick = [this]
    {
        auto& r = Random::getSystemRandom();
        proc.setGeneratorMode ((Gen::Mode) r.nextInt (3));
        proc.setGeneratorStyle (r.nextInt (Gen::styleNames().size()));
        proc.generateSound();
        status = tr ("SORPRESA");
        refreshGenerator();
    };
    undo.onClick = [this] { status = tr (proc.undo() ? "DESHECHO" : "NADA PARA DESHACER"); refreshGenerator(); };
    redo.onClick = [this] { status = tr (proc.redo() ? "REHECHO" : "NADA PARA REHACER"); refreshGenerator(); };

    // ---------------------------------------------------------------- bloqueos
    for (int e = 0; e < numEngines; ++e) lockIds.add (engines()[(size_t) e].id);
    lockIds.add ("macros");
    lockIds.add ("character");
    for (int i = 0; i < lockIds.size(); ++i)
    {
        const String label = i < numEngines ? String (engines()[(size_t) i].technical)
                                            : (lockIds[i] == "macros" ? String ("MACROS") : u8 ("CARÁCTER"));
        auto t = std::make_unique<LabToggle> (label);
        t->onClick = [this, i, raw = t.get()] { proc.setLocked (lockIds[i], raw->getToggleState()); };
        addAndMakeVisible (*t);
        locks.push_back (std::move (t));
    }

    // tablero: animaciones y sonidos del pinball
    animToggle.onClick = [this]
    {
        proc.state.state.setProperty (ids::animOn, animToggle.getToggleState(), nullptr);
        if (onAnimationsChanged) onAnimationsChanged (animToggle.getToggleState());
    };
    sfxToggle.onClick = [this]
    {
        proc.state.state.setProperty (ids::sfxOn, sfxToggle.getToggleState(), nullptr);
        proc.sfx.enabled.store (sfxToggle.getToggleState());
    };
    addAndMakeVisible (animToggle);
    addAndMakeVisible (sfxToggle);

    // idioma de todo el plugin
    langEs.onClick = [] { i18n::setLanguage (i18n::Lang::es); };
    langEn.onClick = [] { i18n::setLanguage (i18n::Lang::en); };
    addAndMakeVisible (langEs);
    addAndMakeVisible (langEn);

    // claves de la ayuda al pasar el mouse
    auto help = [] (Component& c, const char* key) { c.getProperties().set ("help", key); };
    help (search, "ui:search"); help (categoryFilter, "ui:filter"); help (engineFilter, "ui:filter"); help (characterFilter, "ui:filter");
    help (favOnly, "ui:favonly"); help (slotA, "ui:ab"); help (slotB, "ui:ab"); help (copyAB, "ui:copyab"); help (list, "ui:list");
    help (nameField, "ui:save"); help (saveCategory, "ui:save"); help (tagsField, "ui:save"); help (saveButton, "ui:save");
    for (auto& b : modeButtons) help (*b, "ui:gen_mode");
    help (styleBox, "ui:style"); help (seedField, "ui:seed"); help (useSeed, "ui:seed"); help (mutateAmount, "ui:mutate_amt");
    help (generate, "ui:generate"); help (mutate, "ui:mutate"); help (surprise, "ui:surprise"); help (undo, "ui:undo"); help (redo, "ui:redo");
    for (auto& t : locks) help (*t, "ui:lock");
    help (animToggle, "ui:anim"); help (sfxToggle, "ui:sfx"); help (langEs, "ui:lang"); help (langEn, "ui:lang");

    for (Component* c : std::initializer_list<Component*> {
             &search, &categoryFilter, &engineFilter, &characterFilter, &favOnly, &slotA, &slotB, &copyAB,
             &list, &nameField, &saveCategory, &tagsField, &saveButton,
             &styleBox, &seedField, &useSeed, &mutateAmount, &generate, &mutate, &surprise, &undo, &redo })
        addAndMakeVisible (c);
}

//============================================================================== idioma
void LabPanel::fillCombos()
{
    auto refill = [] (ComboBox& box, const StringArray& items, int firstId)
    {
        const int sel = box.getSelectedId();
        box.clear (dontSendNotification);
        for (int i = 0; i < items.size(); ++i) box.addItem (tr (items[i]), firstId + i);
        box.setSelectedId (sel != 0 ? sel : 1, dontSendNotification);
    };
    StringArray cats { u8 ("TODAS LAS CATEGORÍAS") }; cats.addArray (PresetManager::categories());
    refill (categoryFilter, cats, 1);
    StringArray engs { "TODOS LOS MOTORES" }; for (int e = 0; e < numEngines; ++e) engs.add (engines()[(size_t) e].technical);
    refill (engineFilter, engs, 1);
    refill (characterFilter, { u8 ("TODO CARÁCTER"), "LIMPIO", u8 ("CÁLIDO"), "SUCIO", "ROTO" }, 1);
    refill (saveCategory, PresetManager::categories(), 1);
    refill (styleBox, Gen::styleNames(), 1);
}

void LabPanel::languageChanged()
{
    fillCombos();
    for (auto* t : { &search, &nameField, &tagsField, &seedField })
        t->setTextToShowWhenEmpty (tr (t->getProperties()["hint"].toString()), theme::cream.withAlpha (0.35f));
    status = {};
    refresh();
    repaint();
}

//============================================================================== layout
void LabPanel::resized()
{
    animToggle.setBounds (440, 20, 170, 28);
    sfxToggle.setBounds (440, 50, 170, 28);
    langEs.setBounds (24, 44, 52, 30);
    langEn.setBounds (80, 44, 52, 30);

    search.setBounds (20, 122, 250, 34);
    categoryFilter.setBounds (280, 122, 160, 34);
    engineFilter.setBounds (450, 122, 160, 34);

    characterFilter.setBounds (20, 164, 160, 32);
    favOnly.setBounds (190, 164, 170, 32);
    slotA.setBounds (382, 164, 52, 32);
    slotB.setBounds (440, 164, 52, 32);
    copyAB.setBounds (498, 164, 112, 32);

    list.setBounds (20, 208, 590, 300);

    nameField.setBounds (20, 518, 210, 34);
    saveCategory.setBounds (238, 518, 130, 34);
    tagsField.setBounds (376, 518, 138, 34);
    saveButton.setBounds (522, 518, 88, 34);

    for (int i = 0; i < 3; ++i)
        modeButtons[(size_t) i]->setBounds (20 + i * 124, 616, 118, 36);
    styleBox.setBounds (400, 616, 210, 36);

    seedField.setBounds (110, 664, 150, 34);
    useSeed.setBounds (268, 664, 140, 34);
    mutateAmount.setBounds (500, 668, 110, 26);

    const int bw = 112, gap = 7;
    LabButton* actions[] = { &generate, &mutate, &surprise, &undo, &redo };
    for (int i = 0; i < 5; ++i)
        actions[i]->setBounds (20 + i * (bw + gap), 712, i == 4 ? 590 - 4 * (bw + gap) : bw, 44);

    for (int i = 0; i < (int) locks.size(); ++i)
    {
        const int col = i < numEngines ? i % 3 : (i - numEngines);
        const int row = i < numEngines ? i / 3 : 3;
        locks[(size_t) i]->setBounds (20 + col * 198, 850 + row * 38, 190, 34);
    }
}

//============================================================================== paint
static void sectionTitle (Graphics& g, const String& t, float y, float width)
{
    const float h = 19.0f;
    const float w = theme::textWidth (t, h);
    theme::drawPrintedText (g, t, 20.0f + w * 0.5f, y, h, 0.0f, theme::mustard, theme::black, 0.25f);
    g.setColour (theme::gold.withAlpha (0.55f));
    g.fillRect (30.0f + w, y, width - 50.0f - w, 1.5f);
}

void LabPanel::paint (Graphics& g)
{
    auto b = getLocalBounds().toFloat().reduced (4.0f);
    g.setColour (Colours::black.withAlpha (0.55f));
    g.fillRoundedRectangle (b.translated (4, 6), 14.0f);
    g.setGradientFill (ColourGradient (Colour (0xf6231a14), b.getX(), b.getY(), Colour (0xf60e0a08), b.getX(), b.getBottom(), false));
    g.fillRoundedRectangle (b, 14.0f);

    // desgaste: motas fijas
    Random r (1974);
    for (int i = 0; i < 900; ++i)
    {
        g.setColour ((r.nextBool() ? theme::cream : Colours::black).withAlpha (0.03f + r.nextFloat() * 0.05f));
        const float s = 0.8f + r.nextFloat() * 2.2f;
        g.fillEllipse (b.getX() + r.nextFloat() * b.getWidth(), b.getY() + r.nextFloat() * b.getHeight(), s, s);
    }

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

    const float W = (float) getWidth();
    theme::drawPrintedText (g, "LAB", W * 0.5f, 42.0f, 34.0f, 0.0f, theme::mustardHi, theme::black, 0.3f);
    theme::drawPrintedText (g, u8 ("PRESETS  ·  GENERADOR  ·  BLOQUEOS"), W * 0.5f, 74.0f, 13.0f, 0.0f,
                            theme::cream.withAlpha (0.8f), theme::black, 0.0f);

    theme::drawPrintedText (g, "IDIOMA", 78.0f, 32.0f, 11.0f, 0.0f, theme::cream.withAlpha (0.8f), theme::black, 0.0f);
    sectionTitle (g, "PRESETS", 104.0f, W);
    sectionTitle (g, "GENERADOR", 594.0f, W);
    sectionTitle (g, "BLOQUEOS", 828.0f, W);

    // marco de la lista
    auto lb = list.getBounds().toFloat().expanded (2.0f);
    g.setColour (Colour (0xff0b0806));
    g.fillRoundedRectangle (lb, 5.0f);
    g.setColour (Colour (0xff7d6646));
    g.drawRoundedRectangle (lb, 5.0f, 1.3f);
    if (filtered.empty())
        theme::drawPrintedText (g, proc.presets.all().empty() ? u8 ("NO HAY PRESETS")
                                                              : u8 ("NINGÚN PRESET COINCIDE CON LOS FILTROS"),
                                lb.getCentreX(), lb.getCentreY(), 14.0f, 0.0f, theme::cream.withAlpha (0.5f), theme::black, 0.0f);

    // etiquetas sueltas
    theme::drawPrintedText (g, "SEMILLA", 62.0f, 681.0f, 15.0f, 0.0f, theme::cream, theme::black, 0.1f);
    theme::drawPrintedText (g, "INTENSIDAD", 452.0f, 681.0f, 15.0f, 0.0f, theme::cream, theme::black, 0.1f);

    const String hint = u8 ("LO BLOQUEADO NO CAMBIA AL GENERAR, MUTAR NI EN VARIACIONES");
    theme::drawPrintedText (g, hint, W * 0.5f, 1004.0f, 13.0f, 0.0f, theme::cream.withAlpha (0.55f), theme::black, 0.0f);

    // estado: ultima semilla, ranura A/B, mensajes
    String line = tr ("SEMILLA ACTUAL") + " " + String (proc.lastSeed()) + "   ·   " + tr ("EDITANDO") + " " + (proc.getABSlot() == 0 ? "A" : "B");
    if (status.isNotEmpty()) line << "   ·   " << status;
    theme::drawPrintedText (g, line, W * 0.5f, 782.0f, 13.0f, 0.0f, theme::mustard.withAlpha (0.9f), theme::black, 0.0f);

}

//============================================================================== lista
void LabPanel::paintListBoxItem (int row, Graphics& g, int width, int height, bool)
{
    if (row < 0 || row >= (int) filtered.size()) return;
    const auto& pr = proc.presets.all()[(size_t) filtered[(size_t) row]];
    const bool loaded = pr.name == loadedName && pr.factory == loadedFactory;

    if (loaded)
    {
        g.setColour (theme::mustard.withAlpha (0.22f));
        g.fillRect (0, 0, width, height);
    }
    if (row % 2 == 1)
    {
        g.setColour (Colours::white.withAlpha (0.025f));
        g.fillRect (0, 0, width, height);
    }

    const float cy = height * 0.5f;
    const String name = pr.name.toUpperCase();
    const float nh = theme::fitHeight (name, 210.0f, 15.0f);
    float nx = 12.0f;
    if (pr.factory)
    {
        // sello de fabrica
        g.setColour (theme::mustard.withAlpha (0.85f));
        g.fillRoundedRectangle (nx, cy - 7.0f, 16.0f, 14.0f, 2.0f);
        theme::drawPrintedText (g, "F", nx + 8.0f, cy, 10.0f, 0.0f, theme::ink, theme::ink, 0.0f, false);
        nx += 22.0f;
    }
    theme::drawPrintedText (g, name, nx + theme::textWidth (name, nh) * 0.5f, cy, nh, 0.0f,
                            loaded ? theme::mustardHi : theme::cream, theme::black, 0.0f, false);

    theme::drawPrintedText (g, pr.category, 300.0f, cy, 12.0f, 0.0f, theme::mustard, theme::black, 0.0f, false);

    StringArray eng;
    for (int e = 0; e < numEngines; ++e) if (pr.engineOn[(size_t) e]) eng.add (engines()[(size_t) e].technical);
    String engText = eng.size() > 2 ? eng[0] + " + " + String (eng.size() - 1) : eng.joinIntoString (" + ");
    for (auto& tg : pr.tags) engText << "  #" << tr (tg.toUpperCase());
    const float eh = theme::fitHeight (engText, 200.0f, 11.0f);
    theme::drawPrintedText (g, engText, 350.0f + theme::textWidth (engText, eh) * 0.5f, cy, eh, 0.0f,
                            theme::cream.withAlpha (0.6f), theme::black, 0.0f, false);

    // estrella de favorito
    Path star; star.addStar ({ (float) width - 20.0f, cy }, 5, 4.5f, 10.0f, 0.0f);
    if (pr.favorite) { g.setColour (theme::mustardHi); g.fillPath (star); }
    else { g.setColour (theme::cream.withAlpha (0.35f)); g.strokePath (star, PathStrokeType (1.2f)); }
}

void LabPanel::listBoxItemClicked (int row, const MouseEvent& e)
{
    if (row < 0 || row >= (int) filtered.size()) return;
    const auto& pr = proc.presets.all()[(size_t) filtered[(size_t) row]];

    if (e.x > list.getWidth() - 44)
    {
        proc.presets.setFavorite (pr, ! pr.favorite);
        proc.presets.rescan();
        applyFilters();
        return;
    }

    // un click carga el preset (escucharlo); DESHACER vuelve al sonido anterior
    proc.applyWithHistory (pr.values);
    loadedName = pr.name; loadedFactory = pr.factory;
    status = tr ("CARGADO") + ": " + pr.name.toUpperCase();
    list.repaint();
    repaint();
}

void LabPanel::applyFilters()
{
    filtered.clear();
    const auto query = search.getText().trim().toLowerCase();
    const int cat = categoryFilter.getSelectedId() - 2;
    const int eng = engineFilter.getSelectedId() - 2;
    const int chr = characterFilter.getSelectedId() - 2;

    const auto& all = proc.presets.all();
    for (int i = 0; i < (int) all.size(); ++i)
    {
        const auto& pr = all[(size_t) i];
        if (favOnly.getToggleState() && ! pr.favorite) continue;
        if (cat >= 0 && pr.category != PresetManager::categories()[cat]) continue;
        if (eng >= 0 && ! pr.engineOn[(size_t) eng]) continue;
        if (chr >= 0 && jlimit (0, 3, (int) (pr.character * 4.0f)) != chr) continue;
        if (query.isNotEmpty())
        {
            bool match = pr.name.toLowerCase().contains (query);
            for (auto& t : pr.tags) match = match || t.toLowerCase().contains (query);
            if (! match) continue;
        }
        filtered.push_back (i);
    }
    list.updateContent();
    list.repaint();
    repaint();
}

void LabPanel::savePreset()
{
    auto name = nameField.getText().trim();
    if (name.isEmpty()) { status = tr ("PONELE UN NOMBRE"); repaint(); nameField.grabKeyboardFocus(); return; }

    const auto file = proc.presets.save (name, PresetManager::categories()[jmax (0, saveCategory.getSelectedId() - 1)], tagsField.getText(), false,
                                         Gen::capture (proc.state));
    status = file.existsAsFile() ? tr ("GUARDADO") + ": " + name.toUpperCase() : tr ("NO SE PUDO GUARDAR");
    nameField.clear();
    tagsField.clear();
    proc.presets.rescan();
    loadedName = name; loadedFactory = false;
    applyFilters();
}

//============================================================================== refresco
void LabPanel::refreshGenerator()
{
    const auto s = proc.generatorSettings();
    for (int i = 0; i < 3; ++i)
        modeButtons[(size_t) i]->setToggleState (i == (int) s.mode, dontSendNotification);
    styleBox.setSelectedId (s.style + 1, dontSendNotification);
    styleBox.setEnabled (s.mode == Gen::Mode::style);
    styleBox.setAlpha (s.mode == Gen::Mode::style ? 1.0f : 0.45f);
    mutateAmount.setValue (s.mutateAmount, dontSendNotification);
    seedField.setText (String (proc.lastSeed()), false);
    for (int i = 0; i < lockIds.size(); ++i)
        locks[(size_t) i]->setToggleState (s.locks.count (lockIds[i]) > 0, dontSendNotification);
    animToggle.setToggleState ((bool) proc.state.state.getProperty (ids::animOn, true), dontSendNotification);
    sfxToggle.setToggleState ((bool) proc.state.state.getProperty (ids::sfxOn, false), dontSendNotification);
    langEs.setToggleState (! i18n::english(), dontSendNotification);
    langEn.setToggleState (i18n::english(), dontSendNotification);
    slotA.setToggleState (proc.getABSlot() == 0, dontSendNotification);
    slotB.setToggleState (proc.getABSlot() == 1, dontSendNotification);
    repaint();
}

void LabPanel::refresh()
{
    proc.presets.rescan();
    saveCategory.setSelectedId (PresetManager::categories().indexOf (PresetManager::categoryFor (Gen::capture (proc.state))) + 1, dontSendNotification);
    applyFilters();
    refreshGenerator();
}
}
