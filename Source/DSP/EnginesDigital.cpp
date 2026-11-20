// DX7 y M1
#include "Engines.h"
#include <deque>
#include <string>

namespace flipper::dsp
{
using namespace juce;

//============================================================================== DX7
namespace dx7
{
enum { opParams = 9 };   // por operador: coarse, fine, detune, level, vel, attack, decay, sustain, release
enum { cCoarse, cFine, cDetune, cLevel, cVel, cA, cD, cS, cR };
enum { algorithm = opParams * 6, feedback, lfoRate, lfoPitch, lfoAmp, count };

const std::vector<ParamDef>& defs()
{
    static std::deque<std::string> names;   // memoria estable para los ids (deque no mueve sus elementos)
    static const std::vector<ParamDef> list = []
    {
        // patch inicial: piano electrico (algoritmo 5: tres pares portador / modulador)
        const int   coarse[6] = { 1, 1, 1, 14, 1, 1 };
        const float level[6]  = { 0.95f, 0.5f, 0.8f, 0.3f, 0.5f, 0.3f };
        const float decay[6]  = { 2.2f, 1.4f, 2.0f, 0.35f, 1.6f, 0.8f };
        const float release[6]= { 0.5f, 0.4f, 0.5f, 0.2f, 0.4f, 0.3f };
        std::vector<ParamDef> d;
        for (int o = 1; o <= 6; ++o)
        {
            auto add = [&] (const char* suffix, const char* label, char kind, float mn, float mx, float def)
            {
                names.push_back ("op" + std::to_string (o) + "_" + suffix);
                const char* id = names.back().c_str();
                names.push_back ("OP" + std::to_string (o) + " " + label);
                d.push_back ({ id, names.back().c_str(), kind, mn, mx, def });
            };
            add ("coarse", "Coarse", 'i', 0, 31, (float) coarse[o - 1]);
            add ("fine", "Fine", 'p', 0, 0.99f, 0);
            add ("detune", "Detune", 'n', -7, 7, o == 3 ? 2.0f : 0.0f);
            add ("level", "Level", 'p', 0, 1, level[o - 1]);
            add ("vel", "Velocity", 'p', 0, 1, 0.5f);
            add ("attack", "Attack", 't', 0.001f, 10, 0.002f);
            add ("decay", "Decay", 't', 0.001f, 10, decay[o - 1]);
            add ("sustain", "Sustain", 'p', 0, 1, 0);
            add ("release", "Release", 't', 0.001f, 10, release[o - 1]);
        }
        d.push_back ({ "algorithm", "Algorithm", 'i', 1, 32, 5 });
        d.push_back ({ "feedback", "Feedback", 'p', 0, 1, 0.35f });
        d.push_back ({ "lfo_rate", "LFO Rate", 'h', 0.1f, 30, 5.0f });
        d.push_back ({ "lfo_pitch", "LFO Pitch", 'p', 0, 1, 0 });
        d.push_back ({ "lfo_amp", "LFO Amp", 'p', 0, 1, 0 });
        return d;
    }();
    return list;
}
}

const std::array<DX7Engine::Algorithm, 32>& DX7Engine::algorithms()
{
    // aristas "65" = el operador 6 modula al 5; portadores; operador con feedback
    static const std::array<Algorithm, 32> table = []
    {
        struct Src { const char* edges; const char* carriers; int fb; };
        const Src src[32] = {
            { "65 54 43 21", "13", 6 },  { "65 54 43 21", "13", 2 },  { "65 54 32 21", "14", 6 },  { "65 54 32 21", "14", 4 },
            { "65 43 21", "135", 6 },    { "65 43 21", "135", 5 },    { "65 53 43 21", "13", 6 },  { "65 53 43 21", "13", 4 },
            { "65 53 43 21", "13", 2 },  { "32 21 64 54", "14", 3 },  { "32 21 64 54", "14", 6 },  { "21 63 53 43", "13", 2 },
            { "21 63 53 43", "13", 6 },  { "21 43 64 54", "13", 6 },  { "21 43 64 54", "13", 2 },  { "21 31 43 51 65", "1", 6 },
            { "21 31 43 51 65", "1", 2 },{ "21 31 41 54 65", "1", 3 },{ "32 21 64 65", "145", 6 }, { "31 32 64 54", "124", 3 },
            { "31 32 64 65", "1245", 3 },{ "21 63 64 65", "1345", 6 },{ "32 64 65", "1245", 6 },   { "63 64 65", "12345", 6 },
            { "64 65", "12345", 6 },     { "32 54 64", "124", 6 },    { "32 54 64", "124", 3 },    { "21 54 43", "136", 5 },
            { "43 65", "1235", 6 },      { "54 43", "1236", 5 },      { "65", "12345", 6 },        { "", "123456", 6 },
        };
        std::array<Algorithm, 32> t {};
        for (int a = 0; a < 32; ++a)
        {
            for (const char* e = src[a].edges; *e != 0; ++e)
                if (*e != ' ') { const int from = e[0] - '1', to = e[1] - '1'; t[(size_t) a].mods[(size_t) to] |= (uint8) (1 << from); ++e; }
            for (const char* c = src[a].carriers; *c != 0; ++c) t[(size_t) a].carriers |= (uint8) (1 << (*c - '1'));
            t[(size_t) a].feedbackOp = src[a].fb - 1;
        }
        return t;
    }();
    return table;
}

DX7Engine::DX7Engine (AudioProcessorValueTreeState& s) { p.bind (s, "dx7", dx7::defs().data(), (int) dx7::defs().size()); }
void DX7Engine::addParameters (AudioProcessorValueTreeState::ParameterLayout& l) { addParams (l, "dx7", dx7::defs().data(), (int) dx7::defs().size()); }
void DX7Engine::prepare (double rate) { sr = rate; for (auto& x : v) for (auto& e : x.env) e.kill(); }

bool DX7Engine::isActive (int s) const
{
    const auto& alg = algorithms()[(size_t) jlimit (0, 31, p.choice (dx7::algorithm) - 1)];
    for (int o = 0; o < 6; ++o)
        if ((alg.carriers >> o) & 1)
            if (v[(size_t) s].env[(size_t) o].isActive()) return true;
    return false;
}

void DX7Engine::noteOff (int s) { for (auto& e : v[(size_t) s].env) e.noteOff(); }
void DX7Engine::kill (int s) { for (auto& e : v[(size_t) s].env) e.kill(); }

void DX7Engine::noteOn (int s, float note, float vel, float)
{
    auto& x = v[(size_t) s];
    x.note = note; x.vel = vel;
    x.phase.fill (0.0f); x.fb1 = x.fb2 = 0;   // el DX7 reinicia los osciladores en cada nota
    for (auto& e : x.env) e.noteOn();
}

void DX7Engine::beginBlock (int n)
{
    const float inc = p[dx7::lfoRate] / (float) sr;
    for (int i = 0; i < n; ++i) { lfo[(size_t) i] = 1.0f - 4.0f * std::abs (lfoPhase - 0.5f); lfoPhase += inc; if (lfoPhase >= 1) lfoPhase -= 1; }
}

void DX7Engine::render (int s, float* out, int n, const BlockMod& m)
{
    using namespace dx7;
    auto& x = v[(size_t) s];
    if (! isActive (s)) return;
    const auto& alg = algorithms()[(size_t) jlimit (0, 31, p.choice (algorithm) - 1)];
    const float fs = (float) sr;

    float ratio[6], amp[6], A[6], D[6], S[6], R[6];
    int carriers = 0;
    for (int o = 0; o < 6; ++o)
    {
        const int b = o * opParams;
        const int c = p.choice (b + cCoarse);
        ratio[o] = (c == 0 ? 0.5f : (float) c) * (1.0f + p[b + cFine]) * (1.0f + p[b + cDetune] * 0.0007f);
        const float lvl = p[b + cLevel];
        const bool isCarrier = (alg.carriers >> o) & 1;
        amp[o] = (isCarrier ? lvl * lvl : lvl * lvl * 1.6f) * (1.0f - p[b + cVel] * (1.0f - x.vel));
        if (! isCarrier) amp[o] *= std::exp2 (m.brightnessOct * 0.5f) * (1.0f + m.texture * 0.6f);   // BRILLO / TEXTURA = indice FM
        A[o] = p[b + cA] * m.attackMul; D[o] = p[b + cD]; S[o] = p[b + cS]; R[o] = p[b + cR] * m.releaseMul;
        carriers += isCarrier ? 1 : 0;
    }
    const float outGain = 0.5f / std::sqrt ((float) jmax (1, carriers));
    const float fbAmt = p[feedback] * 0.45f;
    const int fbOp = alg.feedbackOp;

    for (int i = 0; i < n; ++i)
    {
        const float l = lfo[(size_t) i];
        const float gl = m.lfo ? m.lfo[i] : 0.0f;
        const float hz = midiToHz (x.note + m.bendSemis + l * p[lfoPitch] * 1.5f + gl * (m.modWheel * 0.6f + m.movement * 0.12f));
        float sum = 0.0f;
        for (int o = 5; o >= 0; --o)   // los moduladores siempre tienen numero mayor
        {
            float mod = 0.0f;
            const uint8 mods = alg.mods[(size_t) o];
            for (int k = o + 1; k < 6; ++k) if ((mods >> k) & 1) mod += x.out[(size_t) k];
            if (o == fbOp) mod += (x.fb1 + x.fb2) * 0.5f * fbAmt;
            const float e = x.env[(size_t) o].process (A[o], D[o], S[o], R[o], fs);
            const float y = fastSin (x.phase[(size_t) o] + mod) * e * amp[o];
            x.phase[(size_t) o] += hz * ratio[o] / fs;
            if (x.phase[(size_t) o] >= 1.0f) x.phase[(size_t) o] -= std::floor (x.phase[(size_t) o]);
            x.out[(size_t) o] = y;
            if (o == fbOp) { x.fb2 = x.fb1; x.fb1 = y; }
            if ((alg.carriers >> o) & 1) sum += y;
        }
        out[i] += sum * outGain * (1.0f - p[lfoAmp] * (0.5f + 0.5f * l)) * (0.55f + 0.45f * x.vel);
    }
}

//============================================================================== M1
namespace m1
{
enum { w1, lv1, co1, fi1, w2, lv2, co2, fi2, cutoff, fenv, velFilter, fA, fD, fS, fR, aA, aD, aS, aR,
       lfoRate, lfoPitch, lfoFilter, fxType, fxMix, delayTime, count };
#define M1_WAVES "PIANO|E.PIANO|ÓRGANO|CUERDAS|CORO|CAMPANA|BAJO|METALES|MARIMBA|PAD UNIVERSO|GUITARRA"
const ParamDef defs[] = {
    { "s1_wave", "Source 1", 'c', 0, 10, 0, M1_WAVES }, { "s1_level", "Source 1 Level", 'p', 0, 1, 0.85f },
    { "s1_coarse", "Source 1 Coarse", 'i', -24, 24, 0 }, { "s1_fine", "Source 1 Fine", 'n', -50, 50, 0 },
    { "s2_wave", "Source 2", 'c', 0, 10, 9, M1_WAVES }, { "s2_level", "Source 2 Level", 'p', 0, 1, 0.35f },
    { "s2_coarse", "Source 2 Coarse", 'i', -24, 24, 0 }, { "s2_fine", "Source 2 Fine", 'n', -50, 50, 7 },
    { "cutoff", "Cutoff", 'h', 60, 18000, 9000 }, { "fenv", "Filter Env", 'p', 0, 1, 0.25f }, { "vel_filter", "Velocity Filter", 'p', 0, 1, 0.4f },
    { "f_attack", "Filter Attack", 't', 0.001f, 10, 0.005f }, { "f_decay", "Filter Decay", 't', 0.001f, 10, 1.0f },
    { "f_sustain", "Filter Sustain", 'p', 0, 1, 0.5f }, { "f_release", "Filter Release", 't', 0.001f, 10, 0.8f },
    { "a_attack", "Amp Attack", 't', 0.001f, 10, 0.003f }, { "a_decay", "Amp Decay", 't', 0.001f, 10, 2.0f },
    { "a_sustain", "Amp Sustain", 'p', 0, 1, 0.8f }, { "a_release", "Amp Release", 't', 0.001f, 10, 0.8f },
    { "lfo_rate", "LFO Rate", 'h', 0.1f, 20, 5 }, { "lfo_pitch", "LFO Pitch", 'p', 0, 1, 0 }, { "lfo_filter", "LFO Filter", 'p', 0, 1, 0 },
    { "fx_type", "Effect", 'c', 0, 3, 1, "NINGUNO|CHORUS|DELAY|CHORUS+DELAY" }, { "fx_mix", "Effect Mix", 'p', 0, 1, 0.35f },
    { "delay_time", "Delay Time", 't', 0.05f, 1, 0.375f },
};
#undef M1_WAVES
static_assert (sizeof (defs) / sizeof (defs[0]) == count);
}

const StringArray& M1Engine::sampleNames()
{
    static const StringArray n = StringArray::fromTokens (String::fromUTF8 (m1::defs[m1::w1].opts), "|", "");
    return n;
}

/**
    Multimuestras PROPIAS, generadas por sintesis aditiva al cargar (32 kHz, raiz Do 4).
    No se usa ningun sample del M1 original.
*/
const std::vector<M1Engine::Multisample>& M1Engine::multisamples()
{
    static const std::vector<Multisample> list = []
    {
        constexpr float rate = 32000.0f, f0 = 261.63f;
        std::vector<Multisample> out;
        Random rnd (1988);

        auto make = [&] (float seconds, bool loop, float loopStartSec) -> Multisample&
        {
            out.emplace_back();
            auto& m = out.back();
            m.data.assign ((size_t) (seconds * rate), 0.0f);
            m.loop = loop; m.loopStart = (int) (loopStartSec * rate); m.rootHz = f0; m.rate = rate;
            return m;
        };
        // parcial: frecuencia, amplitud, decaimiento (1/s); en loops la frecuencia se redondea a 1 Hz (loop de 1 s sin saltos)
        auto partial = [&] (Multisample& m, float hz, float a, float decay, bool roundToLoop)
        {
            if (hz >= rate * 0.45f || a <= 0.0f) return;
            if (roundToLoop) hz = std::round (hz);
            double ph = rnd.nextFloat();
            const double inc = hz / rate;
            const float k = std::exp (-decay / rate);
            float env = a;
            for (auto& s : m.data) { s += fastSin ((float) ph) * env; ph += inc; if (ph >= 1.0) ph -= 1.0; env *= k; }
        };
        auto normalise = [] (Multisample& m)
        {
            float peak = 0.001f; for (float s : m.data) peak = jmax (peak, std::abs (s));
            for (auto& s : m.data) s *= 0.9f / peak;
            // ataque sin click
            for (int i = 0; i < 32 && i < (int) m.data.size(); ++i) m.data[(size_t) i] *= (float) i / 32.0f;
        };
        auto ensembleSaw = [&] (Multisample& m, float base, int voices, float spreadHz, int harmonics, float tilt, float gain)
        {
            for (int vv = 0; vv < voices; ++vv)
            {
                const float hz = base + spreadHz * ((float) vv - (voices - 1) * 0.5f);
                for (int h = 1; h <= harmonics; ++h)
                    partial (m, hz * (float) h, gain / std::pow ((float) h, tilt), 0.0f, true);
            }
        };

        { auto& m = make (2.6f, false, 0); // PIANO
          for (int h = 1; h <= 18; ++h) partial (m, f0 * h * std::sqrt (1.0f + 0.0004f * h * h), 1.0f / std::pow ((float) h, 1.1f), 1.1f + 0.38f * h, false);
          for (int i = 0; i < 160; ++i) m.data[(size_t) i] += rnd.nextFloat() * 0.25f * (1.0f - i / 160.0f);
          normalise (m); }
        { auto& m = make (3.0f, false, 0); // E.PIANO (piano de tines por modulacion de fase)
          for (size_t i = 0; i < m.data.size(); ++i)
          { const float t = (float) i / rate;
            m.data[i] = fastSin (f0 * t + 0.28f * std::exp (-t * 2.5f) * fastSin (f0 * t)) * std::exp (-t * 0.9f)
                      + 0.15f * fastSin (14.0f * f0 * t) * std::exp (-t * 12.0f); }
          normalise (m); }
        { auto& m = make (1.25f, true, 0.25f); // ORGANO (registros 16' 8' 5 1/3' 4' 2 2/3' 2')
          const float h[] = { 1, 2, 3, 4, 6, 8 }, a[] = { 1, 0.8f, 0.6f, 0.5f, 0.3f, 0.25f };
          for (int i = 0; i < 6; ++i) partial (m, f0 * h[i], a[i], 0, true);
          for (int i = 0; i < 90; ++i) m.data[(size_t) i] += rnd.nextFloat() * 0.3f * (1.0f - i / 90.0f);
          normalise (m); }
        { auto& m = make (1.25f, true, 0.25f); ensembleSaw (m, f0, 4, 1.3f, 40, 1.15f, 1.0f); normalise (m); } // CUERDAS
        { auto& m = make (1.25f, true, 0.25f); // CORO (formantes de "A")
          for (int vv = 0; vv < 3; ++vv)
            for (int h = 1; h <= 30; ++h)
            { const float hz = (f0 + (vv - 1) * 1.0f) * h;
              auto g = [hz] (float c, float bw) { return std::exp (-0.5f * std::pow ((hz - c) / bw, 2.0f)); };
              partial (m, hz, 0.15f + g (700, 130) + 0.7f * g (1220, 150) + 0.3f * g (2600, 260), 0, true); }
          normalise (m); }
        { auto& m = make (4.0f, false, 0); // CAMPANA (parciales inarmonicos)
          const float r[] = { 0.56f, 0.92f, 1.19f, 1.71f, 2.0f, 2.74f, 3.0f, 3.76f, 4.07f };
          const float a[] = { 1, 0.67f, 1, 1.8f, 2.67f, 1.67f, 1.46f, 1.33f, 1.33f };
          for (int i = 0; i < 9; ++i) partial (m, f0 * 2.0f * r[i], a[i] / 3.0f, 0.5f + i * 0.45f, false);
          normalise (m); }
        { auto& m = make (2.0f, false, 0); // BAJO
          for (int h = 1; h <= 20; ++h) partial (m, f0 * 0.5f * h, 1.0f / h, 0.8f + 0.55f * h, false);
          normalise (m); }
        { auto& m = make (1.25f, true, 0.25f); ensembleSaw (m, f0, 2, 0.9f, 32, 0.85f, 1.0f); normalise (m); } // METALES
        { auto& m = make (1.6f, false, 0); // MARIMBA
          partial (m, f0, 1.0f, 3.0f, false); partial (m, f0 * 3.93f, 0.4f, 9, false); partial (m, f0 * 9.2f, 0.15f, 20, false);
          normalise (m); }
        { auto& m = make (1.25f, true, 0.25f); // PAD UNIVERSO (ensamble + octava + quinta)
          ensembleSaw (m, f0, 5, 0.8f, 24, 1.45f, 1.0f);
          ensembleSaw (m, f0 * 2.0f, 2, 1.0f, 10, 1.6f, 0.35f);
          ensembleSaw (m, f0 * 1.5f, 2, 1.0f, 10, 1.6f, 0.2f);
          normalise (m); }
        { auto& m = make (2.2f, false, 0); // GUITARRA (pulsada al 20 % de la cuerda)
          for (int h = 1; h <= 26; ++h) partial (m, f0 * h, std::abs (std::sin (h * 0.6283f)) / h, 0.7f + 0.6f * h, false);
          normalise (m); }
        return out;
    }();
    return list;
}

M1Engine::M1Engine (AudioProcessorValueTreeState& s) { p.bind (s, "m1", m1::defs, m1::count); multisamples(); }
void M1Engine::addParameters (AudioProcessorValueTreeState::ParameterLayout& l) { addParams (l, "m1", m1::defs, m1::count); }
void M1Engine::prepare (double rate) { sr = rate; chorus.prepare (rate); delay.prepare (rate); for (auto& x : v) { x.amp.kill(); x.fenv.kill(); } }
bool M1Engine::isActive (int s) const { return v[(size_t) s].amp.isActive(); }
void M1Engine::noteOff (int s) { v[(size_t) s].amp.noteOff(); v[(size_t) s].fenv.noteOff(); }
void M1Engine::kill (int s) { v[(size_t) s].amp.kill(); v[(size_t) s].fenv.kill(); }

void M1Engine::noteOn (int s, float note, float vel, float)
{
    auto& x = v[(size_t) s];
    x.note = note; x.vel = vel; x.pos[0] = x.pos[1] = 0.0; x.filter.reset();
    x.amp.noteOn(); x.fenv.noteOn();
}

void M1Engine::beginBlock (int n)
{
    const float inc = p[m1::lfoRate] / (float) sr;
    for (int i = 0; i < n; ++i) { lfo[(size_t) i] = fastSin (lfoPhase); lfoPhase += inc; if (lfoPhase >= 1) lfoPhase -= 1; }
}

void M1Engine::render (int s, float* out, int n, const BlockMod& m)
{
    using namespace m1;
    auto& x = v[(size_t) s];
    if (! x.amp.isActive()) return;
    const auto& ms = multisamples();
    const float fs = (float) sr;
    const Multisample* src[2] = { &ms[(size_t) jlimit (0, (int) ms.size() - 1, p.choice (w1))], &ms[(size_t) jlimit (0, (int) ms.size() - 1, p.choice (w2))] };
    const float level[2] = { p[lv1], p[lv2] };
    const float offset[2] = { (float) p.choice (co1) + p[fi1] / 100.0f, (float) p.choice (co2) + p[fi2] / 100.0f };
    const float velCut = 1.0f - p[velFilter] * (1.0f - x.vel) * 2.0f;

    for (int i = 0; i < n; ++i)
    {
        const float l = lfo[(size_t) i];
        const float gl = m.lfo ? m.lfo[i] : 0.0f;
        const float pitch = x.note - 60.0f + m.bendSemis + l * p[lfoPitch] * 1.0f + gl * (m.modWheel * 0.6f + m.movement * 0.12f);
        float y = 0.0f;
        for (int k = 0; k < 2; ++k)
        {
            if (level[k] <= 0.001f) continue;
            const auto& smp = *src[k];
            const int size = (int) smp.data.size();
            double& pos = x.pos[k];
            if (pos >= size - 1) { if (! smp.loop) continue; pos -= (double) (size - 1 - smp.loopStart); }
            const int ip = (int) pos;
            const float f = (float) (pos - ip);
            y += (smp.data[(size_t) ip] + (smp.data[(size_t) ip + 1] - smp.data[(size_t) ip]) * f) * level[k];
            pos += std::exp2 ((pitch + offset[k]) / 12.0f) * smp.rate / fs;
        }
        const float fe = x.fenv.process (p[fA] * m.attackMul, p[fD], p[fS], p[fR] * m.releaseMul, fs);
        const float am = x.amp.process (p[aA] * m.attackMul, p[aD], p[aS], p[aR] * m.releaseMul, fs);
        const float cut = p[cutoff] * std::exp2 (m.brightnessOct + p[fenv] * fe * 5.0f + (velCut - 1.0f) * 2.0f
                                                 + l * p[lfoFilter] * 2.0f + gl * m.movement * 0.6f);
        x.filter.process (y + m.texture * 0.05f * (float) (((int) (x.pos[0] * 7919.0)) % 3 - 1), cut, 0.3f, fs); // filtro digital sin resonancia
        out[i] += x.filter.lp * am * (0.55f + 0.45f * x.vel) * 0.5f;
    }
}

void M1Engine::processBus (float* l, float* r, int n)
{
    const int t = p.choice (m1::fxType);
    const float mix = p[m1::fxMix];
    if (t == 1 || t == 3) chorus.process (l, r, n, 1);
    if (t == 2 || t == 3) delay.process (l, r, n, mix, p[m1::delayTime]);
    ignoreUnused (mix);
}
}
