#include "TeacherUI.h"
#include "../UI/Theme.h"
#include "../Help/I18n.h"
#include "../Help/Help.h"
#include "../Core/Parameters.h"

namespace flipper
{
using namespace juce;
using teach::Cond;
using teach::Lesson;
using teach::Step;

static String u8 (const char* s) { return String::fromUTF8 (s); }
static String pk (const char* es, const char* en) { return i18n::pick (es, en); }

TeacherOverlay::TeacherOverlay (AudioProcessorValueTreeState& s) : state (s)
{
    setVisible (false);
    setWantsKeyboardFocus (true);
    setInterceptsMouseClicks (true, true);

    addChildComponent (input);
    input.setFont (theme::font (24.0f));
    input.setJustification (Justification::centredLeft);
    input.setIndents (14, 0);
    input.setColour (TextEditor::backgroundColourId, Colour (0xff0d0907));
    input.setColour (TextEditor::textColourId, theme::cream);
    input.setColour (TextEditor::highlightColourId, theme::mustard.withAlpha (0.35f));
    input.setColour (TextEditor::highlightedTextColourId, theme::cream);
    input.setColour (TextEditor::outlineColourId, theme::gold.withAlpha (0.8f));
    input.setColour (TextEditor::focusedOutlineColourId, theme::mustardHi);
    input.setColour (CaretComponent::caretColourId, theme::mustardHi);
    input.setSelectAllWhenFocused (false);
    input.getProperties().set ("help", "ui:teacher_input");
    input.onTextChange = [this] { refreshResults(); repaint(); };
    input.onReturnKey  = [this] { if (! rows.empty()) startLesson (*rows.front().second, result.modifiers); };
    input.onEscapeKey  = [this] { close(); };
}

TeacherOverlay::~TeacherOverlay() { stopTimer(); }

const char* TeacherOverlay::keep (const String& s)
{
    texts.push_back (s);
    return texts.back().toRawUTF8();
}

//============================================================================== abrir / cerrar
void TeacherOverlay::openAsk()
{
    phase = Phase::ask;
    lesson = nullptr;
    steps.clear();
    setVisible (true);
    toFront (false);
    input.setTextToShowWhenEmpty (pk ("Ej.: bajo 808 sucio, pad oscuro, campana…", "E.g. dirty 808 bass, dark pad, bell…"), theme::cream.withAlpha (0.35f));
    input.setVisible (true);
    resized();
    refreshResults();
    startTimerHz (15);
    repaint();
    SafePointer<TeacherOverlay> safe (this);
    MessageManager::callAsync ([safe] { if (safe != nullptr && safe->isShowing()) safe->input.grabKeyboardFocus(); });
}

void TeacherOverlay::close()
{
    phase = Phase::closed;
    stopTimer();
    input.setVisible (false);
    setVisible (false);
    lesson = nullptr;
    steps.clear();
    texts.clear();
    if (onClose) onClose();
}

void TeacherOverlay::setQuery (const String& q)
{
    if (phase != Phase::ask) openAsk();
    input.setText (q, true);
    refreshResults();
}

void TeacherOverlay::startLesson (const Lesson& l, const std::vector<const teach::Modifier*>& mods)
{
    lesson = &l;
    steps.clear();
    texts.clear();
    input.setVisible (false);


    // presentacion
    steps.push_back ({ "", {},
        keep (u8 (l.titleEs)),
        keep (teach::greeting (l.engine) + "\n" + u8 (l.blurbEs)
              + u8 ("\n\nTrabajamos sobre tu sonido: no toco los otros motores, las macros ni tus modulaciones. Si un paso ya está como lo pido, lo salteo.")),
        keep (u8 (l.titleEn)),
        keep (teach::greeting (l.engine) + "\n" + u8 (l.blurbEn)
              + "\n\nWe work on your sound: I don't touch the other engines, the macros or your modulations. If a step is already done, I skip it.") });

    for (auto& s : l.steps) steps.push_back (s);

    // lo que pidio de mas (oscuro, sucio...), salvo que la receta ya lo toque
    for (auto* m : mods)
    {
        bool dup = false;
        for (auto& s : l.steps) dup |= String (s.target) == m->step.target;
        if (! dup) steps.push_back (m->step);
    }

    steps.push_back ({ "ui:favorite", {},
        keep (u8 ("¡LISTO!")),
        keep (u8 ("Ya armaste tu ") + u8 (l.titleEs).toLowerCase() + u8 (". Ahora es tuyo: cambialo, rompelo, sumale otro motor. Si te gusta, guardalo con FAVORITO.")),
        keep ("DONE!"),
        keep ("You built your " + u8 (l.titleEn).toLowerCase() + ". Now it's yours: change it, break it, add another engine. If you like it, save it with FAVORITE.") });

    if (prepare) prepare (l);

    phase = Phase::lesson;
    index = 0;
    wasOk = false;
    setVisible (true);
    toFront (true);
    layoutLesson();
    startTimerHz (15);
    repaint();
}

//============================================================================== estado
float TeacherOverlay::realValue (const String& id) const
{
    if (auto* p = state.getParameter (id)) return p->convertFrom0to1 (p->getValue());
    return 0.0f;
}

bool TeacherOverlay::condMet (const Cond& c) const
{
    const String id (c.param);
    if (id == "@note") return heldNotes && heldNotes() > 0;
    auto* p = state.getParameter (id);
    if (p == nullptr) return true;
    const float v = p->convertFrom0to1 (p->getValue());
    const bool discrete = dynamic_cast<AudioParameterChoice*> (p) || dynamic_cast<AudioParameterInt*> (p) || dynamic_cast<AudioParameterBool*> (p);
    if (discrete) { const int iv = roundToInt (v); return iv >= roundToInt (c.lo) && iv <= roundToInt (c.hi); }
    const float eps = (c.hi - c.lo) * 0.002f;
    return v >= c.lo - eps && v <= c.hi + eps;
}

bool TeacherOverlay::satisfied() const
{
    auto* st = currentStep();
    if (st == nullptr) return false;
    for (auto& c : st->conds) if (! condMet (c)) return false;
    return true;
}

void TeacherOverlay::showMe()
{
    auto* st = currentStep();
    if (st == nullptr) return;
    for (auto& c : st->conds)
    {
        if (String (c.param) == "@note") continue;
        if (auto* p = state.getParameter (c.param))
        {
            p->beginChangeGesture();
            p->setValueNotifyingHost (p->convertTo0to1 (c.set));
            p->endChangeGesture();
        }
    }
    repaint();
}

String TeacherOverlay::statusLine (bool& ok) const
{
    ok = false;
    auto* st = currentStep();
    if (st == nullptr) return {};
    if (st->conds.empty()) { ok = true; return {}; }

    for (auto& c : st->conds)
    {
        if (condMet (c)) continue;
        const String id (c.param);
        if (id == "@note") return pk ("TOCÁ UNA NOTA EN TU TECLADO", "PLAY A NOTE ON YOUR KEYBOARD");
        auto* p = state.getParameter (id);
        if (p == nullptr) continue;

        auto text = [p] (float real) { return i18n::tr (p->getText (p->convertTo0to1 (real), 24) + p->getLabel()); };
        const String title = help::tooltip (id).upToFirstOccurrenceOf ("\n", false, false);
        const float v = p->convertFrom0to1 (p->getValue());

        if (dynamic_cast<AudioParameterBool*> (p) || (dynamic_cast<AudioParameterChoice*> (p) && p->getNumSteps() == 2
                                                       && static_cast<AudioParameterChoice*> (p)->choices[1] == u8 ("SÍ")))
            return title + ":  " + (c.set > 0.5f ? pk ("PRENDELO", "TURN IT ON") : pk ("APAGALO", "TURN IT OFF"));
        if (dynamic_cast<AudioParameterChoice*> (p))
            return title + ":  " + pk ("ELEGÍ ", "PICK ") + text (c.set) + "   (" + pk ("AHORA ", "NOW ") + text (v) + ")";

        const String want = text (c.lo) == text (c.hi) ? text (c.lo) : text (c.lo) + " - " + text (c.hi);
        const String dir = v < c.lo ? pk ("MÁS ARRIBA", "HIGHER") : pk ("MÁS ABAJO", "LOWER");
        return title + ":  " + pk ("AHORA ", "NOW ") + text (v) + "   " + pk ("BUSCÁ ", "AIM FOR ") + want + "   " + dir;
    }
    ok = true;
    return okAtEntry ? pk ("TU SONIDO YA ESTÁ ASÍ: SIGO", "YOUR SOUND IS ALREADY LIKE THIS: MOVING ON") : pk ("¡ESO ES!", "THAT'S IT!");
}

void TeacherOverlay::timerCallback()
{
    if (phase == Phase::ask) return;
    if (phase != Phase::lesson) { stopTimer(); return; }

    bool ok = false;
    const auto line = statusLine (ok);
    if (line != lastStatus || ok != lastOk) { lastStatus = line; lastOk = ok; repaint (card.toNearestInt().expanded (4)); }

    auto* st = currentStep();
    const bool waits = st != nullptr && ! st->conds.empty();
    if (waits && ok)
    {
        const auto now = Time::getMillisecondCounter();
        if (! wasOk) { wasOk = true; okSince = now; }
        const bool onlyNote = st->conds.size() == 1 && String (st->conds[0].param) == "@note";
        if (now - okSince > (onlyNote ? 1400u : okAtEntry ? 1300u : 900u) && ! ModifierKeys::currentModifiers.isAnyMouseButtonDown())
            next();
    }
    else wasOk = false;

    repaint (hole.toNearestInt().expanded (16));   // brillo de la zona iluminada
}

void TeacherOverlay::next()
{
    if (phase != Phase::lesson) return;
    if (index >= (int) steps.size() - 1) { close(); return; }
    ++index;
    wasOk = false;
    layoutLesson();
    repaint();
}

void TeacherOverlay::previous()
{
    if (phase != Phase::lesson || index == 0) return;
    --index;
    wasOk = false;
    layoutLesson();
    repaint();
}

//============================================================================== PREGUNTAR
void TeacherOverlay::resized()
{
    askCard = { 172.0f, 420.0f, 670.0f, 828.0f };
    input.setBounds (askCard.toNearestInt().getX() + 28, (int) askCard.getY() + 132, (int) askCard.getWidth() - 56, 54);
    closeBtn = { askCard.getRight() - 56.0f, askCard.getY() + 16.0f, 40.0f, 40.0f };
    if (phase == Phase::lesson) layoutLesson();
}

void TeacherOverlay::refreshResults()
{
    const String q = input.getText().trim();
    result = teach::search (q);

    std::vector<const Lesson*> list;
    if (q.isEmpty())   // lo que sabe este personaje, y despues lo basico
    {
        const int e = currentEngine ? currentEngine() : 0;
        for (auto& l : teach::lessons()) if (l.engine == e && ! l.basics) list.push_back (&l);
        for (auto& l : teach::lessons()) if (l.basics) list.push_back (&l);
    }
    else if (result.understood) list = result.lessons;
    else
    {
        const int e = currentEngine ? currentEngine() : 0;
        for (auto& l : teach::lessons()) if (l.engine == e && ! l.basics) list.push_back (&l);
        for (auto& l : teach::lessons()) if (l.basics) list.push_back (&l);
    }

    // ejemplos para tocar sin escribir
    chips.clear();
    const char* es[] = { "bajo 808", "pad oscuro", "campana", "lead con glide", "piano eléctrico", "bajo ácido", "arpegio", "efecto espacial" };
    const char* en[] = { "808 bass", "dark pad", "bell", "glide lead", "electric piano", "acid bass", "arpeggio", "space fx" };
    float x = askCard.getX() + 28.0f, y = askCard.getY() + 202.0f;
    for (int i = 0; i < 8; ++i)
    {
        const String t = pk (es[i], en[i]);
        const float w = theme::textWidth (t, 17.0f) + 26.0f;
        if (x + w > askCard.getRight() - 28.0f) { x = askCard.getX() + 28.0f; y += 40.0f; }
        chips.push_back ({ { x, y, w, 32.0f }, t });
        x += w + 8.0f;
    }

    rows.clear();
    float ry = askCard.getY() + 318.0f;
    for (auto* l : list)
    {
        if (rows.size() >= 6) break;
        rows.push_back ({ { askCard.getX() + 24.0f, ry, askCard.getWidth() - 48.0f, 70.0f }, l });
        ry += 74.0f;
    }
    hoverRow = -1;
}

void TeacherOverlay::paintMedal (Graphics& g, Rectangle<float> r, int engine)
{
    const auto img = theme::image (engines()[(size_t) jlimit (0, numEngines - 1, engine)].portrait);
    {
        Graphics::ScopedSaveState s (g);
        Path clip; clip.addEllipse (r);
        g.reduceClipRegion (clip);
        g.setColour (theme::black);
        g.fillEllipse (r);
        if (img.isValid()) g.drawImage (img, r, RectanglePlacement::centred | RectanglePlacement::fillDestination);
    }
    g.setColour (theme::mustard); g.drawEllipse (r, 2.5f);
    g.setColour (theme::gold);    g.drawEllipse (r.expanded (3.0f), 1.2f);
}

void TeacherOverlay::paintButton (Graphics& g, Rectangle<float> r, const String& text, bool lit, bool enabled)
{
    g.setColour (Colours::black.withAlpha (0.5f)); g.fillRoundedRectangle (r.translated (1, 2), 6.0f);
    g.setGradientFill (lit && enabled ? ColourGradient (theme::mustardHi, r.getX(), r.getY(), Colour (0xffc98314), r.getX(), r.getBottom(), false)
                                      : ColourGradient (Colour (0xff3a2e24), r.getX(), r.getY(), Colour (0xff17110d), r.getX(), r.getBottom(), false));
    g.fillRoundedRectangle (r, 6.0f);
    g.setColour (theme::gold.withAlpha (enabled ? 0.9f : 0.3f)); g.drawRoundedRectangle (r, 6.0f, 1.3f);
    theme::drawPrintedText (g, text, r.getCentreX(), r.getCentreY(), theme::fitHeight (text, r.getWidth() - 14.0f, 15.0f), 0.0f,
                            lit && enabled ? theme::ink : theme::cream.withAlpha (enabled ? 1.0f : 0.3f), theme::black, 0.0f, ! (lit && enabled));
}

static void paintCardFrame (Graphics& g, Rectangle<float> c)
{
    g.setColour (Colours::black.withAlpha (0.6f));
    g.fillRoundedRectangle (c.translated (5, 8), 14.0f);
    g.setGradientFill (ColourGradient (Colour (0xff2a1f17), c.getX(), c.getY(), Colour (0xff100b08), c.getX(), c.getBottom(), false));
    g.fillRoundedRectangle (c, 14.0f);
    g.setColour (theme::gold);
    g.drawRoundedRectangle (c.reduced (3.0f), 12.0f, 3.0f);
    g.setColour (Colour (0xff6b5638));
    g.drawRoundedRectangle (c.reduced (9.0f), 9.0f, 1.2f);
    for (auto p : { c.getTopLeft() + Point<float> (18, 18), c.getBottomLeft() + Point<float> (18, -18),
                    c.getBottomRight() + Point<float> (-18, -18) })
    {
        g.setGradientFill (ColourGradient (Colour (0xffd8c39a), p.x - 3, p.y - 3, Colour (0xff4a3a28), p.x + 4, p.y + 4, false));
        g.fillEllipse (p.x - 6, p.y - 6, 12, 12);
    }
}

void TeacherOverlay::paintAsk (Graphics& g)
{
    const int engine = currentEngine ? currentEngine() : 0;
    const String name = u8 (engines()[(size_t) engine].character);

    // todo oscuro salvo el panel PERSONAJE, con un globo que sale del retrato
    Path shade;
    shade.addRectangle (getLocalBounds().toFloat());
    shade.addRoundedRectangle (Rectangle<float> (854, 736, 160, 443), 12.0f);
    shade.setUsingNonZeroWinding (false);
    g.setColour (Colours::black.withAlpha (0.62f));
    g.fillPath (shade);

    Path tail;
    tail.addTriangle (askCard.getRight() - 6.0f, 850.0f, askCard.getRight() - 6.0f, 920.0f, 884.0f, 889.0f);
    g.setColour (Colour (0xff1e160f)); g.fillPath (tail);
    g.setColour (theme::gold); g.strokePath (tail, PathStrokeType (2.5f));
    paintCardFrame (g, askCard);
    g.setColour (Colour (0xff1e160f));
    g.fillRect (askCard.getRight() - 12.0f, 852.0f, 9.0f, 66.0f);

    // cabecera
    paintMedal (g, { askCard.getX() + 30.0f, askCard.getY() + 26.0f, 78.0f, 94.0f }, engine);
    const String title = pk ("PREGUNTALE A ", "ASK ") + name;
    const float th = theme::fitHeight (title, 430.0f, 34.0f);
    theme::drawPrintedText (g, title, askCard.getX() + 126.0f + theme::textWidth (title, th) * 0.5f, askCard.getY() + 52.0f, th, 0.0f,
                            theme::mustardHi, theme::black, 0.25f);
    {
        AttributedString a;
        a.append (teach::greeting (engine) + "  " + pk ("¿Qué sonido armamos?", "What sound are we making?"),
                  theme::font (16.5f), theme::cream);
        a.setLineSpacing (2.0f);
        TextLayout tl; tl.createLayout (a, askCard.getWidth() - 200.0f);
        tl.draw (g, { askCard.getX() + 126.0f, askCard.getY() + 74.0f, askCard.getWidth() - 200.0f, 52.0f });
    }

    // cerrar
    g.setColour (theme::cream.withAlpha (0.8f));
    const auto x = closeBtn.reduced (12.0f);
    g.drawLine (x.getX(), x.getY(), x.getRight(), x.getBottom(), 3.0f);
    g.drawLine (x.getRight(), x.getY(), x.getX(), x.getBottom(), 3.0f);

    // ejemplos
    for (auto& [r, t] : chips)
    {
        g.setColour (Colour (0xff3a2a1c)); g.fillRoundedRectangle (r, 15.0f);
        g.setColour (theme::gold.withAlpha (0.7f)); g.drawRoundedRectangle (r, 15.0f, 1.2f);
        theme::drawPrintedText (g, t, r.getCentreX(), r.getCentreY() + 1.0f, 17.0f, 0.0f, theme::cream, theme::black, 0.0f, false);
    }

    // titulo de la lista
    const String q = input.getText().trim();
    String head;
    if (q.isEmpty()) head = pk ("LO QUE TE PUEDO ENSEÑAR (O ESCRIBÍ CUALQUIER SONIDO)", "WHAT I CAN TEACH YOU (OR TYPE ANY SOUND)");
    else if (result.understood) head = pk ("RECETAS PARA ", "RECIPES FOR ") + "\"" + q.toUpperCase() + "\"";
    else head = pk ("TODAVÍA NO TENGO ESA RECETA. PROBÁ ESTAS:", "I DON'T HAVE THAT RECIPE YET. TRY THESE:");
    const float hh = theme::fitHeight (head, askCard.getWidth() - 60.0f, 19.0f);
    theme::drawPrintedText (g, head, askCard.getX() + 30.0f + theme::textWidth (head, hh) * 0.5f, askCard.getY() + 300.0f, hh, 0.0f,
                            theme::mustard, theme::black, 0.15f, false);

    // recetas
    for (size_t i = 0; i < rows.size(); ++i)
    {
        auto [r, l] = rows[i];
        g.setColour ((int) i == hoverRow ? Colour (0xff4a3522) : Colour (0xff221810));
        g.fillRoundedRectangle (r, 8.0f);
        g.setColour (theme::gold.withAlpha ((int) i == hoverRow ? 0.9f : 0.35f));
        g.drawRoundedRectangle (r, 8.0f, 1.2f);
        paintMedal (g, { r.getX() + 10.0f, r.getY() + 9.0f, 44.0f, 52.0f }, l->engine);
        const String t = pk (l->titleEs, l->titleEn);
        const float h = theme::fitHeight (t, r.getWidth() - 260.0f, 22.0f);
        theme::drawPrintedText (g, t, r.getX() + 68.0f + theme::textWidth (t, h) * 0.5f, r.getY() + 21.0f, h, 0.0f, theme::mustardHi, theme::black, 0.1f, false);
        const String who = u8 (engines()[(size_t) l->engine].character) + "  ·  " + engines()[(size_t) l->engine].technical;
        theme::drawPrintedText (g, who, r.getRight() - 14.0f - theme::textWidth (who, 13.0f) * 0.5f, r.getY() + 21.0f, 13.0f, 0.0f,
                                theme::cream.withAlpha (0.55f), theme::black, 0.0f, false);
        g.setColour (theme::cream.withAlpha (0.85f));
        g.setFont (theme::font (14.5f));
        g.drawFittedText (pk (l->blurbEs, l->blurbEn), Rectangle<float> (r.getX() + 68.0f, r.getY() + 36.0f, r.getWidth() - 84.0f, 30.0f).toNearestInt(),
                          Justification::centredLeft, 2, 0.9f);
    }

    // lo que entendio de mas
    String foot;
    if (! result.modifiers.empty())
    {
        StringArray m;
        for (auto* mod : result.modifiers) m.add (pk (mod->labelEs, mod->labelEn));
        foot = pk ("Y AL FINAL LO HACEMOS ", "AND AT THE END WE MAKE IT ") + m.joinIntoString (", ").toUpperCase();
    }
    else foot = pk ("ELEGÍ UNA RECETA  ·  ENTER = LA PRIMERA  ·  ESC = SALIR", "PICK A RECIPE  ·  ENTER = THE FIRST  ·  ESC = EXIT");
    theme::drawPrintedText (g, foot, askCard.getCentreX(), askCard.getBottom() - 34.0f, theme::fitHeight (foot, askCard.getWidth() - 80.0f, 15.0f), 0.0f,
                            result.modifiers.empty() ? theme::cream.withAlpha (0.45f) : theme::mustard, theme::black, 0.0f, false);
}

//============================================================================== LECCION
void TeacherOverlay::layoutLesson()
{
    auto* st = currentStep();
    if (st == nullptr) return;
    hole = (resolve && String (st->target).isNotEmpty() ? resolve (st->target) : Rectangle<int>()).toFloat();
    if (hole.getWidth() >= (float) getWidth() - 4.0f) hole = {};

    // el cartel crece con el texto
    const float cw = 660.0f;
    AttributedString body;
    body.append (pk (st->es, st->en), theme::font (16.0f), theme::cream);
    body.setLineSpacing (3.0f);
    TextLayout tl;
    tl.createLayout (body, cw - 140.0f);
    const float ch = jmax (250.0f, tl.getHeight() + 190.0f);
    const auto bounds = getLocalBounds().toFloat().reduced (16.0f);
    float y;
    if (hole.isEmpty()) y = bounds.getCentreY() - ch * 0.5f;
    else if (hole.getBottom() + 24.0f + ch < bounds.getBottom()) y = hole.getBottom() + 24.0f;
    else if (hole.getY() - 24.0f - ch > bounds.getY()) y = hole.getY() - 24.0f - ch;
    else y = bounds.getCentreY() - ch * 0.5f;
    const float cx = jlimit (bounds.getX() + cw * 0.5f, bounds.getRight() - cw * 0.5f, hole.isEmpty() ? bounds.getCentreX() : hole.getCentreX());
    card = { cx - cw * 0.5f, y, cw, ch };

    const float by = card.getBottom() - 54.0f;
    prevBtn = { card.getX() + 24.0f, by, 132.0f, 36.0f };
    exitBtn = { card.getX() + 166.0f, by, 110.0f, 36.0f };
    showBtn = { card.getRight() - 334.0f, by, 150.0f, 36.0f };
    nextBtn = { card.getRight() - 174.0f, by, 150.0f, 36.0f };

    lastStatus = {}; lastOk = false;
    okAtEntry = ! st->conds.empty() && satisfied() && ! (st->conds.size() == 1 && String (st->conds[0].param) == "@note");
}

void TeacherOverlay::paintLesson (Graphics& g)
{
    auto* st = currentStep();
    if (st == nullptr || lesson == nullptr) return;

    Path shade;
    shade.addRectangle (getLocalBounds().toFloat());
    if (! hole.isEmpty()) shade.addRoundedRectangle (hole.expanded (6.0f), 12.0f);
    shade.setUsingNonZeroWinding (false);
    g.setColour (Colours::black.withAlpha (0.6f));
    g.fillPath (shade);

    bool ok = false;
    const auto status = statusLine (ok);

    if (! hole.isEmpty())
    {
        const float pulse = 0.5f + 0.5f * std::sin ((float) Time::getMillisecondCounter() * 0.006f);
        const auto col = ok ? Colour (0xff9be05a) : theme::mustardHi;
        g.setColour (col.withAlpha (0.15f + 0.2f * pulse));
        g.drawRoundedRectangle (hole.expanded (10.0f + 3.0f * pulse), 14.0f, 8.0f);
        g.setColour (col);
        g.drawRoundedRectangle (hole.expanded (6.0f), 12.0f, 3.0f);
    }

    paintCardFrame (g, card);
    paintMedal (g, { card.getX() + 26.0f, card.getY() + 26.0f, 70.0f, 84.0f }, lesson->engine);

    const String title = pk (st->titleEs, st->titleEn);
    const float th = theme::fitHeight (title, card.getWidth() - 220.0f, 26.0f);
    theme::drawPrintedText (g, title, card.getX() + 114.0f + theme::textWidth (title, th) * 0.5f, card.getY() + 40.0f, th, 0.0f,
                            theme::mustardHi, theme::black, 0.2f);
    const String count = String (index + 1) + " / " + String ((int) steps.size());
    theme::drawPrintedText (g, count, card.getRight() - 46.0f, card.getY() + 40.0f, 13.0f, 0.0f, theme::cream.withAlpha (0.6f), theme::black, 0.0f, false);

    AttributedString body;
    body.append (pk (st->es, st->en), theme::font (16.0f), theme::cream);
    body.setLineSpacing (3.0f);
    TextLayout tl;
    tl.createLayout (body, card.getWidth() - 140.0f);
    tl.draw (g, { card.getX() + 114.0f, card.getY() + 62.0f, card.getWidth() - 140.0f, tl.getHeight() + 4.0f });

    if (status.isNotEmpty())
    {
        const float sh = theme::fitHeight (status, card.getWidth() - 60.0f, 16.0f);
        theme::drawPrintedText (g, status, card.getCentreX(), card.getBottom() - 78.0f, sh, 0.0f,
                                ok ? Colour (0xff9be05a) : theme::mustard, theme::black, 0.0f, false);
    }

    const bool waits = ! st->conds.empty();
    const bool onlyNote = waits && st->conds.size() == 1 && String (st->conds[0].param) == "@note";
    const bool last = index == (int) steps.size() - 1;
    paintButton (g, prevBtn, pk ("ANTERIOR", "BACK"), false, index > 0);
    paintButton (g, exitBtn, pk ("SALIR", "EXIT"), false, true);
    paintButton (g, showBtn, pk ("MOSTRAME", "SHOW ME"), false, waits && ! onlyNote && ! ok);
    paintButton (g, nextBtn, last ? pk ("TERMINAR", "FINISH") : (index == 0 ? pk ("EMPEZAR", "START") : pk ("SIGUIENTE", "NEXT")),
                 true, ! waits || ok || onlyNote);
}

void TeacherOverlay::paint (Graphics& g)
{
    if (phase == Phase::ask) paintAsk (g);
    else if (phase == Phase::lesson) paintLesson (g);
}

//============================================================================== mouse / teclado
bool TeacherOverlay::hitTest (int x, int y)
{
    if (phase == Phase::lesson)
        return card.contains ((float) x, (float) y) || ! hole.expanded (2.0f).contains ((float) x, (float) y);
    return phase == Phase::ask;
}

void TeacherOverlay::mouseMove (const MouseEvent& e)
{
    if (phase != Phase::ask) return;
    int h = -1;
    for (size_t i = 0; i < rows.size(); ++i) if (rows[i].first.contains (e.position)) h = (int) i;
    bool onChip = false;
    for (auto& c : chips) onChip |= c.first.contains (e.position);
    setMouseCursor (h >= 0 || onChip || closeBtn.contains (e.position) ? MouseCursor::PointingHandCursor : MouseCursor::NormalCursor);
    if (h != hoverRow) { hoverRow = h; repaint (askCard.toNearestInt()); }
}

void TeacherOverlay::mouseDown (const MouseEvent& e)
{
    if (phase == Phase::ask)
    {
        if (closeBtn.contains (e.position) || ! askCard.contains (e.position)) { close(); return; }
        for (auto& [r, t] : chips) if (r.contains (e.position)) { input.setText (t, true); input.grabKeyboardFocus(); return; }
        for (auto& [r, l] : rows) if (r.contains (e.position)) { startLesson (*l, result.modifiers); return; }
        return;
    }
    if (phase != Phase::lesson || ! card.contains (e.position)) return;

    auto* st = currentStep();
    const bool waits = st != nullptr && ! st->conds.empty();
    const bool onlyNote = waits && st->conds.size() == 1 && String (st->conds[0].param) == "@note";
    if (exitBtn.contains (e.position)) close();
    else if (prevBtn.contains (e.position)) previous();
    else if (showBtn.contains (e.position) && waits && ! onlyNote) showMe();
    else if (nextBtn.contains (e.position) && (! waits || satisfied() || onlyNote)) next();
}

bool TeacherOverlay::keyPressed (const KeyPress& k)
{
    if (phase == Phase::closed) return false;
    if (k == KeyPress::escapeKey) { close(); return true; }
    if (phase == Phase::lesson)
    {
        auto* st = currentStep();
        const bool canNext = st != nullptr && (st->conds.empty() || satisfied());
        if ((k == KeyPress::rightKey || k == KeyPress::returnKey) && canNext) { next(); return true; }
        if (k == KeyPress::leftKey) { previous(); return true; }
    }
    return false;
}
}
