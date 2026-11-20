#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <array>

namespace flipper
{
constexpr int numEngines = 9;
constexpr int numMacros  = 8;

// Coordenadas en el "espacio de diseno" de 1024 x 1536 (la imagen aprobada).
struct EngineInfo
{
    const char* id;            // id interno / prefijo de parametros
    const char* technical;     // nombre tecnico (panel MEZCLA DE MOTORES)
    const char* character;     // nombre del personaje (placa del playfield)
    const char* portrait;      // recurso BinaryData del retrato
    float plateX, plateY;      // centro del texto de la placa
    float plateW, plateH;      // ancho maximo del texto y alto de letra
    float plateAngle;          // inclinacion de la placa (grados)
    juce::Rectangle<float> hotspot; // elipse clickeable del personaje
    float mixRowY;             // fila en MEZCLA DE MOTORES
};

const std::array<EngineInfo, numEngines>& engines();

struct MacroInfo { const char* id; const char* label; const char* paramName; float x; };  // label en UTF-8
const std::array<MacroInfo, numMacros>& macros();

namespace ids
{
    juce::String engineOn    (int engine);
    juce::String engineLevel (int engine);
    juce::String macro       (int macro);
    inline const juce::String character { "character" };

    // propiedades de UI guardadas en el estado (no automatizables)
    inline const juce::Identifier selectedEngine { "selectedEngine" };
    inline const juce::Identifier uiMode         { "uiMode" };
    inline const juce::Identifier lastSeed       { "lastSeed" };
    inline const juce::Identifier editorWidth    { "editorWidth" };
    inline const juce::Identifier genMode        { "genMode" };
    inline const juce::Identifier genStyle       { "genStyle" };
    inline const juce::Identifier mutateAmount   { "mutateAmount" };
    inline const juce::Identifier locks          { "locks" };
    inline const juce::Identifier animOn         { "animOn" };
    inline const juce::Identifier sfxOn          { "sfxOn" };
}

enum class UiMode { play = 0, design = 1, lab = 2 };

juce::AudioProcessorValueTreeState::ParameterLayout createLayout();

// Texto que describe el valor del control CHARACTER (se muestra en el panel PERSONAJE)
juce::String characterDescriptor (float value01);
}
