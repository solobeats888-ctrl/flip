#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "Theme.h"

namespace flipper
{
/** Perilla con la imagen recortada de la referencia: la textura gira con el valor. */
class SpriteKnob : public juce::Slider
{
public:
    SpriteKnob (const char* spriteResource, float faceRadius, float arcRadius, juce::String label = {});

    /** Centro de la perilla en coordenadas locales. */
    juce::Point<float> centre;
    /** Etiqueta: distancia bajo el centro y altura de mayuscula (0 = estilo de las macros). */
    float labelOffset = 54.0f, labelCap = 0.0f;
    /** Muestra siempre el valor en una segunda linea (pantalla DISENO). */
    bool valueLine = false;

    void paint (juce::Graphics&) override;
    void mouseEnter (const juce::MouseEvent&) override;
    void mouseExit (const juce::MouseEvent&) override;

private:
    juce::Image sprite;
    float faceRadius, arcRadius;
    juce::String label;
    bool hovering = false;
};

/** LED de activacion de un motor (sobre el LED pintado del panel). */
class EngineLed : public juce::ToggleButton
{
public:
    EngineLed() { setClickingTogglesState (true); setMouseCursor (juce::MouseCursor::PointingHandCursor); }
    void paintButton (juce::Graphics&, bool highlighted, bool down) override;
    float activity = 0.0f; // 0..1, brillo extra cuando el motor esta sonando
};

/** Capsula amarilla de nivel del panel MEZCLA DE MOTORES. */
class LevelPill : public juce::Slider
{
public:
    LevelPill();
    void paint (juce::Graphics&) override;
    bool enabledLook = true;
};

/** Boton GENERAR / VARIACIONES / FAVORITO: el boton fisico es parte del arte, el texto y los estados los pinta JUCE. */
class ArcadeButton : public juce::Button, private juce::Timer
{
public:
    enum class Style { lit, dark };
    ArcadeButton (const juce::String& text, Style, juce::Rectangle<float> textArea);
    void paintButton (juce::Graphics&, bool highlighted, bool down) override;
    void flash();

private:
    void timerCallback() override;
    juce::String text;
    Style style;
    juce::Rectangle<float> textArea; // local
    juce::uint32 flashStart = 0;
};

/** Boton de modo (JUGAR / DISENO / LAB). */
class ModeButton : public juce::Button
{
public:
    ModeButton (const juce::String& text, juce::Point<float> ledCentre, float ledRadius,
                juce::Rectangle<float> textArea);
    void paintButton (juce::Graphics&, bool highlighted, bool down) override;

private:
    juce::String text;
    juce::Point<float> ledCentre;
    float ledRadius;
    juce::Rectangle<float> textArea;
    juce::Image ledOn;
};

/** Boton de chapa para LAB (normal o encendido en mostaza). */
class LabButton : public juce::Button
{
public:
    explicit LabButton (const juce::String& text, bool lit = false);
    void paintButton (juce::Graphics&, bool highlighted, bool down) override;
    bool lit;
};

/** Interruptor con lamparita (bloqueos, solo favoritos). */
class LabToggle : public juce::ToggleButton
{
public:
    explicit LabToggle (const juce::String& text);
    void paintButton (juce::Graphics&, bool highlighted, bool down) override;
};

/** Menues, combos y campos de texto con la estetica del plugin. */
class FlipperLookAndFeel : public juce::LookAndFeel_V4
{
public:
    FlipperLookAndFeel();
    juce::Font getPopupMenuFont() override { return theme::font (14.0f); }
    juce::Font getComboBoxFont (juce::ComboBox&) override { return theme::font (14.0f); }
    void drawComboBox (juce::Graphics&, int width, int height, bool down,
                       int, int, int, int, juce::ComboBox&) override;
    void positionComboBoxText (juce::ComboBox&, juce::Label&) override;
    void fillTextEditorBackground (juce::Graphics&, int width, int height, juce::TextEditor&) override;
    void drawTextEditorOutline (juce::Graphics&, int width, int height, juce::TextEditor&) override;
    juce::Rectangle<int> getTooltipBounds (const juce::String&, juce::Point<int> screenPos, juce::Rectangle<int> parentArea) override;
    void drawTooltip (juce::Graphics&, const juce::String& text, int width, int height) override;
    static juce::TextLayout tooltipLayout (const juce::String&, float width);
};
}
