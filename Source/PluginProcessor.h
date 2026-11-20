#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "Core/Parameters.h"
#include "Core/History.h"
#include "Core/PresetManager.h"
#include "Core/PinballSfx.h"
#include "DSP/SynthCore.h"
#include <atomic>

class FlipperProcessor : public juce::AudioProcessor
{
public:
    FlipperProcessor();
    ~FlipperProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout&) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "FLIPPER"; }
    bool acceptsMidi() const override  { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    juce::UndoManager undoManager;
    juce::AudioProcessorValueTreeState state;

    //==== acciones de sonido (solo hilo de mensajes) ===========================
    flipper::SoundGenerator::Settings generatorSettings() const;
    void setGeneratorMode (flipper::SoundGenerator::Mode);
    void setGeneratorStyle (int);
    void setMutateAmount (float);
    void setLocked (const juce::String& lockId, bool);

    juce::uint32 generateSound();                    // semilla nueva
    void generateSound (juce::uint32 seed);          // semilla dada (reproducible)
    void mutateSound();
    flipper::Snapshot variation (juce::uint32 seed) const;
    juce::uint32 lastSeed() const;

    void pushHistory();
    bool undo();
    bool redo();
    void applyWithHistory (const flipper::Snapshot&);

    int  getABSlot() const noexcept { return abSlot; }
    void switchAB (int slot);
    void copyABToOther();

    flipper::PresetManager presets;
    flipper::PinballSfx sfx;
    flipper::History history;

    /** Cantidad de notas MIDI sostenidas (lo escribe el hilo de audio, lo lee la UI). */
    int getHeldNotes() const noexcept { return heldNotes.load (std::memory_order_relaxed); }

private:
    std::unique_ptr<flipper::dsp::SynthCore> synth;
    std::array<flipper::Snapshot, 2> abSlots;
    int abSlot = 0;

    std::array<bool, 128> noteDown {};
    int noteCount = 0;
    std::atomic<int> heldNotes { 0 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FlipperProcessor)
};
