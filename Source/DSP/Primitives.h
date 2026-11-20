#pragma once
#include <juce_core/juce_core.h>
#include <cmath>
#include <array>
#include <vector>

/** Bloques de DSP compartidos por los motores. Todo sin reservas de memoria. */
namespace flipper::dsp
{
constexpr float twoPi = juce::MathConstants<float>::twoPi;

inline float midiToHz (float note) { return 440.0f * std::exp2 ((note - 69.0f) / 12.0f); }

/** Saturacion suave rapida (aprox. de tanh). */
inline float softClip (float x)
{
    x = juce::jlimit (-3.0f, 3.0f, x);
    return x * (27.0f + x * x) / (27.0f + 9.0f * x * x);
}

/** Ruido blanco barato (xorshift). */
struct Noise
{
    juce::uint32 state = 0x12345678u;
    float next() noexcept
    {
        state ^= state << 13; state ^= state >> 17; state ^= state << 5;
        return (float) (state * (1.0 / 4294967296.0)) * 2.0f - 1.0f;
    }
};

/** Correccion PolyBLEP para discontinuidades (reduce el aliasing de sierra y pulso). */
inline float polyBlep (float t, float dt) noexcept
{
    if (t < dt)        { t /= dt;          return t + t - t * t - 1.0f; }
    if (t > 1.0f - dt) { t = (t - 1.0f) / dt; return t * t + t + t + 1.0f; }
    return 0.0f;
}

/** Oscilador analogico virtual. */
struct Oscillator
{
    enum Wave { triangle = 0, sharktooth, saw, square, widePulse, narrowPulse, numWaves };

    float phase = 0.0f;
    float triState = 0.0f;

    void reset (float startPhase) noexcept { phase = startPhase; triState = 0.0f; }

    float process (float freq, float sampleRate, int wave) noexcept
    {
        const float dt = juce::jlimit (0.0f, 0.45f, freq / sampleRate);
        float out = 0.0f;
        switch (wave)
        {
            case saw:
                out = 2.0f * phase - 1.0f - polyBlep (phase, dt);
                break;
            case square: case widePulse: case narrowPulse:
            {
                const float pw = wave == square ? 0.5f : (wave == widePulse ? 0.3f : 0.12f);
                out = (phase < pw ? 1.0f : -1.0f) + polyBlep (phase, dt) - polyBlep (std::fmod (phase + 1.0f - pw, 1.0f), dt);
                break;
            }
            case triangle:
            {
                // cuadrada integrada: triangulo con poco aliasing
                float sq = (phase < 0.5f ? 1.0f : -1.0f) + polyBlep (phase, dt) - polyBlep (std::fmod (phase + 0.5f, 1.0f), dt);
                triState = dt * 4.0f * sq + (1.0f - dt * 0.2f) * triState;
                out = triState;
                break;
            }
            case sharktooth: // mezcla triangulo / sierra, como la "triangle-saw" del Minimoog
            {
                const float s = 2.0f * phase - 1.0f - polyBlep (phase, dt);
                float sq = (phase < 0.5f ? 1.0f : -1.0f) + polyBlep (phase, dt) - polyBlep (std::fmod (phase + 0.5f, 1.0f), dt);
                triState = dt * 4.0f * sq + (1.0f - dt * 0.2f) * triState;
                out = 0.55f * s + 0.6f * triState;
                break;
            }
            default: break;
        }
        phase += dt;
        if (phase >= 1.0f) phase -= 1.0f;
        return out;
    }
};

/**
    Filtro escalera de 4 polos (24 dB/oct) de "retardo cero" (TPT), con saturacion en la
    entrada de la realimentacion como en un Moog. Estable con resonancia alta.
*/
struct LadderFilter
{
    float s[4] {};

    void reset() noexcept { for (auto& v : s) v = 0.0f; }

    float process (float x, float cutoffHz, float resonance, float sampleRate, float drive = 1.0f) noexcept
    {
        cutoffHz = juce::jlimit (20.0f, sampleRate * 0.45f, cutoffHz);
        const float g = std::tan (juce::MathConstants<float>::pi * cutoffHz / sampleRate);
        const float G = g / (1.0f + g);
        const float k = juce::jlimit (0.0f, 1.0f, resonance) * 3.95f;

        const float inv = 1.0f / (1.0f + g);
        const float S = (G * G * G * s[0] + G * G * s[1] + G * s[2] + s[3]) * inv;
        const float G4 = G * G * G * G;

        // compensacion de volumen en graves al subir la resonancia
        float u = (x * (1.0f + k * 0.45f) - k * S) / (1.0f + k * G4);
        u = softClip (u * drive) / juce::jmax (1.0f, drive * 0.7f);

        float y = u;
        for (auto& st : s)
        {
            const float v = (y - st) * G;
            const float out = v + st;
            st = out + v;
            y = out;
        }
        return y;
    }
};

/** Filtro de un polo para suavizar y para ecualizar. */
struct OnePole
{
    float z = 0.0f;
    float lowpass (float x, float coeff) noexcept { z += coeff * (x - z); return z; }
    static float coeffFor (float hz, float sampleRate) { return 1.0f - std::exp (-twoPi * hz / sampleRate); }
};

/** Envolvente ADSR exponencial (curvas analogicas). Tiempos en segundos. */
struct Envelope
{
    enum Stage { idle, attack, decay, sustain, release };
    Stage stage = idle;
    float value = 0.0f;

    void noteOn() noexcept  { stage = attack; }
    void noteOff() noexcept { if (stage != idle) stage = release; }
    void kill() noexcept    { stage = idle; value = 0.0f; }
    bool isActive() const noexcept { return stage != idle; }

    float process (float a, float d, float s, float r, float sampleRate) noexcept
    {
        auto rate = [sampleRate] (float seconds) { return 1.0f - std::exp (-1.0f / (juce::jmax (0.0005f, seconds) * sampleRate * 0.25f)); };
        switch (stage)
        {
            case attack:
                value += (1.25f - value) * rate (a);
                if (value >= 1.0f) { value = 1.0f; stage = decay; }
                break;
            case decay:
                value += (s - value) * rate (d);
                if (std::abs (value - s) < 1.0e-4f) stage = sustain;
                break;
            case sustain:
                value = s;
                break;
            case release:
                value += (0.0f - value) * rate (r);
                if (value < 1.0e-4f) kill();
                break;
            case idle: break;
        }
        return value;
    }
};

/** Seno por tabla (fase en ciclos). */
inline float fastSin (float phaseCycles) noexcept
{
    static const auto table = []
    {
        std::array<float, 2049> t {};
        for (size_t i = 0; i < t.size(); ++i) t[i] = (float) std::sin (juce::MathConstants<double>::twoPi * (double) i / 2048.0);
        return t;
    }();
    phaseCycles -= std::floor (phaseCycles);
    const float x = phaseCycles * 2048.0f;
    const int i = (int) x;
    return table[(size_t) i] + (table[(size_t) i + 1] - table[(size_t) i]) * (x - (float) i);
}

/** Filtro de estado variable (TPT, 12 dB): pasabajos, pasabanda y pasaaltos a la vez. */
struct SVF
{
    float ic1 = 0.0f, ic2 = 0.0f;
    float lp = 0.0f, bp = 0.0f, hp = 0.0f;
    void reset() noexcept { ic1 = ic2 = 0.0f; }
    void process (float x, float cutoffHz, float res, float sampleRate) noexcept
    {
        cutoffHz = juce::jlimit (20.0f, sampleRate * 0.45f, cutoffHz);
        const float g = std::tan (juce::MathConstants<float>::pi * cutoffHz / sampleRate);
        const float k = 2.0f - 1.96f * juce::jlimit (0.0f, 1.0f, res);
        const float a1 = 1.0f / (1.0f + g * (g + k)), a2 = g * a1, a3 = g * a2;
        const float v3 = x - ic2;
        const float v1 = a1 * ic1 + a2 * v3;
        const float v2 = ic2 + a2 * ic1 + a3 * v3;
        ic1 = 2.0f * v1 - ic1; ic2 = 2.0f * v2 - ic2;
        lp = v2; bp = v1; hp = x - k * v1 - v2;
    }
};

/** Pasaaltos de un polo (sin resonancia). */
struct HighPass
{
    float z = 0.0f, prev = 0.0f;
    float process (float x, float cutoffHz, float sampleRate) noexcept
    {
        const float a = std::exp (-twoPi * juce::jlimit (5.0f, sampleRate * 0.45f, cutoffHz) / sampleRate);
        z = a * (z + x - prev); prev = x;
        return z;
    }
};

/** Linea de retardo con lectura interpolada; la memoria se reserva en prepare(). */
struct DelayLine
{
    std::vector<float> buf;
    int write = 0;
    void prepare (int maxSamples) { buf.assign ((size_t) juce::jmax (4, maxSamples), 0.0f); write = 0; }
    void clear() { std::fill (buf.begin(), buf.end(), 0.0f); }
    void push (float x) noexcept { buf[(size_t) write] = x; if (++write >= (int) buf.size()) write = 0; }
    float read (float delaySamples) const noexcept
    {
        const int n = (int) buf.size();
        delaySamples = juce::jlimit (1.0f, (float) (n - 2), delaySamples);
        float pos = (float) write - delaySamples;
        while (pos < 0.0f) pos += (float) n;
        const int i = (int) pos;
        const float f = pos - (float) i;
        return buf[(size_t) i] + (buf[(size_t) ((i + 1) % n)] - buf[(size_t) i]) * f;
    }
};

/** Chorus estereo estilo BBD (Juno): modos I, II, I+II. */
struct Chorus
{
    DelayLine line;
    float phase = 0.0f;
    double sr = 44100.0;
    void prepare (double sampleRate) { sr = sampleRate; line.prepare ((int) (sampleRate * 0.03)); }
    void process (float* l, float* r, int n, int mode) noexcept // mode 1 = I, 2 = II, 3 = I+II
    {
        if (mode <= 0) return;
        const float rate  = mode == 1 ? 0.513f : (mode == 2 ? 0.863f : 9.75f);
        const float depth = (mode == 3 ? 0.4f : 1.85f) * 0.001f * (float) sr;
        const float centre = 3.6f * 0.001f * (float) sr;
        for (int i = 0; i < n; ++i)
        {
            const float in = 0.5f * (l[i] + r[i]);
            line.push (in);
            const float tri = 4.0f * std::abs (phase - 0.5f) - 1.0f;
            phase += rate / (float) sr; if (phase >= 1.0f) phase -= 1.0f;
            const float a = line.read (centre + depth * tri), b = line.read (centre - depth * tri);
            l[i] = 0.7f * (l[i] + a); r[i] = 0.7f * (r[i] + b);
        }
    }
};

/** Reverb de resorte (VCS3): dispersion por pasatodos + lazo con realimentacion. */
struct SpringReverb
{
    std::array<DelayLine, 6> ap;
    DelayLine loopL, loopR;
    OnePole damp;
    double sr = 44100.0;
    void prepare (double sampleRate)
    {
        sr = sampleRate;
        for (auto& a : ap) a.prepare ((int) (sampleRate * 0.01));
        loopL.prepare ((int) (sampleRate * 0.1)); loopR.prepare ((int) (sampleRate * 0.1));
    }
    void process (float* l, float* r, int n, float mix) noexcept
    {
        if (mix <= 0.0005f) return;
        const float scale = (float) sr / 44100.0f;
        const float apD[6] = { 37, 53, 71, 97, 113, 131 };
        const float coeff = damp.coeffFor (3200.0f, (float) sr);
        for (int i = 0; i < n; ++i)
        {
            float x = 0.5f * (l[i] + r[i]) + 0.72f * loopL.read (33.7f * 0.001f * (float) sr);
            for (int k = 0; k < 6; ++k) // dispersion "boing"
            {
                const float d = ap[(size_t) k].read (apD[k] * scale);
                const float y = -0.62f * x + d;
                ap[(size_t) k].push (x + 0.62f * y);
                x = y;
            }
            x = damp.lowpass (x, coeff);
            loopL.push (x);
            loopR.push (x);
            const float wl = loopL.read (27.1f * 0.001f * (float) sr), wr = loopR.read (41.3f * 0.001f * (float) sr);
            l[i] += (wl - l[i] * 0.0f) * mix;
            r[i] += wr * mix;
        }
    }
};

/** Delay estereo cruzado (efectos del M1). */
struct StereoDelay
{
    DelayLine dl, dr;
    double sr = 44100.0;
    void prepare (double sampleRate) { sr = sampleRate; dl.prepare ((int) (sampleRate * 1.1)); dr.prepare ((int) (sampleRate * 1.1)); }
    void process (float* l, float* r, int n, float mix, float seconds) noexcept
    {
        const float d = seconds * (float) sr;
        for (int i = 0; i < n; ++i)
        {
            const float a = dl.read (d), b = dr.read (d);
            dl.push (l[i] + b * 0.38f);
            dr.push (r[i] + a * 0.38f);
            l[i] += a * mix; r[i] += b * mix;
        }
    }
};
}
