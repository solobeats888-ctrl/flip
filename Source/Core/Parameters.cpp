#include "Parameters.h"
#include "../DSP/MinimoogEngine.h"
#include "../DSP/Engines.h"
#include "Modulation.h"

namespace flipper
{
using R = juce::Rectangle<float>;

const std::array<EngineInfo, numEngines>& engines()
{
    static const std::array<EngineInfo, numEngines> list {{
        { "minimoog", "MINIMOOG",    "KROKO",          "minimoog_character_png", 296.f,  446.f,  98.f, 25.f, -7.f, R (190, 262, 185, 210), 336.f },
        { "prophet5", "PROPHET-5",   "PROFETA",        "prophet5_character_png", 497.f,  443.f, 130.f, 23.f, -1.f, R (385, 178, 222, 290), 380.f },
        { "jupiter8", "JUPITER-8",   "PITER",          "jupiter8_character_png", 707.f,  452.f, 116.f, 25.f,  7.f, R (610, 240, 188, 242), 424.f },
        { "juno",     "JUNO-60/106", "NO JUNA",        "juno_character_png",     310.f,  744.f, 132.f, 25.f, -7.f, R (150, 480, 272, 292), 468.f },
        { "cs80",     "CS-80",       "ILLUMINARY",     "cs80_character_png",     507.f,  687.f,  94.f, 23.f,  0.f, R (420, 505, 172, 205), 512.f },
        { "dx7",      "DX7",         "TRAP PROD",      "dx7_character_png",      698.f,  746.f, 100.f, 25.f,  3.f, R (595, 480, 228, 292), 556.f },
        { "m1",       "M1",          "ARTURITO",       "m1_character_png",       268.f, 1003.f, 140.f, 23.f, -3.f, R ( 88, 780, 285, 252), 600.f },
        { "arp2600",  "ARP 2600",    "CALAVERA",       "arp2600_character_png",  535.f, 1093.f, 160.f, 26.f, -5.f, R (360, 815, 292, 312), 642.f },
        { "vcs3",     "VCS3",        "EL QUE LA MATA", "vcs3_character_png",     760.f, 1117.f, 134.f, 24.f,  0.f, R (640, 850, 222, 298), 684.f },
    }};
    return list;
}

const std::array<MacroInfo, numMacros>& macros()
{
    static const std::array<MacroInfo, numMacros> list {{
        { "body",       "CUERPO",     "Cuerpo",     85.f },
        { "brightness", "BRILLO",     "Brillo",     183.f },
        { "movement",   "MOVIMIENTO", "Movimiento", 285.f },
        { "texture",    "TEXTURA",    "Textura",    385.f },
        { "width",      "ANCHURA",    "Anchura",    479.f },
        { "space",      "ESPACIO",    "Espacio",    577.f },
        { "attack",     "ATAQUE",     "Ataque",     668.f },
        { "release",    "LIBERACIÓN", "Liberacion", 758.f },
    }};
    return list;
}

namespace ids
{
    juce::String engineOn    (int e) { return juce::String (engines()[(size_t) e].id) + "_on"; }
    juce::String engineLevel (int e) { return juce::String (engines()[(size_t) e].id) + "_level"; }
    juce::String macro       (int m) { return juce::String ("macro_") + macros()[(size_t) m].id; }
}

juce::AudioProcessorValueTreeState::ParameterLayout createLayout()
{
    using namespace juce;
    AudioProcessorValueTreeState::ParameterLayout layout;

    auto percent = AudioParameterFloatAttributes()
        .withStringFromValueFunction ([] (float v, int) { return String (roundToInt (v * 100.0f)) + "%"; })
        .withValueFromStringFunction ([] (const String& s) { return s.getFloatValue() / 100.0f; });

    for (int e = 0; e < numEngines; ++e)
    {
        const auto& info = engines()[(size_t) e];
        layout.add (std::make_unique<AudioParameterBool> (ParameterID { ids::engineOn (e), 1 },
                                                          String (info.technical) + " On", e == 0));
        layout.add (std::make_unique<AudioParameterFloat> (ParameterID { ids::engineLevel (e), 1 },
                                                           String (info.technical) + " Level",
                                                           NormalisableRange<float> (0.0f, 1.0f), 0.7f, percent));
    }

    for (int m = 0; m < numMacros; ++m)
        layout.add (std::make_unique<AudioParameterFloat> (ParameterID { ids::macro (m), 1 },
                                                           macros()[(size_t) m].paramName,
                                                           NormalisableRange<float> (0.0f, 1.0f), 0.5f, percent));

    layout.add (std::make_unique<AudioParameterFloat> (ParameterID { ids::character, 1 }, "Character",
                                                       NormalisableRange<float> (0.0f, 1.0f), 0.6f,
                                                       AudioParameterFloatAttributes()
                                                           .withStringFromValueFunction ([] (float v, int) { return characterDescriptor (v); })));

    // parametros propios de cada arquitectura (se agregan al final: no cambia el orden anterior)
    dsp::MinimoogEngine::addParameters (layout);
    dsp::Prophet5Engine::addParameters (layout);
    dsp::Jupiter8Engine::addParameters (layout);
    dsp::JunoEngine::addParameters (layout);
    dsp::CS80Engine::addParameters (layout);
    dsp::DX7Engine::addParameters (layout);
    dsp::M1Engine::addParameters (layout);
    dsp::Arp2600Engine::addParameters (layout);
    dsp::VCS3Engine::addParameters (layout);

    // arpegiador (se maneja desde JUPITER-8 y JUNO-60/106, toca todos los motores encendidos)
    layout.add (std::make_unique<AudioParameterChoice> (ParameterID { "arp_on", 1 }, "Arp On", StringArray { "NO", String::fromUTF8 ("SÍ") }, 0));
    layout.add (std::make_unique<AudioParameterChoice> (ParameterID { "arp_mode", 1 }, "Arp Mode",
                                                        StringArray { "ARRIBA", "ABAJO", "ARRIBA-ABAJO", "AZAR" }, 0));
    layout.add (std::make_unique<AudioParameterChoice> (ParameterID { "arp_rate", 1 }, "Arp Rate",
                                                        StringArray { "1/4", "1/8", "1/8T", "1/16", "1/16T", "1/32" }, 3));
    layout.add (std::make_unique<AudioParameterChoice> (ParameterID { "arp_oct", 1 }, "Arp Octaves", StringArray { "1", "2", "3" }, 0));
    layout.add (std::make_unique<AudioParameterFloat> (ParameterID { "arp_gate", 1 }, "Arp Gate", NormalisableRange<float> (0.05f, 1.0f), 0.6f, percent));

    // modulacion (MOD A-D y LFO 1-2): al final, no cambia el orden anterior
    ModMatrix::addParameters (layout);
    return layout;
}

juce::String characterDescriptor (float v)
{
    if (v < 0.25f) return "LIMPIO";
    if (v < 0.50f) return juce::String::fromUTF8 ("CÁLIDO");
    if (v < 0.75f) return "SUCIO";
    return "ROTO";
}
}
