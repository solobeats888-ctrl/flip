#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include <array>
#include <atomic>
#include <vector>

namespace flipper
{
/**
    Sonidos del tablero (impacto, rebote metalico, poder, bonus), sintetizados al preparar
    el audio: no hay archivos ni carga desde disco.
    - trigger() se llama desde la UI y no bloquea (cola sin locks, un solo productor).
    - render() se llama desde el hilo de audio: no reserva memoria ni usa locks.
    - Apagado por defecto; nunca suena en loop.
*/
class PinballSfx
{
public:
    enum Sound { launch = 0, impact, power, bonus, numSounds };

    void prepare (double sampleRate);
    void trigger (int sound, int engine);                   // hilo de mensajes
    void render (juce::AudioBuffer<float>&, int numSamples); // hilo de audio

    std::atomic<bool> enabled { false };

private:
    struct Event { int sound = 0; float rate = 1.0f; };
    struct Voice { bool active = false; int sound = 0; double pos = 0.0, rate = 1.0; };

    std::array<std::vector<float>, numSounds> buffers;
    std::array<Event, 32> queue;
    juce::AbstractFifo fifo { 32 };
    std::array<Voice, 8> voices;
    std::atomic<bool> ready { false };
};
}
