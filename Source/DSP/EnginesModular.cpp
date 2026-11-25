// ARP 2600 y VCS3
#include "Engines.h"
#include "../Core/Modulation.h"

namespace flipper::dsp
{
using namespace juce;

namespace
{
inline float saw (float ph, float dt)            { return 2.0f * ph - 1.0f - polyBlep (ph, dt); }
inline float pulse (float ph, float dt, float pw) { return (ph < pw ? 1.0f : -1.0f) + polyBlep (ph, dt) - polyBlep (std::fmod (ph + 1.0f - pw, 1.0f), dt); }
inline float tri (float ph)                       { return 1.0f - 4.0f * std::abs (ph - 0.5f); }
inline bool  advance (float& ph, float dt)        { ph += dt; if (ph >= 1.0f) { ph -= 1.0f; return true; } return false; }
}

//============================================================================== ARP 2600
namespace arp
{
enum { w1, lv1, w2, tune2, pw2, lv2, w3, tune3, lf3, lfRate3, lv3, noise, noiseColor, ring,
       mode, cutoff, res, fenv, kbd, a, d, s, r, arA, arR, shRate, porta,
       c1s, c1d, c1a, c2s, c2d, c2a, c3s, c3d, c3a, c4s, c4d, c4a, count };
#define ARP_SRC "ADSR|AR|S&H|VCO 3|RUIDO|TECLADO"
#define ARP_DST "VCO 1|VCO 2|VCO 3|FILTRO|PWM VCO 2|VCA"
const ParamDef defs[] = {
    { "vco1_wave", "VCO1 Wave", 'c', 0, 1, 0, "SIERRA|CUADRADA" }, { "vco1_level", "VCO1 Level", 'p', 0, 1, 0.7f },
    { "vco2_wave", "VCO2 Wave", 'c', 0, 4, 0, "SIERRA|CUADRADA|TRIÁNGULO|SENO|PULSO" }, { "vco2_tune", "VCO2 Tune", 's', -24, 24, 0.12f },
    { "vco2_pw", "VCO2 PW", 'p', 0.05f, 0.95f, 0.3f }, { "vco2_level", "VCO2 Level", 'p', 0, 1, 0.5f },
    { "vco3_wave", "VCO3 Wave", 'c', 0, 1, 1, "SIERRA|PULSO" }, { "vco3_tune", "VCO3 Tune", 's', -24, 24, -12 },
    { "vco3_lf", "VCO3 LF", 'b', 0, 1, 0 }, { "vco3_lfrate", "VCO3 LF Rate", 'h', 0.05f, 30, 5 }, { "vco3_level", "VCO3 Level", 'p', 0, 1, 0 },
    { "noise", "Noise", 'p', 0, 1, 0 }, { "noise_color", "Noise Color", 'c', 0, 1, 0, "BLANCO|ROSA" }, { "ring", "Ring Mod", 'p', 0, 1, 0 },
    { "filter_mode", "Filter Mode", 'c', 0, 2, 0, "PASABAJOS|PASABANDA|PASAALTOS" },
    { "cutoff", "Cutoff", 'h', 20, 18000, 1400 }, { "res", "Resonance", 'p', 0, 1, 0.3f }, { "fenv", "ADSR to Filter", 'p', 0, 1, 0.45f },
    { "kbd", "Keyboard", 'p', 0, 1, 0.5f },
    { "attack", "Attack", 't', 0.001f, 10, 0.005f }, { "decay", "Decay", 't', 0.001f, 10, 0.5f },
    { "sustain", "Sustain", 'p', 0, 1, 0.6f }, { "release", "Release", 't', 0.001f, 10, 0.4f },
    { "ar_attack", "AR Attack", 't', 0.001f, 10, 0.2f }, { "ar_release", "AR Release", 't', 0.001f, 10, 0.6f },
    { "sh_rate", "S&H Rate", 'h', 0.3f, 40, 8 }, { "porta", "Portamento", 't', 0.001f, 3, 0.001f },
    { "c1_src", "Cable 1 Source", 'c', 0, 5, 2, ARP_SRC }, { "c1_dst", "Cable 1 Dest", 'c', 0, 5, 3, ARP_DST }, { "c1_amt", "Cable 1 Amount", 'n', -1, 1, 0 },
    { "c2_src", "Cable 2 Source", 'c', 0, 5, 3, ARP_SRC }, { "c2_dst", "Cable 2 Dest", 'c', 0, 5, 0, ARP_DST }, { "c2_amt", "Cable 2 Amount", 'n', -1, 1, 0 },
    { "c3_src", "Cable 3 Source", 'c', 0, 5, 1, ARP_SRC }, { "c3_dst", "Cable 3 Dest", 'c', 0, 5, 3, ARP_DST }, { "c3_amt", "Cable 3 Amount", 'n', -1, 1, 0 },
    { "c4_src", "Cable 4 Source", 'c', 0, 5, 5, ARP_SRC }, { "c4_dst", "Cable 4 Dest", 'c', 0, 5, 4, ARP_DST }, { "c4_amt", "Cable 4 Amount", 'n', -1, 1, 0 },
};
static_assert (sizeof (defs) / sizeof (defs[0]) == count);
}

const StringArray& Arp2600Engine::cableSources()      { static const StringArray s = StringArray::fromTokens (ARP_SRC, "|", ""); return s; }
const StringArray& Arp2600Engine::cableDestinations() { static const StringArray s = StringArray::fromTokens (ARP_DST, "|", ""); return s; }
#undef ARP_SRC
#undef ARP_DST

Arp2600Engine::Arp2600Engine (AudioProcessorValueTreeState& s) { p.bind (s, "arp2600", arp::defs, arp::count); }
void Arp2600Engine::addParameters (AudioProcessorValueTreeState::ParameterLayout& l) { addParams (l, "arp2600", arp::defs, arp::count); }
void Arp2600Engine::prepare (double rate) { sr = rate; for (auto& x : v) { x.adsr.kill(); x.ar.kill(); x.ladder.reset(); x.svf.reset(); } }
bool Arp2600Engine::isActive (int s) const { return v[(size_t) s].adsr.isActive(); }
void Arp2600Engine::noteOff (int s) { v[(size_t) s].adsr.noteOff(); v[(size_t) s].ar.noteOff(); }
void Arp2600Engine::kill (int s) { v[(size_t) s].adsr.kill(); v[(size_t) s].ar.kill(); }

void Arp2600Engine::noteOn (int s, float note, float vel, float from)
{
    auto& x = v[(size_t) s];
    if (! x.adsr.isActive()) { x.o1.reset (x.noise.next() * 0.5f + 0.5f); x.o2.reset (x.noise.next() * 0.5f + 0.5f); x.ladder.reset(); x.svf.reset(); }
    x.target = note; x.note = (p[arp::porta] > 0.01f && from >= 0) ? from : note; x.vel = vel;
    x.adsr.noteOn(); x.ar.noteOn();
}

void Arp2600Engine::render (int s, float* out, int n, const BlockMod& m)
{
    using namespace arp;
    auto& x = v[(size_t) s];
    if (! x.adsr.isActive()) return;
    const float fs = (float) sr;
    const float gc = p[porta] > 0.005f ? 1.0f - std::exp (-1.0f / (p[porta] * fs * 0.3f)) : 1.0f;
    const bool lf = p.on (lf3);
    const int fmode = p.choice (mode);
    const int wave2 = p.choice (w2);
    struct Cable { int src, dst; float amt; } cables[4];
    for (int c = 0; c < 4; ++c) cables[c] = { p.choice (c1s + c * 3), p.choice (c1d + c * 3), p[c1a + c * 3] };
    const float shInc = p[shRate] / fs;
    const float pinkCoeff = OnePole::coeffFor (900.0f, fs);

    for (int i = 0; i < n; ++i)
    {
        x.note += (x.target - x.note) * gc;
        const float adsr = x.adsr.process (p[a] * m.attackMul, p[d], p[s], p[r] * m.releaseMul, fs);
        const float ar = x.ar.process (p[arA] * m.attackMul, 0.001f, 1.0f, p[arR] * m.releaseMul, fs);
        const float wn = x.noise.next();
        x.pink += (wn - x.pink) * pinkCoeff;
        const float nz = p.choice (noiseColor) == 1 ? x.pink * 3.0f : wn;
        x.shClock += shInc;
        if (x.shClock >= 1.0f) { x.shClock -= 1.0f; x.sh = x.noise.next(); }

        // VCO 3 primero: tambien es fuente de modulacion
        const float base = x.note + m.bendSemis + (m.lfo ? m.lfo[i] : 0.0f) * (m.modWheel * 0.6f + m.movement * 0.12f);
        float modV[6] {}; // destinos: VCO1, VCO2, VCO3, FILTRO, PWM, VCA
        const float srcPrev[6] = { adsr, ar, x.sh, x.o3.phase < 0.5f ? 1.0f : -1.0f, nz, (x.note - 60.0f) / 24.0f };
        for (auto& c : cables) if (c.amt != 0.0f) modV[c.dst] += srcPrev[c.src] * c.amt;

        const float f3 = lf ? p[lfRate3] : midiToHz (base + p[tune3] + modV[2] * 24.0f);
        const float o3 = x.o3.process (f3, fs, p.choice (w3) == 0 ? Oscillator::saw : Oscillator::widePulse);
        const float o1 = x.o1.process (midiToHz (base + modV[0] * 24.0f), fs, p.choice (w1) == 0 ? Oscillator::saw : Oscillator::square);
        const float pw = jlimit (0.05f, 0.95f, p[pw2] + modV[4] * 0.45f);
        const float f2 = midiToHz (base + p[tune2] + modV[1] * 24.0f);
        float o2;
        switch (wave2)
        {
            case 0: o2 = x.o2.process (f2, fs, Oscillator::saw); break;
            case 1: o2 = x.o2.process (f2, fs, Oscillator::square); break;
            case 2: o2 = x.o2.process (f2, fs, Oscillator::triangle); break;
            case 3: o2 = fastSin (x.o2.phase); x.o2.phase += f2 / fs; if (x.o2.phase >= 1) x.o2.phase -= 1; break;
            default: { const float dt = jlimit (0.0f, 0.45f, f2 / fs); o2 = pulse (x.o2.phase, dt, pw); advance (x.o2.phase, dt); break; }
        }

        float in = o1 * p[lv1] + o2 * p[lv2] + (lf ? 0.0f : o3 * p[lv3]) + nz * (p[noise] + m.texture * 0.1f) + o1 * o2 * p[ring];
        in *= 0.4f;
        const float cut = p[cutoff] * std::exp2 (m.brightnessOct + p[fenv] * adsr * 5.0f + p[kbd] * (x.note - 60.0f) / 12.0f
                                                 + modV[3] * 5.0f + (m.lfo ? m.lfo[i] : 0.0f) * m.movement * 0.6f);
        float y;
        if (fmode == 0) y = x.ladder.process (in, cut, p[res], fs, 1.5f);
        else { x.svf.process (in, cut, p[res], fs); y = fmode == 1 ? x.svf.bp * 1.6f : x.svf.hp; }
        const float vca = jmax (0.0f, adsr * (1.0f + modV[5]));
        out[i] += y * vca * (0.6f + 0.4f * x.vel) * 1.25f;
    }
}

//============================================================================== VCS3
namespace vcs3
{
enum { t1, shape1, sine1, saw1, t2, pw2, sq2, tri2, lf3, rate3, t3, sq3, tri3, noise, noiseColor, ring,
       cutoff, res, eA, eOn, eD, eOff, eTrig, eVca, eFilter, reverb, joyX, joyY, dPitch, dFilter, dVca, count };
const ParamDef defs[] = {
    { "osc1_tune", "Osc1 Tune", 's', -24, 24, 0 }, { "osc1_shape", "Osc1 Shape", 'p', 0.05f, 0.95f, 0.5f },
    { "osc1_sine", "Osc1 Sine", 'p', 0, 1, 0.7f }, { "osc1_saw", "Osc1 Saw", 'p', 0, 1, 0.35f },
    { "osc2_tune", "Osc2 Tune", 's', -24, 24, 7.03f }, { "osc2_pw", "Osc2 PW", 'p', 0.05f, 0.95f, 0.5f },
    { "osc2_square", "Osc2 Square", 'p', 0, 1, 0.45f }, { "osc2_tri", "Osc2 Triangle", 'p', 0, 1, 0.2f },
    { "osc3_lf", "Osc3 LF", 'b', 0, 1, 1 }, { "osc3_rate", "Osc3 LF Rate", 'h', 0.05f, 40, 0.35f }, { "osc3_tune", "Osc3 Tune", 's', -24, 24, -12 },
    { "osc3_square", "Osc3 Square", 'p', 0, 1, 0.5f }, { "osc3_tri", "Osc3 Triangle", 'p', 0, 1, 0.5f },
    { "noise", "Noise", 'p', 0, 1, 0.5f }, { "noise_color", "Noise Color", 'p', 0, 1, 0.5f }, { "ring", "Ring Mod", 'p', 0, 1, 0.6f },
    { "cutoff", "Cutoff", 'h', 20, 18000, 1200 }, { "res", "Response", 'p', 0, 1, 0.35f },
    { "env_attack", "Env Attack", 't', 0.001f, 10, 0.01f }, { "env_on", "Env On", 't', 0.001f, 10, 0.25f },
    { "env_decay", "Env Decay", 't', 0.001f, 10, 0.7f }, { "env_off", "Env Off", 't', 0.001f, 10, 0.2f },
    { "env_trig", "Env Trigger", 'c', 0, 1, 0, "TECLA|REPETIR" }, { "env_vca", "Env to VCA", 'p', 0, 1, 1 },
    { "env_filter", "Env to Filter", 'p', 0, 1, 0.35f }, { "reverb", "Spring Reverb", 'p', 0, 1, 0.22f },
    { "joy_x", "Joystick X", 'n', -1, 1, 0 }, { "joy_y", "Joystick Y", 'n', -1, 1, 0 },
    { "depth_pitch", "Pitch Depth", 'p', 0, 1, 0.25f }, { "depth_filter", "Filter Depth", 'p', 0, 1, 0.35f }, { "depth_vca", "VCA Depth", 'p', 0, 1, 0.5f },
};
static_assert (sizeof (defs) / sizeof (defs[0]) == count);
// pines encendidos de fabrica: OSC1 y OSC2 al filtro, OSC3 (LFO) y trapezoide al corte, joystick X al corte
const bool defaultPin[8][7] = {
    { 1, 0, 0, 0, 0, 0, 0 }, { 1, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 1, 0 }, { 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 1, 0 }, { 0, 0, 0, 0, 0, 1, 0 }, { 0, 0, 1, 0, 0, 0, 0 },
};
}

const StringArray& VCS3Engine::pinSources()      { static const StringArray s { "OSC 1", "OSC 2", "OSC 3", "RUIDO", "ANILLO", "TRAPEZOIDE", "JOYSTICK X", "JOYSTICK Y" }; return s; }
const StringArray& VCS3Engine::pinDestinations() { static const StringArray s { "FILTRO", "SALIDA", "TONO 1", "TONO 2", "TONO 3", "CORTE", "VCA" }; return s; }
String VCS3Engine::pinId (int row, int col) { return "vcs3_pin_" + String (row) + String (col); }

void VCS3Engine::addParameters (AudioProcessorValueTreeState::ParameterLayout& l)
{
    addParams (l, "vcs3", vcs3::defs, vcs3::count);
    for (int r = 0; r < pinRows; ++r)
        for (int c = 0; c < pinCols; ++c)
            l.add (std::make_unique<AudioParameterBool> (ParameterID { pinId (r, c), 1 },
                                                         "VCS3 Pin " + pinSources()[r] + " > " + pinDestinations()[c], vcs3::defaultPin[r][c]));
}

VCS3Engine::VCS3Engine (AudioProcessorValueTreeState& s)
{
    p.bind (s, "vcs3", vcs3::defs, vcs3::count);
    for (int r = 0; r < pinRows; ++r)
        for (int c = 0; c < pinCols; ++c)
            pins[(size_t) r][(size_t) c] = ModMatrix::raw (s, pinId (r, c));
}

void VCS3Engine::prepare (double rate) { sr = rate; spring.prepare (rate); for (auto& x : v) { x.stage = tIdle; x.trap = 0; x.gateAmp = 0; x.filter.reset(); } }
bool VCS3Engine::isActive (int s) const { const auto& x = v[(size_t) s]; return x.stage != tIdle || x.gateAmp > 1.0e-4f; }
void VCS3Engine::kill (int s) { auto& x = v[(size_t) s]; x.stage = tIdle; x.trap = 0; x.gateAmp = 0; x.gate = false; }

void VCS3Engine::noteOn (int s, float note, float vel, float)
{
    auto& x = v[(size_t) s];
    if (! isActive (s)) { x.filter.reset(); for (auto& z : x.src) z = 0; }
    x.note = note; x.vel = vel; x.gate = true; x.stage = tAttack; x.hold = 0;
}

void VCS3Engine::noteOff (int s)
{
    auto& x = v[(size_t) s];
    x.gate = false;
    if (x.stage == tAttack || x.stage == tOn || x.stage == tOff) x.stage = tDecay;
}

void VCS3Engine::render (int s, float* out, int n, const BlockMod& m)
{
    using namespace vcs3;
    auto& x = v[(size_t) s];
    if (! isActive (s)) return;
    const float fs = (float) sr, dt = 1.0f / fs;
    bool pin[8][7];
    for (int r = 0; r < 8; ++r) for (int c = 0; c < 7; ++c) pin[r][c] = pins[(size_t) r][(size_t) c]->load() > 0.5f;
    const bool repeat = p.choice (eTrig) == 1, lf = p.on (lf3);
    const float gateCoeff = OnePole::coeffFor (60.0f, fs);
    const float colourCoeff = OnePole::coeffFor (300.0f + 12000.0f * p[noiseColor], fs);

    for (int i = 0; i < n; ++i)
    {
        // trapezoide: ataque -> encendido -> caida -> apagado (y repite si REPETIR)
        switch (x.stage)
        {
            case tAttack: x.trap += dt / jmax (0.001f, p[eA] * m.attackMul); if (x.trap >= 1) { x.trap = 1; x.stage = tOn; x.hold = 0; } break;
            case tOn: x.hold += dt; if (x.hold >= p[eOn] && (repeat || ! x.gate)) x.stage = tDecay; break;
            case tDecay:
                x.trap -= dt / jmax (0.001f, p[eD] * m.releaseMul);
                if (x.trap <= 0) { x.trap = 0; x.hold = 0; x.stage = (repeat && x.gate) ? tOff : tIdle; }
                break;
            case tOff: x.hold += dt; if (x.hold >= p[eOff]) x.stage = x.gate ? tAttack : tIdle; break;
            case tIdle: break;
        }
        x.gateAmp += ((x.gate ? 1.0f : 0.0f) - x.gateAmp) * gateCoeff;

        // modulacion desde la matriz (con las fuentes de la muestra anterior)
        float pitchMod[3] {}, cutMod = 0, vcaMod = 0, toFilter = 0, toOut = 0;
        for (int r = 0; r < 8; ++r)
        {
            const float sv = x.src[r];
            if (sv == 0.0f) continue;
            if (pin[r][0]) toFilter += sv;
            if (pin[r][1]) toOut += sv;
            for (int k = 0; k < 3; ++k) if (pin[r][2 + k]) pitchMod[k] += sv;
            if (pin[r][5]) cutMod += sv;
            if (pin[r][6]) vcaMod += sv;
        }
        const float base = x.note + m.bendSemis + (m.lfo ? m.lfo[i] : 0.0f) * (m.modWheel * 0.6f + m.movement * 0.12f);
        const float pd = p[dPitch] * 24.0f;

        // OSC 1: seno con simetria variable + sierra
        const float dt1 = jlimit (0.0f, 0.45f, midiToHz (base + p[t1] + pitchMod[0] * pd) / fs);
        const float sh = p[shape1];
        const float warped = x.s1ph < sh ? 0.5f * x.s1ph / sh : 0.5f + 0.5f * (x.s1ph - sh) / (1.0f - sh);
        const float sine1v = fastSin (warped);
        const float o1 = sine1v * p[sine1] + saw (x.s1ph, dt1) * p[saw1];
        advance (x.s1ph, dt1);
        // OSC 2: pulso + triangulo
        const float dt2 = jlimit (0.0f, 0.45f, midiToHz (base + p[t2] + pitchMod[1] * pd) / fs);
        const float sq2v = pulse (x.o2.phase, dt2, p[pw2]);
        const float o2 = sq2v * p[sq2] + tri (x.o2.phase) * p[tri2];
        advance (x.o2.phase, dt2);
        // OSC 3: cuadrada + triangulo (de baja frecuencia o de audio)
        const float f3 = lf ? p[rate3] * std::exp2 (pitchMod[2] * p[dPitch] * 4.0f) : midiToHz (base + p[t3] + pitchMod[2] * pd);
        const float dt3 = jlimit (0.0f, 0.45f, f3 / fs);
        const float o3 = pulse (x.o3.phase, dt3, 0.5f) * p[sq3] + tri (x.o3.phase) * p[tri3];
        advance (x.o3.phase, dt3);

        x.lp += (x.noise.next() - x.lp) * colourCoeff;
        const float nz = x.lp * (1.5f - p[noiseColor]) * p[noise];
        const float rg = sine1v * sq2v * p[ring];

        x.src[0] = o1; x.src[1] = o2; x.src[2] = o3; x.src[3] = nz; x.src[4] = rg; x.src[5] = x.trap;
        x.src[6] = p[joyX]; x.src[7] = p[joyY];

        const float cut = p[cutoff] * std::exp2 (m.brightnessOct + cutMod * p[dFilter] * 5.0f + x.trap * p[eFilter] * 5.0f
                                                 + (m.lfo ? m.lfo[i] : 0.0f) * m.movement * 0.6f);
        const float filtered = x.filter.process ((toFilter + nz * m.texture * 0.3f) * 0.35f, cut, p[res] * 1.02f, fs, 1.6f);
        const float vca = jmax (0.0f, (p[eVca] * x.trap + (1.0f - p[eVca]) * x.gateAmp) * (1.0f + vcaMod * p[dVca]));
        out[i] += (filtered + toOut * 0.2f) * vca * (0.6f + 0.4f * x.vel) * 0.7f;
    }
}

void VCS3Engine::processBus (float* l, float* r, int n)
{
    // quita continua (el joystick puede entrar como audio) y agrega el resorte
    for (int i = 0; i < n; ++i)
    {
        dcL += (l[i] - dcL) * 0.0005f; l[i] -= dcL;
        dcR += (r[i] - dcR) * 0.0005f; r[i] -= dcR;
    }
    spring.process (l, r, n, p[vcs3::reverb] * 0.9f);
}
}
