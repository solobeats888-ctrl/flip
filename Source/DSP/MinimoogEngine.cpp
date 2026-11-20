#include "MinimoogEngine.h"

namespace flipper::dsp
{
using namespace juce;
using APVTS = AudioProcessorValueTreeState;

static NormalisableRange<float> timeRange (float maxSeconds)
{
    NormalisableRange<float> r (0.001f, maxSeconds);
    r.setSkewForCentre (0.25f);
    return r;
}

static AudioParameterFloatAttributes seconds()
{
    return AudioParameterFloatAttributes().withStringFromValueFunction ([] (float v, int)
    {
        return v < 1.0f ? String (roundToInt (v * 1000.0f)) + " ms" : String (v, 2) + " s";
    });
}

static AudioParameterFloatAttributes percent()
{
    return AudioParameterFloatAttributes().withStringFromValueFunction ([] (float v, int) { return String (roundToInt (v * 100.0f)) + "%"; });
}

void MinimoogEngine::addParameters (APVTS::ParameterLayout& layout)
{
    const StringArray ranges { "32'", "16'", "8'", "4'", "2'" };
    const StringArray waves  { "TRIANGULO", "TRI-SIERRA", "SIERRA", "CUADRADA", "PULSO ANCHO", "PULSO FINO" };
    const int defRange[3] = { 2, 2, 1 };
    const int defWave[3]  = { 2, 2, 2 };
    const float defDetune[3] = { 0.0f, 0.07f, -0.06f };
    const float defLevel[3]  = { 0.85f, 0.65f, 0.45f };

    for (int i = 0; i < 3; ++i)
    {
        const String n (i + 1);
        layout.add (std::make_unique<AudioParameterChoice> (ParameterID { pid (("osc" + n + "_range").toRawUTF8()), 1 },
                                                            "Minimoog Osc " + n + " Range", ranges, defRange[i]));
        layout.add (std::make_unique<AudioParameterChoice> (ParameterID { pid (("osc" + n + "_wave").toRawUTF8()), 1 },
                                                            "Minimoog Osc " + n + " Wave", waves, defWave[i]));
        if (i > 0)
            layout.add (std::make_unique<AudioParameterFloat> (ParameterID { pid (("osc" + n + "_detune").toRawUTF8()), 1 },
                                                               "Minimoog Osc " + n + " Detune", NormalisableRange<float> (-7.0f, 7.0f), defDetune[i],
                                                               AudioParameterFloatAttributes().withStringFromValueFunction ([] (float v, int)
                                                               { return (v >= 0 ? "+" : "") + String (v, 2) + " st"; })));
        layout.add (std::make_unique<AudioParameterFloat> (ParameterID { pid (("mix" + n).toRawUTF8()), 1 },
                                                           "Minimoog Mix Osc " + n, NormalisableRange<float> (0.0f, 1.0f), defLevel[i], percent()));
    }
    layout.add (std::make_unique<AudioParameterFloat> (ParameterID { pid ("noise"), 1 }, "Minimoog Noise",
                                                       NormalisableRange<float> (0.0f, 1.0f), 0.0f, percent()));

    NormalisableRange<float> cut (20.0f, 18000.0f);
    cut.setSkewForCentre (900.0f);
    layout.add (std::make_unique<AudioParameterFloat> (ParameterID { pid ("cutoff"), 1 }, "Minimoog Cutoff", cut, 1100.0f,
                                                       AudioParameterFloatAttributes().withStringFromValueFunction ([] (float v, int)
                                                       { return v < 1000 ? String (roundToInt (v)) + " Hz" : String (v / 1000.0f, 1) + " kHz"; })));
    layout.add (std::make_unique<AudioParameterFloat> (ParameterID { pid ("emphasis"), 1 }, "Minimoog Emphasis", NormalisableRange<float> (0, 1), 0.28f, percent()));
    layout.add (std::make_unique<AudioParameterFloat> (ParameterID { pid ("contour"), 1 }, "Minimoog Contour", NormalisableRange<float> (0, 1), 0.45f, percent()));
    layout.add (std::make_unique<AudioParameterFloat> (ParameterID { pid ("kbd"), 1 }, "Minimoog Keyboard Track", NormalisableRange<float> (0, 1), 0.33f, percent()));
    layout.add (std::make_unique<AudioParameterFloat> (ParameterID { pid ("drive"), 1 }, "Minimoog Overload", NormalisableRange<float> (0, 1), 0.25f, percent()));

    layout.add (std::make_unique<AudioParameterFloat> (ParameterID { pid ("f_attack"), 1 },  "Minimoog Filter Attack",  timeRange (10), 0.004f, seconds()));
    layout.add (std::make_unique<AudioParameterFloat> (ParameterID { pid ("f_decay"), 1 },   "Minimoog Filter Decay",   timeRange (10), 0.35f, seconds()));
    layout.add (std::make_unique<AudioParameterFloat> (ParameterID { pid ("f_sustain"), 1 }, "Minimoog Filter Sustain", NormalisableRange<float> (0, 1), 0.3f, percent()));
    layout.add (std::make_unique<AudioParameterFloat> (ParameterID { pid ("f_release"), 1 }, "Minimoog Filter Release", timeRange (10), 0.3f, seconds()));
    layout.add (std::make_unique<AudioParameterFloat> (ParameterID { pid ("a_attack"), 1 },  "Minimoog Amp Attack",     timeRange (10), 0.003f, seconds()));
    layout.add (std::make_unique<AudioParameterFloat> (ParameterID { pid ("a_decay"), 1 },   "Minimoog Amp Decay",      timeRange (10), 0.3f, seconds()));
    layout.add (std::make_unique<AudioParameterFloat> (ParameterID { pid ("a_sustain"), 1 }, "Minimoog Amp Sustain",    NormalisableRange<float> (0, 1), 0.85f, percent()));
    layout.add (std::make_unique<AudioParameterFloat> (ParameterID { pid ("a_release"), 1 }, "Minimoog Amp Release",    timeRange (10), 0.22f, seconds()));
    layout.add (std::make_unique<AudioParameterFloat> (ParameterID { pid ("glide"), 1 }, "Minimoog Glide", timeRange (3), 0.001f, seconds()));
}

MinimoogEngine::MinimoogEngine (APVTS& state)
{
    auto raw = [&state] (const char* name) { return Raw { state.getRawParameterValue (pid (name)) }; };
    const char* ranges[] = { "osc1_range", "osc2_range", "osc3_range" };
    const char* waves[]  = { "osc1_wave", "osc2_wave", "osc3_wave" };
    const char* detune[] = { "", "osc2_detune", "osc3_detune" };
    const char* mix[]    = { "mix1", "mix2", "mix3" };
    for (int i = 0; i < 3; ++i)
    {
        rRange[i] = raw (ranges[i]); rWave[i] = raw (waves[i]); rLevel[i] = raw (mix[i]);
        if (i > 0) rDetune[i] = raw (detune[i]);
    }
    rNoise = raw ("noise"); rCutoff = raw ("cutoff"); rEmphasis = raw ("emphasis"); rContour = raw ("contour");
    rKbd = raw ("kbd"); rDrive = raw ("drive");
    rFa = raw ("f_attack"); rFd = raw ("f_decay"); rFs = raw ("f_sustain"); rFr = raw ("f_release");
    rAa = raw ("a_attack"); rAd = raw ("a_decay"); rAs = raw ("a_sustain"); rAr = raw ("a_release");
    rGlide = raw ("glide");
}

void MinimoogEngine::prepare (double sampleRate)
{
    sr = sampleRate;
    for (auto& v : voices) { v.amp.kill(); v.fenv.kill(); v.filter.reset(); }
}

void MinimoogEngine::updateParameters()
{
    for (int i = 0; i < 3; ++i)
    {
        p.range[i] = (int) rRange[i].get();
        p.wave[i] = (int) rWave[i].get();
        p.detune[i] = rDetune[i].get();
        p.level[i] = rLevel[i].get();
    }
    p.noise = rNoise.get(); p.cutoff = rCutoff.get(); p.emphasis = rEmphasis.get(); p.contour = rContour.get();
    p.kbd = rKbd.get(); p.drive = rDrive.get();
    p.fa = rFa.get(); p.fd = rFd.get(); p.fs = rFs.get(); p.fr = rFr.get();
    p.aa = rAa.get(); p.ad = rAd.get(); p.as = rAs.get(); p.ar = rAr.get();
    p.glide = rGlide.get();
}

void MinimoogEngine::noteOn (int slot, float note, float velocity, float glideFrom)
{
    auto& v = voices[(size_t) slot];
    const bool wasSilent = ! v.amp.isActive();
    v.target = note;
    v.note = (p.glide > 0.01f && glideFrom >= 0.0f) ? glideFrom : note;
    v.velocity = velocity;
    if (wasSilent)
    {
        // fase libre, como un analogico: cada nota arranca distinta
        for (int i = 0; i < 3; ++i) v.osc[i].reset (v.noise.next() * 0.5f + 0.5f);
        v.filter.reset();
    }
    v.amp.noteOn();
    v.fenv.noteOn();
}

void MinimoogEngine::noteOff (int slot)
{
    voices[(size_t) slot].amp.noteOff();
    voices[(size_t) slot].fenv.noteOff();
}

void MinimoogEngine::kill (int slot)
{
    voices[(size_t) slot].amp.kill();
    voices[(size_t) slot].fenv.kill();
}

void MinimoogEngine::render (int slot, float* out, int n, const BlockMod& m)
{
    auto& v = voices[(size_t) slot];
    if (! v.amp.isActive()) return;

    const float fs = (float) sr;
    const float glideCoeff = p.glide > 0.01f ? 1.0f - std::exp (-1.0f / (p.glide * fs * 0.3f)) : 1.0f;
    const float rangeOffset[] = { -24.0f, -12.0f, 0.0f, 12.0f, 24.0f };
    const float vibDepth = m.modWheel * 0.6f + m.movement * 0.12f;          // semitonos
    const float lfoFilterOct = m.movement * 1.2f;
    const float noiseAmt = p.noise + m.texture * 0.12f;
    const float spread = m.texture * 0.12f;                                   // desafinacion extra
    const float drive = 1.0f + p.drive * 4.0f;
    const float velAmp = 0.55f + 0.45f * v.velocity;
    const float aA = p.aa * m.attackMul, aR = p.ar * m.releaseMul;
    const float fA = p.fa * m.attackMul, fR = p.fr * m.releaseMul;

    for (int i = 0; i < n; ++i)
    {
        v.note += (v.target - v.note) * glideCoeff;
        const float lfo = m.lfo != nullptr ? m.lfo[i] : 0.0f;
        const float base = v.note + m.bendSemis + lfo * vibDepth;

        float x = 0.0f;
        for (int o = 0; o < 3; ++o)
        {
            if (p.level[o] <= 0.0005f) continue;
            const float semis = base + rangeOffset[jlimit (0, 4, p.range[o])] + p.detune[o] + (o == 1 ? spread : (o == 2 ? -spread : 0.0f));
            x += v.osc[o].process (midiToHz (semis), fs, p.wave[o]) * p.level[o];
        }
        x += v.noise.next() * noiseAmt;
        x *= 0.45f;

        const float fenv = v.fenv.process (fA, p.fd, p.fs, fR, fs);
        const float amp  = v.amp.process (aA, p.ad, p.as, aR, fs);

        const float octaves = m.brightnessOct + p.contour * fenv * 6.0f
                            + p.kbd * (v.note - 60.0f) / 12.0f + lfo * lfoFilterOct * 0.5f;
        const float cutoff = p.cutoff * std::exp2 (octaves);
        const float y = v.filter.process (x, cutoff, p.emphasis, fs, drive);

        out[i] += y * amp * velAmp * 0.55f;
    }
}
}
