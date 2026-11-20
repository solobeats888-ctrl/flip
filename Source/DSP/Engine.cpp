#include "Engine.h"

namespace flipper::dsp
{
using namespace juce;

void addParams (AudioProcessorValueTreeState::ParameterLayout& layout, const char* prefix, const ParamDef* defs, int count)
{
    for (int i = 0; i < count; ++i)
    {
        const auto& d = defs[i];
        const ParameterID id { String (prefix) + "_" + d.id, 1 };
        const String name = String (prefix).toUpperCase() + " " + d.name;

        if (d.kind == 'c' || d.kind == 'b')
        {
            const auto opts = d.kind == 'b' ? StringArray { "NO", String::fromUTF8 ("SÍ") }
                                            : StringArray::fromTokens (String::fromUTF8 (d.opts), "|", "");
            layout.add (std::make_unique<AudioParameterChoice> (id, name, opts, (int) d.def));
            continue;
        }
        if (d.kind == 'i')
        {
            layout.add (std::make_unique<AudioParameterInt> (id, name, (int) d.min, (int) d.max, (int) d.def));
            continue;
        }

        NormalisableRange<float> range (d.min, d.max);
        AudioParameterFloatAttributes attr;
        switch (d.kind)
        {
            case 't':
                range.setSkewForCentre (jmin (d.max * 0.15f, 0.4f));
                attr = attr.withStringFromValueFunction ([] (float v, int)
                       { return v < 1.0f ? String (roundToInt (v * 1000.0f)) + " ms" : String (v, 2) + " s"; });
                break;
            case 'h':
                range.setSkewForCentre (std::sqrt (d.min * d.max));
                attr = attr.withStringFromValueFunction ([] (float v, int)
                       { return v < 10.0f ? String (v, 2) + " Hz" : (v < 1000.0f ? String (roundToInt (v)) + " Hz" : String (v / 1000.0f, 1) + " kHz"); });
                break;
            case 's':
                attr = attr.withStringFromValueFunction ([] (float v, int) { return (v >= 0 ? "+" : "") + String (v, 2) + " st"; });
                break;
            case 'p':
                attr = attr.withStringFromValueFunction ([] (float v, int) { return String (roundToInt (v * 100.0f)) + "%"; });
                break;
            default:
                attr = attr.withStringFromValueFunction ([] (float v, int) { return String (v, 2); });
                break;
        }
        layout.add (std::make_unique<AudioParameterFloat> (id, name, range, d.def, attr));
    }
}

void ParamBank::bind (AudioProcessorValueTreeState& state, const char* prefix, const ParamDef* defs, int count)
{
    raw.clear();
    for (int i = 0; i < count; ++i)
    {
        auto* r = state.getRawParameterValue (String (prefix) + "_" + defs[i].id);
        jassert (r != nullptr);
        raw.push_back (r);
    }
}
}
