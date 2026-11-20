#pragma once
#include "Primitives.h"
#include <juce_audio_processors/juce_audio_processors.h>

namespace flipper::dsp
{
constexpr int maxVoices = 12;
constexpr int maxBlock  = 256;

/**
    Modulacion global de cada bloque: macros de JUGAR, MIDI y LFO compartido.
    Los valores neutros dejan el sonido tal cual lo diseno cada motor.
*/
struct BlockMod
{
    float brightnessOct = 0.0f;   // BRILLO: desplazamiento del corte en octavas
    float attackMul = 1.0f;       // ATAQUE: multiplica los tiempos de ataque
    float releaseMul = 1.0f;      // LIBERACION: multiplica los tiempos de liberacion
    float movement = 0.0f;        // MOVIMIENTO: profundidad del LFO (0..1)
    float texture = 0.0f;         // TEXTURA: ruido y desafinacion extra (0..1)
    float bendSemis = 0.0f;       // pitch bend
    float modWheel = 0.0f;        // rueda de modulacion (0..1)
    const float* lfo = nullptr;   // LFO global por muestra (-1..1)
};

/**
    Interfaz de un motor de sintesis. Cada motor guarda el estado de sus voces en arreglos
    fijos (maxVoices): no reserva memoria en el hilo de audio.
*/
class Engine
{
public:
    virtual ~Engine() = default;

    virtual void prepare (double sampleRate) = 0;
    /** Lee sus parametros (una vez por bloque). */
    virtual void updateParameters() = 0;

    virtual void noteOn (int slot, float note, float velocity, float glideFromNote) = 0;
    virtual void noteOff (int slot) = 0;
    virtual void kill (int slot) = 0;
    virtual bool isActive (int slot) const = 0;

    /** Suma la voz en out (mono). */
    virtual void render (int slot, float* out, int numSamples, const BlockMod&) = 0;

    /** Antes de las voces de cada tramo (LFO propio del motor, etc.). */
    virtual void beginBlock (int /*numSamples*/) {}
    /** Efectos propios del motor sobre la suma estereo de sus voces (chorus, delay, resorte). */
    virtual void processBus (float* /*left*/, float* /*right*/, int /*numSamples*/) {}
    /** Segundos de cola que deja processBus despues de la ultima voz. */
    virtual double tailSeconds() const { return 0.0; }
    /** Aftertouch por voz (0..1). */
    virtual void aftertouch (int /*slot*/, float /*value*/) {}
};

//============================================================================== parametros declarativos
/**
    Un parametro de motor en una linea. kind:
      'p' porcentaje 0..1      't' tiempo en segundos     'h' frecuencia en Hz
      's' semitonos            'n' numero                 'i' entero
      'c' opciones (opts "A|B|C")   'b' interruptor (NO / SI)
*/
struct ParamDef { const char* id; const char* name; char kind; float min, max, def; const char* opts = nullptr; };

void addParams (juce::AudioProcessorValueTreeState::ParameterLayout&, const char* prefix, const ParamDef* defs, int count);

/** Lectura sin locks de los parametros de un motor (en el mismo orden de su tabla). */
class ParamBank
{
public:
    void bind (juce::AudioProcessorValueTreeState&, const char* prefix, const ParamDef* defs, int count);
    float operator[] (int index) const noexcept { return raw[(size_t) index]->load (std::memory_order_relaxed); }
    int   choice (int index) const noexcept     { return (int) (raw[(size_t) index]->load() + 0.5f); }
    bool  on (int index) const noexcept         { return raw[(size_t) index]->load() > 0.5f; }
private:
    std::vector<std::atomic<float>*> raw;
};
}
