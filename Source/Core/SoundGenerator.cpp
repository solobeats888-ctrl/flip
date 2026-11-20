#include "SoundGenerator.h"

namespace flipper
{
using namespace juce;

namespace
{
float centred (Random& r, float spread)
{
    // suma de dos uniformes: el centro es mas probable que los extremos
    return jlimit (0.0f, 1.0f, 0.5f + (r.nextFloat() + r.nextFloat() - 1.0f) * spread);
}

float inRange (Random& r, float lo, float hi) { return lo + r.nextFloat() * (hi - lo); }

// Estilos: rango de cada macro (cuerpo, brillo, movimiento, textura, anchura, espacio, ataque, liberacion),
// rango de CHARACTER y motores preferidos.
struct Style
{
    std::array<std::pair<float, float>, numMacros> macro;
    std::pair<float, float> character;
    std::vector<int> engines;
};

const std::vector<Style>& styles()
{
    // indices de motor: 0 minimoog, 1 prophet5, 2 jupiter8, 3 juno, 4 cs80, 5 dx7, 6 m1, 7 arp2600, 8 vcs3
    static const std::vector<Style> list {
        // OSCURO
        { {{ {0.6f,1.f},{0.f,0.35f},{0.2f,0.6f},{0.3f,0.7f},{0.3f,0.7f},{0.3f,0.7f},{0.f,0.5f},{0.4f,0.9f} }}, {0.4f,0.8f}, { 0, 7, 4, 8 } },
        // BRILLANTE
        { {{ {0.2f,0.6f},{0.7f,1.f},{0.2f,0.6f},{0.f,0.4f},{0.5f,1.f},{0.2f,0.6f},{0.f,0.3f},{0.2f,0.6f} }}, {0.f,0.4f}, { 5, 3, 2, 1 } },
        // AMBIENTE
        { {{ {0.3f,0.7f},{0.3f,0.7f},{0.5f,1.f},{0.3f,0.8f},{0.6f,1.f},{0.7f,1.f},{0.5f,1.f},{0.7f,1.f} }}, {0.f,0.5f}, { 4, 2, 6, 8 } },
        // AGRESIVO
        { {{ {0.7f,1.f},{0.6f,1.f},{0.3f,0.8f},{0.4f,0.9f},{0.2f,0.6f},{0.f,0.4f},{0.f,0.2f},{0.f,0.4f} }}, {0.6f,1.f}, { 0, 1, 5, 7 } },
        // LO-FI
        { {{ {0.4f,0.8f},{0.1f,0.45f},{0.2f,0.6f},{0.6f,1.f},{0.f,0.4f},{0.2f,0.6f},{0.f,0.5f},{0.3f,0.7f} }}, {0.45f,0.85f}, { 6, 3, 8 } },
    };
    return list;
}
}

const StringArray& SoundGenerator::styleNames()
{
    static const StringArray names { "OSCURO", "BRILLANTE", "AMBIENTE", "AGRESIVO", "LO-FI" };
    return names;
}

const StringArray& SoundGenerator::modeNames()
{
    static const StringArray names { String::fromUTF8 ("CLÁSICO"), "ESTILO", String::fromUTF8 ("HÍBRIDO") };
    return names;
}

Snapshot SoundGenerator::capture (const AudioProcessorValueTreeState& state)
{
    // todos los parametros del instrumento (mezcla, macros, CHARACTER y cada motor)
    Snapshot s;
    for (auto* p : state.processor.getParameters())
        if (auto* rp = dynamic_cast<RangedAudioParameter*> (p))
            s.push_back ({ rp->getParameterID(), rp->getValue() });
    return s;
}

namespace
{
/** Reglas del MINIMOOG: sonidos musicales (bajos, leads, pads) en vez de valores sueltos al azar. */
void randomizeMinimoog (Random& r, Snapshot& s, const AudioProcessorValueTreeState& state)
{
    auto set = [&] (const char* name, float realValue)
    {
        const String id = String ("minimoog_") + name;
        if (auto* p = state.getParameter (id)) s.push_back ({ id, p->convertTo0to1 (realValue) });
    };
    const int kind = r.nextInt (3); // 0 bajo, 1 lead, 2 pad
    const int baseRange = kind == 0 ? 1 : 2;
    for (int o = 1; o <= 3; ++o)
    {
        const String n (o);
        const int range = jlimit (0, 4, baseRange + (o == 3 ? -r.nextInt (2) : (r.nextInt (4) == 0 ? 1 : 0)));
        const int waves[] = { 2, 2, 2, 1, 3, 4, 5, 0 };
        set (("osc" + n + "_range").toRawUTF8(), (float) range);
        set (("osc" + n + "_wave").toRawUTF8(), (float) waves[r.nextInt (8)]);
        if (o > 1) set (("osc" + n + "_detune").toRawUTF8(), r.nextInt (6) == 0 ? (float) (r.nextBool() ? 7 : 5) : inRange (r, -0.15f, 0.15f));
        set (("mix" + n).toRawUTF8(), o == 1 ? inRange (r, 0.6f, 1.0f) : inRange (r, 0.0f, 0.85f));
    }
    set ("noise", r.nextInt (4) == 0 ? inRange (r, 0.05f, 0.3f) : 0.0f);
    set ("cutoff", kind == 0 ? inRange (r, 150, 900) : (kind == 1 ? inRange (r, 500, 3500) : inRange (r, 300, 2500)));
    set ("emphasis", inRange (r, 0.0f, kind == 1 ? 0.75f : 0.5f));
    set ("contour", inRange (r, 0.1f, 0.8f));
    set ("kbd", (float) r.nextInt (3) / 3.0f);
    set ("drive", inRange (r, 0.0f, 0.7f));
    const bool pad = kind == 2;
    set ("f_attack", pad ? inRange (r, 0.2f, 2.0f) : inRange (r, 0.001f, 0.05f));
    set ("f_decay", inRange (r, 0.08f, 1.2f));
    set ("f_sustain", inRange (r, 0.0f, 0.6f));
    set ("f_release", inRange (r, 0.05f, pad ? 2.5f : 0.6f));
    set ("a_attack", pad ? inRange (r, 0.15f, 1.5f) : inRange (r, 0.001f, 0.02f));
    set ("a_decay", inRange (r, 0.1f, 1.5f));
    set ("a_sustain", kind == 0 ? inRange (r, 0.4f, 1.0f) : inRange (r, 0.6f, 1.0f));
    set ("a_release", pad ? inRange (r, 0.6f, 3.0f) : inRange (r, 0.05f, 0.5f));
    set ("glide", kind == 1 && r.nextBool() ? inRange (r, 0.03f, 0.25f) : 0.001f);
}

/**
    Reglas generales para los demas motores: cada parametro se mueve dentro de un rango musical
    segun lo que es (ataques cortos casi siempre, ruido y anillo casi nunca, filtros en rangos utiles).
    Los interruptores que cambian el comportamiento del teclado (zona, split) no se tocan.
*/
void randomizeEngine (Random& r, Snapshot& s, const AudioProcessorValueTreeState& state, const String& prefix)
{
    for (auto* prm : state.processor.getParameters())
    {
        auto* rp = dynamic_cast<RangedAudioParameter*> (prm);
        if (rp == nullptr) continue;
        const String id = rp->getParameterID();
        if (! id.startsWith (prefix) || id == prefix + "on" || id == prefix + "level") continue;   // mezcla: la decide el modo
        const String n = id.substring (prefix.length());
        auto real = [&] (float v) { s.push_back ({ id, rp->convertTo0to1 (jlimit (rp->getNormalisableRange().start, rp->getNormalisableRange().end, v)) }); };
        auto keep = [&] { s.push_back ({ id, rp->getDefaultValue() }); };
        auto near = [&] (float spread) { s.push_back ({ id, jlimit (0.0f, 1.0f, rp->getDefaultValue() + (r.nextFloat() * 2.0f - 1.0f) * spread) }); };
        auto chance = [&] (float p) { return r.nextFloat() < p; };

        if (n == "zone" || n == "split" || n == "ribbon" || n == "gliss") { keep(); continue; }
        if (n.startsWith ("pin_"))
        {
            const bool def = rp->getDefaultValue() > 0.5f;
            s.push_back ({ id, (chance (0.12f) ? ! def : def) ? 1.0f : 0.0f });
            continue;
        }
        if (dynamic_cast<AudioParameterChoice*> (rp) != nullptr || dynamic_cast<AudioParameterInt*> (rp) != nullptr)
        {
            if (n.contains ("unison") || n == "sync" || n == "lf" || n.endsWith ("_lf")) { s.push_back ({ id, chance (0.25f) ? 1.0f - rp->getDefaultValue() : rp->getDefaultValue() }); continue; }
            if (n.contains ("semi") || n.contains ("coarse") && ! prefix.startsWith ("dx7")) { s.push_back ({ id, chance (0.7f) ? rp->getDefaultValue() : r.nextFloat() }); continue; }
            if (n.contains ("_src") || n.contains ("_dst") || n.contains ("wave") || n.contains ("feet") || n.contains ("range")
                || n == "algorithm" || n.contains ("coarse") || n.contains ("mode") || n.contains ("chorus") || n.contains ("fx_type")
                || n.contains ("hpf") || n.contains ("slope") || n.contains ("trig") || n.contains ("color") || n.contains ("saw") || n.contains ("pulse") || n.contains ("tri"))
            { s.push_back ({ id, r.nextFloat() }); continue; }
            keep();
            continue;
        }
        if (n.contains ("attack"))       real (chance (0.75f) ? 0.001f + r.nextFloat() * 0.04f : 0.05f + r.nextFloat() * 1.2f);
        else if (n.contains ("release")) real (0.05f + r.nextFloat() * 1.5f);
        else if (n.contains ("decay"))   real (0.08f + r.nextFloat() * 1.6f);
        else if (n.contains ("sustain")) real (r.nextFloat());
        else if (n == "cutoff" || n.endsWith ("lpf")) real (250.0f * std::pow (24.0f, r.nextFloat()));
        else if (n.endsWith ("hpf"))     real (chance (0.7f) ? 20.0f : 20.0f + r.nextFloat() * 600.0f);
        else if (n == "res" || n.endsWith ("_res")) real (r.nextFloat() * 0.65f);
        else if (n.contains ("noise") || n.contains ("ring")) real (chance (0.7f) ? 0.0f : r.nextFloat() * 0.35f);
        else if (n.startsWith ("lfo_") && ! n.contains ("rate")) real (chance (0.6f) ? 0.0f : r.nextFloat() * 0.35f);
        else if (n.contains ("glide") || n.contains ("porta")) real (chance (0.8f) ? 0.001f : r.nextFloat() * 0.2f);
        else if (n.contains ("fine") || n.contains ("detune") || n.contains ("tune")) near (0.04f);
        else if (n.contains ("_amt") && n.startsWith ("c")) real (chance (0.5f) ? 0.0f : r.nextFloat() * 1.2f - 0.6f);
        else if (n.startsWith ("joy"))   real (r.nextFloat() - 0.5f);
        else if (n.contains ("level") || n.startsWith ("mix")) real (n.startsWith ("op") ? r.nextFloat() * r.nextFloat() + (n == "op1_level" ? 0.5f : 0.0f) : 0.3f + r.nextFloat() * 0.7f);
        else near (0.35f);
    }
}
}

Snapshot SoundGenerator::generate (uint32 seed, const Settings& set, const AudioProcessorValueTreeState& state)
{
    Random r ((int64) seed);
    Snapshot s;
    std::array<bool, numEngines> on {};
    std::array<float, numMacros> mac {};
    float character = 0.5f;
    int lead = 0;

    switch (set.mode)
    {
        case Mode::classic:
        {
            // una sola arquitectura: el motor elegido en el playfield
            lead = jlimit (0, numEngines - 1, set.selectedEngine);
            on[(size_t) lead] = true;
            for (auto& m : mac) m = centred (r, 0.9f);
            character = r.nextFloat();
            break;
        }
        case Mode::style:
        {
            const auto& st = styles()[(size_t) jlimit (0, (int) styles().size() - 1, set.style)];
            const int count = 1 + r.nextInt (2);
            lead = st.engines[(size_t) r.nextInt ((int) st.engines.size())];
            on[(size_t) lead] = true;
            for (int i = 1; i < count; ++i)
                on[(size_t) st.engines[(size_t) r.nextInt ((int) st.engines.size())]] = true;
            for (int m = 0; m < numMacros; ++m)
                mac[(size_t) m] = inRange (r, st.macro[(size_t) m].first, st.macro[(size_t) m].second);
            character = inRange (r, st.character.first, st.character.second);
            break;
        }
        case Mode::hybrid:
        {
            // entre 2 y 4 motores distintos
            const int count = 2 + r.nextInt (3);
            lead = r.nextInt (numEngines);
            on[(size_t) lead] = true;
            int placed = 1;
            while (placed < count)
            {
                const int e = r.nextInt (numEngines);
                if (! on[(size_t) e]) { on[(size_t) e] = true; ++placed; }
            }
            for (auto& m : mac) m = centred (r, 0.9f);
            character = r.nextFloat();
            break;
        }
    }

    for (int e = 0; e < numEngines; ++e)
    {
        s.push_back ({ ids::engineOn (e), on[(size_t) e] ? 1.0f : 0.0f });
        s.push_back ({ ids::engineLevel (e), e == lead ? inRange (r, 0.75f, 0.95f) : inRange (r, 0.35f, 0.8f) });
    }
    for (int m = 0; m < numMacros; ++m)
        s.push_back ({ ids::macro (m), mac[(size_t) m] });
    s.push_back ({ ids::character, character });

    // arquitecturas con DSP: sus propios parametros (semilla separada para no alterar la mezcla)
    // (siempre, aunque el motor quede apagado: misma semilla = mismo estado completo)
    for (int e = 0; e < numEngines; ++e)
    {
        Random er ((int64) seed * 7919 + 1 + e * 104729);
        if (e == 0) randomizeMinimoog (er, s, state);
        else randomizeEngine (er, s, state, String (engines()[(size_t) e].id) + "_");
    }

    return withoutLocked (std::move (s), set.locks);
}

Snapshot SoundGenerator::mutate (const AudioProcessorValueTreeState& state, uint32 seed, const Settings& set)
{
    Random r ((int64) seed);
    const float amount = jlimit (0.0f, 1.0f, set.mutateAmount);
    Snapshot s;
    int activeCount = 0;

    auto current = [&] (const String& id) { auto* p = state.getParameter (id); return p != nullptr ? p->getValue() : 0.0f; };
    auto nudge   = [&] (float v, float scale) { return jlimit (0.0f, 1.0f, v + (r.nextFloat() * 2.0f - 1.0f) * amount * scale); };

    for (int e = 0; e < numEngines; ++e)
    {
        bool on = current (ids::engineOn (e)) > 0.5f;
        if (r.nextFloat() < amount * 0.25f) on = ! on;
        activeCount += on ? 1 : 0;
        s.push_back ({ ids::engineOn (e), on ? 1.0f : 0.0f });
        s.push_back ({ ids::engineLevel (e), nudge (current (ids::engineLevel (e)), 0.5f) });
    }
    if (activeCount == 0) // nunca dejar el instrumento mudo
        s[(size_t) (r.nextInt (numEngines) * 2)].second = 1.0f;

    for (int m = 0; m < numMacros; ++m)
        s.push_back ({ ids::macro (m), nudge (current (ids::macro (m)), 1.0f) });
    s.push_back ({ ids::character, nudge (current (ids::character), 0.6f) });

    // parametros continuos de cada motor encendido (no cambia formas de onda ni rangos)
    for (int e = 0; e < numEngines; ++e)
    {
        if (current (ids::engineOn (e)) < 0.5f) continue;
        const String prefix = String (engines()[(size_t) e].id) + "_";
        for (auto* p : state.processor.getParameters())
        {
            auto* fp = dynamic_cast<AudioParameterFloat*> (p);
            if (fp == nullptr) continue;
            const auto id = fp->getParameterID();
            if (! id.startsWith (prefix) || id == ids::engineLevel (e)) continue;
            s.push_back ({ id, nudge (static_cast<RangedAudioParameter*> (fp)->getValue(), 0.35f) });
        }
    }

    return withoutLocked (std::move (s), set.locks);
}

void SoundGenerator::apply (AudioProcessorValueTreeState& state, const Snapshot& snapshot)
{
    for (const auto& [id, value] : snapshot)
    {
        if (auto* p = state.getParameter (id))
        {
            if (std::abs (p->getValue() - value) < 1.0e-6f) continue;
            p->beginChangeGesture();
            p->setValueNotifyingHost (value);
            p->endChangeGesture();
        }
    }
}

Snapshot SoundGenerator::withoutLocked (Snapshot s, const std::set<String>& locks)
{
    if (locks.empty()) return s;
    auto isLocked = [&] (const String& id)
    {
        if (id == ids::character) return locks.count ("character") > 0;
        if (id.startsWith ("macro_")) return locks.count ("macros") > 0;
        for (int e = 0; e < numEngines; ++e)
            if (id.startsWith (String (engines()[(size_t) e].id) + "_"))
                return locks.count (engines()[(size_t) e].id) > 0;
        return false;
    };
    s.erase (std::remove_if (s.begin(), s.end(), [&] (const auto& kv) { return isLocked (kv.first); }), s.end());
    return s;
}

bool SoundGenerator::equal (const Snapshot& a, const Snapshot& b)
{
    if (a.size() != b.size()) return false;
    for (size_t i = 0; i < a.size(); ++i)
        if (a[i].first != b[i].first || std::abs (a[i].second - b[i].second) > 1.0e-4f) return false;
    return true;
}

std::set<String> SoundGenerator::parseLocks (const String& text)
{
    std::set<String> out;
    for (auto& t : StringArray::fromTokens (text, ",", "")) if (t.isNotEmpty()) out.insert (t);
    return out;
}

String SoundGenerator::joinLocks (const std::set<String>& locks)
{
    StringArray a; for (auto& l : locks) a.add (l);
    return a.joinIntoString (",");
}
}
