// PROPHET-5, JUPITER-8, JUNO-60/106 y CS-80
#include "Engines.h"

namespace flipper::dsp
{
using namespace juce;

namespace
{
/** Formas analogicas desde una sola fase (con PolyBLEP en sierra y pulso). */
inline float saw (float ph, float dt)            { return 2.0f * ph - 1.0f - polyBlep (ph, dt); }
inline float pulse (float ph, float dt, float pw) { return (ph < pw ? 1.0f : -1.0f) + polyBlep (ph, dt) - polyBlep (std::fmod (ph + 1.0f - pw, 1.0f), dt); }
inline float tri (float ph)                       { return 1.0f - 4.0f * std::abs (ph - 0.5f); }
inline bool  advance (float& ph, float dt)        { ph += dt; if (ph >= 1.0f) { ph -= 1.0f; return true; } return false; }
inline float dtFor (float note, float sr)         { return jlimit (0.0f, 0.45f, midiToHz (note) / sr); }
inline float glideCoeff (float seconds, float sr) { return seconds > 0.005f ? 1.0f - std::exp (-1.0f / (seconds * sr * 0.3f)) : 1.0f; }
const float feetOffset[] = { -12.0f, 0.0f, 12.0f, 24.0f };
}

//============================================================================== PROPHET-5
namespace prophet
{
enum { aSaw, aPulse, aPw, bSemi, bFine, bSaw, bTri, bPulse, bPw, sync, mixA, mixB, noise,
       cutoff, res, fenvAmt, kbd, pmEnv, pmOscB, pmFreqA, pmFilter, lfoRate, lfoFreq, lfoPw, lfoFilter,
       fA, fD, fS, fR, aA, aD, aS, aR, unison, uniDetune, glide, count };
const ParamDef defs[] = {
    { "a_saw", "Osc A Saw", 'b', 0, 1, 1 }, { "a_pulse", "Osc A Pulse", 'b', 0, 1, 0 }, { "a_pw", "Osc A PW", 'p', 0.05f, 0.95f, 0.5f },
    { "b_semi", "Osc B Semi", 'i', -24, 24, 0 }, { "b_fine", "Osc B Fine", 's', -1, 1, 0.08f },
    { "b_saw", "Osc B Saw", 'b', 0, 1, 1 }, { "b_tri", "Osc B Tri", 'b', 0, 1, 0 }, { "b_pulse", "Osc B Pulse", 'b', 0, 1, 0 },
    { "b_pw", "Osc B PW", 'p', 0.05f, 0.95f, 0.5f }, { "sync", "Sync", 'b', 0, 1, 0 },
    { "mix_a", "Mix A", 'p', 0, 1, 0.8f }, { "mix_b", "Mix B", 'p', 0, 1, 0.6f }, { "noise", "Noise", 'p', 0, 1, 0 },
    { "cutoff", "Cutoff", 'h', 20, 18000, 1500 }, { "res", "Resonance", 'p', 0, 1, 0.2f }, { "fenv", "Filter Env", 'p', 0, 1, 0.45f },
    { "kbd", "Keyboard", 'p', 0, 1, 0.5f }, { "pm_env", "PolyMod Env", 'p', 0, 1, 0 }, { "pm_oscb", "PolyMod Osc B", 'p', 0, 1, 0 },
    { "pm_freqa", "PolyMod Freq A", 'b', 0, 1, 1 }, { "pm_filter", "PolyMod Filter", 'b', 0, 1, 0 },
    { "lfo_rate", "LFO Rate", 'h', 0.05f, 25, 4.5f }, { "lfo_freq", "LFO Freq", 'p', 0, 1, 0 }, { "lfo_pw", "LFO PW", 'p', 0, 1, 0 },
    { "lfo_filter", "LFO Filter", 'p', 0, 1, 0 },
    { "f_attack", "Filter Attack", 't', 0.001f, 10, 0.005f }, { "f_decay", "Filter Decay", 't', 0.001f, 10, 0.6f },
    { "f_sustain", "Filter Sustain", 'p', 0, 1, 0.35f }, { "f_release", "Filter Release", 't', 0.001f, 10, 0.5f },
    { "a_attack", "Amp Attack", 't', 0.001f, 10, 0.004f }, { "a_decay", "Amp Decay", 't', 0.001f, 10, 0.5f },
    { "a_sustain", "Amp Sustain", 'p', 0, 1, 0.8f }, { "a_release", "Amp Release", 't', 0.001f, 10, 0.45f },
    { "unison", "Unison", 'b', 0, 1, 0 }, { "uni_detune", "Unison Detune", 'p', 0, 1, 0.3f }, { "glide", "Glide", 't', 0.001f, 3, 0.001f },
};
static_assert (sizeof (defs) / sizeof (defs[0]) == count);
}

Prophet5Engine::Prophet5Engine (AudioProcessorValueTreeState& s) { p.bind (s, "prophet5", prophet::defs, prophet::count); }
void Prophet5Engine::addParameters (AudioProcessorValueTreeState::ParameterLayout& l) { addParams (l, "prophet5", prophet::defs, prophet::count); }
void Prophet5Engine::prepare (double rate) { sr = rate; for (auto& x : v) { x.amp.kill(); x.fenv.kill(); x.filter.reset(); } }
bool Prophet5Engine::isActive (int s) const { return v[(size_t) s].amp.isActive(); }
void Prophet5Engine::noteOff (int s) { v[(size_t) s].amp.noteOff(); v[(size_t) s].fenv.noteOff(); }
void Prophet5Engine::kill (int s) { v[(size_t) s].amp.kill(); v[(size_t) s].fenv.kill(); }

void Prophet5Engine::noteOn (int s, float note, float vel, float from)
{
    auto& x = v[(size_t) s];
    if (! x.amp.isActive()) { for (int u = 0; u < 3; ++u) { x.aPh[u] = x.noise.next() * 0.5f + 0.5f; x.bPh[u] = x.noise.next() * 0.5f + 0.5f; } x.filter.reset(); }
    x.target = note; x.note = (p[prophet::glide] > 0.01f && from >= 0) ? from : note; x.vel = vel;
    x.amp.noteOn(); x.fenv.noteOn();
}

void Prophet5Engine::beginBlock (int n)
{
    const float inc = p[prophet::lfoRate] / (float) sr;
    for (int i = 0; i < n; ++i) { lfo[(size_t) i] = tri (lfoPhase) ; advance (lfoPhase, inc); }
}

void Prophet5Engine::render (int s, float* out, int n, const BlockMod& m)
{
    using namespace prophet;
    auto& x = v[(size_t) s];
    if (! x.amp.isActive()) return;
    const float fs = (float) sr;
    const int copies = p.on (unison) ? 3 : 1;
    const float gc = glideCoeff (p[glide], fs);
    const float aSawOn = p.on (aSaw) ? 1.0f : 0.0f, aPulseOn = p.on (aPulse) ? 1.0f : 0.0f;
    const float bSawOn = p.on (bSaw) ? 1.0f : 0.0f, bTriOn = p.on (bTri) ? 1.0f : 0.0f, bPulseOn = p.on (bPulse) ? 1.0f : 0.0f;
    const bool doSync = p.on (sync), toFreq = p.on (pmFreqA), toFilter = p.on (pmFilter);
    const float bOffset = (float) p.choice (bSemi) + p[bFine];
    const float velAmp = 0.6f + 0.4f * x.vel;

    for (int i = 0; i < n; ++i)
    {
        x.note += (x.target - x.note) * gc;
        const float gl = m.lfo ? m.lfo[i] : 0.0f;
        const float base = x.note + m.bendSemis + gl * (m.modWheel * 0.6f + m.movement * 0.12f) + lfo[(size_t) i] * p[lfoFreq] * 2.0f;
        const float fe = x.fenv.process (p[fA] * m.attackMul, p[fD], p[fS], p[fR] * m.releaseMul, fs);
        const float am = x.amp.process (p[aA] * m.attackMul, p[aD], p[aS], p[aR] * m.releaseMul, fs);
        const float pmA = toFreq ? (fe * p[pmEnv] * 24.0f + x.lastB * p[pmOscB] * 24.0f) : 0.0f;
        const float pwA = jlimit (0.05f, 0.95f, p[aPw] + lfo[(size_t) i] * p[lfoPw] * 0.4f);
        const float pwB = jlimit (0.05f, 0.95f, p[bPw] + lfo[(size_t) i] * p[lfoPw] * 0.4f);

        float sum = 0.0f, bSum = 0.0f;
        for (int u = 0; u < copies; ++u)
        {
            const float det = copies == 1 ? 0.0f : ((float) u - 1.0f) * p[uniDetune] * 0.3f;
            const float dtB = dtFor (base + bOffset + det, fs);
            const float b = bSawOn * saw (x.bPh[u], dtB) + bTriOn * tri (x.bPh[u]) + bPulseOn * pulse (x.bPh[u], dtB, pwB);
            const bool wrapped = advance (x.bPh[u], dtB);
            const float dtA = dtFor (base + pmA + det * 0.7f, fs);
            const float a = aSawOn * saw (x.aPh[u], dtA) + aPulseOn * pulse (x.aPh[u], dtA, pwA);
            advance (x.aPh[u], dtA);
            if (doSync && wrapped) x.aPh[u] = 0.0f;
            sum += a * p[mixA] + b * p[mixB];
            bSum += b;
        }
        x.lastB = bSum / (float) copies;
        float in = sum / std::sqrt ((float) copies) + x.noise.next() * (p[noise] + m.texture * 0.1f);
        in *= 0.42f;

        const float oct = m.brightnessOct + p[fenvAmt] * fe * 6.0f + p[kbd] * (x.note - 60.0f) / 12.0f
                        + lfo[(size_t) i] * p[lfoFilter] * 2.0f + gl * m.movement * 0.6f
                        + (toFilter ? fe * p[pmEnv] * 4.0f + x.lastB * p[pmOscB] * 3.0f : 0.0f);
        out[i] += x.filter.process (in, p[cutoff] * std::exp2 (oct), p[res], fs, 1.4f) * am * velAmp * 0.6f;
    }
}

//============================================================================== JUPITER-8
namespace jupiter
{
enum { w1, r1, w2, r2, fine2, cross, sync, pw, lfoPwm, mix, hpf, cutoff, res, slope, envAmt, kbd,
       lfoRate, lfoWave, lfoVco, lfoVcf, a1, d1, s1, rl1, a2, d2, s2, rl2, zone, split, count };
const ParamDef defs[] = {
    { "vco1_wave", "VCO1 Wave", 'c', 0, 3, 1, "TRI|SIERRA|PULSO|CUADRADA" }, { "vco1_range", "VCO1 Range", 'c', 0, 3, 1, "16'|8'|4'|2'" },
    { "vco2_wave", "VCO2 Wave", 'c', 0, 3, 1, "SENO|SIERRA|PULSO|RUIDO" }, { "vco2_range", "VCO2 Range", 'c', 0, 3, 1, "16'|8'|4'|2'" },
    { "vco2_fine", "VCO2 Fine", 's', -1, 1, 0.1f }, { "cross", "Cross Mod", 'p', 0, 1, 0 }, { "sync", "Sync", 'b', 0, 1, 0 },
    { "pw", "Pulse Width", 'p', 0.05f, 0.95f, 0.5f }, { "lfo_pwm", "LFO PWM", 'p', 0, 1, 0 }, { "mix", "VCO Mix", 'p', 0, 1, 0.5f },
    { "hpf", "HPF", 'h', 20, 2000, 20 }, { "cutoff", "Cutoff", 'h', 20, 18000, 2200 }, { "res", "Resonance", 'p', 0, 1, 0.15f },
    { "slope", "Slope", 'c', 0, 1, 1, "12 DB|24 DB" }, { "env_amt", "Env Amount", 'p', 0, 1, 0.45f }, { "kbd", "Keyboard", 'p', 0, 1, 0.5f },
    { "lfo_rate", "LFO Rate", 'h', 0.05f, 25, 5 }, { "lfo_wave", "LFO Wave", 'c', 0, 4, 0, "SENO|TRI|SIERRA|CUADRADA|AZAR" },
    { "lfo_vco", "LFO VCO", 'p', 0, 1, 0 }, { "lfo_vcf", "LFO VCF", 'p', 0, 1, 0 },
    { "e1_attack", "Env1 Attack", 't', 0.001f, 10, 0.01f }, { "e1_decay", "Env1 Decay", 't', 0.001f, 10, 0.8f },
    { "e1_sustain", "Env1 Sustain", 'p', 0, 1, 0.4f }, { "e1_release", "Env1 Release", 't', 0.001f, 10, 0.6f },
    { "e2_attack", "Env2 Attack", 't', 0.001f, 10, 0.01f }, { "e2_decay", "Env2 Decay", 't', 0.001f, 10, 0.6f },
    { "e2_sustain", "Env2 Sustain", 'p', 0, 1, 0.85f }, { "e2_release", "Env2 Release", 't', 0.001f, 10, 0.7f },
    { "zone", "Key Zone", 'c', 0, 2, 0, "TODO|GRAVES|AGUDOS" }, { "split", "Split Point", 'i', 36, 84, 60 },
};
static_assert (sizeof (defs) / sizeof (defs[0]) == count);
}

Jupiter8Engine::Jupiter8Engine (AudioProcessorValueTreeState& s) { p.bind (s, "jupiter8", jupiter::defs, jupiter::count); }
void Jupiter8Engine::addParameters (AudioProcessorValueTreeState::ParameterLayout& l) { addParams (l, "jupiter8", jupiter::defs, jupiter::count); }
void Jupiter8Engine::prepare (double rate) { sr = rate; for (auto& x : v) { x.e1.kill(); x.e2.kill(); x.ladder.reset(); x.svf.reset(); } }
bool Jupiter8Engine::isActive (int s) const { return v[(size_t) s].e2.isActive(); }
void Jupiter8Engine::noteOff (int s) { v[(size_t) s].e1.noteOff(); v[(size_t) s].e2.noteOff(); }
void Jupiter8Engine::kill (int s) { v[(size_t) s].e1.kill(); v[(size_t) s].e2.kill(); }

void Jupiter8Engine::noteOn (int s, float note, float vel, float)
{
    using namespace jupiter;
    const int z = p.choice (zone), sp = p.choice (split);
    if ((z == 1 && note >= (float) sp) || (z == 2 && note < (float) sp)) return; // fuera de la zona del teclado
    auto& x = v[(size_t) s];
    if (! x.e2.isActive()) { x.ph1 = x.noise.next() * 0.5f + 0.5f; x.ph2 = x.noise.next() * 0.5f + 0.5f; x.ladder.reset(); x.svf.reset(); }
    x.note = note; x.vel = vel;
    x.e1.noteOn(); x.e2.noteOn();
}

void Jupiter8Engine::beginBlock (int n)
{
    const float inc = p[jupiter::lfoRate] / (float) sr;
    const int wave = p.choice (jupiter::lfoWave);
    for (int i = 0; i < n; ++i)
    {
        float y = 0;
        switch (wave)
        {
            case 0: y = fastSin (lfoPhase); break;
            case 1: y = tri (lfoPhase); break;
            case 2: y = 2.0f * lfoPhase - 1.0f; break;
            case 3: y = lfoPhase < 0.5f ? 1.0f : -1.0f; break;
            default: y = lfoHold; break;
        }
        lfo[(size_t) i] = y;
        if (advance (lfoPhase, inc)) lfoHold = lfoNoise.next();
    }
}

void Jupiter8Engine::render (int s, float* out, int n, const BlockMod& m)
{
    using namespace jupiter;
    auto& x = v[(size_t) s];
    if (! x.e2.isActive()) return;
    const float fs = (float) sr;
    const int wave1 = p.choice (w1), wave2 = p.choice (w2);
    const float off1 = feetOffset[jlimit (0, 3, p.choice (r1))], off2 = feetOffset[jlimit (0, 3, p.choice (r2))] + p[fine2];
    const bool doSync = p.on (sync), use24 = p.choice (slope) == 1;
    const float velAmp = 0.6f + 0.4f * x.vel;

    for (int i = 0; i < n; ++i)
    {
        const float l = lfo[(size_t) i];
        const float gl = m.lfo ? m.lfo[i] : 0.0f;
        const float base = x.note + m.bendSemis + l * p[lfoVco] * 2.0f + gl * (m.modWheel * 0.6f + m.movement * 0.12f);
        const float e1 = x.e1.process (p[a1] * m.attackMul, p[d1], p[s1], p[rl1] * m.releaseMul, fs);
        const float e2 = x.e2.process (p[a2] * m.attackMul, p[d2], p[s2], p[rl2] * m.releaseMul, fs);
        const float pwv = jlimit (0.05f, 0.95f, p[pw] + l * p[lfoPwm] * 0.4f);

        // VCO 2 primero: modula al VCO 1 (cross mod)
        const float dt2 = dtFor (base + off2, fs);
        float o2 = 0;
        switch (wave2)
        {
            case 0: o2 = fastSin (x.ph2); break;
            case 1: o2 = saw (x.ph2, dt2); break;
            case 2: o2 = pulse (x.ph2, dt2, pwv); break;
            default: o2 = x.noise.next(); break;
        }
        advance (x.ph2, dt2);

        const float dt1 = jlimit (0.0f, 0.45f, dtFor (base + off1, fs) * std::exp2 (o2 * p[cross] * 3.0f));
        float o1 = 0;
        switch (wave1)
        {
            case 0: o1 = tri (x.ph1); break;
            case 1: o1 = saw (x.ph1, dt1); break;
            case 2: o1 = pulse (x.ph1, dt1, pwv); break;
            default: o1 = pulse (x.ph1, dt1, 0.5f); break;
        }
        if (advance (x.ph1, dt1) && doSync) x.ph2 = 0.0f; // VCO 2 sincronizado al VCO 1

        float in = (o1 * (1.0f - p[mix]) + o2 * p[mix]) * 0.6f + x.noise.next() * m.texture * 0.1f;
        in = x.hpf.process (in, p[hpf], fs);

        const float cut = p[cutoff] * std::exp2 (m.brightnessOct + p[envAmt] * e1 * 6.0f + p[kbd] * (x.note - 60.0f) / 12.0f
                                                 + l * p[lfoVcf] * 2.0f + gl * m.movement * 0.6f);
        float y;
        if (use24) y = x.ladder.process (in, cut, p[res], fs, 1.3f);
        else { x.svf.process (in, cut, p[res] * 0.95f, fs); y = x.svf.lp; }
        out[i] += y * e2 * velAmp * 0.6f;
    }
}

//============================================================================== JUNO-60/106
namespace juno
{
enum { sawOn, pulseOn, pw, pwmSrc, sub, noise, range, hpf, cutoff, res, envAmt, lfoVcf, kbd,
       lfoRate, lfoDelay, lfoDco, vcaMode, a, d, s, r, chorus, count };
const ParamDef defs[] = {
    { "saw", "Saw", 'b', 0, 1, 1 }, { "pulse", "Pulse", 'b', 0, 1, 1 }, { "pw", "Pulse Width", 'p', 0, 1, 0.35f },
    { "pwm_src", "PWM Source", 'c', 0, 1, 1, "MANUAL|LFO" }, { "sub", "Sub Osc", 'p', 0, 1, 0.45f }, { "noise", "Noise", 'p', 0, 1, 0 },
    { "range", "Range", 'c', 0, 2, 1, "16'|8'|4'" }, { "hpf", "HPF", 'c', 0, 3, 1, "0|1|2|3" },
    { "cutoff", "Cutoff", 'h', 20, 18000, 2600 }, { "res", "Resonance", 'p', 0, 1, 0.15f }, { "env_amt", "Env Amount", 'p', 0, 1, 0.3f },
    { "lfo_vcf", "LFO VCF", 'p', 0, 1, 0 }, { "kbd", "Keyboard", 'p', 0, 1, 0.5f },
    { "lfo_rate", "LFO Rate", 'h', 0.1f, 20, 2.5f }, { "lfo_delay", "LFO Delay", 't', 0.001f, 4, 0.001f }, { "lfo_dco", "LFO DCO", 'p', 0, 1, 0 },
    { "vca_mode", "VCA Mode", 'c', 0, 1, 0, "ENV|GATE" },
    { "attack", "Attack", 't', 0.001f, 10, 0.01f }, { "decay", "Decay", 't', 0.001f, 10, 0.9f },
    { "sustain", "Sustain", 'p', 0, 1, 0.7f }, { "release", "Release", 't', 0.001f, 10, 0.7f },
    { "chorus", "Chorus", 'c', 0, 3, 1, "OFF|I|II|I+II" },
};
static_assert (sizeof (defs) / sizeof (defs[0]) == count);
const float hpfHz[] = { 5.0f, 225.0f, 700.0f, 1600.0f };
}

JunoEngine::JunoEngine (AudioProcessorValueTreeState& s) { p.bind (s, "juno", juno::defs, juno::count); }
void JunoEngine::addParameters (AudioProcessorValueTreeState::ParameterLayout& l) { addParams (l, "juno", juno::defs, juno::count); }
void JunoEngine::prepare (double rate) { sr = rate; chorus.prepare (rate); for (auto& x : v) { x.env.kill(); x.gateEnv.kill(); x.filter.reset(); } }
bool JunoEngine::isActive (int s) const { return v[(size_t) s].env.isActive() || v[(size_t) s].gateEnv.isActive(); }
void JunoEngine::noteOff (int s) { v[(size_t) s].env.noteOff(); v[(size_t) s].gateEnv.noteOff(); }
void JunoEngine::kill (int s) { v[(size_t) s].env.kill(); v[(size_t) s].gateEnv.kill(); }

void JunoEngine::noteOn (int s, float note, float vel, float)
{
    auto& x = v[(size_t) s];
    if (! isActive (s)) { x.filter.reset(); x.lfoAge = 0; } // DCO: la fase sigue (sin click)
    x.note = note; x.vel = vel;
    x.env.noteOn(); x.gateEnv.noteOn();
}

void JunoEngine::beginBlock (int n)
{
    const float inc = p[juno::lfoRate] / (float) sr;
    for (int i = 0; i < n; ++i) { lfo[(size_t) i] = tri (lfoPhase); advance (lfoPhase, inc); }
}

void JunoEngine::render (int s, float* out, int n, const BlockMod& m)
{
    using namespace juno;
    auto& x = v[(size_t) s];
    if (! isActive (s)) return;
    const float fs = (float) sr;
    const float sawAmt = p.on (sawOn) ? 1.0f : 0.0f, pulseAmt = p.on (pulseOn) ? 1.0f : 0.0f;
    const bool pwmLfo = p.choice (pwmSrc) == 1, gateMode = p.choice (vcaMode) == 1;
    const float rangeOff = (float) (p.choice (range) - 1) * 12.0f;
    const float hp = hpfHz[jlimit (0, 3, p.choice (hpf))];
    const float delay = jmax (0.001f, p[lfoDelay]);

    for (int i = 0; i < n; ++i)
    {
        x.lfoAge += 1.0f / fs;
        const float lfoAmt = jmin (1.0f, x.lfoAge / delay);
        const float l = lfo[(size_t) i] * lfoAmt;
        const float gl = m.lfo ? m.lfo[i] : 0.0f;
        const float pitch = x.note + rangeOff + m.bendSemis + l * p[lfoDco] * 1.5f + gl * (m.modWheel * 0.6f + m.movement * 0.12f);
        const float dt = dtFor (pitch, fs);
        const float pwv = pwmLfo ? 0.5f - 0.45f * p[pw] * (0.5f + 0.5f * l) : 0.5f - 0.45f * p[pw];

        float y = sawAmt * saw (x.phase, dt) + pulseAmt * pulse (x.phase, dt, pwv) + x.subSign * p[sub];
        y += x.noise.next() * (p[noise] + m.texture * 0.1f);
        if (advance (x.phase, dt)) x.subSign = -x.subSign;
        y = x.hpf.process (y * 0.45f, hp, fs);

        const float e = x.env.process (p[a] * m.attackMul, p[d], p[s], p[r] * m.releaseMul, fs);
        const float g = x.gateEnv.process (0.002f, 0.01f, 1.0f, 0.012f * m.releaseMul, fs);
        const float cut = p[cutoff] * std::exp2 (m.brightnessOct + p[envAmt] * e * 6.0f + p[kbd] * (x.note - 60.0f) / 12.0f
                                                 + l * p[lfoVcf] * 2.5f + gl * m.movement * 0.6f);
        y = x.filter.process (y, cut, p[res], fs, 1.2f);
        out[i] += y * (gateMode ? g : e) * (0.6f + 0.4f * x.vel) * 0.62f;
    }
    if (! gateMode && ! x.env.isActive()) x.gateEnv.kill();
    if (gateMode && ! x.gateEnv.isActive()) x.env.kill();
}

void JunoEngine::processBus (float* l, float* r, int n)
{
    chorus.process (l, r, n, p.choice (juno::chorus));
}

//============================================================================== CS-80
namespace cs80
{
// por capa (I = 0, II = 1): feet, saw, pulse, pw, noise, hpf, lpf, res, fenv, a, d, s, r
enum { lFeet, lSaw, lPulse, lPw, lNoise, lHpf, lLpf, lRes, lFenv, lA, lD, lS, lR, perLayer };
enum { balance = perLayer * 2, detune, ringDepth, ringSpeed, atBright, atVib, porta, gliss, ribbon, count };
const ParamDef defs[] = {
    { "l1_feet", "I Feet", 'c', 0, 2, 1, "16'|8'|4'" }, { "l1_saw", "I Saw", 'p', 0, 1, 0.8f }, { "l1_pulse", "I Pulse", 'p', 0, 1, 0 },
    { "l1_pw", "I PW", 'p', 0.05f, 0.95f, 0.5f }, { "l1_noise", "I Noise", 'p', 0, 1, 0 }, { "l1_hpf", "I HPF", 'h', 20, 4000, 40 },
    { "l1_lpf", "I LPF", 'h', 20, 18000, 2200 }, { "l1_res", "I Resonance", 'p', 0, 1, 0.25f }, { "l1_fenv", "I Filter Env", 'p', 0, 1, 0.4f },
    { "l1_attack", "I Attack", 't', 0.001f, 10, 0.12f }, { "l1_decay", "I Decay", 't', 0.001f, 10, 1.2f },
    { "l1_sustain", "I Sustain", 'p', 0, 1, 0.7f }, { "l1_release", "I Release", 't', 0.001f, 10, 1.4f },
    { "l2_feet", "II Feet", 'c', 0, 2, 1, "16'|8'|4'" }, { "l2_saw", "II Saw", 'p', 0, 1, 0.3f }, { "l2_pulse", "II Pulse", 'p', 0, 1, 0.6f },
    { "l2_pw", "II PW", 'p', 0.05f, 0.95f, 0.35f }, { "l2_noise", "II Noise", 'p', 0, 1, 0 }, { "l2_hpf", "II HPF", 'h', 20, 4000, 120 },
    { "l2_lpf", "II LPF", 'h', 20, 18000, 3500 }, { "l2_res", "II Resonance", 'p', 0, 1, 0.1f }, { "l2_fenv", "II Filter Env", 'p', 0, 1, 0.3f },
    { "l2_attack", "II Attack", 't', 0.001f, 10, 0.3f }, { "l2_decay", "II Decay", 't', 0.001f, 10, 1.5f },
    { "l2_sustain", "II Sustain", 'p', 0, 1, 0.75f }, { "l2_release", "II Release", 't', 0.001f, 10, 1.8f },
    { "balance", "Balance", 'p', 0, 1, 0.5f }, { "detune", "Detune II", 's', -0.5f, 0.5f, 0.07f },
    { "ring_depth", "Ring Depth", 'p', 0, 1, 0 }, { "ring_speed", "Ring Speed", 'h', 0.3f, 80, 6 },
    { "at_bright", "Aftertouch Brilliance", 'p', 0, 1, 0.5f }, { "at_vib", "Aftertouch Vibrato", 'p', 0, 1, 0.4f },
    { "porta", "Portamento", 't', 0.001f, 3, 0.001f }, { "gliss", "Glissando", 'b', 0, 1, 0 },
    { "ribbon", "Ribbon", 'n', -1, 1, 0 },
};
static_assert (sizeof (defs) / sizeof (defs[0]) == count);
}

CS80Engine::CS80Engine (AudioProcessorValueTreeState& s) { p.bind (s, "cs80", cs80::defs, cs80::count); }
void CS80Engine::addParameters (AudioProcessorValueTreeState::ParameterLayout& l) { addParams (l, "cs80", cs80::defs, cs80::count); }
void CS80Engine::prepare (double rate) { sr = rate; for (auto& x : v) for (auto& L : x.l) { L.env.kill(); L.hp.reset(); L.lp.reset(); } }
bool CS80Engine::isActive (int s) const { return v[(size_t) s].l[0].env.isActive() || v[(size_t) s].l[1].env.isActive(); }
void CS80Engine::noteOff (int s) { for (auto& L : v[(size_t) s].l) L.env.noteOff(); }
void CS80Engine::kill (int s) { for (auto& L : v[(size_t) s].l) L.env.kill(); v[(size_t) s].at = 0; }

void CS80Engine::noteOn (int s, float note, float vel, float from)
{
    auto& x = v[(size_t) s];
    if (! isActive (s)) for (auto& L : x.l) { L.ph = x.noise.next() * 0.5f + 0.5f; L.hp.reset(); L.lp.reset(); }
    x.target = note; x.note = (p[cs80::porta] > 0.01f && from >= 0) ? from : note; x.vel = vel; x.at = 0;
    for (auto& L : x.l) L.env.noteOn();
}

void CS80Engine::render (int s, float* out, int n, const BlockMod& m)
{
    using namespace cs80;
    auto& x = v[(size_t) s];
    if (! isActive (s)) return;
    const float fs = (float) sr;
    const float gc = glideCoeff (p[porta], fs);
    const bool gl = p.on (gliss);
    const float bal = p[balance];
    const float gains[2] = { jmin (1.0f, 2.0f * (1.0f - bal)), jmin (1.0f, 2.0f * bal) };
    const float ringInc = p[ringSpeed] / fs;
    const float atCoeff = OnePole::coeffFor (12.0f, fs);

    for (int i = 0; i < n; ++i)
    {
        x.note += (x.target - x.note) * gc;
        x.atSmooth += (x.at - x.atSmooth) * atCoeff;
        const float pitchNote = gl ? std::round (x.note) : x.note;
        const float vib = (m.lfo ? m.lfo[i] : 0.0f) * (x.atSmooth * p[atVib] * 0.5f + m.modWheel * 0.6f + m.movement * 0.12f);
        const float base = pitchNote + m.bendSemis + p[ribbon] * 12.0f + vib;

        float mixOut = 0.0f;
        for (int L = 0; L < 2; ++L)
        {
            auto& ly = x.l[L];
            const int o = L * perLayer;
            const float e = ly.env.process (p[o + lA] * m.attackMul, p[o + lD], p[o + lS], p[o + lR] * m.releaseMul, fs);
            if (gains[L] <= 0.001f) continue;
            const float pitch = base + feetOffset[jlimit (0, 2, p.choice (o + lFeet))] + (L == 1 ? p[detune] : 0.0f);
            const float dt = dtFor (pitch, fs);
            float y = p[o + lSaw] * saw (ly.ph, dt) + p[o + lPulse] * pulse (ly.ph, dt, p[o + lPw]) + x.noise.next() * p[o + lNoise];
            advance (ly.ph, dt);
            y *= 0.5f;
            ly.hp.process (y, p[o + lHpf], p[o + lRes] * 0.6f, fs);
            const float cut = p[o + lLpf] * std::exp2 (m.brightnessOct + p[o + lFenv] * e * 5.0f + x.atSmooth * p[atBright] * 3.0f
                                                         + (x.note - 60.0f) / 24.0f + (m.lfo ? m.lfo[i] : 0.0f) * m.movement * 0.6f);
            ly.lp.process (ly.hp.hp, cut, p[o + lRes], fs);
            mixOut += ly.lp.lp * e * gains[L];
        }
        // modulador en anillo (CS-80): modula la amplitud con su propio oscilador
        const float ring = 1.0f - p[ringDepth] + p[ringDepth] * fastSin (x.ringPhase);
        advance (x.ringPhase, ringInc);
        out[i] += mixOut * ring * (0.6f + 0.4f * x.vel) * 0.7f;
    }
}
}
