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
#include "Teach/TeacherUI.h"
#include "UI/ModUI.h"

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

/**
    Cartel de lectura: nombre y valor del control que esta bajo el mouse (o que estas moviendo),
    dibujado a tamano real de pantalla, fuera del tablero escalado. Asi se lee aunque la ventana sea chica.
*/
class ReadoutBubble : public juce::Component
{
public:
    ReadoutBubble() { setInterceptsMouseClicks (false, false); setAlwaysOnTop (true); }
    void show (const juce::String& title, const juce::String& value, const juce::String& extra, juce::Colour accent, juce::Point<int> anchor);
    void paint (juce::Graphics&) override;
private:
    juce::String title, value, extra;
    juce::Colour accent;
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
    flipper::TeacherOverlay& getTeacher() { return teacher; }
    void openTeacher();
    /** Zona de un control por su clave de ayuda (id de parametro o "ui:..."), si esta a la vista. */
    juce::Rectangle<int> findControl (const juce::String& helpKey, bool reveal = true);

    /** FL Studio y otros DAW preguntan que parametro hay bajo el mouse (Link to controller). */
    int getControlParameterIndex (juce::Component&) override;
    flipper::ModOverlay& getModOverlay() { return modOverlay; }
    /** Crea una modulacion soltando una fuente en un punto del tablero (tambien para pruebas). */
    bool dropModulation (int source, juce::Point<float> boardPoint);
    juce::String paramAt (juce::Point<float> boardPoint);
    /** Para capturas: muestra el cartel de lectura de un control como si el mouse estuviera encima. */
    void showReadoutFor (const juce::String& key);

private:
    void timerCallback() override;
    void changeListenerCallback (juce::ChangeBroadcaster*) override;   // cambio de idioma
    void mouseDown (const juce::MouseEvent&) override; // foto para DESHACER antes de tocar un control
    void generate();
    void showVariations();
    void saveFavorite();
    void refreshEngineStates (bool force);
    void prepareLesson (const flipper::teach::Lesson&);
    juce::String paramKeyOf (juce::Component*) const;
    void showParamMenu (const juce::String& paramId, juce::Point<int> screenPos);
    std::vector<std::pair<juce::String, juce::Rectangle<int>>> modulatableControls();

    struct RightClick : juce::MouseListener
    {
        explicit RightClick (FlipperEditor& e) : ed (e) {}
        void mouseDown (const juce::MouseEvent&) override;
        FlipperEditor& ed;
    } rightClick { *this };

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
    flipper::TeacherOverlay teacher;
    flipper::ModPanel modPanel;
    flipper::ModOverlay modOverlay;
    ReadoutBubble readout;
    juce::String readoutKey;
    juce::uint32 readoutSince = 0;
    void updateReadout();
    juce::String forcedReadout;
    juce::Point<int> forcedAnchor;
    std::map<juce::String, int> paramIndex;
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
