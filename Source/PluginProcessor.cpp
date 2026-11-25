#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Core/FactoryPresets.h"

using namespace flipper;
using Gen = flipper::SoundGenerator;

FlipperProcessor::FlipperProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      state (*this, &undoManager, "FLIPPER", flipper::createLayout())
{
    mod = std::make_unique<flipper::ModMatrix> (state);
    synth = std::make_unique<flipper::dsp::SynthCore> (state);
    presets.setFactory (flipper::buildFactoryPresets (state));
    state.state.setProperty (flipper::ids::selectedEngine, 0, nullptr); // arranca con KROKO (el unico motor encendido)
    state.state.setProperty (flipper::ids::uiMode, (int) flipper::UiMode::play, nullptr);
    state.state.setProperty (flipper::ids::animOn, true, nullptr);
    state.state.setProperty (flipper::ids::sfxOn, false, nullptr);
}

void FlipperProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    sfx.prepare (sampleRate);
    mod->prepare (sampleRate);
    synth->prepare (sampleRate, samplesPerBlock);
    noteDown.fill (false);
    noteCount = 0;
    heldNotes.store (0);
}

bool FlipperProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto out = layouts.getMainOutputChannelSet();
    return out == juce::AudioChannelSet::stereo() || out == juce::AudioChannelSet::mono();
}

void FlipperProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;

    buffer.clear();

    for (const auto metadata : midi)
    {
        const auto msg = metadata.getMessage();
        if (msg.isNoteOn())
        {
            const int n = msg.getNoteNumber();
            if (! noteDown[(size_t) n]) { noteDown[(size_t) n] = true; ++noteCount; }
        }
        else if (msg.isNoteOff())
        {
            const int n = msg.getNoteNumber();
            if (noteDown[(size_t) n]) { noteDown[(size_t) n] = false; --noteCount; }
        }
        else if (msg.isAllNotesOff() || msg.isAllSoundOff())
        {
            noteDown.fill (false);
            noteCount = 0;
        }
    }
    heldNotes.store (noteCount, std::memory_order_relaxed);

    // tempo del DAW para el arpegiador y los LFO
    double bpm = 0.0, ppq = -1.0;
    bool playing = false;
    if (auto* ph = getPlayHead())
        if (auto pos = ph->getPosition())
        {
            if (auto b = pos->getBpm()) { bpm = *b; synth->setBpm (*b); }
            if (auto q = pos->getPpqPosition()) ppq = *q;
            playing = pos->getIsPlaying();
        }

    // valores modulados (MOD A-D, LFO 1-2) para este bloque
    mod->beginBlock (bpm, playing, ppq);

    // los nueve motores (solo procesan los encendidos)
    synth->process (buffer, midi);

    // sonidos del tablero (solo si el usuario los activo en LAB)
    sfx.render (buffer, buffer.getNumSamples());
}

void FlipperProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    if (auto xml = state.copyState().createXml())
        copyXmlToBinary (*xml, dest);
}

void FlipperProcessor::setStateInformation (const void* data, int size)
{
    if (auto xml = getXmlFromBinary (data, size))
        if (xml->hasTagName (state.state.getType()))
        {
            state.replaceState (juce::ValueTree::fromXml (*xml));
            mod->loadFromState();
            sfx.enabled.store ((bool) state.state.getProperty (flipper::ids::sfxOn, false));
        }
}

//============================================================================== acciones
Gen::Settings FlipperProcessor::generatorSettings() const
{
    Gen::Settings s;
    s.mode = (Gen::Mode) juce::jlimit (0, 2, (int) state.state.getProperty (ids::genMode, (int) Gen::Mode::hybrid));
    s.style = (int) state.state.getProperty (ids::genStyle, 0);
    s.selectedEngine = (int) state.state.getProperty (ids::selectedEngine, 8);
    s.mutateAmount = (float) state.state.getProperty (ids::mutateAmount, 0.25f);
    s.locks = Gen::parseLocks (state.state.getProperty (ids::locks).toString());
    return s;
}

void FlipperProcessor::setGeneratorMode (Gen::Mode m) { state.state.setProperty (ids::genMode, (int) m, nullptr); }
void FlipperProcessor::setGeneratorStyle (int s)      { state.state.setProperty (ids::genStyle, s, nullptr); }
void FlipperProcessor::setMutateAmount (float a)      { state.state.setProperty (ids::mutateAmount, a, nullptr); }

void FlipperProcessor::setLocked (const juce::String& id, bool locked)
{
    auto locks = Gen::parseLocks (state.state.getProperty (ids::locks).toString());
    if (locked) locks.insert (id); else locks.erase (id);
    state.state.setProperty (ids::locks, Gen::joinLocks (locks), nullptr);
}

juce::uint32 FlipperProcessor::lastSeed() const
{
    return (juce::uint32) (juce::int64) state.state.getProperty (ids::lastSeed, 0);
}

juce::uint32 FlipperProcessor::generateSound()
{
    const auto seed = (juce::uint32) juce::Random::getSystemRandom().nextInt (999999) + 1;
    generateSound (seed);
    return seed;
}

void FlipperProcessor::generateSound (juce::uint32 seed)
{
    // el sonido generado que estabas usando se guarda con tus cambios antes de pasar al nuevo
    if (genIndex >= 0 && genIndex < (int) generated.size()) generated[(size_t) genIndex] = Gen::capture (state);
    applyWithHistory (Gen::generate (seed, generatorSettings(), state));
    state.state.setProperty (ids::lastSeed, (juce::int64) seed, nullptr);

    if (genIndex >= 0) generated.resize ((size_t) genIndex + 1);   // lo que habia "adelante" se descarta
    generated.push_back (Gen::capture (state));
    if (generated.size() > 50) generated.erase (generated.begin());
    genIndex = (int) generated.size() - 1;
}

bool FlipperProcessor::previousGenerated()
{
    if (genIndex <= 0 || genIndex >= (int) generated.size()) return false;
    generated[(size_t) genIndex] = Gen::capture (state);
    --genIndex;
    applyWithHistory (generated[(size_t) genIndex]);
    return true;
}

bool FlipperProcessor::nextGenerated()
{
    if (genIndex < 0 || genIndex >= (int) generated.size() - 1) return false;
    generated[(size_t) genIndex] = Gen::capture (state);
    ++genIndex;
    applyWithHistory (generated[(size_t) genIndex]);
    return true;
}

void FlipperProcessor::mutateSound()
{
    const auto seed = (juce::uint32) juce::Random::getSystemRandom().nextInt (999999) + 1;
    applyWithHistory (Gen::mutate (state, seed, generatorSettings()));
}

Snapshot FlipperProcessor::variation (juce::uint32 seed) const
{
    return Gen::mutate (state, seed, generatorSettings());
}

void FlipperProcessor::pushHistory() { history.push (Gen::capture (state)); }

void FlipperProcessor::applyWithHistory (const Snapshot& s)
{
    pushHistory();
    Gen::apply (state, s);
}

bool FlipperProcessor::undo()
{
    if (auto target = history.undo (Gen::capture (state))) { Gen::apply (state, *target); return true; }
    return false;
}

bool FlipperProcessor::redo()
{
    if (auto target = history.redo (Gen::capture (state))) { Gen::apply (state, *target); return true; }
    return false;
}

void FlipperProcessor::switchAB (int slot)
{
    slot = juce::jlimit (0, 1, slot);
    if (slot == abSlot) return;
    abSlots[(size_t) abSlot] = Gen::capture (state);
    if (abSlots[(size_t) slot].empty()) abSlots[(size_t) slot] = abSlots[(size_t) abSlot];
    abSlot = slot;
    applyWithHistory (abSlots[(size_t) slot]);
}

void FlipperProcessor::copyABToOther()
{
    abSlots[(size_t) (1 - abSlot)] = Gen::capture (state);
}

juce::AudioProcessorEditor* FlipperProcessor::createEditor()
{
    return new FlipperEditor (*this);
}

#if ! FLIPPER_SNAPSHOT
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new FlipperProcessor();
}
#endif
