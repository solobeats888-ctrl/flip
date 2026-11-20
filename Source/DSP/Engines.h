#pragma once
#include "Engine.h"

/**
    Los ocho motores que se suman al MINIMOOG. Cada uno declara sus parametros en una tabla
    (Engines*.cpp) y respeta la arquitectura de su sintetizador original.
*/
namespace flipper::dsp
{
#define FLIPPER_ENGINE_COMMON(ClassName)                                                         \
public:                                                                                          \
    explicit ClassName (juce::AudioProcessorValueTreeState&);                                    \
    static void addParameters (juce::AudioProcessorValueTreeState::ParameterLayout&);            \
    void prepare (double sampleRate) override;                                                   \
    void updateParameters() override {}                                                          \
    void noteOn (int slot, float note, float velocity, float glideFromNote) override;            \
    void noteOff (int slot) override;                                                            \
    void kill (int slot) override;                                                               \
    bool isActive (int slot) const override;                                                     \
    void render (int slot, float* out, int numSamples, const BlockMod&) override;                \
private:                                                                                         \
    ParamBank p;                                                                                 \
    double sr = 44100.0;

/** PROPHET-5 (PROFETA): 2 osciladores con formas sumables, sync, poly-mod, LFO, 24 dB, unisono. */
class Prophet5Engine : public Engine
{
    FLIPPER_ENGINE_COMMON (Prophet5Engine)
    void beginBlock (int n) override;
    struct Voice
    {
        float aPh[3] {}, bPh[3] {};
        LadderFilter filter; Envelope amp, fenv; Noise noise;
        float note = 60, target = 60, vel = 1, lastB = 0;
    };
    std::array<Voice, maxVoices> v;
    std::array<float, maxBlock> lfo {};
    float lfoPhase = 0;
};

/** JUPITER-8 (PITER): VCO 1 y 2, cross-mod, sync, HPF + VCF 12/24 dB, 2 envolventes, LFO, zona de teclado. */
class Jupiter8Engine : public Engine
{
    FLIPPER_ENGINE_COMMON (Jupiter8Engine)
    void beginBlock (int n) override;
    struct Voice
    {
        float ph1 = 0, ph2 = 0; LadderFilter ladder; SVF svf; HighPass hpf;
        Envelope e1, e2; Noise noise;
        float note = 60, vel = 1; bool playing = false;
    };
    std::array<Voice, maxVoices> v;
    std::array<float, maxBlock> lfo {};
    float lfoPhase = 0, lfoHold = 0;
    Noise lfoNoise;
};

/** JUNO-60/106 (NO JUNA): un DCO con sierra/pulso/sub/ruido, HPF, VCF 24 dB, una envolvente, LFO con delay, chorus. */
class JunoEngine : public Engine
{
    FLIPPER_ENGINE_COMMON (JunoEngine)
    void beginBlock (int n) override;
    void processBus (float* l, float* r, int n) override;
    double tailSeconds() const override { return 0.05; }
    struct Voice
    {
        float phase = 0, subSign = 1; LadderFilter filter; HighPass hpf; Envelope env, gateEnv; Noise noise;
        float note = 60, vel = 1, lfoAge = 0; bool gate = false;
    };
    std::array<Voice, maxVoices> v;
    std::array<float, maxBlock> lfo {};
    float lfoPhase = 0;
    Chorus chorus;
};

/** CS-80 (ILLUMINARY): dos capas (I y II) con HPF + LPF cada una, ring mod, aftertouch, cinta, portamento/glissando. */
class CS80Engine : public Engine
{
    FLIPPER_ENGINE_COMMON (CS80Engine)
    void aftertouch (int slot, float value) override { v[(size_t) slot].at = value; }
    struct Layer { float ph = 0; SVF hp, lp; Envelope env; };
    struct Voice
    {
        Layer l[2]; Noise noise;
        float note = 60, target = 60, vel = 1, at = 0, atSmooth = 0, ringPhase = 0;
    };
    std::array<Voice, maxVoices> v;
};

/** DX7 (TRAP PROD): 6 operadores FM, 32 algoritmos, envolvente por operador, feedback, LFO. */
class DX7Engine : public Engine
{
    FLIPPER_ENGINE_COMMON (DX7Engine)
    void beginBlock (int n) override;
public:
    /** Algoritmos 1..32: moduladores de cada operador y portadores (bits 0..5 = OP1..OP6). */
    struct Algorithm { std::array<juce::uint8, 6> mods; juce::uint8 carriers; int feedbackOp; };
    static const std::array<Algorithm, 32>& algorithms();
private:
    struct Voice
    {
        std::array<float, 6> phase {}, out {};
        std::array<Envelope, 6> env;
        float note = 60, vel = 1, fb1 = 0, fb2 = 0;
    };
    std::array<Voice, maxVoices> v;
    std::array<float, maxBlock> lfo {};
    float lfoPhase = 0;
};

/** M1 (ARTURITO): dos fuentes PCM propias (generadas por sintesis), filtro digital, envolventes, LFO y efectos. */
class M1Engine : public Engine
{
    FLIPPER_ENGINE_COMMON (M1Engine)
    void beginBlock (int n) override;
    void processBus (float* l, float* r, int n) override;
    double tailSeconds() const override { return 3.0; }
public:
    struct Multisample { std::vector<float> data; int loopStart = 0; bool loop = false; float rootHz = 261.63f; float rate = 32000.0f; };
    static const std::vector<Multisample>& multisamples();
    static const juce::StringArray& sampleNames();
private:
    struct Voice
    {
        double pos[2] {}; SVF filter; Envelope amp, fenv;
        float note = 60, vel = 1;
    };
    std::array<Voice, maxVoices> v;
    std::array<float, maxBlock> lfo {};
    float lfoPhase = 0;
    Chorus chorus; StereoDelay delay;
};

/** ARP 2600 (CALAVERA): 3 VCO (VCO 3 tambien como LFO), ruido, ring mod, filtro multimodo, ADSR + AR, S&H y patch bay. */
class Arp2600Engine : public Engine
{
    FLIPPER_ENGINE_COMMON (Arp2600Engine)
public:
    static const juce::StringArray& cableSources();
    static const juce::StringArray& cableDestinations();
private:
    struct Voice
    {
        Oscillator o1, o2, o3; LadderFilter ladder; SVF svf; Envelope adsr, ar; Noise noise;
        float note = 60, target = 60, vel = 1, sh = 0, shClock = 0, pink = 0;
    };
    std::array<Voice, maxVoices> v;
};

/** VCS3 (EL QUE LA MATA): 3 osciladores, ruido, ring, filtro con autooscilacion, trapezoide, matriz de pines, joystick y resorte. */
class VCS3Engine : public Engine
{
    FLIPPER_ENGINE_COMMON (VCS3Engine)
    void processBus (float* l, float* r, int n) override;
    double tailSeconds() const override { return 2.5; }
public:
    static constexpr int pinRows = 8, pinCols = 7;
    static const juce::StringArray& pinSources();
    static const juce::StringArray& pinDestinations();
    static juce::String pinId (int row, int col);
private:
    enum TrapStage { tIdle, tAttack, tOn, tDecay, tOff };
    struct Voice
    {
        Oscillator o1, o2, o3; LadderFilter filter; Noise noise;
        float s1ph = 0, note = 60, vel = 1, trap = 0, gateAmp = 0, lp = 0, hold = 0;
        float src[8] {};
        TrapStage stage = tIdle; bool gate = false;
    };
    std::array<Voice, maxVoices> v;
    std::array<std::array<std::atomic<float>*, 7>, 8> pins {};
    SpringReverb spring;
    float dcL = 0, dcR = 0;
};

#undef FLIPPER_ENGINE_COMMON
}
