#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "../Core/Parameters.h"
#include <functional>

namespace flipper::anim
{
/** Sonidos del tablero que pide la animacion (los genera el procesador si estan activados). */
enum class Sfx { launch = 0, impact, power, bonus };

/** Fases de la maquina de estados de cada personaje. */
enum class Phase { idle, launch, hit, power, bonus, reset };

/**
    Capa de animacion del tablero (1024 x 1536, no recibe clicks).

    - Cada personaje tiene su propia maquina de estados: IDLE -> LAUNCH -> HIT -> POWER -> BONUS -> RESET.
    - El cuerpo del personaje es una "capa" recortada del arte aprobado (mismo dibujo, sin
      reinterpretar) que se deforma, rebota y gira; las piezas sueltas (craneo, gorra, cabeza)
      son recortes del mismo arte. Objetos y efectos salen de las hojas de animacion.
    - La bola es la bola metalica del arte original; su trayectoria es simulada (sin fisica real
      todavia): cada personaje tiene punto de impacto y direccion propios.
    - Cooldown por personaje; un personaje nunca dispara a otro.
    - El tiempo se avanza con advance(): en el plugin lo hace un Timer, en las pruebas un reloj fijo.
*/
class AnimationLayer : public juce::Component, private juce::Timer
{
public:
    AnimationLayer();
    ~AnimationLayer() override;

    void trigger (int engine);                 // click en un personaje
    void setIdleEngine (int engine);           // el seleccionado respira
    void setEnabled (bool);                    // ANIMACIONES on/off
    bool isAnimating (int engine) const;
    Phase phaseOf (int engine) const;

    std::function<void (Sfx, int engine)> onSfx;

    /** Avanza la simulacion hasta nowMs (ms). Publico para pruebas y capturas. */
    void advance (double nowMs);
    void useManualClock (bool manual) { manualClock = manual; if (manual) stopTimer(); }

    void paint (juce::Graphics&) override;

    static constexpr double cooldownMs = 900.0;

private:
    struct CharState
    {
        bool active = false;
        double start = 0.0;          // ms del click
        double cooldownUntil = 0.0;
        int bonusShown = 0;
        bool sfxImpact = false, sfxPower = false, sfxBonus = false, spawnedPower = false, spawnedLate = false;
        juce::Point<float> ballDir;  // direccion de la bola al impactar
        juce::uint32 seed = 0;
        double lastSpawn = 0.0;
    };

    struct Particle
    {
        bool alive = false;
        int owner = -1, sprite = -1, kind = 0; // kind: 0 sprite, 1 chispa, 2 anillo
        juce::Point<float> p, v;
        float age = 0, life = 1, size = 20, grow = 0, rot = 0, vrot = 0, gravity = 0, drag = 0;
        juce::Colour colour;
    };

    struct Popup
    {
        bool alive = false;
        juce::Point<float> p;
        double start = 0;
        juce::String text, extra;
    };

    struct Patch { juce::Image image; juce::Rectangle<float> area; };

    void timerCallback() override;
    void ensureTimer();
    void buildPatches();

    // fases
    double phaseLength (int engine, Phase) const;
    Phase phaseAt (int engine, double elapsed, double& u) const;
    juce::Point<float> ballPosition (int engine, double elapsed, float& scale, float& alpha) const;

    // dibujo
    void paintCharacter (juce::Graphics&, int engine, double elapsed);
    void paintIdle (juce::Graphics&, int engine);
    void paintPatch (juce::Graphics&, const Patch&, const juce::AffineTransform&, float alpha = 1.0f);
    void paintHole (juce::Graphics&, juce::Rectangle<float>, juce::Colour, float alpha);
    void paintPowerFx (juce::Graphics&, int engine, float u, double elapsed);
    void paintBall (juce::Graphics&, juce::Point<float>, float scale, float alpha, juce::Point<float> trailDir);
    void paintPopup (juce::Graphics&, const Popup&);
    juce::AffineTransform bodyTransform (int engine, Phase, float u, double elapsed) const;

    // particulas
    Particle* spawn (int owner);
    void spawnSprite (int owner, int sprite, juce::Point<float> p, juce::Point<float> v, float size, float life,
                      float gravity = 0, float vrot = 0, float grow = 0, float drag = 0);
    void spawnSpark (int owner, juce::Point<float> p, juce::Point<float> v, juce::Colour, float size, float life, float gravity = 0);
    void spawnRing (int owner, juce::Point<float> p, juce::Colour, float size, float grow, float life);
    void burst (int owner, juce::Point<float> p, int count, juce::Colour, float speed);
    void spawnPowerParticles (int engine);
    void spawnLateParticles (int engine);
    void clearOwner (int owner);

    juce::Rectangle<int> dirtyArea() const;

    std::array<CharState, numEngines> chars;
    std::array<Particle, 220> particles;
    std::array<Popup, 10> popups;
    std::array<Patch, numEngines> bodies;
    Patch junoHead, dxCap, arpSkull;
    std::vector<juce::Image> sprites;

    juce::Random rng { 1974 };
    double now = 0.0, lastAdvance = 0.0, lastIdleSpawn = 0.0;
    int idleEngine = 8;
    bool enabled = true, manualClock = false;
    juce::Rectangle<int> lastDirty;
};
}
