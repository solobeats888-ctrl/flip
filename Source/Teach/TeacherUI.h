#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "Lessons.h"
#include <deque>

namespace flipper
{
/**
    MAESTRO: se abre tocando el retrato del panel PERSONAJE.

    1) PREGUNTAR: el personaje pregunta qué sonido querés; escribís (o tocás un ejemplo) y
       aparecen las recetas que coinciden.
    2) LECCION: ilumina un control, explica qué hace y por qué, y espera a que lo muevas
       al rango pedido. Solo la zona iluminada recibe el mouse; el resto queda bloqueado.
    Se dibuja en el espacio del tablero (1024 x 1536).
*/
class TeacherOverlay : public juce::Component, private juce::Timer
{
public:
    explicit TeacherOverlay (juce::AudioProcessorValueTreeState&);
    ~TeacherOverlay() override;

    // conexiones con el editor
    std::function<juce::Rectangle<int> (const juce::String& target)> resolve;   // zona en el tablero
    std::function<void (const teach::Lesson&)> prepare;                          // motor, modo DISENO, sonido base
    std::function<int()> currentEngine;
    std::function<int()> heldNotes;
    std::function<void()> onClose;

    void openAsk();
    void startLesson (const teach::Lesson&, const std::vector<const teach::Modifier*>& mods);
    void close();
    bool isOpen() const noexcept { return phase != Phase::closed; }
    bool inLesson() const noexcept { return phase == Phase::lesson; }

    // para pruebas y capturas
    void setQuery (const juce::String&);
    const teach::Result& lastResult() const noexcept { return result; }
    int stepCount() const noexcept { return (int) steps.size(); }
    int stepIndex() const noexcept { return index; }
    const teach::Step* currentStep() const { return index >= 0 && index < (int) steps.size() ? &steps[(size_t) index] : nullptr; }
    juce::Rectangle<int> currentHole() const { return hole.toNearestInt(); }
    bool satisfied() const;
    void showMe();
    void next();
    void previous();

    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseMove (const juce::MouseEvent&) override;
    bool hitTest (int x, int y) override;
    bool keyPressed (const juce::KeyPress&) override;

private:
    enum class Phase { closed, ask, lesson };
    void timerCallback() override;
    void layoutLesson();
    void refreshResults();
    void paintAsk (juce::Graphics&);
    void paintLesson (juce::Graphics&);
    void paintMedal (juce::Graphics&, juce::Rectangle<float>, int engine);
    void paintButton (juce::Graphics&, juce::Rectangle<float>, const juce::String&, bool lit, bool enabled);
    juce::String statusLine (bool& ok) const;
    bool condMet (const teach::Cond&) const;
    float realValue (const juce::String& id) const;

    juce::AudioProcessorValueTreeState& state;
    Phase phase = Phase::closed;

    // PREGUNTAR
    juce::TextEditor input;
    teach::Result result;
    juce::Rectangle<float> askCard, closeBtn;
    std::vector<std::pair<juce::Rectangle<float>, juce::String>> chips;
    std::vector<std::pair<juce::Rectangle<float>, const teach::Lesson*>> rows;
    int hoverRow = -1;

    // LECCION
    const teach::Lesson* lesson = nullptr;
    std::vector<teach::Step> steps;
    std::deque<juce::String> texts;   // textos armados en tiempo real (deque: no mueve sus elementos)
    const char* keep (const juce::String&);
    int index = 0;
    juce::Rectangle<float> hole, card, prevBtn, exitBtn, showBtn, nextBtn;
    bool wasOk = false, okAtEntry = false;
    juce::uint32 okSince = 0;
    juce::String lastStatus;
    bool lastOk = false;
};
}
