#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <array>
#include <atomic>
#include <map>
#include <vector>

/**
    MODULACION de FLIPPER (estilo Serum: se arrastra una fuente a una perilla).

    Fuentes: MOD A, B, C, D (perillas automatizables: una mueve varias a la vez) y LFO 1 y 2
    (libres o al tempo del DAW).

    Cada parametro tiene un "valor modulado" (shadow) que es lo que leen los motores: el valor
    de la perilla + la suma de sus modulaciones, en el espacio 0..1 de la perilla. Los parametros
    que no tienen modulacion pasan tal cual. Solo los parametros continuos se pueden modular.

    Hilo de audio: sin memoria nueva ni locks. Las rutas se editan desde la UI con atomicos.
*/
namespace flipper
{
class ModMatrix
{
public:
    enum Source { modA = 0, modB, modC, modD, lfo1, lfo2, numSources };
    static constexpr int maxRoutes = 64;

    explicit ModMatrix (juce::AudioProcessorValueTreeState&);
    ~ModMatrix();

    static void addParameters (juce::AudioProcessorValueTreeState::ParameterLayout&);
    static juce::String sourceName (int source);           // "MOD A", "LFO 1"...
    static juce::Colour sourceColour (int source);
    static juce::String paramFor (int source);              // perilla de la fuente ("mod_a", "lfo1_rate")

    /** La matriz de un APVTS (los motores la buscan al construirse). */
    static ModMatrix* forState (const juce::AudioProcessorValueTreeState&);
    /** Valor que leen los motores: modulado si hay matriz, si no el crudo. */
    static std::atomic<float>* raw (juce::AudioProcessorValueTreeState&, const juce::String& paramId);

    //==== audio ===============================================================
    void prepare (double sampleRate);
    void beginBlock (double bpm, bool playing, double ppq);   // copia valores base, fija tempo
    void advance (int numSamples);                           // avanza LFOs y recalcula lo modulado

    //==== UI / mensajes =========================================================
    bool canModulate (const juce::String& paramId) const;
    /** Crea o actualiza la ruta fuente > parametro. Devuelve false si no se puede. */
    bool setRoute (int source, const juce::String& paramId, float depth);
    void removeRoute (int source, const juce::String& paramId);
    void clearRoutes();
    float depthOf (int source, const juce::String& paramId) const;   // 0 si no hay ruta

    struct Route { int source; juce::String param; float depth; };
    std::vector<Route> routes() const;
    std::vector<Route> routesFor (const juce::String& paramId) const;

    /** Valor actual de la fuente (-1..1 en LFOs, 0..1 en MOD), para dibujar. */
    float sourceValue (int s) const noexcept { return srcValue[(size_t) s].load (std::memory_order_relaxed); }
    /** Valor modulado (0..1 de la perilla) de un parametro, para dibujar. */
    float modulatedNormalised (const juce::String& paramId) const;

    /** Guardar / leer las rutas en el estado del plugin (proyecto del DAW). */
    void storeToState();
    void loadFromState();
    juce::String toString() const;
    void fromString (const juce::String&);

    std::function<void()> onRoutesChanged;   // hilo de mensajes

private:
    struct Slot { std::atomic<int> source { -1 }, target { -1 }; std::atomic<float> depth { 0.0f }; };
    struct Param
    {
        juce::RangedAudioParameter* param = nullptr;
        std::atomic<float>* base = nullptr;
        std::unique_ptr<std::atomic<float>> shadow;
        bool continuous = false;
    };

    int indexOf (const juce::String& id) const;
    void routesEdited();

    juce::AudioProcessorValueTreeState& state;
    std::vector<Param> params;
    std::map<juce::String, int> lookup;
    std::array<Slot, maxRoutes> slots;
    std::array<std::atomic<float>, numSources> srcValue {};
    std::array<int, maxRoutes> activeTargets {};
    int numActive = 0;

    // LFOs
    std::array<std::atomic<float>*, 2> lfoRate {}, lfoShape {}, lfoSync {};
    std::array<std::atomic<float>*, 4> macroRaw {};
    std::array<double, 2> lfoPhase {};
    std::array<float, 2> lfoHold {};
    double sampleRate = 44100.0, bpm = 120.0;
    juce::Random rnd { 2600 };
};
}
