#include "PinballSfx.h"

namespace flipper
{
using namespace juce;

void PinballSfx::prepare (double sr)
{
    ready.store (false);
    for (auto& v : voices) v.active = false;
    Random r (2600);
    const double tau = MathConstants<double>::twoPi;

    auto make = [&] (int sound, double seconds, auto fn)
    {
        auto& b = buffers[(size_t) sound];
        b.assign ((size_t) (seconds * sr), 0.0f);
        for (size_t i = 0; i < b.size(); ++i) b[i] = (float) fn ((double) i / sr);
    };

    // lanzamiento: resorte (barrido descendente + golpe)
    make (launch, 0.18, [&] (double t) { return std::sin (tau * (420.0 - 900.0 * t) * t) * std::exp (-t * 22.0) * 0.5
                                                + (r.nextFloat() * 2 - 1) * std::exp (-t * 90.0) * 0.25; });
    // impacto: golpe sordo + rebote metalico (parciales inarmonicos)
    make (impact, 0.55, [&] (double t)
    {
        const double thump = std::sin (tau * (140.0 * std::exp (-t * 18.0) + 55.0) * t) * std::exp (-t * 24.0) * 0.7;
        const double ping  = (std::sin (tau * 1830.0 * t) + 0.6 * std::sin (tau * 2710.0 * t) + 0.4 * std::sin (tau * 4180.0 * t))
                             * std::exp (-t * 9.0) * 0.16;
        return thump + ping + (r.nextFloat() * 2 - 1) * std::exp (-t * 140.0) * 0.3;
    });
    // poder: barrido ascendente con brillo
    make (power, 0.6, [&] (double t)
    {
        const double f = 180.0 + 900.0 * t * t * 4.0;
        return (std::sin (tau * f * t) * 0.35 + std::sin (tau * f * 1.5 * t) * 0.15) * std::sin (MathConstants<double>::pi * t / 0.6) * 0.8;
    });
    // bonus: arpegio de campanitas
    make (bonus, 0.75, [&] (double t)
    {
        const double notes[] = { 1046.5, 1318.5, 1568.0, 2093.0 };
        double s = 0;
        for (int i = 0; i < 4; ++i)
        {
            const double st = t - i * 0.07;
            if (st > 0) s += std::sin (tau * notes[i] * st) * std::exp (-st * 7.0) * 0.18;
        }
        return s;
    });
    ready.store (true);
}

void PinballSfx::trigger (int sound, int engine)
{
    if (! enabled.load() || sound < 0 || sound >= numSounds) return;
    const auto scope = fifo.write (1);
    if (scope.blockSize1 > 0) queue[(size_t) scope.startIndex1] = { sound, 0.82f + 0.045f * (float) engine };
    else if (scope.blockSize2 > 0) queue[(size_t) scope.startIndex2] = { sound, 0.82f + 0.045f * (float) engine };
}

void PinballSfx::render (AudioBuffer<float>& out, int numSamples)
{
    if (! ready.load())
        return;

    const auto scope = fifo.read (fifo.getNumReady());
    auto start = [this] (const Event& ev)
    {
        for (auto& v : voices)
            if (! v.active) { v = { true, ev.sound, 0.0, ev.rate }; return; }
    };
    for (int i = 0; i < scope.blockSize1; ++i) start (queue[(size_t) (scope.startIndex1 + i)]);
    for (int i = 0; i < scope.blockSize2; ++i) start (queue[(size_t) (scope.startIndex2 + i)]);

    if (! enabled.load()) { for (auto& v : voices) v.active = false; return; }

    constexpr float gain = 0.3f;
    for (auto& v : voices)
    {
        if (! v.active) continue;
        const auto& b = buffers[(size_t) v.sound];
        for (int i = 0; i < numSamples; ++i)
        {
            const auto idx = (size_t) v.pos;
            if (idx + 1 >= b.size()) { v.active = false; break; }
            const float frac = (float) (v.pos - (double) idx);
            const float s = (b[idx] + (b[idx + 1] - b[idx]) * frac) * gain;
            for (int ch = 0; ch < out.getNumChannels(); ++ch)
                out.addSample (ch, i, s);
            v.pos += v.rate;
        }
    }
}
}
