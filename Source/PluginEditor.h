#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "UI/Controls.h"
#include "UI/Playfield.h"
#include "UI/CharacterPanel.h"
#include "UI/LabPanel.h"
#include "UI/DesignPanel.h"
#include "Anim/AnimationLayer.h"
#include "Help/HelpUI.h"

/** Tablero de 1024 x 1536 (espacio de la imagen aprobada); el editor lo escala. */
class FlipperBoard : public juce::Component
{
public:
    FlipperBoard();
    void paint (juce::Graphics&) override;
    void invalidateCache() { cacheValid = false; }

    std::array<bool, flipper::numEngines> engineOn {};

private:
    void paintContent (juce::Graphics&);
    juce::Image background, cache;
    float cacheScale = 0.0f;
    bool cacheValid = false;
};

class FlipperEditor : public juce::AudioProcessorEditor, private juce::Timer, private juce::ChangeListener
{
public:
    explicit FlipperEditor (FlipperProcessor&);
    ~FlipperEditor() override;

    void resized() override;

    // usados por la herramienta de captura
    void selectEngine (int engine);
    void setMode (flipper::UiMode);
    void startTour();
    void tourNext() { tour.next(); }
    juce::Rectangle<int> resolveTarget (const juce::String&);
    void selectEngineForTest (int e) { selectEngine (e); }
    flipper::anim::AnimationLayer& getAnimationLayer() { return animLayer; }

private:
    void timerCallback() override;
    void changeListenerCallback (juce::ChangeBroadcaster*) override;   // cambio de idioma
    void mouseDown (const juce::MouseEvent&) override; // foto para DESHACER antes de tocar un control
    void generate();
    void showVariations();
    void saveFavorite();
    void refreshEngineStates (bool force);

    FlipperProcessor& proc;
    juce::AudioProcessorValueTreeState& state;
    flipper::FlipperLookAndFeel lnf;

    FlipperBoard board;
    flipper::Playfield playfield;
    flipper::anim::AnimationLayer animLayer;
    flipper::CharacterPanel characterPanel;
    flipper::LabPanel labPanel;
    flipper::DesignPanel designPanel;

    std::array<std::unique_ptr<flipper::ModeButton>, 3> modeButtons;
    std::array<std::unique_ptr<flipper::EngineLed>, flipper::numEngines> leds;
    std::array<std::unique_ptr<flipper::LevelPill>, flipper::numEngines> pills;
    std::array<std::unique_ptr<flipper::SpriteKnob>, flipper::numMacros> macroKnobs;
    flipper::ArcadeButton generateButton, variationsButton, favoriteButton;
    flipper::HelpButton helpButton;
    flipper::TourOverlay tour;
    std::unique_ptr<flipper::HelpTooltipWindow> tooltipWindow;

    using SliderAtt = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAtt = juce::AudioProcessorValueTreeState::ButtonAttachment;
    std::vector<std::unique_ptr<SliderAtt>> sliderAtts;
    std::vector<std::unique_ptr<ButtonAtt>> buttonAtts;

    std::array<juce::RangedAudioParameter*, flipper::numEngines> onParams {}, levelParams {};
    float activityPhase = 0.0f;
    bool lastActive = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FlipperEditor)
};
