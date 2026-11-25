// Pruebas de la logica de sonido (sin interfaz).
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"
#include "Anim/AnimationLayer.h"
#include "PluginEditor.h"
#include "Core/FactoryPresets.h"
#include "Help/Help.h"
#include "Help/I18n.h"
#include "Teach/Lessons.h"
#include "Core/Modulation.h"
#include "Core/FactoryPresets.h"
#include "Core/SoundGenerator.h"

using namespace flipper;
using Gen = SoundGenerator;
static int failures = 0;
#define CHECK(cond) do { if (! (cond)) { std::printf ("FALLA: %s (linea %d)\n", #cond, __LINE__); ++failures; } else std::printf ("ok: %s\n", #cond); } while (0)

static float val (FlipperProcessor& p, const juce::String& id) { return p.state.getParameter (id)->getValue(); }

int main()
{
    juce::ScopedJuceInitialiser_GUI init;
    auto tmp = juce::File::getSpecialLocation (juce::File::tempDirectory).getChildFile ("flipper_tests");
    tmp.deleteRecursively();

    FlipperProcessor p;
    p.presets = PresetManager (tmp);
    p.presets.setFactory (flipper::buildFactoryPresets (p.state));

    // semilla reproducible
    p.setGeneratorMode (Gen::Mode::hybrid);
    p.generateSound (777); auto a = Gen::capture (p.state);
    p.generateSound (12);
    p.generateSound (777); auto b = Gen::capture (p.state);
    CHECK (Gen::equal (a, b));

    // HIBRIDO: entre 2 y 4 motores
    int on = 0; for (int e = 0; e < numEngines; ++e) on += val (p, ids::engineOn (e)) > 0.5f;
    CHECK (on >= 2 && on <= 4);

    // CLASICO: solo el motor elegido
    p.state.state.setProperty (ids::selectedEngine, 5, nullptr);
    p.setGeneratorMode (Gen::Mode::classic);
    p.generateSound (99);
    on = 0; for (int e = 0; e < numEngines; ++e) on += val (p, ids::engineOn (e)) > 0.5f;
    CHECK (on == 1 && val (p, ids::engineOn (5)) > 0.5f);

    // bloqueos: macros y un motor no cambian
    p.setGeneratorMode (Gen::Mode::hybrid);
    p.setLocked ("macros", true); p.setLocked ("dx7", true);
    auto before = Gen::capture (p.state);
    for (juce::uint32 s = 1; s < 20; ++s) p.generateSound (s);
    p.mutateSound();
    bool macrosSame = true;
    for (int m = 0; m < numMacros; ++m) macrosSame &= std::abs (val (p, ids::macro (m)) - before[(size_t) (numEngines * 2 + m)].second) < 1e-6f;
    CHECK (macrosSame);
    CHECK (std::abs (val (p, ids::engineLevel (5)) - before[11].second) < 1e-6f);
    CHECK (val (p, ids::engineOn (5)) > 0.5f);
    p.setLocked ("macros", false); p.setLocked ("dx7", false);

    // deshacer / rehacer
    p.generateSound (501); auto s1 = Gen::capture (p.state);
    p.generateSound (502); auto s2 = Gen::capture (p.state);
    CHECK (p.undo()); CHECK (Gen::equal (Gen::capture (p.state), s1));
    CHECK (p.redo()); CHECK (Gen::equal (Gen::capture (p.state), s2));

    // A/B
    p.switchAB (1); p.generateSound (900); auto bSound = Gen::capture (p.state);
    p.switchAB (0); CHECK (Gen::equal (Gen::capture (p.state), s2));
    p.switchAB (1); CHECK (Gen::equal (Gen::capture (p.state), bSound));

    // presets: guardar, leer, favorito, categoria
    auto snap = Gen::capture (p.state);
    auto f = p.presets.save ("Prueba / 1", "FM", "oscuro, 808", false, snap);
    CHECK (f.existsAsFile());
    p.presets.rescan();
    const auto& mine = p.presets.all().back();          // los de fabrica van primero
    CHECK (! mine.factory && mine.name == "Prueba / 1");
    CHECK (mine.tags.size() == 2);
    CHECK (Gen::equal (mine.values, snap));
    p.presets.setFavorite (f, true); p.presets.rescan();
    CHECK (p.presets.all().back().favorite);
    // fabrica: hay de los 9 motores y cada uno suena (se prueba abajo); favorito de fabrica se guarda aparte
    int factoryCount = 0; std::array<int, numEngines> perEngine {};
    for (auto& pr : p.presets.all()) if (pr.factory) { ++factoryCount; for (int e = 0; e < numEngines; ++e) perEngine[(size_t) e] += pr.engineOn[(size_t) e]; }
    std::printf ("presets de fabrica: %d\n", factoryCount);
    bool allEngines = true; for (int n : perEngine) allEngines &= n >= 4;
    CHECK (factoryCount >= 36 && allEngines);
    p.presets.setFavorite (p.presets.all()[2], true); p.presets.rescan();
    CHECK (p.presets.all()[2].favorite);
    p.generateSound (3); p.applyWithHistory (p.presets.all().back().values);
    CHECK (Gen::equal (Gen::capture (p.state), snap));

    // estado del plugin: guardar y restaurar
    juce::MemoryBlock mb; p.getStateInformation (mb);
    FlipperProcessor q; q.setStateInformation (mb.getData(), (int) mb.getSize());
    CHECK (Gen::equal (Gen::capture (q.state), snap));
    CHECK (q.generatorSettings().mode == Gen::Mode::hybrid);

    // ===================================================== AUDIO
    {
        FlipperProcessor ap;
        const double sr = 48000.0; const int bs = 256;
        ap.prepareToPlay (sr, bs);
        auto setP = [&] (const juce::String& id, float real)
        { auto* prm = ap.state.getParameter (id); prm->setValueNotifyingHost (prm->convertTo0to1 (real)); };
        auto render = [&] (double seconds, juce::MidiBuffer first = {}) -> std::vector<float>
        {
            std::vector<float> out;
            const int blocks = (int) (seconds * sr / bs);
            for (int b = 0; b < blocks; ++b)
            {
                juce::AudioBuffer<float> buf (2, bs);
                juce::MidiBuffer m; if (b == 0) m = first;
                ap.processBlock (buf, m);
                for (int i = 0; i < bs; ++i) out.push_back (buf.getSample (0, i));
            }
            return out;
        };
        auto noteOn  = [] (int n) { juce::MidiBuffer m; m.addEvent (juce::MidiMessage::noteOn (1, n, (juce::uint8) 100), 10); return m; };
        auto noteOff = [] (int n) { juce::MidiBuffer m; m.addEvent (juce::MidiMessage::noteOff (1, n), 0); return m; };
        auto rms = [] (const std::vector<float>& v, size_t from = 0)
        { double s = 0; for (size_t i = from; i < v.size(); ++i) s += v[i] * v[i]; return std::sqrt (s / juce::jmax ((size_t) 1, v.size() - from)); };
        auto finite = [] (const std::vector<float>& v) { for (float x : v) if (! std::isfinite (x) || std::abs (x) > 1.01f) return false; return true; };

        // sonido base: solo MINIMOOG, sin reverb
        for (int e = 0; e < numEngines; ++e) setP (ids::engineOn (e), e == 0 ? 1.0f : 0.0f);
        setP (ids::macro (5), 0.0f); // ESPACIO 0
        auto a1 = render (0.5, noteOn (48));
        std::printf ("rms nota MINIMOOG: %.4f\n", rms (a1));
        CHECK (rms (a1) > 0.02);
        CHECK (finite (a1));

        // al soltar, se apaga
        auto a2 = render (1.5, noteOff (48));
        CHECK (rms (a2, a2.size() - 4800) < 1.0e-4);
        CHECK (ap.getHeldNotes() == 0);

        // todos los motores apagados: silencio total
        for (int e = 0; e < numEngines; ++e) setP (ids::engineOn (e), 0.0f);
        render (0.05);
        auto a3 = render (0.4, noteOn (60));
        CHECK (rms (a3) == 0.0);
        render (0.3, noteOff (60));
        setP (ids::engineOn (0), 1.0f);

        // afinacion: solo OSC 1 en sierra 8', filtro abierto -> 440 Hz en el La 69
        setP ("minimoog_mix2", 0); setP ("minimoog_mix3", 0); setP ("minimoog_cutoff", 18000); setP ("minimoog_contour", 0);
        setP (ids::character, 0.0f);
        for (int e = 1; e < numEngines; ++e) setP (ids::engineOn (e), 0.0f);
        auto a4 = render (1.0, noteOn (69));
        int crossings = 0;
        bool low = false; // con histeresis: el ripple fino no cuenta como ciclo
        for (size_t i = 24000; i < a4.size(); ++i)
        {
            if (a4[i] < -0.03f) low = true;
            else if (low && a4[i] > 0.03f) { low = false; ++crossings; }
        }
        const double hz = crossings / ((a4.size() - 24000) / sr);
        std::printf ("frecuencia medida (La 69): %.1f Hz\n", hz);
        CHECK (std::abs (hz - 440.0) < 8.0);
        render (1.0, noteOff (69));

        // el filtro corta agudos: energia de la diferencia (aprox. de agudos) baja con el corte
        auto brightness = [&] (float cutoff)
        {
            setP ("minimoog_cutoff", cutoff);
            auto v = render (0.5, noteOn (57));
            render (1.2, noteOff (57));
            double d = 0; for (size_t i = 1; i < v.size(); ++i) d += (v[i] - v[i - 1]) * (v[i] - v[i - 1]);
            return d;
        };
        const double open = brightness (12000), closed = brightness (300);
        std::printf ("agudos filtro abierto %.3f / cerrado %.3f\n", open, closed);
        CHECK (closed < open * 0.2);

        // BRILLO (macro) tambien mueve el filtro
        setP ("minimoog_cutoff", 800);
        setP (ids::macro (1), 0.0f); const double dark = brightness (800);
        setP (ids::macro (1), 1.0f); const double bright = brightness (800);
        setP (ids::macro (1), 0.5f);
        CHECK (bright > dark * 3.0);

        // pedal de sustain sostiene la nota
        {
            juce::MidiBuffer m; m.addEvent (juce::MidiMessage::controllerEvent (1, 64, 127), 0);
            m.addEvent (juce::MidiMessage::noteOn (1, 52, (juce::uint8) 100), 1);
            render (0.2, m);
            auto held = render (0.6, noteOff (52));
            CHECK (rms (held, held.size() - 4800) > 0.01);
            juce::MidiBuffer up; up.addEvent (juce::MidiMessage::controllerEvent (1, 64, 0), 0);
            auto gone = render (1.5, up);
            CHECK (rms (gone, gone.size() - 4800) < 1.0e-4);
        }

        // ROTO + todo al maximo: sin valores invalidos
        setP (ids::character, 1.0f); setP (ids::macro (5), 1.0f); setP ("minimoog_emphasis", 1.0f); setP ("minimoog_drive", 1.0f);
        {
            juce::MidiBuffer m; for (int n = 36; n < 84; n += 4) m.addEvent (juce::MidiMessage::noteOn (1, n, (juce::uint8) 127), 0);
            auto loud = render (1.0, m);
            CHECK (finite (loud));
            juce::MidiBuffer off; off.addEvent (juce::MidiMessage::allNotesOff (1), 0);
            render (0.1, off);
        }

        // cada motor solo: suena, se apaga al soltar, sin valores invalidos (patch de fabrica)
        setP (ids::character, 0.3f); setP (ids::macro (5), 0.3f);
        for (int e = 0; e < numEngines; ++e)
        {
            for (int k = 0; k < numEngines; ++k) setP (ids::engineOn (k), k == e ? 1.0f : 0.0f);
            juce::MidiBuffer chord; for (int nn : { 48, 55, 60, 64 }) chord.addEvent (juce::MidiMessage::noteOn (1, nn, (juce::uint8) 100), 0);
            auto on = render (0.8, chord);
            juce::MidiBuffer offs; for (int nn : { 48, 55, 60, 64 }) offs.addEvent (juce::MidiMessage::noteOff (1, nn), 0);
            auto off = render (5.0, offs);
            std::printf ("motor %-12s rms %.4f  cola final %.6f\n", engines()[(size_t) e].technical, rms (on), rms (off, off.size() - 4800));
            CHECK (rms (on) > 0.02);
            CHECK (finite (on) && finite (off));
            CHECK (rms (off, off.size() - 4800) < 1.0e-3);
        }
        setP (ids::engineOn (0), 1.0f);

        // cada preset de fabrica suena (acorde corto) y no se rompe
        {
            int silent = 0;
            for (auto& pr : ap.presets.all())
            {
                if (! pr.factory) continue;
                flipper::SoundGenerator::apply (ap.state, pr.values);
                juce::MidiBuffer ch; for (int nn : { 48, 60, 64, 67 }) ch.addEvent (juce::MidiMessage::noteOn (1, nn, (juce::uint8) 100), 0);
                auto v = render (0.6, ch);
                juce::MidiBuffer off; off.addEvent (juce::MidiMessage::allNotesOff (1), 0);
                render (0.2, off);
                const double level = rms (v);
                if (level < 0.01 || ! finite (v)) { ++silent; std::printf ("  preset debil: %s (%.4f)\n", pr.name.toRawUTF8(), level); }
            }
            CHECK (silent == 0);
            ap.state.getParameter ("arp_on")->setValueNotifyingHost (0.0f);
        }

        // arpegiador: una tecla sostenida da notas repetidas (ataques separados)
        {
            for (int k = 0; k < numEngines; ++k) setP (ids::engineOn (k), k == 0 ? 1.0f : 0.0f);
            ap.state.getParameter ("arp_on")->setValueNotifyingHost (1.0f);
            setP ("minimoog_a_release", 0.01f); setP (ids::macro (5), 0.0f); setP (ids::macro (7), 0.5f);
            juce::MidiBuffer m; m.addEvent (juce::MidiMessage::noteOn (1, 60, (juce::uint8) 100), 0);
            m.addEvent (juce::MidiMessage::noteOn (1, 64, (juce::uint8) 100), 0);
            auto a5 = render (2.0, m);
            int onsets = 0; float prev = 0;
            for (size_t i = 0; i < a5.size(); i += 240)
            {
                double e2 = 0; for (size_t k = i; k < juce::jmin (a5.size(), i + 240); ++k) e2 += a5[k] * a5[k];
                const float lvl = (float) std::sqrt (e2 / 240.0);
                if (lvl > 0.02f && prev < 0.5f * lvl) ++onsets;
                prev = lvl;
            }
            std::printf ("arp rms %.4f\n", rms (a5));
            std::printf ("arpegiador: %d ataques en 2 s (1/16 a 120 bpm = 16)\n", onsets);
            CHECK (onsets >= 10);
            ap.state.getParameter ("arp_on")->setValueNotifyingHost (0.0f);
            juce::MidiBuffer off; off.addEvent (juce::MidiMessage::allNotesOff (1), 0);
            render (0.5, off);
        }

        // CPU: 12 voces del MINIMOOG + reverb, 10 s de audio
        {
            ap.setStateInformation (nullptr, 0);
            FlipperProcessor cp; cp.prepareToPlay (sr, bs);
            juce::MidiBuffer m; for (int n = 40; n < 64; n += 2) m.addEvent (juce::MidiMessage::noteOn (1, n, (juce::uint8) 100), 0);
            const double t0 = juce::Time::getMillisecondCounterHiRes();
            const int blocks = (int) (10.0 * sr / bs);
            for (int b = 0; b < blocks; ++b)
            {
                juce::AudioBuffer<float> buf (2, bs);
                juce::MidiBuffer mm; if (b == 0) mm = m;
                cp.processBlock (buf, mm);
            }
            const double ms = juce::Time::getMillisecondCounterHiRes() - t0;
            std::printf ("CPU 12 voces MINIMOOG: %.1f ms para 10 s de audio (%.1f%% de un nucleo)\n", ms, ms / 100.0);
            CHECK (ms < 3000.0);

            // los 9 motores encendidos a la vez, 8 voces
            FlipperProcessor all; all.prepareToPlay (sr, bs);
            for (int e = 0; e < numEngines; ++e) all.state.getParameter (ids::engineOn (e))->setValueNotifyingHost (1.0f);
            juce::MidiBuffer m8; for (int nn = 48; nn < 64; nn += 2) m8.addEvent (juce::MidiMessage::noteOn (1, nn, (juce::uint8) 100), 0);
            const double t1 = juce::Time::getMillisecondCounterHiRes();
            for (int b = 0; b < blocks; ++b)
            {
                juce::AudioBuffer<float> buf (2, bs);
                juce::MidiBuffer mm; if (b == 0) mm = m8;
                all.processBlock (buf, mm);
            }
            const double ms9 = juce::Time::getMillisecondCounterHiRes() - t1;
            std::printf ("CPU 9 motores x 8 voces: %.1f ms para 10 s de audio (%.1f%% de un nucleo)\n", ms9, ms9 / 100.0);
            CHECK (ms9 < 8000.0);
        }
    }

    // animaciones: maquina de estados, cooldown, independencia, limpieza
    {
        using namespace flipper::anim;
        AnimationLayer layer;
        layer.useManualClock (true);
        std::vector<std::pair<int, Sfx>> sounds;
        layer.onSfx = [&] (Sfx s, int e) { sounds.push_back ({ e, s }); };
        double t = 50000.0;
        layer.advance (t);
        auto step = [&] (double ms) { for (double end = t + ms; t < end; ) { t += 16.0; layer.advance (t); } };

        layer.trigger (3);
        CHECK (layer.phaseOf (3) == Phase::launch);
        CHECK (layer.phaseOf (5) == Phase::idle);             // un personaje no activa a otro
        step (250); CHECK (layer.phaseOf (3) == Phase::hit);
        step (200); CHECK (layer.phaseOf (3) == Phase::power);
        layer.trigger (3);                                    // click repetido dentro del cooldown
        CHECK (layer.phaseOf (3) == Phase::power);            // no reinicia
        layer.trigger (5);                                    // otro personaje en paralelo
        CHECK (layer.phaseOf (5) == Phase::launch);
        step (3500);
        CHECK (! layer.isAnimating (3) && ! layer.isAnimating (5)); // vuelven a reposo
        int launches = 0, impacts = 0, powers = 0, bonuses = 0;
        for (auto& [e, s] : sounds) { launches += s == Sfx::launch; impacts += s == Sfx::impact; powers += s == Sfx::power; bonuses += s == Sfx::bonus; }
        CHECK (launches == 2 && impacts == 2 && powers == 2 && bonuses == 2);

        // todos los personajes, varias veces seguidas: nada queda trabado
        for (int round = 0; round < 3; ++round)
        {
            for (int e = 0; e < numEngines; ++e) layer.trigger (e);
            step (950);
        }
        step (4000);
        bool anyActive = false;
        for (int e = 0; e < numEngines; ++e) anyActive |= layer.isAnimating (e);
        CHECK (! anyActive);

        layer.trigger (2); step (300);
        layer.setEnabled (false);                             // apagar animaciones corta todo
        CHECK (! layer.isAnimating (2));
        layer.trigger (2);
        CHECK (! layer.isAnimating (2));                      // apagadas no arrancan
    }

    // rendimiento: un cuadro con los 9 personajes animados a la vez (640 x 960)
    {
        FlipperProcessor bp;
        std::unique_ptr<FlipperEditor> ed (static_cast<FlipperEditor*> (bp.createEditor()));
        ed->setSize (640, 960);
        auto& layer = ed->getAnimationLayer();
        layer.useManualClock (true);
        double t = 90000.0; layer.advance (t);
        juce::Image frame (juce::Image::ARGB, 640, 960, true);
        {
            double tot = 0;
            for (int i = 0; i < 20; ++i)
            {
                const double t0 = juce::Time::getMillisecondCounterHiRes();
                { juce::Graphics g (frame); ed->paintEntireComponent (g, true); }
                tot += juce::Time::getMillisecondCounterHiRes() - t0;
            }
            std::printf ("cuadro completo sin animaciones: %.2f ms\n", tot / 20);
        }
        for (int e = 0; e < numEngines; ++e) layer.trigger (e);
        double worst = 0, total = 0; int frames = 0;
        for (int i = 0; i < 150; ++i)
        {
            t += 16.0; layer.advance (t);
            const double t0 = juce::Time::getMillisecondCounterHiRes();
            { juce::Graphics g (frame); ed->paintEntireComponent (g, true); }
            const double ms = juce::Time::getMillisecondCounterHiRes() - t0;
            worst = juce::jmax (worst, ms); total += ms; ++frames;
        }
        std::printf ("cuadro completo, 9 animaciones: promedio %.2f ms, peor %.2f ms\n", total / frames, worst);
        CHECK (total / frames < 16.0);
    }

    // ayuda: cada parametro tiene texto en los dos idiomas; cada paso de recorrido apunta a algo que existe
    {
        FlipperProcessor hp;
        int missing = 0, total = 0;
        for (auto lang : { i18n::Lang::es, i18n::Lang::en })
        {
            i18n::setLanguage (lang);
            for (auto* prm : hp.getParameters())
                if (auto* rp = dynamic_cast<juce::RangedAudioParameter*> (prm))
                {
                    ++total;
                    const auto tip = help::tooltip (rp->getParameterID());
                    if (! tip.contains ("\n") || tip.length() < 12) { ++missing; if (! rp->getParameterID().contains ("pin_")) std::printf ("  sin ayuda: %s\n", rp->getParameterID().toRawUTF8()); }
                }
        }
        std::printf ("ayuda de parametros: %d de %d con texto\n", total - missing, total);
        CHECK (missing == 0);

        std::unique_ptr<FlipperEditor> ed (static_cast<FlipperEditor*> (hp.createEditor()));
        int bad = 0, steps = 0;
        juce::StringArray screens { "play", "lab" };
        for (int e = 0; e < numEngines; ++e) screens.add ("design" + juce::String (e));
        for (auto& sc : screens)
        {
            if (sc.startsWith ("design")) ed->selectEngineForTest (sc.getTrailingIntValue());
            for (auto& st : help::tour (sc))
            {
                ++steps;
                if (ed->resolveTarget (st.target).isEmpty()) { ++bad; std::printf ("  paso sin destino: %s %s\n", sc.toRawUTF8(), st.target.toRawUTF8()); }
                if (juce::String::fromUTF8 (st.textEn).isEmpty() || juce::String::fromUTF8 (st.textEs).isEmpty()) ++bad;
            }
        }
        std::printf ("recorridos: %d pantallas, %d pasos\n", screens.size(), steps);
        CHECK (bad == 0 && steps > 50);
        i18n::setLanguage (i18n::Lang::es);
    }

    // ------------------------------------------------------------------ MOTORES QUE SE PRENDEN CON NOTAS SONANDO
    {
        FlipperProcessor mp;
        mp.prepareToPlay (48000.0, 512);
        auto setReal = [&mp] (const juce::String& id, float v) { auto* q = mp.state.getParameter (id); q->setValueNotifyingHost (q->convertTo0to1 (v)); };
        for (int e = 0; e < flipper::numEngines; ++e) setReal (flipper::ids::engineOn (e), e == 0 ? 1.0f : 0.0f);
        setReal (flipper::ids::macro (5), 0.0f);
        juce::AudioBuffer<float> b (2, 512);
        juce::MidiBuffer on; on.addEvent (juce::MidiMessage::noteOn (1, 60, (juce::uint8) 100), 0);
        mp.processBlock (b, on);
        for (int i = 0; i < 4; ++i) { juce::MidiBuffer e; b.clear(); mp.processBlock (b, e); }
        // nota retenida con KROKO; se cambia a PROFETA solo: la nota tiene que seguir sonando, ahora con PROFETA
        setReal (flipper::ids::engineOn (0), 0.0f);
        setReal (flipper::ids::engineOn (1), 1.0f);
        float peak = 0;
        for (int i = 0; i < 40; ++i) { juce::MidiBuffer e; b.clear(); mp.processBlock (b, e); if (i > 20) peak = juce::jmax (peak, b.getMagnitude (0, 512)); }
        std::printf ("motor nuevo en nota retenida: pico %.3f\n", peak);
        CHECK (peak > 0.01f);
    }

    // ------------------------------------------------------------------ MODULACION
    {
        FlipperProcessor mp;
        mp.prepareToPlay (48000.0, 512);
        auto& m = *mp.mod;
        auto* cut = mp.state.getParameter ("minimoog_cutoff");
        auto* shadow = flipper::ModMatrix::raw (mp.state, "minimoog_cutoff");
        juce::AudioBuffer<float> b (2, 512); juce::MidiBuffer none;
        const float base = cut->getValue();
        CHECK (m.canModulate ("minimoog_cutoff"));
        CHECK (! m.canModulate ("minimoog_osc1_wave"));   // selector: no se modula
        CHECK (! m.canModulate ("mod_a"));
        CHECK (m.setRoute (flipper::ModMatrix::modA, "minimoog_cutoff", 0.3f));
        CHECK (m.setRoute (flipper::ModMatrix::modA, "macro_brightness", 0.2f));   // una MOD mueve varias perillas
        auto* modA = mp.state.getParameter ("mod_a");
        modA->setValueNotifyingHost (1.0f);
        mp.processBlock (b, none);
        CHECK (std::abs (cut->convertTo0to1 (shadow->load()) - juce::jmin (1.0f, base + 0.3f)) < 0.01f);
        CHECK (std::abs (cut->getValue() - base) < 1e-6f);   // la perilla no se mueve, solo el sonido
        modA->setValueNotifyingHost (0.0f);
        mp.processBlock (b, none);
        CHECK (std::abs (cut->convertTo0to1 (shadow->load()) - base) < 0.01f);

        // LFO: el valor se mueve con el tiempo
        m.clearRoutes();
        CHECK (m.setRoute (flipper::ModMatrix::lfo1, "minimoog_cutoff", 0.4f));
        float lo = 1, hi = 0;
        for (int i = 0; i < 120; ++i) { mp.processBlock (b, none); const float v = cut->convertTo0to1 (shadow->load()); lo = juce::jmin (lo, v); hi = juce::jmax (hi, v); }
        std::printf ("LFO 1 > corte: %.2f .. %.2f\n", lo, hi);
        CHECK (hi - lo > 0.4f);

        // se guarda con el proyecto
        juce::MemoryBlock mb; mp.getStateInformation (mb);
        FlipperProcessor mp2; mp2.setStateInformation (mb.getData(), (int) mb.getSize());
        CHECK (std::abs (mp2.mod->depthOf (flipper::ModMatrix::lfo1, "minimoog_cutoff") - 0.4f) < 0.001f);

        // y con los presets / deshacer (capture + apply); los de fabrica limpian las rutas
        auto snap = flipper::SoundGenerator::capture (mp.state);
        m.clearRoutes();
        flipper::SoundGenerator::apply (mp.state, snap);
        CHECK (m.routes().size() == 1);
        auto factory = flipper::buildFactoryPresets (mp.state);
        flipper::SoundGenerator::apply (mp.state, factory.front().values);
        CHECK (m.routes().empty());

        // suena y no explota con muchas rutas
        for (int s = 0; s < flipper::ModMatrix::numSources; ++s) m.setRoute (s, "jupiter8_cutoff", 0.5f);
        mp.state.getParameter (flipper::ids::engineOn (2))->setValueNotifyingHost (1.0f);
        juce::MidiBuffer on; on.addEvent (juce::MidiMessage::noteOn (1, 57, (juce::uint8) 100), 0);
        bool finite = true; float pk = 0;
        for (int i = 0; i < 60; ++i) { b.clear(); mp.processBlock (b, i == 0 ? on : none);
            for (int c = 0; c < 2; ++c) for (int k = 0; k < 512; ++k) { const float v = b.getSample (c, k); finite &= std::isfinite (v); pk = juce::jmax (pk, std::abs (v)); } }
        CHECK (finite && pk > 0.01f && pk < 4.0f);

        // editor: arrastrar una fuente a una perilla crea la ruta; FL encuentra el parametro de cada control
        m.clearRoutes();
        std::unique_ptr<FlipperEditor> ed (static_cast<FlipperEditor*> (mp.createEditor()));
        ed->setMode (flipper::UiMode::design);
        int found = 0, total = 0, noIndex = 0;
        for (int e = 0; e < flipper::numEngines; ++e)
        {
            ed->selectEngineForTest (e);
            for (auto* prm : mp.getParameters())
                if (auto* rp = dynamic_cast<juce::RangedAudioParameter*> (prm))
                {
                    const auto id = rp->getParameterID();
                    if (! id.startsWith (juce::String (flipper::engines()[(size_t) e].id) + "_")) continue;
                    const auto r = ed->findControl (id, true);
                    if (r.isEmpty()) continue;
                    ++total;
                    if (auto* c = ed->getChildComponent (0)->getComponentAt (r.getCentre()))
                    {
                        if (ed->getControlParameterIndex (*c) >= 0) ++found; else { ++noIndex; if (noIndex < 6) std::printf ("  sin link: %s\n", id.toRawUTF8()); }
                    }
                }
        }
        std::printf ("link to controller: %d de %d controles de DISENO\n", found, total);
        CHECK (total > 300 && noIndex == 0);
        ed->selectEngineForTest (0);
        const auto knob = ed->findControl ("minimoog_cutoff", true);
        CHECK (ed->dropModulation (flipper::ModMatrix::lfo2, knob.getCentre().toFloat()));
        CHECK (m.depthOf (flipper::ModMatrix::lfo2, "minimoog_cutoff") != 0.0f);
        const auto macroKnob = ed->findControl ("macro_space", true);
        CHECK (ed->dropModulation (flipper::ModMatrix::modB, macroKnob.getCentre().toFloat()));
        ed = nullptr;
    }

    // ------------------------------------------------------------------ SONIDOS GENERADOS: ANTERIOR / SIGUIENTE
    {
        FlipperProcessor gp;
        gp.generateSound ((juce::uint32) 11);
        auto* cut = gp.state.getParameter ("minimoog_cutoff");
        cut->setValueNotifyingHost (0.123f);                       // le cambio algo al primero
        const auto first = flipper::SoundGenerator::capture (gp.state);
        gp.generateSound ((juce::uint32) 22);
        gp.generateSound ((juce::uint32) 33);
        CHECK (gp.generatedCount() == 3 && gp.generatedIndex() == 2);
        CHECK (gp.previousGenerated() && gp.previousGenerated());
        CHECK (! gp.previousGenerated());
        CHECK (std::abs (cut->getValue() - 0.123f) < 1e-4f);       // volvio con mi cambio
        int diff = 0;
        const auto now = flipper::SoundGenerator::capture (gp.state);
        for (size_t k = 0; k < now.size() && k < first.size(); ++k) diff += std::abs (now[k].second - first[k].second) > 1e-5f ? 1 : 0;
        CHECK (diff == 0);
        CHECK (gp.nextGenerated() && gp.nextGenerated() && ! gp.nextGenerated());
        gp.previousGenerated();
        gp.generateSound ((juce::uint32) 44);                      // generar desde el medio descarta lo de adelante
        CHECK (gp.generatedCount() == 3 && gp.generatedIndex() == 2);
    }

    // ------------------------------------------------------------------ MAESTRO
    {
        using namespace flipper::teach;
        // buscador
        auto top = [] (const char* q) { auto r = search (juce::String::fromUTF8 (q)); return r.lessons.empty() ? juce::String() : juce::String (r.lessons.front()->id); };
        auto hasMod = [] (const char* q, const char* target) { for (auto* m : search (juce::String::fromUTF8 (q)).modifiers) if (juce::String (m->step.target) == target) return true; return false; };
        CHECK (top ("bajo 808 sucio") == "mm_808");
        CHECK (hasMod ("bajo 808 sucio", "character"));
        CHECK (top ("campana con el dx7") == "dx_bell");
        CHECK (top ("Piano Eléctrico") == "dx_ep");
        CHECK (top ("acid bass") == "arp_acid");
        CHECK (top ("arpegio") == "j8_arp");
        CHECK (top ("dark pad") .startsWith ("j8_pad") || top ("dark pad").contains ("pad"));
        CHECK (hasMod ("pad oscuro", "macro_brightness"));
        CHECK (top ("qué es un filtro") == "basics_filter");
        CHECK (top ("efecto espacial") == "vcs_laser");
        CHECK (! search ("xyzzy").understood);
        CHECK (search ("kroko").lessons.size() >= 3);
        std::printf ("maestro: %d recetas\n", (int) lessons().size());
        CHECK (lessons().size() >= 25);

        // cada receta: los pasos apuntan a controles visibles, no arrancan cumplidos, MOSTRAME los cumple y al final suena
        FlipperProcessor tp;
        tp.prepareToPlay (48000.0, 256);
        std::unique_ptr<FlipperEditor> ed (static_cast<FlipperEditor*> (tp.createEditor()));
        auto& t = ed->getTeacher();
        int badTarget = 0, preMet = 0, notMet = 0, badText = 0, silent = 0, total = 0;
        auto resetAll = [&tp]
        {
            for (auto* prm : tp.getParameters()) prm->setValueNotifyingHost (prm->getDefaultValue());
            tp.mod->clearRoutes();
        };
        const bool baseMode = true;
        for (auto& l : lessons())
        {
            resetAll();   // desde el sonido base: ningun paso tiene que arrancar cumplido
            t.startLesson (l, {});
            for (int i = 0; i < t.stepCount(); ++i)
            {
                auto* st = t.currentStep();
                ++total;
                if (juce::String::fromUTF8 (st->es).isEmpty() || juce::String::fromUTF8 (st->en).isEmpty()
                    || juce::String::fromUTF8 (st->titleEs).isEmpty() || juce::String::fromUTF8 (st->titleEn).isEmpty()) ++badText;
                if (juce::String (st->target).isNotEmpty() && t.currentHole().isEmpty())
                { ++badTarget; std::printf ("  %s paso %d sin control: %s\n", l.id, i, st->target); }
                bool real = false;
                for (auto& c : st->conds)
                {
                    if (juce::String (c.param) == "@note") continue;
                    real = true;
                    if (tp.state.getParameter (c.param) == nullptr) { ++badTarget; std::printf ("  %s paso %d parametro inexistente %s\n", l.id, i, c.param); }
                    if (c.set < c.lo || c.set > c.hi) { ++notMet; std::printf ("  %s paso %d: set fuera de rango (%s)\n", l.id, i, c.param); }
                }
                if (real && t.satisfied() && baseMode) { ++preMet; std::printf ("  %s paso %d ya estaba cumplido\n", l.id, i); }
                if (real) { t.showMe(); if (! t.satisfied()) { ++notMet; std::printf ("  %s paso %d: MOSTRAME no lo cumple\n", l.id, i); } }
                if (i < t.stepCount() - 1) t.next();
            }
            // el sonido terminado suena y no explota
            juce::AudioBuffer<float> buf (2, 256); juce::MidiBuffer midi;
            float peak = 0; bool finite = true;
            for (int b = 0; b < 160; ++b)
            {
                midi.clear(); buf.clear();
                if (b == 0) midi.addEvent (juce::MidiMessage::noteOn (1, l.engine == 5 || l.engine == 6 ? 64 : 45, (juce::uint8) 100), 0);
                if (b == 120) midi.addEvent (juce::MidiMessage::noteOff (1, l.engine == 5 || l.engine == 6 ? 64 : 45), 0);
                tp.processBlock (buf, midi);
                for (int ch = 0; ch < 2; ++ch) for (int s = 0; s < 256; ++s) { const float v = buf.getSample (ch, s); finite &= std::isfinite (v); peak = juce::jmax (peak, std::abs (v)); }
            }
            if (! finite || peak < 0.003f || peak > 4.0f) { ++silent; std::printf ("  %s: pico %.4f %s\n", l.id, peak, finite ? "" : "NaN"); }
            t.close();
        }
        std::printf ("maestro: %d pasos revisados\n", total);
        CHECK (badTarget == 0);
        CHECK (preMet == 0);
        CHECK (notMet == 0);
        CHECK (badText == 0);
        CHECK (silent == 0);

        // el maestro no destruye tu sonido: otros motores, macros, CHARACTER y modulaciones quedan igual
        {
            resetAll();
            tp.generateSound ((juce::uint32) 4242);
            tp.mod->setRoute (flipper::ModMatrix::lfo1, "jupiter8_cutoff", 0.3f);
            const auto before = flipper::SoundGenerator::capture (tp.state);
            t.startLesson (*find ("dx_bell"), {});
            const auto after = flipper::SoundGenerator::capture (tp.state);
            int changed = 0;
            for (size_t k = 0; k < before.size() && k < after.size(); ++k)
            {
                const auto& id = before[k].first;
                if (id == flipper::ids::engineOn (5) || id == flipper::ids::engineLevel (5)) continue;   // puede prender su motor
                if (std::abs (before[k].second - after[k].second) > 1e-6f) { ++changed; if (changed < 4) std::printf ("  el maestro cambio %s\n", id.toRawUTF8()); }
            }
            CHECK (changed == 0 && before.size() == after.size());
            // y se puede terminar igual desde un sonido cualquiera (los pasos cumplidos se saltean)
            for (int i = 0; i < t.stepCount() - 1; ++i) { t.showMe(); CHECK (t.currentStep()->conds.empty() || t.satisfied() || juce::String (t.currentStep()->conds[0].param) == "@note"); t.next(); }
            t.close();
        }

        // con modificadores: el paso extra aparece y apunta a su perilla
        auto r = search ("bajo 808 oscuro con espacio");
        t.startLesson (*r.lessons.front(), r.modifiers);
        CHECK (t.stepCount() == (int) r.lessons.front()->steps.size() + 2 + 2);
        t.close();
        ed = nullptr;
    }

    tmp.deleteRecursively();
    std::printf ("\n%s (%d fallas)\n", failures == 0 ? "TODO OK" : "HAY FALLAS", failures);
    return failures == 0 ? 0 : 1;
}
