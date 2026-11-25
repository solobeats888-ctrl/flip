#pragma once
#include "Controls.h"
#include "../PluginProcessor.h"

namespace flipper
{
/**
    Pantalla LAB: explorador de presets, generador (modos, estilo, semilla, mutar,
    sorpresa, deshacer/rehacer, A/B) y bloqueos. Todo actua sobre el mismo estado
    que JUGAR: cambiar de modo no cambia el sonido.
*/
class LabPanel : public juce::Component, private juce::ListBoxModel
{
public:
    static juce::Rectangle<int> boardBounds() { return { 188, 196, 632, 1064 }; }

    explicit LabPanel (FlipperProcessor&);
    void refresh();            // al mostrarse o cuando cambian presets / ajustes
    void languageChanged();
    void paint (juce::Graphics&) override;
    void resized() override;

    std::function<void (bool)> onAnimationsChanged;

private:
    // ListBoxModel
    int getNumRows() override { return (int) filtered.size(); }
    void paintListBoxItem (int row, juce::Graphics&, int width, int height, bool selected) override;
    void listBoxItemClicked (int row, const juce::MouseEvent&) override;

    void applyFilters();
    void fillCombos();
    void refreshGenerator();
    void savePreset();

    FlipperProcessor& proc;
    std::vector<int> filtered;
    juce::String loadedName; bool loadedFactory = false;

    juce::TextEditor search, nameField, tagsField, seedField;
    juce::ComboBox categoryFilter, engineFilter, characterFilter, saveCategory, styleBox;
    LabToggle favOnly { "SOLO FAVORITOS" };
    LabButton slotA { "A" }, slotB { "B" }, copyAB { "COPIAR A/B" }, saveButton { "GUARDAR", true };
    juce::ListBox list;

    std::array<std::unique_ptr<LabButton>, 3> modeButtons;
    LabButton useSeed { "USAR SEMILLA" };
    LevelPill mutateAmount;
    juce::String genStatus() const;
    LabButton genPrev { "<" }, genNext { ">" };
    LabButton generate { "GENERAR", true }, mutate { "MUTAR" }, surprise { "SORPRESA" },
              undo { "DESHACER" }, redo { "REHACER" };

    LabToggle animToggle { "ANIMACIONES" }, sfxToggle { "SONIDOS DEL TABLERO" };
    LabButton langEs { "ES" }, langEn { "EN" };
    std::vector<std::unique_ptr<LabToggle>> locks;
    juce::StringArray lockIds;

    juce::String status;
};
}
