#pragma once
#include "Engine.h"

namespace flipper::dsp
{
/**
    MINIMOOG (KROKO) - arquitectura del Minimoog Model D:
    3 osciladores (rango 32'..2', 6 formas de onda, afinacion de OSC 2 y 3), mezclador con ruido,
    filtro escalera de 24 dB con enfasis, contorno y seguimiento de teclado, saturacion,
    envolvente de filtro y de amplitud, y glide.
*/
class MinimoogEngine : public Engine
{
public:
    static void addParameters (juce::AudioProcessorValueTreeState::ParameterLayout&);
    static juce::String pid (const char* name) { return juce::String ("minimoog_") + name; }

    explicit MinimoogEngine (juce::AudioProcessorValueTreeState&);

    void prepare (double sampleRate) override;
    void updateParameters() override;
    void noteOn (int slot, float note, float velocity, float glideFromNote) override;
    void noteOff (int slot) override;
    void kill (int slot) override;
    bool isActive (int slot) const override { return voices[(size_t) slot].amp.isActive(); }
    void render (int slot, float* out, int numSamples, const BlockMod&) override;

private:
    struct Voice
    {
        Oscillator osc[3];
        LadderFilter filter;
        Envelope amp, fenv;
        Noise noise;
        float note = 60.0f, target = 60.0f, velocity = 1.0f;
    };

    struct Params
    {
        int range[3] {}, wave[3] {};
        float detune[3] {}, level[3] {}, noise = 0;
        float cutoff = 1000, emphasis = 0, contour = 0, kbd = 0, drive = 0;
        float fa = 0, fd = 0, fs = 0, fr = 0, aa = 0, ad = 0, as = 1, ar = 0, glide = 0;
    };

    std::array<Voice, maxVoices> voices;
    Params p;
    double sr = 44100.0;

    struct Raw { std::atomic<float>* v = nullptr; float get() const { return v != nullptr ? v->load() : 0.0f; } };
    Raw rRange[3], rWave[3], rDetune[3], rLevel[3], rNoise, rCutoff, rEmphasis, rContour, rKbd, rDrive,
        rFa, rFd, rFs, rFr, rAa, rAd, rAs, rAr, rGlide;
};
}
