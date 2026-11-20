#pragma once
#include "Parameters.h"
#include <vector>
#include <set>

namespace flipper
{
/** Valores normalizados de parametros (id -> valor). */
using Snapshot = std::vector<std::pair<juce::String, float>>;

/**
    Generador procedural offline (sin IA, sin internet).
    Con la misma semilla y los mismos ajustes siempre da el mismo resultado.

    Etapa actual: trabaja sobre los parametros que existen hoy (mezcla de motores,
    macros y CHARACTER). Cuando cada motor tenga su DSP, sus parametros se suman aca
    con reglas propias por arquitectura.
*/
class SoundGenerator
{
public:
    enum class Mode { classic = 0, style = 1, hybrid = 2 };

    struct Settings
    {
        Mode mode = Mode::hybrid;
        int style = 0;                 // indice en styleNames()
        int selectedEngine = 8;        // motor base del modo CLASICO
        float mutateAmount = 0.25f;
        std::set<juce::String> locks;  // ids de motor, "macros", "character"
    };

    static const juce::StringArray& styleNames();
    static const juce::StringArray& modeNames();

    static Snapshot capture  (const juce::AudioProcessorValueTreeState&);
    static Snapshot generate (juce::uint32 seed, const Settings&, const juce::AudioProcessorValueTreeState&);
    static Snapshot mutate   (const juce::AudioProcessorValueTreeState&, juce::uint32 seed, const Settings&);
    static void     apply    (juce::AudioProcessorValueTreeState&, const Snapshot&);

    /** Saca de la foto todo lo que esta bloqueado. */
    static Snapshot withoutLocked (Snapshot, const std::set<juce::String>& locks);
    static bool     equal (const Snapshot&, const Snapshot&);

    static std::set<juce::String> parseLocks (const juce::String&);
    static juce::String           joinLocks  (const std::set<juce::String>&);
};
}
