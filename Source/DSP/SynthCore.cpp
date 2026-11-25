#include "SynthCore.h"
#include "../Core/Modulation.h"
#include "MinimoogEngine.h"
#include "Engines.h"

namespace flipper::dsp
{
using namespace juce;

SynthCore::SynthCore (AudioProcessorValueTreeState& state)
{
    modMatrix = ModMatrix::forState (state);
    engines[0] = std::make_unique<MinimoogEngine> (state);
    engines[1] = std::make_unique<Prophet5Engine> (state);
    engines[2] = std::make_unique<Jupiter8Engine> (state);
    engines[3] = std::make_unique<JunoEngine> (state);
    engines[4] = std::make_unique<CS80Engine> (state);
    engines[5] = std::make_unique<DX7Engine> (state);
    engines[6] = std::make_unique<M1Engine> (state);
    engines[7] = std::make_unique<Arp2600Engine> (state);
    engines[8] = std::make_unique<VCS3Engine> (state);
    arpOnRaw = ModMatrix::raw (state, "arp_on");
    arpModeRaw = ModMatrix::raw (state, "arp_mode");
    arpRateRaw = ModMatrix::raw (state, "arp_rate");
    arpOctRaw = ModMatrix::raw (state, "arp_oct");
    arpGateRaw = ModMatrix::raw (state, "arp_gate");

    for (int e = 0; e < numEngines; ++e)
    {
        onRaw[(size_t) e] = ModMatrix::raw (state, ids::engineOn (e));
        levelRaw[(size_t) e] = ModMatrix::raw (state, ids::engineLevel (e));
    }
    for (int m = 0; m < numMacros; ++m)
        macroRaw[(size_t) m] = ModMatrix::raw (state, ids::macro (m));
    characterRaw = ModMatrix::raw (state, ids::character);
}

SynthCore::~SynthCore() = default;

void SynthCore::prepare (double sr, int)
{
    sampleRate = sr;
    for (auto& e : engines) if (e) e->prepare (sr);
    for (auto& v : voices) v = {};
    tail.fill (0);
    heldCount = 0; arpNote = -1; samplesToOff = -1;
    reverb.reset();
    reverb.setSampleRate (sr);
    spaceSmooth.reset (sr, 0.05);
    for (auto& l : levelSmooth) l.reset (sr, 0.02);
    bodyLowL.z = bodyLowR.z = 0.0f;
}

int SynthCore::activeVoiceCount() const noexcept
{
    int n = 0;
    for (int s = 0; s < maxVoices; ++s) n += voiceBusy (s) ? 1 : 0;
    return n;
}

bool SynthCore::voiceBusy (int slot) const
{
    for (auto& e : engines)
        if (e && e->isActive (slot)) return true;
    return false;
}

void SynthCore::readParameters()
{
    for (int e = 0; e < numEngines; ++e)
    {
        engineOn[(size_t) e] = onRaw[(size_t) e]->load() > 0.5f;

        // motor que se prende con notas sonando (luz, preset, GENERAR, maestro): entra tambien en
        // las notas retenidas, asi todas las notas suenan con la misma mezcla de motores
        if (engineOn[(size_t) e] && ! prevOn[(size_t) e] && engines[(size_t) e])
            for (int s = 0; s < maxVoices; ++s)
            {
                const auto& v = voices[(size_t) s];
                if (v.note >= 0 && (v.gate || v.sustained))
                    engines[(size_t) e]->noteOn (s, (float) v.note, v.velocity, (float) v.note);
            }
        prevOn[(size_t) e] = engineOn[(size_t) e];
        engineLevel[(size_t) e] = levelRaw[(size_t) e]->load();
        levelSmooth[e].setTargetValue (engineOn[(size_t) e] ? engineLevel[(size_t) e] : 0.0f);
    }
    for (int m = 0; m < numMacros; ++m) macro[(size_t) m] = macroRaw[(size_t) m]->load();
    character = characterRaw->load();

    // macros -> modulacion (0.5 = neutro en las que son "mas/menos")
    enum { body, brightness, movement, texture, width, space, attack, release };
    mod.brightnessOct = (macro[brightness] - 0.5f) * 4.0f;
    mod.attackMul = std::exp2 ((macro[attack] - 0.5f) * 6.0f);
    mod.releaseMul = std::exp2 ((macro[release] - 0.5f) * 6.0f);
    mod.movement = macro[movement] * macro[movement];
    mod.texture = macro[texture] * macro[texture];
    mod.bendSemis = bend * 2.0f;
    mod.modWheel = modWheel;

    const float wet = macro[space] * macro[space] * 0.55f;
    spaceSmooth.setTargetValue (wet);
    reverbParams.roomSize = 0.55f + macro[space] * 0.4f;
    reverbParams.damping = 0.45f;
    reverbParams.width = 0.5f + macro[width] * 0.5f;
    reverbParams.wetLevel = 1.0f;
    reverbParams.dryLevel = 0.0f;
    reverb.setParameters (reverbParams);

    for (auto& e : engines) if (e) e->updateParameters();
}

void SynthCore::handleMidi (const MidiMessage& msg)
{
    if (msg.isNoteOn())                        keyDown (msg.getNoteNumber(), msg.getFloatVelocity());
    else if (msg.isNoteOff())                  keyUp (msg.getNoteNumber());
    else if (msg.isAftertouch())
    {
        for (int s = 0; s < maxVoices; ++s)
            if (voices[(size_t) s].note == msg.getNoteNumber() && voices[(size_t) s].gate)
                for (auto& e : engines) if (e) e->aftertouch (s, (float) msg.getAfterTouchValue() / 127.0f);
    }
    else if (msg.isChannelPressure())
    {
        for (int s = 0; s < maxVoices; ++s)
            if (voices[(size_t) s].gate)
                for (auto& e : engines) if (e) e->aftertouch (s, (float) msg.getChannelPressureValue() / 127.0f);
    }
    else if (msg.isPitchWheel())               bend = (float) (msg.getPitchWheelValue() - 8192) / 8192.0f;
    else if (msg.isControllerOfType (1))       modWheel = (float) msg.getControllerValue() / 127.0f;
    else if (msg.isSustainPedalOn())           sustainPedal = true;
    else if (msg.isSustainPedalOff())
    {
        sustainPedal = false;
        for (int s = 0; s < maxVoices; ++s)
            if (voices[(size_t) s].sustained)
            {
                voices[(size_t) s].sustained = false;
                for (auto& e : engines) if (e) e->noteOff (s);
            }
    }
    else if (msg.isAllNotesOff() || msg.isAllSoundOff()) allNotesOff();
}

//============================================================================== arpegiador
void SynthCore::keyDown (int note, float velocity)
{
    if (arpOnRaw->load() < 0.5f) { noteOn (note, velocity); return; }
    for (int i = 0; i < heldCount; ++i) if (held[(size_t) i] == note) return;
    if (heldCount < (int) held.size()) { held[(size_t) heldCount] = note; heldVel[(size_t) heldCount] = velocity; ++heldCount; }
    if (heldCount == 1) { arpIndex = 0; arpDir = 1; samplesToStep = 0; }   // la primera tecla suena ya
}

void SynthCore::keyUp (int note)
{
    for (int i = 0; i < heldCount; ++i)
        if (held[(size_t) i] == note)
        {
            for (int k = i; k < heldCount - 1; ++k) { held[(size_t) k] = held[(size_t) k + 1]; heldVel[(size_t) k] = heldVel[(size_t) k + 1]; }
            --heldCount;
            if (heldCount == 0) arpRelease();
            return;
        }
    noteOff (note);
}

void SynthCore::arpRelease()
{
    if (arpNote >= 0) noteOff (arpNote);
    arpNote = -1; samplesToOff = -1;
}

void SynthCore::arpStep()
{
    static const double beats[] = { 1.0, 0.5, 1.0 / 3.0, 0.25, 1.0 / 6.0, 0.125 };
    const double stepLen = 60.0 / hostBpm * beats[jlimit (0, 5, (int) (arpRateRaw->load() + 0.5f))] * sampleRate;
    samplesToStep += stepLen;
    if (heldCount == 0) return;

    // notas ordenadas de grave a agudo, repetidas en octavas
    std::array<int, 32> sorted {}; std::array<float, 32> vel {};
    for (int i = 0; i < heldCount; ++i) { sorted[(size_t) i] = held[(size_t) i]; vel[(size_t) i] = heldVel[(size_t) i]; }
    for (int i = 1; i < heldCount; ++i)
        for (int k = i; k > 0 && sorted[(size_t) k] < sorted[(size_t) k - 1]; --k)
        { std::swap (sorted[(size_t) k], sorted[(size_t) k - 1]); std::swap (vel[(size_t) k], vel[(size_t) k - 1]); }
    const int octs = 1 + jlimit (0, 2, (int) (arpOctRaw->load() + 0.5f));
    const int total = heldCount * octs;
    const int mode = (int) (arpModeRaw->load() + 0.5f);

    int idx = 0;
    switch (mode)
    {
        case 0: idx = arpIndex % total; ++arpIndex; break;                                   // arriba
        case 1: idx = total - 1 - (arpIndex % total); ++arpIndex; break;                     // abajo
        case 2:                                                                               // arriba-abajo
            if (total == 1) { idx = 0; break; }
            if (arpIndex >= total - 1) arpDir = -1; else if (arpIndex <= 0) arpDir = 1;
            idx = jlimit (0, total - 1, arpIndex); arpIndex += arpDir; break;
        default: idx = arpRandom.nextInt (total); break;                                     // azar
    }
    const int note = sorted[(size_t) (idx % heldCount)] + 12 * (idx / heldCount);

    if (arpNote >= 0) noteOff (arpNote);
    noteOn (jlimit (0, 127, note), vel[(size_t) (idx % heldCount)]);
    arpNote = jlimit (0, 127, note);
    samplesToOff = stepLen * jlimit (0.05, 1.0, (double) arpGateRaw->load());
}

void SynthCore::allNotesOff()
{
    heldCount = 0; arpNote = -1; samplesToOff = -1;
    for (int s = 0; s < maxVoices; ++s)
    {
        voices[(size_t) s] = {};
        for (auto& e : engines) if (e) e->kill (s);
    }
}

void SynthCore::noteOn (int note, float velocity)
{
    // misma nota sonando: se reutiliza la voz
    int slot = -1;
    for (int s = 0; s < maxVoices && slot < 0; ++s)
        if (voices[(size_t) s].note == note && voiceBusy (s)) slot = s;
    // voz libre
    for (int s = 0; s < maxVoices && slot < 0; ++s)
        if (! voiceBusy (s)) slot = s;
    // robar la mas vieja (preferir las que ya se soltaron)
    if (slot < 0)
    {
        uint32 best = std::numeric_limits<uint32>::max();
        for (int pass = 0; pass < 2 && slot < 0; ++pass)
            for (int s = 0; s < maxVoices; ++s)
                if ((pass == 1 || ! voices[(size_t) s].gate) && voices[(size_t) s].age < best)
                { best = voices[(size_t) s].age; slot = s; }
    }

    auto& v = voices[(size_t) slot];
    v.note = note;
    v.gate = true;
    v.sustained = false;
    v.velocity = velocity;
    v.age = ++ageCounter;
    v.pan = (slot % 2 == 0 ? -1.0f : 1.0f) * (0.35f + 0.65f * (float) ((slot / 2) % 3) / 2.0f);

    for (int e = 0; e < numEngines; ++e)
        if (engines[(size_t) e] && engineOn[(size_t) e])
            engines[(size_t) e]->noteOn (slot, (float) note, velocity, lastNote);
    lastNote = (float) note;
}

void SynthCore::noteOff (int note)
{
    for (int s = 0; s < maxVoices; ++s)
    {
        auto& v = voices[(size_t) s];
        if (v.note != note || ! v.gate) continue;
        v.gate = false;
        if (sustainPedal) { v.sustained = true; continue; }
        for (auto& e : engines) if (e) e->noteOff (s);
    }
}

void SynthCore::process (AudioBuffer<float>& buffer, MidiBuffer& midi)
{
    readParameters();

    float* left = buffer.getWritePointer (0);
    float* right = buffer.getNumChannels() > 1 ? buffer.getWritePointer (1) : nullptr;
    const int total = buffer.getNumSamples();

    int pos = 0;
    auto it = midi.cbegin();
    while (pos < total)
    {
        // eventos MIDI en su muestra exacta
        while (it != midi.cend() && (*it).samplePosition <= pos)
        {
            handleMidi ((*it).getMessage());
            ++it;
        }
        // arpegiador: pasos y fin de nota en su muestra exacta
        const bool arpOn = arpOnRaw->load() > 0.5f;
        if (arpOn != arpWasOn)
        {
            arpWasOn = arpOn;
            if (! arpOn) { arpRelease(); heldCount = 0; }
        }
        if (arpOn)
        {
            if (samplesToOff >= 0 && samplesToOff < 1.0) { if (arpNote >= 0) noteOff (arpNote); arpNote = -1; samplesToOff = -1; }
            if (samplesToStep < 1.0) arpStep();
        }

        int end = total;
        if (it != midi.cend()) end = jmin (end, (*it).samplePosition);
        end = jmin (end, pos + maxBlock);
        if (arpOn)
        {
            if (heldCount > 0 || samplesToStep > 0) end = jmin (end, pos + jmax (1, (int) samplesToStep));
            if (samplesToOff >= 0) end = jmin (end, pos + jmax (1, (int) samplesToOff));
        }
        if (end <= pos) end = jmin (total, pos + 1);

        renderChunk (left + pos, right != nullptr ? right + pos : nullptr, end - pos);
        if (arpOn)
        {
            samplesToStep = heldCount > 0 ? samplesToStep - (end - pos) : 0.0;
            if (samplesToOff >= 0) samplesToOff -= (end - pos);
        }
        pos = end;
    }
    while (it != midi.cend()) { handleMidi ((*it).getMessage()); ++it; }

    if (right == nullptr) return;

    // FX globales: CUERPO, CHARACTER, ESPACIO
    enum { body = 0, space = 5 };
    const float bodyGain = (macro[body] - 0.5f) * 2.0f;              // -1..1
    const float bodyCoeff = OnePole::coeffFor (140.0f, (float) sampleRate);
    const float drive = 1.0f + character * character * 9.0f;
    const float makeup = 1.0f / (0.6f + 0.4f * std::sqrt (drive));
    const bool broken = character > 0.75f;
    const float crushAmt = broken ? (character - 0.75f) * 4.0f : 0.0f;  // 0..1
    const float levels = std::exp2 (12.0f - crushAmt * 7.0f);
    const int hold = 1 + (int) (crushAmt * 5.0f);

    for (int i = 0; i < total; ++i)
    {
        float l = left[i], r = right[i];
        l += bodyLowL.lowpass (l, bodyCoeff) * bodyGain * 1.2f;
        r += bodyLowR.lowpass (r, bodyCoeff) * bodyGain * 1.2f;
        l = softClip (l * drive) * makeup;
        r = softClip (r * drive) * makeup;
        if (broken)
        {
            if (++crushCounter >= hold) { crushCounter = 0; crushHoldL = l; crushHoldR = r; }
            l = std::round (crushHoldL * levels) / levels;
            r = std::round (crushHoldR * levels) / levels;
        }
        left[i] = l; right[i] = r;
    }

    // reverb en paralelo (ESPACIO); se saltea si esta en cero
    if (spaceSmooth.getTargetValue() > 0.0005f || spaceSmooth.isSmoothing())
    {
        std::array<float, maxBlock> wetL {}, wetR {};
        for (int startI = 0; startI < total; startI += maxBlock)
        {
            const int n = jmin (maxBlock, total - startI);
            std::copy (left + startI, left + startI + n, wetL.begin());
            std::copy (right + startI, right + startI + n, wetR.begin());
            reverb.processStereo (wetL.data(), wetR.data(), n);
            for (int i = 0; i < n; ++i)
            {
                const float w = spaceSmooth.getNextValue();
                left[startI + i] += wetL[(size_t) i] * w;
                right[startI + i] += wetR[(size_t) i] * w;
            }
        }
    }
    else spaceSmooth.skip (total);

    // limitador suave de salida
    for (int i = 0; i < total; ++i) { left[i] = softClip (left[i]); right[i] = softClip (right[i]); }
    ignoreUnused (space);
}

void SynthCore::renderChunk (float* left, float* right, int n)
{
    if (modMatrix != nullptr) modMatrix->advance (n);   // LFOs y valores modulados de este tramo

    // LFO global (MOVIMIENTO y rueda de modulacion): ~5.2 Hz
    const float inc = 5.2f / (float) sampleRate;
    for (int i = 0; i < n; ++i)
    {
        lfoBuf[(size_t) i] = std::sin (lfoPhase * twoPi);
        lfoPhase += inc; if (lfoPhase >= 1.0f) lfoPhase -= 1.0f;
    }
    mod.lfo = lfoBuf.data();

    enum { width = 4 };
    const float widthAmt = macro[width];

    // niveles por motor (suavizados) para este tramo
    std::array<float, numEngines> lvl {};
    for (int e = 0; e < numEngines; ++e)
    {
        lvl[(size_t) e] = levelSmooth[e].getNextValue();
        levelSmooth[e].skip (n - 1);
    }

    for (int e = 0; e < numEngines; ++e)
    {
        auto* eng = engines[(size_t) e].get();
        if (eng == nullptr) continue;
        if (! engineOn[(size_t) e] && lvl[(size_t) e] <= 0.0005f)
        {
            // motor apagado: corta sus voces y no procesa nada
            for (int s = 0; s < maxVoices; ++s) if (eng->isActive (s)) eng->kill (s);
            tail[(size_t) e] = 0;
            continue;
        }

        bool any = false;
        for (int s = 0; s < maxVoices; ++s)
        {
            if (! eng->isActive (s)) continue;
            if (! any)
            {
                std::fill (busL.begin(), busL.begin() + n, 0.0f);
                std::fill (busR.begin(), busR.begin() + n, 0.0f);
                eng->beginBlock (n);
                any = true;
            }
            std::fill (engineBuf.begin(), engineBuf.begin() + n, 0.0f);
            eng->render (s, engineBuf.data(), n, mod);
            const float pan = voices[(size_t) s].pan * widthAmt;
            const float gl = std::sqrt (0.5f * (1.0f - pan)), gr = std::sqrt (0.5f * (1.0f + pan));
            for (int i = 0; i < n; ++i) { busL[(size_t) i] += engineBuf[(size_t) i] * gl; busR[(size_t) i] += engineBuf[(size_t) i] * gr; }
        }

        if (any) tail[(size_t) e] = (int) (eng->tailSeconds() * sampleRate);
        else if (tail[(size_t) e] > 0)
        {
            std::fill (busL.begin(), busL.begin() + n, 0.0f);
            std::fill (busR.begin(), busR.begin() + n, 0.0f);
            tail[(size_t) e] -= n;
        }
        else continue;

        eng->processBus (busL.data(), busR.data(), n);
        const float g = lvl[(size_t) e];
        for (int i = 0; i < n; ++i)
        {
            left[i] += busL[(size_t) i] * g;
            if (right != nullptr) right[i] += busR[(size_t) i] * g;
        }
    }
}
}
