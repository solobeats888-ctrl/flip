#include "FactoryPresets.h"
#include <map>

namespace flipper
{
using namespace juce;

namespace
{
/**
    Presets de fabrica: sonidos ORIGINALES que recrean la funcion de los clasicos de cada
    maquina (bajo grueso, brass, piano de tines, organo, pad con chorus...), con valores y
    nombres propios. No contienen datos de los bancos de fabrica de ningun sintetizador.

    engines: indices de los motores encendidos. values: "clave=valor" en unidades reales;
    la clave sin '@' lleva el prefijo del primer motor; con '@' es un id completo.
*/
struct Def { const char* name; const char* engines; const char* values; };

const Def defs[] = {
    // ---------------------------------------------------------------- MINIMOOG (KROKO)
    { "Bajo Gordo de Kroko", "0", "osc1_range=1 osc2_range=1 osc3_range=0 mix1=1 mix2=0.8 mix3=0.6 osc2_detune=0.1 osc3_detune=-0.08 cutoff=260 emphasis=0.25 contour=0.55 f_decay=0.35 f_sustain=0.1 a_sustain=0.9 a_release=0.12 drive=0.45" },
    { "Lead Pantano", "0", "osc2_wave=5 mix2=0.7 osc2_detune=7 cutoff=1400 emphasis=0.55 contour=0.4 glide=0.08 a_release=0.3 drive=0.5" },
    { "Pluck Reptil", "0", "cutoff=420 emphasis=0.6 contour=0.7 f_decay=0.18 f_sustain=0 a_decay=0.35 a_sustain=0 a_release=0.15 osc3_range=1 mix3=0.5" },
    { "Brass Cocodrilo", "0", "f_attack=0.08 f_decay=0.6 f_sustain=0.5 contour=0.5 cutoff=700 a_attack=0.03 a_release=0.35 mix3=0.3 osc2_detune=0.12" },
    // ---------------------------------------------------------------- PROPHET-5 (PROFETA)
    { "Brass Profetico", "1", "b_fine=0.12 cutoff=650 fenv=0.6 f_attack=0.12 f_decay=0.7 f_sustain=0.5 a_attack=0.06 a_release=0.4 res=0.15" },
    { "Campana Poly-Mod", "1", "pm_env=0.45 pm_oscb=0.3 pm_freqa=1 b_semi=19 b_tri=1 b_saw=0 sync=1 cutoff=3000 fenv=0.3 a_sustain=0 a_decay=1.6 a_release=1.4 f_sustain=0" },
    { "Unisono Divino", "1", "unison=1 uni_detune=0.45 cutoff=1800 res=0.3 glide=0.05 a_release=0.3" },
    { "Sync Sagrado", "1", "sync=1 pm_env=0.6 pm_freqa=1 mix_b=0 cutoff=4000 f_decay=0.8" },
    // ---------------------------------------------------------------- JUPITER-8 (PITER)
    { "Brass de Piter", "2", "vco2_fine=0.15 cutoff=900 env_amt=0.55 e1_attack=0.08 e1_decay=0.6 e1_sustain=0.45 e2_attack=0.05" },
    { "Cross Mod Planetario", "2", "cross=0.45 vco2_range=0 sync=1 cutoff=3500 res=0.25 env_amt=0.3" },
    { "Arpegio Orbital", "2", "@arp_on=1 @arp_rate=3 @arp_oct=1 cutoff=1200 res=0.4 env_amt=0.6 e1_decay=0.25 e1_sustain=0 e2_decay=0.3 e2_sustain=0 e2_release=0.2" },
    { "Pad Saturno", "2", "vco1_wave=2 lfo_pwm=0.5 lfo_rate=0.6 cutoff=1600 e2_attack=0.6 e2_release=1.8 @macro_space=0.5" },
    // ---------------------------------------------------------------- JUNO-60/106 (NO JUNA)
    { "Pad de la Wacha", "3", "attack=0.45 release=1.6 cutoff=1800 env_amt=0.2 @macro_space=0.4" },
    { "Bajo Sub Juna", "3", "pulse=0 sub=0.8 cutoff=500 res=0.2 env_amt=0.45 decay=0.4 sustain=0.5 release=0.15 chorus=0 range=0" },
    { "Cuerdas Rosadas", "3", "pulse=0 cutoff=3200 attack=0.7 release=2 chorus=3" },
    { "Pluck Arpegiado", "3", "@arp_on=1 @arp_mode=2 cutoff=900 res=0.35 env_amt=0.6 decay=0.25 sustain=0 release=0.2" },
    // ---------------------------------------------------------------- CS-80 (ILLUMINARY)
    { "Metales del Ojo", "4", "l1_attack=0.15 l2_attack=0.25 l1_fenv=0.5 at_bright=0.7" },
    { "Pad Iluminado", "4", "l1_attack=0.9 l2_attack=1.4 l1_release=2.5 l2_release=3 l1_lpf=1500 l2_lpf=2500 @macro_space=0.55" },
    { "Lead Cinta", "4", "balance=0.2 l1_saw=1 l1_res=0.45 l1_lpf=1800 porta=0.12 at_vib=0.7" },
    { "Anillo Cosmico", "4", "ring_depth=0.8 ring_speed=18 l2_feet=2 balance=0.6" },
    // ---------------------------------------------------------------- DX7 (TRAP PROD)
    { "Piano de Tines Trap", "5", "@macro_space=0.35" },
    { "Campana de Cristal", "5", "op2_coarse=3 op2_level=0.55 op4_coarse=7 op4_level=0.45 op6_coarse=14 op6_level=0.3 op1_decay=3.5 op3_decay=3 op5_decay=2.5 op1_release=1.5 op3_release=1.5 op5_release=1.2 feedback=0.2" },
    { "Bajo FM Sucio", "5", "algorithm=16 op1_decay=0.8 op1_sustain=0.6 op2_level=0.6 op2_decay=0.3 op3_level=0.4 op3_coarse=2 op5_level=0.3 op5_coarse=3 op6_level=0.5 feedback=0.6 @character=0.55" },
    { "Marimba Mental", "5", "op2_coarse=4 op1_decay=0.6 op3_decay=0.5 op5_level=0 op4_coarse=10 op4_level=0.2" },
    // ---------------------------------------------------------------- M1 (ARTURITO)
    { "Piano Pad Noventoso", "6", "" },
    { "Organo de Arturito", "6", "s1_wave=2 s1_level=0.9 s2_level=0 a_sustain=1 a_release=0.1" },
    { "Coro Universo", "6", "s1_wave=4 s2_wave=9 s2_level=0.6 a_attack=0.6 a_release=2.2 cutoff=7000 fx_type=3 fx_mix=0.3 @macro_space=0.6" },
    { "Bajo Digital", "6", "s1_wave=6 s1_coarse=-12 s2_level=0 a_sustain=0.7 a_decay=1 fx_type=0" },
    { "Campanas Robot", "6", "s1_wave=5 s2_wave=8 s2_level=0.5 s2_coarse=12 fx_type=2 fx_mix=0.3" },
    // ---------------------------------------------------------------- ARP 2600 (CALAVERA)
    { "Robot Calavera", "7", "c1_amt=0.6 res=0.6 cutoff=900 sh_rate=9" },
    { "Bajo Hueso", "7", "vco2_tune=-12 vco1_wave=1 cutoff=400 res=0.3 fenv=0.6 decay=0.3 sustain=0.3" },
    { "Viento de Ultratumba", "7", "vco1_level=0 vco2_level=0 noise=0.9 noise_color=1 filter_mode=1 res=0.8 c3_amt=0.6 ar_attack=1.5 ar_release=2 attack=1 release=2" },
    { "Anillo de Huesos", "7", "ring=0.8 vco2_tune=7.3 vco1_level=0.3 vco2_level=0.3 cutoff=5000 sustain=0 decay=1.2 release=1" },
    // ---------------------------------------------------------------- VCS3 (EL QUE LA MATA)
    { "El Que La Mata", "8", "" },
    { "Repetidor Oscuro", "8", "env_trig=1 env_attack=0.005 env_on=0.05 env_decay=0.12 env_off=0.08 res=0.55 reverb=0.35" },
    { "Ruido del Espectro", "8", "pin_30=1 pin_00=0 pin_10=0 res=0.85 cutoff=600 depth_filter=0.6" },
    { "Silbido Autooscilante", "8", "res=1 pin_00=0 pin_10=0 pin_30=1 noise=0.05 cutoff=1500 reverb=0.4" },
    // ---------------------------------------------------------------- HIBRIDOS
    { "Kroko y el Profeta", "0,1", "@minimoog_cutoff=320 @minimoog_a_release=0.15 @prophet5_cutoff=1200 @prophet5_a_attack=0.2 @prophet5_a_release=0.8 @prophet5_level=0.55" },
    { "Wacha en Trap", "3,5", "@juno_attack=0.3 @juno_release=1.2 @juno_level=0.6 @dx7_level=0.8 @macro_space=0.4" },
    { "Robot y Espectro", "6,8", "@m1_s1_wave=5 @m1_s2_level=0 @vcs3_reverb=0.5 @vcs3_level=0.5" },
};
}

std::vector<PresetManager::Preset> buildFactoryPresets (const AudioProcessorValueTreeState& state)
{
    std::vector<PresetManager::Preset> out;
    for (const auto& d : defs)
    {
        PresetManager::Preset p;
        p.factory = true;
        p.name = d.name;

        std::map<String, float> v;   // id -> valor normalizado
        for (auto* prm : state.processor.getParameters())
            if (auto* rp = dynamic_cast<RangedAudioParameter*> (prm))
                v[rp->getParameterID()] = rp->getDefaultValue();

        const auto list = StringArray::fromTokens (d.engines, ",", "");
        for (int e = 0; e < numEngines; ++e)
        {
            const bool on = list.contains (String (e));
            v[ids::engineOn (e)] = on ? 1.0f : 0.0f;
            v[ids::engineLevel (e)] = 0.8f;
        }
        if (auto* c = state.getParameter (ids::character)) v[ids::character] = c->convertTo0to1 (0.3f);

        const String prefix = String (engines()[(size_t) list[0].getIntValue()].id) + "_";
        for (auto& token : StringArray::fromTokens (d.values, " ", ""))
        {
            if (! token.contains ("=")) continue;
            String key = token.upToFirstOccurrenceOf ("=", false, false);
            const float real = token.fromFirstOccurrenceOf ("=", false, false).getFloatValue();
            key = key.startsWith ("@") ? key.substring (1) : prefix + key;
            auto* prm = state.getParameter (key);
            jassert (prm != nullptr);   // un nombre mal escrito en la tabla
            if (prm != nullptr) v[key] = prm->convertTo0to1 (real);
        }

        for (auto& [id, val] : v) p.values.push_back ({ id, val });
        p.category = PresetManager::categoryFor (p.values);
        for (int e = 0; e < numEngines; ++e) p.engineOn[(size_t) e] = v[ids::engineOn (e)] > 0.5f;
        p.character = v[ids::character];
        p.tags.add (String::fromUTF8 ("FÁBRICA"));
        out.push_back (std::move (p));
    }
    return out;
}
}
