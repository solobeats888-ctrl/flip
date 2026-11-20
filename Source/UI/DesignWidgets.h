#pragma once
#include "VintageControls.h"
#include <juce_audio_processors/juce_audio_processors.h>

namespace flipper
{
/** Identidad visual de cada motor en DISENO (paleta del personaje + forma de la maquina). */
struct Identity
{
    juce::Colour accent, panelTop, panelBottom;
    VintageKnob::Look look;
    bool wood;
    const char* subtitle;
    const char* help;       // una linea: por donde empezar
};
const Identity& identityFor (int engine);

/** Botones de seleccion (formas, pies, modos) o un interruptor iluminado si el parametro es SI/NO. */
class ChoiceButtons : public juce::Component
{
public:
    ChoiceButtons (juce::RangedAudioParameter&, juce::String label, const Identity&, juce::UndoManager*);
    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    static float preferredWeight (const juce::RangedAudioParameter&);

private:
    juce::Rectangle<float> buttonRect (int i) const;
    juce::RangedAudioParameter& param;
    juce::StringArray options;
    juce::String label;
    const Identity& id;
    bool isSwitch;
    int index = 0;
    juce::ParameterAttachment attachment;
};

/** Pantalla LCD con flechas, para listas largas (muestras del M1, algoritmos del DX7, origen y destino de cables). */
class LcdSelector : public juce::Component
{
public:
    LcdSelector (juce::RangedAudioParameter&, juce::String label, const Identity&, juce::UndoManager*);
    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails&) override;
    std::function<void()> onChange;
    int getIndex() const noexcept { return index; }

private:
    void step (int delta);
    juce::RangedAudioParameter& param;
    juce::String label;
    const Identity& id;
    int index = 0, count = 1, offset = 0;
    juce::ParameterAttachment attachment;
};

/**
    Pagina generica de un motor: paneles con filas de controles armadas desde los ids de parametro.
    Elige solo el control segun el tipo: perilla o fader (con la forma de la maquina), botones para
    pocas opciones, LCD para listas largas.
*/
class EnginePage : public juce::Component
{
public:
    struct Item { juce::String id; juce::String label; };
    struct Group { juce::String title; std::vector<Item> items; };

    EnginePage (int engine, juce::AudioProcessorValueTreeState&, juce::MouseListener& history);

    /** Fila de grupos en y (alto automatico). Devuelve el alto usado. */
    float row (float y, std::vector<Group> groups, float height = 0.0f);
    /** Un control suelto en un rectangulo. */
    juce::Component& place (const juce::String& paramId, const juce::String& label, juce::Rectangle<int>, juce::Colour cap = {});
    void paint (juce::Graphics&) override;
    /** Zona de una banda por su titulo (para los recorridos guiados). */
    juce::Rectangle<int> findBand (const juce::String& title) const
    {
        for (auto& b : bands) if (b.title == title) return b.area.toNearestInt().expanded (4);
        return {};
    }

    const int engine;
    const Identity& id;
    juce::AudioProcessorValueTreeState& state;
    juce::MouseListener& history;

    /** Controles creados por una llamada (para mostrar/ocultar grupos, ej. operadores del DX7). */
    std::vector<juce::Component*> lastCreated;

protected:
    struct Band { juce::String title; juce::Rectangle<float> area; };
    std::vector<Band> bands;
    std::vector<std::unique_ptr<juce::Component>> owned;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> attachments;
    int capIndex = 0;
};

/** Construye la pagina DISENO de los motores 1..8 (el MINIMOOG tiene su panel propio). */
std::unique_ptr<EnginePage> createEnginePage (int engine, juce::AudioProcessorValueTreeState&, juce::MouseListener& history);
}
