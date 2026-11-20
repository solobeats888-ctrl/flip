#pragma once
#include "Engine.h"
#include "../Core/Parameters.h"
#include <juce_dsp/juce_dsp.h>

namespace flipper::dsp
{
/**
    Nucleo del instrumento: un solo juego de voces compartido por los nueve motores.

    - Cada nota abre una voz; cada motor encendido (y con nivel > 0) suma su sonido en esa voz.
      Los motores apagados no procesan nada. No hay limite de motores activos a la vez.
    - Hilo de audio: sin reservas de memoria, sin locks, sin archivos.
    - MIDI: notas con velocidad, pitch bend (+-2 st), rueda de modulacion, pedal de sustain,
      all notes off.
    - Macros de JUGAR conectadas al sonido; CHARACTER = saturacion (LIMPIO..ROTO);
      CUERPO = graves; ANCHURA = apertura estereo de las voces; ESPACIO = reverb.
*/
class SynthCore
{
public:
    explicit SynthCore (juce::AudioProcessorValueTreeState&);
    ~SynthCore();

    void prepare (double sampleRate, int maxBlockSize);
    void setBpm (double bpm) noexcept { if (bpm > 20.0 && bpm < 400.0) hostBpm = bpm; }
    void process (juce::AudioBuffer<float>&, juce::MidiBuffer&);
    void allNotesOff();

    /** Motores con DSP implementado. */
    static bool isImplemented (int engine) { return engine >= 0 && engine < numEngines; }

    int activeVoiceCount() const noexcept;

private:
    struct Voice
    {
        int note = -1;
        bool gate = false, sustained = false;
        juce::uint32 age = 0;
        float velocity = 0.0f, pan = 0.0f;
    };

    void handleMidi (const juce::MidiMessage&);
    void noteOn (int note, float velocity);
    void noteOff (int note);
    void keyDown (int note, float velocity);   // teclas reales (pasan por el arpegiador)
    void keyUp (int note);
    void arpStep();
    void arpRelease();
    void renderChunk (float* left, float* right, int numSamples);
    void readParameters();
    bool voiceBusy (int slot) const;

    std::array<std::unique_ptr<Engine>, numEngines> engines;
    std::array<Voice, maxVoices> voices;
    std::array<std::atomic<float>*, numEngines> onRaw {}, levelRaw {};
    std::array<std::atomic<float>*, numMacros> macroRaw {};
    std::atomic<float>* characterRaw = nullptr;

    std::array<bool, numEngines> engineOn {};
    std::array<float, numEngines> engineLevel {};
    std::array<float, numMacros> macro {};
    float character = 0.5f;

    BlockMod mod;
    std::array<float, maxBlock> lfoBuf {}, engineBuf {}, busL {}, busR {};
    std::array<int, numEngines> tail {};

    // arpegiador
    std::atomic<float>* arpOnRaw = nullptr, *arpModeRaw = nullptr, *arpRateRaw = nullptr, *arpOctRaw = nullptr, *arpGateRaw = nullptr;
    std::array<int, 32> held {}; std::array<float, 32> heldVel {};
    int heldCount = 0, arpIndex = 0, arpNote = -1, arpDir = 1;
    double samplesToStep = 0, samplesToOff = -1, hostBpm = 120.0;
    bool arpWasOn = false;
    juce::Random arpRandom { 77 };
    float lfoPhase = 0.0f, bend = 0.0f, modWheel = 0.0f;
    bool sustainPedal = false;
    float lastNote = -1.0f;
    juce::uint32 ageCounter = 0;
    double sampleRate = 44100.0;

    // FX globales
    OnePole bodyLowL, bodyLowR;
    float crushHoldL = 0, crushHoldR = 0;
    int crushCounter = 0;
    juce::Reverb reverb;
    juce::Reverb::Parameters reverbParams;
    juce::SmoothedValue<float> spaceSmooth, levelSmooth[numEngines];
};
}
