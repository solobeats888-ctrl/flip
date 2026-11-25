#pragma once
#include <juce_core/juce_core.h>
#include <vector>

/**
    MAESTRO de FLIPPER: recetas de diseño de sonido que el personaje enseña paso a paso.

    Cada receta es para un motor. Cada paso ilumina un control (o una banda) y espera a que
    el usuario lo lleve a un rango. Todo funciona sin internet: el buscador compara las
    palabras que escribe el usuario con las palabras clave de cada receta (castellano e ingles).
*/
namespace flipper::teach
{
/** Condicion de un paso: el parametro (en unidades reales) tiene que quedar entre lo y hi.
    "set" es el valor que pone MOSTRAME. param "@note" = esperar a que toque una nota. */
struct Cond { const char* param; float lo, hi, set; };

struct Step
{
    const char* target;          // id de control ("help"), "band:TITULO", "rect:x,y,w,h" o "" (sin zona)
    std::vector<Cond> conds;     // vacio = paso de explicacion
    const char* titleEs; const char* es;
    const char* titleEn; const char* en;
};

struct Lesson
{
    const char* id;
    int engine;                  // 0..8
    const char* titleEs; const char* titleEn;
    const char* keywords;        // palabras clave sin tildes (es + en), separadas por espacios
    const char* blurbEs; const char* blurbEn;
    std::vector<Step> steps;
    bool basics = false;         // aparece en PRIMEROS PASOS
};

const std::vector<Lesson>& lessons();
const Lesson* find (const juce::String& id);

/** Saludo del personaje de cada motor. */
juce::String greeting (int engine);

/** Palabras que cambian el resultado (oscuro, sucio, largo...): se agregan como pasos al final. */
struct Modifier
{
    const char* words;           // sin tildes, separadas por espacios
    const char* labelEs; const char* labelEn;
    Step step;
};
const std::vector<Modifier>& modifiers();

struct Result
{
    std::vector<const Lesson*> lessons;       // mejores primero
    std::vector<const Modifier*> modifiers;   // detectados en el pedido
    bool understood = false;                  // hubo al menos una receta que coincide
};

/** Busca recetas para lo que escribio el usuario. */
Result search (const juce::String& text);

/** minusculas, sin tildes ni signos. */
juce::String normalise (const juce::String&);
}
