#pragma once
#include "PresetManager.h"

namespace flipper
{
/** Presets de fabrica (solo lectura), armados sobre los valores por defecto de cada parametro. */
std::vector<PresetManager::Preset> buildFactoryPresets (const juce::AudioProcessorValueTreeState&);
}
