#include "Modulation.h"

namespace flipper
{
using namespace juce;

namespace
{
CriticalSection registryLock;
std::map<const AudioProcessorValueTreeState*, ModMatrix*>& registry()
{
    static std::map<const AudioProcessorValueTreeState*, ModMatrix*> r;
    return r;
}
const Identifier modsProperty { "mods" };
const float syncBeats[] = { 0.0f, 0.25f, 0.5f, 1.0f, 2.0f, 4.0f, 8.0f, 16.0f };
}

//============================================================================== parametros de las fuentes
void ModMatrix::addParameters (AudioProcessorValueTreeState::ParameterLayout& layout)
{
    auto percent = AudioParameterFloatAttributes()
        .withStringFromValueFunction ([] (float v, int) { return String (roundToInt (v * 100.0f)) + "%"; });
    const char* macroIds[] = { "mod_a", "mod_b", "mod_c", "mod_d" };
    const char* macroNames[] = { "Mod A", "Mod B", "Mod C", "Mod D" };
    for (int i = 0; i < 4; ++i)
        layout.add (std::make_unique<AudioParameterFloat> (ParameterID { macroIds[i], 1 }, macroNames[i],
                                                           NormalisableRange<float> (0.0f, 1.0f), 0.0f, percent));
    for (int l = 1; l <= 2; ++l)
    {
        const String n (l);
        NormalisableRange<float> rate (0.05f, 20.0f);
        rate.setSkewForCentre (2.0f);
        layout.add (std::make_unique<AudioParameterFloat> (ParameterID { "lfo" + n + "_rate", 1 }, "LFO " + n + " Rate", rate, l == 1 ? 1.0f : 0.25f,
                                                           AudioParameterFloatAttributes().withStringFromValueFunction ([] (float v, int)
                                                           { return String (v, v < 1.0f ? 2 : 1) + " Hz"; })));
        layout.add (std::make_unique<AudioParameterChoice> (ParameterID { "lfo" + n + "_shape", 1 }, "LFO " + n + " Shape",
                                                            StringArray { "SENO", String::fromUTF8 ("TRIÁNGULO"), "SIERRA", "CUADRADA", "AZAR" }, 0));
        layout.add (std::make_unique<AudioParameterChoice> (ParameterID { "lfo" + n + "_sync", 1 }, "LFO " + n + " Sync",
                                                            StringArray { "LIBRE", "1/16", "1/8", "1/4", "1/2", String::fromUTF8 ("1 COMPÁS"), "2 COMPASES", "4 COMPASES" }, 0));
    }
}

String ModMatrix::sourceName (int s)
{
    static const char* n[] = { "MOD A", "MOD B", "MOD C", "MOD D", "LFO 1", "LFO 2" };
    return n[jlimit (0, numSources - 1, s)];
}

Colour ModMatrix::sourceColour (int s)
{
    static const Colour c[] = { Colour (0xffe8463a), Colour (0xff3fa9f5), Colour (0xff9be05a), Colour (0xffd36bf0),
                                Colour (0xff3fe0c8), Colour (0xffff9d2e) };
    return c[jlimit (0, numSources - 1, s)];
}

String ModMatrix::paramFor (int s)
{
    static const char* n[] = { "mod_a", "mod_b", "mod_c", "mod_d", "lfo1_rate", "lfo2_rate" };
    return n[jlimit (0, numSources - 1, s)];
}

//============================================================================== registro
ModMatrix::ModMatrix (AudioProcessorValueTreeState& s) : state (s)
{
    for (auto* p : s.processor.getParameters())
        if (auto* rp = dynamic_cast<RangedAudioParameter*> (p))
        {
            Param prm;
            prm.param = rp;
            prm.base = s.getRawParameterValue (rp->getParameterID());
            prm.shadow = std::make_unique<std::atomic<float>> (prm.base != nullptr ? prm.base->load() : 0.0f);
            const auto id = rp->getParameterID();
            const bool isSource = id.startsWith ("mod_") || id.startsWith ("lfo1_") || id.startsWith ("lfo2_");
            prm.continuous = ! isSource && dynamic_cast<AudioParameterFloat*> (rp) != nullptr;
            lookup[id] = (int) params.size();
            params.push_back (std::move (prm));
        }
    for (int l = 0; l < 2; ++l)
    {
        const String n (l + 1);
        lfoRate[(size_t) l]  = s.getRawParameterValue ("lfo" + n + "_rate");
        lfoShape[(size_t) l] = s.getRawParameterValue ("lfo" + n + "_shape");
        lfoSync[(size_t) l]  = s.getRawParameterValue ("lfo" + n + "_sync");
    }
    const char* m[] = { "mod_a", "mod_b", "mod_c", "mod_d" };
    for (int i = 0; i < 4; ++i) macroRaw[(size_t) i] = s.getRawParameterValue (m[i]);

    const ScopedLock sl (registryLock);
    registry()[&s] = this;
}

ModMatrix::~ModMatrix()
{
    const ScopedLock sl (registryLock);
    registry().erase (&state);
}

ModMatrix* ModMatrix::forState (const AudioProcessorValueTreeState& s)
{
    const ScopedLock sl (registryLock);
    auto it = registry().find (&s);
    return it != registry().end() ? it->second : nullptr;
}

std::atomic<float>* ModMatrix::raw (AudioProcessorValueTreeState& s, const String& id)
{
    if (auto* m = forState (s))
    {
        const int i = m->indexOf (id);
        if (i >= 0) return m->params[(size_t) i].shadow.get();
    }
    return s.getRawParameterValue (id);
}

int ModMatrix::indexOf (const String& id) const
{
    auto it = lookup.find (id);
    return it != lookup.end() ? it->second : -1;
}

//============================================================================== audio
void ModMatrix::prepare (double sr) { sampleRate = sr; }

void ModMatrix::beginBlock (double newBpm, bool playing, double ppq)
{
    if (newBpm > 20.0 && newBpm < 400.0) bpm = newBpm;

    // LFO al tempo: si el DAW esta tocando, se alinea con la grilla
    for (int l = 0; l < 2; ++l)
    {
        const int sync = jlimit (0, 7, (int) (lfoSync[(size_t) l]->load() + 0.5f));
        if (sync > 0 && playing && ppq >= 0.0)
        {
            const double cyc = ppq / syncBeats[sync];
            const double ph = cyc - std::floor (cyc);
            if (ph < lfoPhase[(size_t) l]) lfoHold[(size_t) l] = rnd.nextFloat() * 2.0f - 1.0f;   // empezo un ciclo nuevo
            lfoPhase[(size_t) l] = ph;
        }
    }

    for (auto& p : params)
        if (p.base != nullptr) p.shadow->store (p.base->load(), std::memory_order_relaxed);

    numActive = 0;
    for (auto& s : slots)
    {
        const int t = s.target.load (std::memory_order_relaxed);
        if (t < 0 || s.source.load (std::memory_order_relaxed) < 0) continue;
        bool dup = false;
        for (int i = 0; i < numActive; ++i) dup |= activeTargets[(size_t) i] == t;
        if (! dup && numActive < maxRoutes) activeTargets[(size_t) numActive++] = t;
    }
    advance (0);
}

void ModMatrix::advance (int n)
{
    // fuentes
    for (int i = 0; i < 4; ++i) srcValue[(size_t) i].store (macroRaw[(size_t) i]->load(), std::memory_order_relaxed);
    for (int l = 0; l < 2; ++l)
    {
        const int sync = jlimit (0, 7, (int) (lfoSync[(size_t) l]->load() + 0.5f));
        const double hz = sync == 0 ? (double) lfoRate[(size_t) l]->load() : bpm / 60.0 / syncBeats[sync];
        double& ph = lfoPhase[(size_t) l];
        ph += hz * n / sampleRate;
        if (ph >= 1.0) { ph -= std::floor (ph); lfoHold[(size_t) l] = rnd.nextFloat() * 2.0f - 1.0f; }
        const float x = (float) ph;
        float y = 0.0f;
        switch ((int) (lfoShape[(size_t) l]->load() + 0.5f))
        {
            case 0:  y = std::sin (x * MathConstants<float>::twoPi); break;
            case 1:  y = 1.0f - 4.0f * std::abs (x - 0.5f); break;
            case 2:  y = 2.0f * x - 1.0f; break;
            case 3:  y = x < 0.5f ? 1.0f : -1.0f; break;
            default: y = lfoHold[(size_t) l]; break;
        }
        srcValue[(size_t) (lfo1 + l)].store (y, std::memory_order_relaxed);
    }

    // parametros modulados
    for (int a = 0; a < numActive; ++a)
    {
        const int t = activeTargets[(size_t) a];
        auto& p = params[(size_t) t];
        float norm = p.param->convertTo0to1 (p.base->load (std::memory_order_relaxed));
        for (auto& s : slots)
        {
            if (s.target.load (std::memory_order_relaxed) != t) continue;
            const int src = s.source.load (std::memory_order_relaxed);
            if (src < 0) continue;
            norm += s.depth.load (std::memory_order_relaxed) * srcValue[(size_t) src].load (std::memory_order_relaxed);
        }
        p.shadow->store (p.param->convertFrom0to1 (jlimit (0.0f, 1.0f, norm)), std::memory_order_relaxed);
    }
}

//============================================================================== rutas
bool ModMatrix::canModulate (const String& id) const
{
    const int i = indexOf (id);
    return i >= 0 && params[(size_t) i].continuous;
}

bool ModMatrix::setRoute (int source, const String& id, float depth)
{
    const int t = indexOf (id);
    if (t < 0 || ! params[(size_t) t].continuous || source < 0 || source >= numSources) return false;
    depth = jlimit (-1.0f, 1.0f, depth);
    for (auto& s : slots)
        if (s.target.load() == t && s.source.load() == source) { s.depth.store (depth); routesEdited(); return true; }
    for (auto& s : slots)
        if (s.target.load() < 0)
        {
            s.depth.store (depth);
            s.source.store (source);
            s.target.store (t);
            routesEdited();
            return true;
        }
    return false;
}

void ModMatrix::removeRoute (int source, const String& id)
{
    const int t = indexOf (id);
    for (auto& s : slots)
        if (s.target.load() == t && s.source.load() == source) { s.target.store (-1); s.source.store (-1); s.depth.store (0.0f); }
    routesEdited();
}

void ModMatrix::clearRoutes()
{
    for (auto& s : slots) { s.target.store (-1); s.source.store (-1); s.depth.store (0.0f); }
    routesEdited();
}

float ModMatrix::depthOf (int source, const String& id) const
{
    const int t = indexOf (id);
    for (auto& s : slots) if (s.target.load() == t && s.source.load() == source) return s.depth.load();
    return 0.0f;
}

std::vector<ModMatrix::Route> ModMatrix::routes() const
{
    std::vector<Route> r;
    for (auto& s : slots)
    {
        const int t = s.target.load(), src = s.source.load();
        if (t >= 0 && src >= 0) r.push_back ({ src, params[(size_t) t].param->getParameterID(), s.depth.load() });
    }
    return r;
}

std::vector<ModMatrix::Route> ModMatrix::routesFor (const String& id) const
{
    std::vector<Route> r;
    for (auto& x : routes()) if (x.param == id) r.push_back (x);
    return r;
}

float ModMatrix::modulatedNormalised (const String& id) const
{
    const int i = indexOf (id);
    if (i < 0) return 0.0f;
    auto& p = params[(size_t) i];
    return p.param->convertTo0to1 (p.shadow->load (std::memory_order_relaxed));
}

void ModMatrix::routesEdited()
{
    storeToState();
    if (onRoutesChanged) onRoutesChanged();
}

String ModMatrix::toString() const
{
    StringArray a;
    for (auto& r : routes()) a.add (String (r.source) + ":" + r.param + ":" + String (r.depth, 4));
    return a.joinIntoString (";");
}

void ModMatrix::fromString (const String& text)
{
    for (auto& s : slots) { s.target.store (-1); s.source.store (-1); s.depth.store (0.0f); }
    size_t k = 0;
    for (auto& item : StringArray::fromTokens (text, ";", ""))
    {
        auto parts = StringArray::fromTokens (item, ":", "");
        if (parts.size() != 3 || k >= slots.size()) continue;
        const int t = indexOf (parts[1]), src = parts[0].getIntValue();
        if (t < 0 || ! params[(size_t) t].continuous || src < 0 || src >= numSources) continue;
        slots[k].depth.store (jlimit (-1.0f, 1.0f, parts[2].getFloatValue()));
        slots[k].source.store (src);
        slots[k].target.store (t);
        ++k;
    }
    storeToState();
    if (onRoutesChanged) onRoutesChanged();
}

void ModMatrix::storeToState() { state.state.setProperty (modsProperty, toString(), nullptr); }
void ModMatrix::loadFromState() { fromString (state.state.getProperty (modsProperty).toString()); }
}
