#include "Help.h"
#include "I18n.h"
#include "../Core/Parameters.h"
#include "../DSP/Engines.h"
#include <map>

namespace flipper::help
{
using namespace juce;

namespace
{
struct Text { const char* titleEs; const char* es; const char* titleEn; const char* en; };

/**
    Claves: id exacto del parametro, "motor:clave" (clave sin el prefijo del motor, con # en lugar
    del numero de oscilador/operador/canal/cable), clave generica, o "ui:..." para botones.
*/
const std::map<String, Text>& table()
{
    static const std::map<String, Text> t {
        // ------------------------------------------------------------------ JUGAR
        { "ui:mode_play",   { "JUGAR", "Pantalla principal: elegí personajes, mezclá motores y movés las macros. Acá se hace casi todo.",
                              "PLAY", "Main screen: pick characters, mix engines and move the macros. Most sounds are made here." } },
        { "ui:mode_design", { "DISEÑO", "Los controles completos del motor elegido, con la forma de su sintetizador original.",
                              "DESIGN", "The full controls of the selected engine, shaped like its original synthesizer." } },
        { "ui:mode_lab",    { "LAB", "Presets, generador de sonidos, bloqueos, comparación A/B, idioma y opciones.",
                              "LAB", "Presets, sound generator, locks, A/B comparison, language and options." } },
        { "ui:character",   { "PERSONAJE", "Click: lo elegís (su motor aparece en DISEÑO y en el panel PERSONAJE) y dispara su animación. Cada personaje es un motor de síntesis.",
                              "CHARACTER", "Click: select it (its engine shows in DESIGN and in the CHARACTER panel) and trigger its animation. Each character is a synth engine." } },
        { "ui:engine_on",   { "ENCENDER MOTOR", "Prende o apaga este motor. Podés tener los nueve sonando a la vez. Apagado no gasta CPU.",
                              "ENGINE ON/OFF", "Turns this engine on or off. All nine can play at once. When off it uses no CPU." } },
        { "ui:engine_level",{ "NIVEL DEL MOTOR", "Volumen de este motor en la mezcla. Arrastrá a los costados. Doble click = 70 %.",
                              "ENGINE LEVEL", "This engine's volume in the mix. Drag sideways. Double click = 70%." } },
        { "ui:teacher",     { "MAESTRO", "Tocá el retrato (o el globo) y pedile un sonido: bajo 808, pad oscuro, campana… Te enseña a armarlo paso a paso con las perillas de verdad.",
                              "TEACHER", "Click the portrait (or the bubble) and ask for a sound: 808 bass, dark pad, bell… It teaches you to build it step by step with the real knobs." } },
        { "ui:teacher_input", { "¿QUÉ SONIDO QUERÉS?", "Escribí lo que buscás, en castellano o en inglés. Podés sumar cómo lo querés: oscuro, sucio, largo, con espacio…",
                                "WHAT SOUND DO YOU WANT?", "Type what you're after, in English or Spanish. Add how you want it: dark, dirty, long, spacious…" } },
        { "character",      { "CHARACTER", "Saturación de todo el plugin: LIMPIO, CÁLIDO, SUCIO y ROTO (este último además reduce bits).",
                              "CHARACTER", "Saturation for the whole plugin: CLEAN, WARM, DIRTY and BROKEN (which also reduces bits)." } },
        { "macro_body",      { "CUERPO", "Graves: a la derecha más peso, a la izquierda más fino.", "BODY", "Low end: right adds weight, left makes it thinner." } },
        { "macro_brightness",{ "BRILLO", "Abre o cierra los filtros de todos los motores (en el DX7, más o menos brillo FM).",
                               "BRIGHTNESS", "Opens or closes the filters of every engine (on the DX7, more or less FM brightness)." } },
        { "macro_movement",  { "MOVIMIENTO", "Agrega vibrato y movimiento de filtro con un LFO. En cero el sonido queda quieto.",
                               "MOVEMENT", "Adds vibrato and filter motion with an LFO. At zero the sound stays still." } },
        { "macro_texture",   { "TEXTURA", "Suma ruido y desafinación entre osciladores: más sucio y orgánico.",
                               "TEXTURE", "Adds noise and detune between oscillators: grittier and more organic." } },
        { "macro_width",     { "ANCHURA", "Abre las notas en el estéreo. En cero, todo al centro.", "WIDTH", "Spreads notes across the stereo field. At zero, everything is centered." } },
        { "macro_space",     { "ESPACIO", "Reverb de todo el plugin.", "SPACE", "Reverb for the whole plugin." } },
        { "macro_attack",    { "ATAQUE", "Hace que las notas entren más lento (derecha) o más rápido (izquierda) en todos los motores.",
                               "ATTACK", "Makes notes fade in slower (right) or faster (left) on every engine." } },
        { "macro_release",   { "LIBERACIÓN", "Cuánto queda sonando la nota después de soltarla, en todos los motores.",
                               "RELEASE", "How long notes ring after you let go, on every engine." } },
        { "ui:generate",    { "GENERAR", "Crea un sonido nuevo con el modo, estilo y bloqueos elegidos en LAB. Sin internet. Click derecho: volver al sonido generado anterior (con lo que le cambiaste) o ir al siguiente. En LAB también están las flechas < y >.",
                              "GENERATE", "Creates a new sound using the mode, style and locks set in LAB. No internet needed. Right click: go back to the previous generated sound (with your changes) or forward to the next. LAB also has the < and > arrows." } },
        { "ui:gen_prev",    { "GENERADO ANTERIOR", "Vuelve al sonido generado anterior, con los cambios que le habías hecho. El que dejás también queda guardado con tus cambios.",
                              "PREVIOUS GENERATED", "Goes back to the previous generated sound, with the changes you made to it. The one you leave is kept with your changes too." } },
        { "ui:gen_next",    { "GENERADO SIGUIENTE", "Va al siguiente sonido generado (si volviste atrás).",
                              "NEXT GENERATED", "Goes to the next generated sound (if you went back)." } },
        { "ui:variations",  { "VARIACIONES", "Cuatro versiones parecidas al sonido actual para elegir.", "VARIATIONS", "Four versions close to the current sound to choose from." } },
        { "ui:favorite",    { "FAVORITO", "Guarda el sonido actual como preset favorito (aparece en LAB con ★).",
                              "FAVORITE", "Saves the current sound as a favorite preset (shown in LAB with ★)." } },
        { "ui:help",        { "AYUDA", "Abre el recorrido guiado de esta pantalla. Pasando el mouse por cualquier control también ves qué hace.",
                              "HELP", "Opens the guided tour for this screen. Hovering any control also tells you what it does." } },
        // ------------------------------------------------------------------ LAB
        { "ui:search",      { "BUSCAR", "Escribí parte del nombre o un tag.", "SEARCH", "Type part of a name or a tag." } },
        { "ui:filter",      { "FILTRO", "Mostrá solo presets de una categoría, un motor o un carácter.", "FILTER", "Show only presets of one category, engine or character." } },
        { "ui:favonly",     { "SOLO FAVORITOS", "Muestra solo los presets marcados con ★.", "FAVORITES ONLY", "Shows only presets marked with ★." } },
        { "ui:list",        { "PRESETS", "Un click carga el preset. La ★ lo marca como favorito. F = de fábrica (no se borran).",
                              "PRESETS", "One click loads a preset. The ★ marks it as favorite. F = factory (can't be deleted)." } },
        { "ui:ab",          { "A / B", "Dos sonidos para comparar: editá en A, pasá a B, probá otra idea y volvé.",
                              "A / B", "Two sounds to compare: edit in A, switch to B, try another idea and go back." } },
        { "ui:copyab",      { "COPIAR A/B", "Copia el sonido actual en la otra memoria.", "COPY A/B", "Copies the current sound into the other slot." } },
        { "ui:save",        { "GUARDAR", "Guarda el sonido con nombre, categoría y tags (separados por coma).",
                              "SAVE", "Saves the sound with a name, category and tags (comma separated)." } },
        { "ui:gen_mode",    { "MODO DEL GENERADOR", "CLÁSICO: solo el motor elegido. ESTILO: un carácter (oscuro, brillante…). HÍBRIDO: mezcla de 2 a 4 motores.",
                              "GENERATOR MODE", "CLASSIC: only the selected engine. STYLE: a mood (dark, bright…). HYBRID: blends 2 to 4 engines." } },
        { "ui:style",       { "ESTILO", "El tipo de sonido que busca el modo ESTILO.", "STYLE", "The kind of sound STYLE mode goes for." } },
        { "ui:seed",        { "SEMILLA", "Cada sonido generado tiene un número. La misma semilla da siempre el mismo sonido: anotala si te gustó.",
                              "SEED", "Every generated sound has a number. The same seed always gives the same sound: write it down if you like it." } },
        { "ui:mutate_amt",  { "INTENSIDAD", "Cuánto cambia MUTAR y las VARIACIONES.", "AMOUNT", "How much MUTATE and VARIATIONS change." } },
        { "ui:mutate",      { "MUTAR", "Cambia un poco el sonido actual, sin empezar de cero.", "MUTATE", "Changes the current sound a little, without starting over." } },
        { "ui:surprise",    { "SORPRESA", "Modo y estilo al azar y un sonido nuevo.", "SURPRISE ME", "Random mode and style and a new sound." } },
        { "ui:undo",        { "DESHACER", "Vuelve al sonido anterior (hasta 100 pasos): perillas, generador y presets.",
                              "UNDO", "Goes back to the previous sound (up to 100 steps): knobs, generator and presets." } },
        { "ui:redo",        { "REHACER", "Rehace lo que deshiciste.", "REDO", "Redoes what you undid." } },
        { "ui:lock",        { "BLOQUEO", "Lo bloqueado no cambia al GENERAR, MUTAR ni en VARIACIONES. Ideal para quedarte con un bajo y buscar el resto.",
                              "LOCK", "Locked items don't change on GENERATE, MUTATE or VARIATIONS. Great to keep a bass and search for the rest." } },
        { "ui:anim",        { "ANIMACIONES", "Prende o apaga las animaciones de los personajes. Apagadas, la interfaz usa menos CPU.",
                              "ANIMATIONS", "Turns character animations on or off. Off uses less CPU." } },
        { "ui:sfx",         { "SONIDOS DEL TABLERO", "Sonidos de pinball al tocar los personajes (golpe, poder, bonus). Vienen apagados.",
                              "TABLE SOUNDS", "Pinball sounds when you hit the characters (impact, power, bonus). Off by default." } },
        { "ui:lang",        { "IDIOMA", "Cambia el idioma de todo el plugin: castellano o inglés.", "LANGUAGE", "Changes the whole plugin's language: Spanish or English." } },
        // ------------------------------------------------------------------ ARPEGIADOR
        { "arp_on",   { "ARPEGIADOR", "Toca las notas que mantenés apretadas una por una, al tempo del DAW. Vale para todos los motores encendidos.",
                        "ARPEGGIATOR", "Plays the notes you hold one at a time, at the DAW tempo. Works on every engine that is on." } },
        { "arp_mode", { "MODO", "Orden de las notas: arriba, abajo, arriba-abajo o al azar.", "MODE", "Note order: up, down, up-down or random." } },
        { "arp_rate", { "VALOR", "Duración de cada paso: de negras (1/4) a fusas (1/32). T = tresillo.", "RATE", "Step length: from quarter notes (1/4) to 1/32. T = triplet." } },
        { "arp_oct",  { "OCTAVAS", "En cuántas octavas repite el patrón.", "OCTAVES", "How many octaves the pattern spans." } },
        { "arp_gate", { "GATE", "Qué tan largas suenan las notas: corto = staccato.", "GATE", "How long each note lasts: short = staccato." } },
        // ------------------------------------------------------------------ MODULACION
        { "mod_a", { "MOD A", "Una perilla que mueve todas las perillas que le arrastraste, cada una con su cantidad. Automatizala o linkeala a tu controlador para mover varias cosas juntas.",
                         "MOD A", "One knob that moves every knob you dragged it onto, each by its own amount. Automate it or link it to your controller to move several things together." } },
        { "mod_b", { "MOD B", "Una perilla que mueve todas las perillas que le arrastraste, cada una con su cantidad. Automatizala o linkeala a tu controlador para mover varias cosas juntas.",
                         "MOD B", "One knob that moves every knob you dragged it onto, each by its own amount. Automate it or link it to your controller to move several things together." } },
        { "mod_c", { "MOD C", "Una perilla que mueve todas las perillas que le arrastraste, cada una con su cantidad. Automatizala o linkeala a tu controlador para mover varias cosas juntas.",
                         "MOD C", "One knob that moves every knob you dragged it onto, each by its own amount. Automate it or link it to your controller to move several things together." } },
        { "mod_d", { "MOD D", "Una perilla que mueve todas las perillas que le arrastraste, cada una con su cantidad. Automatizala o linkeala a tu controlador para mover varias cosas juntas.",
                         "MOD D", "One knob that moves every knob you dragged it onto, each by its own amount. Automate it or link it to your controller to move several things together." } },
        { "lfo1_rate", { "VELOCIDAD DEL LFO 1", "Qué tan rápido sube y baja. En LIBRE va en Hz; con un valor de tempo, sigue al DAW.",
                             "LFO 1 RATE", "How fast it goes up and down. On FREE it's in Hz; with a tempo value it follows the DAW." } },
        { "lfo1_shape", { "FORMA DEL LFO 1", "Seno suave, triángulo, sierra (sube y cae de golpe), cuadrada (salta entre dos valores) o azar (un valor nuevo en cada ciclo).",
                              "LFO 1 SHAPE", "Smooth sine, triangle, saw (rises then drops), square (jumps between two values) or random (a new value every cycle)." } },
        { "lfo1_sync", { "TEMPO DEL LFO 1", "LIBRE usa la velocidad en Hz. Las figuras (1/16 … 4 COMPASES) lo atan al tempo y a la grilla del DAW.",
                             "LFO 1 SYNC", "FREE uses the rate in Hz. The note values (1/16 … 4 BARS) lock it to the DAW tempo and grid." } },
        { "lfo2_rate", { "VELOCIDAD DEL LFO 2", "Qué tan rápido sube y baja. En LIBRE va en Hz; con un valor de tempo, sigue al DAW.",
                             "LFO 2 RATE", "How fast it goes up and down. On FREE it's in Hz; with a tempo value it follows the DAW." } },
        { "lfo2_shape", { "FORMA DEL LFO 2", "Seno suave, triángulo, sierra (sube y cae de golpe), cuadrada (salta entre dos valores) o azar (un valor nuevo en cada ciclo).",
                              "LFO 2 SHAPE", "Smooth sine, triangle, saw (rises then drops), square (jumps between two values) or random (a new value every cycle)." } },
        { "lfo2_sync", { "TEMPO DEL LFO 2", "LIBRE usa la velocidad en Hz. Las figuras (1/16 … 4 COMPASES) lo atan al tempo y a la grilla del DAW.",
                             "LFO 2 SYNC", "FREE uses the rate in Hz. The note values (1/16 … 4 BARS) lock it to the DAW tempo and grid." } },
        { "ui:mod_source", { "ARRASTRAR MODULACIÓN", "Arrastrá esta fuente hasta cualquier perilla (de DISEÑO o de JUGAR) para que la mueva. Podés arrastrarla a todas las que quieras.",
                             "DRAG MODULATION", "Drag this source onto any knob (in DESIGN or PLAY) to make it move that knob. Drag it onto as many as you like." } },
        { "ui:mod_badge", { "MODULACIÓN", "Arrastrá arriba o abajo para cambiar cuánto mueve (hacia abajo la invierte). Doble click o click derecho para quitarla.",
                            "MODULATION", "Drag up or down to change how much it moves (down inverts it). Double click or right click to remove it." } },
        // ------------------------------------------------------------------ GENERICOS (todos los motores)
        { "attack",   { "ATAQUE", "Tiempo que tarda la nota en llegar al máximo. Corto = golpe, largo = entra suave.",
                        "ATTACK", "Time for the note to reach full level. Short = punchy, long = fades in." } },
        { "decay",    { "CAÍDA", "Tiempo que tarda en bajar del máximo al nivel de sostén.", "DECAY", "Time to fall from the peak to the sustain level." } },
        { "sustain",  { "SOSTÉN", "Nivel que se mantiene mientras apretás la tecla. En cero, el sonido se apaga solo (pluck).",
                        "SUSTAIN", "Level held while the key is down. At zero the sound dies on its own (pluck)." } },
        { "release",  { "LIBERACIÓN", "Tiempo que sigue sonando después de soltar la tecla.", "RELEASE", "How long it keeps sounding after you release the key." } },
        { "cutoff",   { "CORTE", "Frecuencia del filtro: bajo = oscuro y redondo, alto = brillante.", "CUTOFF", "Filter frequency: low = dark and round, high = bright." } },
        { "res",      { "RESONANCIA", "Realza la zona del corte. Alta = silbido y carácter ácido.", "RESONANCE", "Boosts around the cutoff. High = whistling, acid character." } },
        { "kbd",      { "SEGUIMIENTO DE TECLADO", "Hace que el filtro se abra en las notas agudas para que no suenen apagadas.",
                        "KEYBOARD TRACKING", "Opens the filter on higher notes so they don't sound dull." } },
        { "noise",    { "RUIDO", "Agrega ruido: aire, viento, platillos o textura.", "NOISE", "Adds noise: air, wind, cymbals or texture." } },
        { "glide",    { "GLIDE", "Las notas se deslizan de una a otra. En cero, saltan.", "GLIDE", "Notes slide from one to the next. At zero they jump." } },
        { "porta",    { "PORTAMENTO", "Las notas se deslizan de una a otra. En cero, saltan.", "PORTAMENTO", "Notes slide from one to the next. At zero they jump." } },
        { "sync",     { "SYNC", "Sincroniza un oscilador con otro: sonido rasgado y agresivo, sobre todo moviendo la afinación.",
                        "SYNC", "Hard-syncs one oscillator to another: tearing, aggressive tone, especially when you sweep the tuning." } },
        { "ring",     { "MODULADOR EN ANILLO", "Multiplica dos señales: tonos metálicos y de campana.", "RING MODULATOR", "Multiplies two signals: metallic, bell-like tones." } },
        { "lfo_rate", { "VELOCIDAD DEL LFO", "Qué tan rápido oscila el LFO (la modulación lenta).", "LFO RATE", "How fast the LFO (slow modulation) moves." } },
        { "pw",       { "ANCHO DE PULSO", "Cambia el timbre de la onda de pulso: 50 % hueco, extremos finos y nasales.",
                        "PULSE WIDTH", "Changes the pulse wave tone: 50% hollow, extremes thin and nasal." } },
        { "level",    { "NIVEL", "Volumen de esta parte.", "LEVEL", "Volume of this part." } },
        // ------------------------------------------------------------------ MINIMOOG
        { "minimoog:osc#_range",  { "RANGO", "Octava del oscilador, como los pies de un órgano: 32' muy grave, 2' muy agudo.",
                                    "RANGE", "Oscillator octave, like organ feet: 32' very low, 2' very high." } },
        { "minimoog:osc#_wave",   { "FORMA DE ONDA", "Triángulo suave, sierra brillante, cuadrada hueca, pulsos más nasales. Click en un ícono para saltar ahí.",
                                    "WAVEFORM", "Soft triangle, bright saw, hollow square, nasal pulses. Click an icon to jump there." } },
        { "minimoog:osc#_detune", { "FRECUENCIA", "Afinación de este oscilador. Un poco = sonido gordo; 7 = quinta; 12 en otro rango = octava.",
                                    "FREQUENCY", "This oscillator's tuning. A little = fat sound; 7 = fifth." } },
        { "minimoog:mix#",        { "VOLUMEN DEL OSCILADOR", "Cuánto entra este oscilador al filtro.", "OSCILLATOR VOLUME", "How much of this oscillator goes into the filter." } },
        { "minimoog:emphasis",    { "ÉNFASIS", "La resonancia del Minimoog: realza el corte, hasta silbar.", "EMPHASIS", "The Minimoog's resonance: boosts the cutoff, up to whistling." } },
        { "minimoog:contour",     { "CANTIDAD DE CONTORNO", "Cuánto abre el filtro su envolvente: lo que le da el golpe al bajo.",
                                    "CONTOUR AMOUNT", "How much the filter envelope opens the filter: what gives the bass its punch." } },
        { "minimoog:drive",       { "SATURACIÓN", "Sobrecarga la entrada del filtro: más grueso y sucio.", "OVERLOAD", "Overdrives the filter input: thicker and dirtier." } },
        { "minimoog:f_attack",    { "ATAQUE DEL FILTRO", "Qué tan rápido se abre el filtro.", "FILTER ATTACK", "How fast the filter opens." } },
        { "minimoog:f_decay",     { "CAÍDA DEL FILTRO", "Qué tan rápido se cierra después del golpe. Corto = pluck.", "FILTER DECAY", "How fast it closes after the hit. Short = pluck." } },
        { "minimoog:f_sustain",   { "SOSTÉN DEL FILTRO", "Cuánto queda abierto mientras mantenés la tecla.", "FILTER SUSTAIN", "How open it stays while you hold the key." } },
        // ------------------------------------------------------------------ PROPHET-5
        { "prophet5:a_saw",    { "SIERRA (OSC A)", "Prende la sierra del oscilador A. Se puede sumar con el pulso.", "SAW (OSC A)", "Turns on oscillator A's saw. Can be combined with the pulse." } },
        { "prophet5:a_pulse",  { "PULSO (OSC A)", "Prende el pulso del oscilador A.", "PULSE (OSC A)", "Turns on oscillator A's pulse." } },
        { "prophet5:b_semi",   { "SEMITONOS (OSC B)", "Afinación del oscilador B: 7 = quinta, 12 = octava, 19 = octava + quinta.",
                                 "SEMITONES (OSC B)", "Oscillator B tuning: 7 = fifth, 12 = octave, 19 = octave + fifth." } },
        { "prophet5:b_fine",   { "FINO (OSC B)", "Desafinación leve: dos osciladores apenas desafinados suenan gordos.", "FINE (OSC B)", "Slight detune: two slightly detuned oscillators sound fat." } },
        { "prophet5:b_saw",    { "SIERRA (OSC B)", "Prende la sierra del oscilador B.", "SAW (OSC B)", "Turns on oscillator B's saw." } },
        { "prophet5:b_tri",    { "TRIÁNGULO (OSC B)", "Prende el triángulo del oscilador B (suave, ideal para poly-mod).", "TRIANGLE (OSC B)", "Turns on oscillator B's triangle (soft, great for poly-mod)." } },
        { "prophet5:b_pulse",  { "PULSO (OSC B)", "Prende el pulso del oscilador B.", "PULSE (OSC B)", "Turns on oscillator B's pulse." } },
        { "prophet5:mix_a",    { "OSC A", "Volumen del oscilador A.", "OSC A", "Oscillator A volume." } },
        { "prophet5:mix_b",    { "OSC B", "Volumen del oscilador B.", "OSC B", "Oscillator B volume." } },
        { "prophet5:fenv",     { "ENV DEL FILTRO", "Cuánto abre el filtro su envolvente.", "FILTER ENV", "How much the envelope opens the filter." } },
        { "prophet5:pm_env",   { "POLY-MOD: ENV FILTRO", "La envolvente del filtro mueve la afinación de A o el filtro: barridos y campanas.",
                                 "POLY-MOD: FILTER ENV", "The filter envelope moves A's pitch or the filter: sweeps and bells." } },
        { "prophet5:pm_oscb",  { "POLY-MOD: OSC B", "El oscilador B modula a A (FM): metálico, campanas, sonidos raros.",
                                 "POLY-MOD: OSC B", "Oscillator B modulates A (FM): metallic, bells, strange tones." } },
        { "prophet5:pm_freqa", { "POLY-MOD > FREC A", "Manda el poly-mod a la frecuencia del oscilador A.", "POLY-MOD > FREQ A", "Sends poly-mod to oscillator A's frequency." } },
        { "prophet5:pm_filter",{ "POLY-MOD > FILTRO", "Manda el poly-mod al filtro.", "POLY-MOD > FILTER", "Sends poly-mod to the filter." } },
        { "prophet5:lfo_freq", { "LFO > FRECUENCIA", "Vibrato con el LFO.", "LFO > FREQUENCY", "Vibrato from the LFO." } },
        { "prophet5:lfo_pw",   { "LFO > ANCHO", "El LFO mueve el ancho de pulso: sonido que respira (PWM).", "LFO > PW", "The LFO moves the pulse width: a breathing sound (PWM)." } },
        { "prophet5:lfo_filter",{ "LFO > FILTRO", "El LFO abre y cierra el filtro (wah).", "LFO > FILTER", "The LFO opens and closes the filter (wah)." } },
        { "prophet5:unison",   { "UNÍSONO", "Tres osciladores por nota: muy gordo, ideal para leads y bajos.", "UNISON", "Three oscillators per note: huge, great for leads and basses." } },
        { "prophet5:uni_detune",{ "DESAFINACIÓN DEL UNÍSONO", "Cuánto se separan las copias del unísono.", "UNISON DETUNE", "How far apart the unison copies are." } },
        { "prophet5:f_attack", { "ATAQUE DEL FILTRO", "Qué tan rápido se abre el filtro.", "FILTER ATTACK", "How fast the filter opens." } },
        { "prophet5:f_decay",  { "CAÍDA DEL FILTRO", "Qué tan rápido se cierra después del golpe.", "FILTER DECAY", "How fast it closes after the hit." } },
        { "prophet5:f_sustain",{ "SOSTÉN DEL FILTRO", "Cuánto queda abierto mientras mantenés la tecla.", "FILTER SUSTAIN", "How open it stays while you hold the key." } },
        // ------------------------------------------------------------------ JUPITER-8
        { "jupiter8:vco#_wave",  { "FORMA DEL VCO", "Elegí la forma de onda. En el VCO-2, RUIDO lo convierte en fuente de ruido.",
                                   "VCO WAVE", "Pick the waveform. On VCO-2, NOISE turns it into a noise source." } },
        { "jupiter8:vco#_range", { "PIES", "Octava del VCO: 16' grave, 2' agudo.", "FEET", "VCO octave: 16' low, 2' high." } },
        { "jupiter8:vco2_fine",  { "FINO", "Desafina el VCO-2 apenas para engordar.", "FINE", "Slightly detunes VCO-2 to fatten the sound." } },
        { "jupiter8:cross",      { "CROSS MOD", "El VCO-2 modula la frecuencia del VCO-1: sonidos metálicos y agresivos.",
                                   "CROSS MOD", "VCO-2 modulates VCO-1's frequency: metallic, aggressive tones." } },
        { "jupiter8:lfo_pwm",    { "LFO PWM", "El LFO mueve el ancho de pulso.", "LFO PWM", "The LFO moves the pulse width." } },
        { "jupiter8:mix",        { "VCO 1 / 2", "Balance entre los dos VCO.", "VCO 1 / 2", "Balance between both VCOs." } },
        { "jupiter8:hpf",        { "PASAALTOS", "Saca graves antes del filtro: más fino, ideal para no ensuciar el bajo.",
                                   "HIGH-PASS", "Removes lows before the filter: thinner, keeps your bass clean." } },
        { "jupiter8:slope",      { "PENDIENTE", "12 dB = filtro más suave y abierto; 24 dB = más redondo y profundo.",
                                   "SLOPE", "12 dB = gentler, more open filter; 24 dB = rounder and deeper." } },
        { "jupiter8:env_amt",    { "ENV-1", "Cuánto abre el filtro la envolvente 1.", "ENV-1", "How much envelope 1 opens the filter." } },
        { "jupiter8:lfo_wave",   { "FORMA DEL LFO", "Seno y triángulo suaves, sierra y cuadrada marcadas, AZAR = sample & hold.",
                                   "LFO WAVE", "Sine and triangle smooth, saw and square stepped, RANDOM = sample & hold." } },
        { "jupiter8:lfo_vco",    { "LFO > VCO", "Vibrato.", "LFO > VCO", "Vibrato." } },
        { "jupiter8:lfo_vcf",    { "LFO > VCF", "El LFO mueve el filtro.", "LFO > VCF", "The LFO moves the filter." } },
        { "jupiter8:e1_attack",  { "ENV-1 ATAQUE", "Envolvente del filtro: qué tan rápido se abre.", "ENV-1 ATTACK", "Filter envelope: how fast it opens." } },
        { "jupiter8:e1_decay",   { "ENV-1 CAÍDA", "Envolvente del filtro: qué tan rápido se cierra.", "ENV-1 DECAY", "Filter envelope: how fast it closes." } },
        { "jupiter8:e1_sustain", { "ENV-1 SOSTÉN", "Envolvente del filtro: cuánto queda abierto.", "ENV-1 SUSTAIN", "Filter envelope: how open it stays." } },
        { "jupiter8:e1_release", { "ENV-1 LIBERACIÓN", "Envolvente del filtro al soltar.", "ENV-1 RELEASE", "Filter envelope after release." } },
        { "jupiter8:e2_attack",  { "ENV-2 ATAQUE", "Envolvente del volumen: entrada de la nota.", "ENV-2 ATTACK", "Volume envelope: note fade-in." } },
        { "jupiter8:e2_decay",   { "ENV-2 CAÍDA", "Envolvente del volumen: caída.", "ENV-2 DECAY", "Volume envelope: decay." } },
        { "jupiter8:e2_sustain", { "ENV-2 SOSTÉN", "Envolvente del volumen: nivel mientras apretás.", "ENV-2 SUSTAIN", "Volume envelope: level while held." } },
        { "jupiter8:e2_release", { "ENV-2 LIBERACIÓN", "Envolvente del volumen: cola al soltar.", "ENV-2 RELEASE", "Volume envelope: tail after release." } },
        { "jupiter8:zone",       { "ZONA DEL TECLADO", "TODO = suena en todo el teclado. GRAVES/AGUDOS = solo debajo o arriba del split. Así hacés un bajo con PITER y otra cosa arriba con otro motor.",
                                   "KEY ZONE", "ALL = plays on the whole keyboard. LOW/HIGH = only below or above the split. Play a bass with PITER and something else on top with another engine." } },
        { "jupiter8:split",      { "SPLIT", "Nota donde se divide el teclado (60 = Do central).", "SPLIT", "Note where the keyboard splits (60 = middle C)." } },
        // ------------------------------------------------------------------ JUNO
        { "juno:saw",      { "SIERRA", "Prende la sierra del DCO.", "SAW", "Turns on the DCO saw." } },
        { "juno:pulse",    { "PULSO", "Prende el pulso del DCO.", "PULSE", "Turns on the DCO pulse." } },
        { "juno:pwm_src",  { "FUENTE PWM", "MANUAL = el ancho queda fijo; LFO = el ancho se mueve solo (el sonido Juno que respira).",
                             "PWM SOURCE", "MANUAL = fixed width; LFO = the width moves on its own (the breathing Juno sound)." } },
        { "juno:sub",      { "SUB", "Onda cuadrada una octava abajo: engorda bajos y pads.", "SUB", "Square wave one octave down: fattens basses and pads." } },
        { "juno:range",    { "PIES", "Octava del DCO.", "FEET", "DCO octave." } },
        { "juno:hpf",      { "PASAALTOS", "0 = todo el grave; 1 a 3 sacan cada vez más graves.", "HIGH-PASS", "0 = full low end; 1 to 3 remove more and more lows." } },
        { "juno:env_amt",  { "ENV", "Cuánto abre el filtro la envolvente.", "ENV", "How much the envelope opens the filter." } },
        { "juno:lfo_vcf",  { "LFO > VCF", "El LFO mueve el filtro.", "LFO > VCF", "The LFO moves the filter." } },
        { "juno:lfo_delay",{ "RETARDO DEL LFO", "El LFO entra de a poco después de tocar la nota (vibrato que aparece tarde).",
                             "LFO DELAY", "The LFO fades in after you play the note (late vibrato)." } },
        { "juno:lfo_dco",  { "LFO > DCO", "Vibrato.", "LFO > DCO", "Vibrato." } },
        { "juno:vca_mode", { "MODO DEL VCA", "ENV = el volumen sigue la envolvente; GATE = prendido/apagado como un órgano.",
                             "VCA MODE", "ENV = volume follows the envelope; GATE = on/off like an organ." } },
        { "juno:chorus",   { "CHORUS", "El chorus del Juno: I suave, II más profundo, I+II rápido y brillante. Es la mitad del sonido Juno.",
                             "CHORUS", "The Juno chorus: I gentle, II deeper, I+II fast and shimmering. Half of the Juno sound." } },
        // ------------------------------------------------------------------ CS-80
        { "cs80:l#_feet",   { "PIES DEL CANAL", "Octava de este canal.", "CHANNEL FEET", "This channel's octave." } },
        { "cs80:l#_saw",    { "SIERRA", "Volumen de la sierra de este canal.", "SAW", "This channel's saw level." } },
        { "cs80:l#_pulse",  { "PULSO", "Volumen del pulso de este canal.", "PULSE", "This channel's pulse level." } },
        { "cs80:l#_hpf",    { "PASAALTOS", "Saca graves de este canal.", "HIGH-PASS", "Removes lows from this channel." } },
        { "cs80:l#_lpf",    { "PASABAJOS", "Corte del filtro de este canal.", "LOW-PASS", "This channel's filter cutoff." } },
        { "cs80:l#_fenv",   { "ENV DEL FILTRO", "Cuánto abre el filtro la envolvente.", "FILTER ENV", "How much the envelope opens the filter." } },
        { "cs80:balance",   { "BALANCE I / II", "Mezcla entre los dos canales (las dos capas del CS-80).", "BALANCE I / II", "Blend between the two channels (the CS-80's two layers)." } },
        { "cs80:detune",    { "DESAFINACIÓN II", "Desafina el canal II contra el I: el sonido enorme del CS-80.", "DETUNE II", "Detunes channel II against I: the huge CS-80 sound." } },
        { "cs80:ring_depth",{ "PROFUNDIDAD DEL ANILLO", "Cuánto modula el anillo: trémolo o sonido metálico.", "RING DEPTH", "How much the ring modulator acts: tremolo or metallic tone." } },
        { "cs80:ring_speed",{ "VELOCIDAD DEL ANILLO", "Lento = trémolo; rápido = metálico.", "RING SPEED", "Slow = tremolo; fast = metallic." } },
        { "cs80:at_bright", { "AFTERTOUCH > BRILLO", "Si apretás más fuerte la tecla (aftertouch), el sonido se abre.", "AFTERTOUCH > BRIGHTNESS", "Pressing harder on the key (aftertouch) opens the sound." } },
        { "cs80:at_vib",    { "AFTERTOUCH > VIBRATO", "Si apretás más fuerte la tecla, aparece vibrato.", "AFTERTOUCH > VIBRATO", "Pressing harder on the key adds vibrato." } },
        { "cs80:gliss",     { "GLISSANDO", "El portamento sube por semitonos en vez de deslizarse.", "GLISSANDO", "Portamento steps by semitones instead of sliding." } },
        { "cs80_ribbon",    { "CINTA", "Apoyá y deslizá para doblar el tono hasta una octava, como el ribbon del CS-80. Al soltar vuelve.",
                              "RIBBON", "Press and slide to bend the pitch up to an octave, like the CS-80 ribbon. Springs back when released." } },
        { "ui:cs_layer",    { "CANAL I / II", "El CS-80 tiene dos sintes en capa. Elegí cuál editar.", "CHANNEL I / II", "The CS-80 has two layered synths. Pick which one to edit." } },
        // ------------------------------------------------------------------ DX7
        { "dx7_algorithm",  { "ALGORITMO", "Cómo se conectan los 6 operadores. Los de abajo (P) suenan; los de arriba modulan y dan brillo. Click en un operador para editarlo.",
                              "ALGORITHM", "How the 6 operators connect. The bottom ones (C) make the sound; the ones above modulate and add brightness. Click an operator to edit it." } },
        { "ui:dx_op",       { "OPERADOR", "Elegí qué operador editar. P = portador (suena), M = modulador (cambia el timbre).",
                              "OPERATOR", "Pick which operator to edit. C = carrier (makes sound), M = modulator (shapes the tone)." } },
        { "dx7:op#_coarse", { "RATIO", "Relación de frecuencia con la nota (0 = 0,5). En un modulador, 1 = cálido, 2-3 = brillante, 14 = tine de piano eléctrico.",
                              "RATIO", "Frequency ratio to the note (0 = 0.5). On a modulator, 1 = warm, 2-3 = bright, 14 = electric piano tine." } },
        { "dx7:op#_fine",   { "FINO", "Agrega decimales al ratio: inarmónico, campanas.", "FINE", "Adds decimals to the ratio: inharmonic, bells." } },
        { "dx7:op#_detune", { "DESAFINACIÓN", "Desafinación mínima: le da movimiento al sonido.", "DETUNE", "Tiny detune: gives the sound motion." } },
        { "dx7:op#_level",  { "NIVEL", "En un portador = volumen. En un modulador = brillo (subilo despacio).", "LEVEL", "On a carrier = volume. On a modulator = brightness (raise it slowly)." } },
        { "dx7:op#_vel",    { "VELOCIDAD", "Cuánto cambia con la fuerza de la tecla. En moduladores: tocar fuerte = más brillo.",
                              "VELOCITY", "How much it reacts to key velocity. On modulators: hit harder = brighter." } },
        { "dx7:feedback",   { "FEEDBACK", "El operador marcado con el lazo se modula a sí mismo: de suave a ruidoso.", "FEEDBACK", "The looped operator modulates itself: from smooth to noisy." } },
        { "dx7:lfo_pitch",  { "LFO > TONO", "Vibrato.", "LFO > PITCH", "Vibrato." } },
        { "dx7:lfo_amp",    { "LFO > VOLUMEN", "Trémolo.", "LFO > AMP", "Tremolo." } },
        // ------------------------------------------------------------------ M1
        { "m1:s#_wave",     { "MUESTRA", "Elegí la muestra PCM (propias, generadas por síntesis). Flechas o rueda del mouse.", "SAMPLE", "Pick the PCM sample (original, made by synthesis). Arrows or mouse wheel." } },
        { "m1:s#_level",    { "NIVEL", "Volumen de esta fuente.", "LEVEL", "This source's volume." } },
        { "m1:s#_coarse",   { "SEMITONOS", "Transpone esta fuente: 12 = una octava.", "SEMITONES", "Transposes this source: 12 = one octave." } },
        { "m1:s#_fine",     { "CENTS", "Desafinación fina: 5 a 10 cents entre las dos fuentes = sonido más ancho.", "CENTS", "Fine detune: 5 to 10 cents between both sources = wider sound." } },
        { "m1:fenv",        { "ENV DEL FILTRO", "Cuánto abre el filtro su envolvente.", "FILTER ENV", "How much the envelope opens the filter." } },
        { "m1:vel_filter",  { "VELOCIDAD > FILTRO", "Tocar fuerte = más brillante, como un piano.", "VELOCITY > FILTER", "Play harder = brighter, like a piano." } },
        { "m1:lfo_pitch",   { "LFO > TONO", "Vibrato.", "LFO > PITCH", "Vibrato." } },
        { "m1:lfo_filter",  { "LFO > FILTRO", "El LFO mueve el filtro.", "LFO > FILTER", "The LFO moves the filter." } },
        { "m1:fx_type",     { "EFECTO", "Efecto propio del motor M1: chorus, delay o los dos.", "EFFECT", "The M1 engine's own effect: chorus, delay or both." } },
        { "m1:fx_mix",      { "MEZCLA DEL EFECTO", "Cuánto efecto se escucha.", "EFFECT MIX", "How much effect you hear." } },
        { "m1:delay_time",  { "TIEMPO DEL DELAY", "Distancia entre repeticiones.", "DELAY TIME", "Time between repeats." } },
        { "m1:f_attack",    { "ATAQUE DEL FILTRO", "Qué tan rápido se abre el filtro.", "FILTER ATTACK", "How fast the filter opens." } },
        { "m1:f_decay",     { "CAÍDA DEL FILTRO", "Qué tan rápido se cierra.", "FILTER DECAY", "How fast it closes." } },
        { "m1:f_sustain",   { "SOSTÉN DEL FILTRO", "Cuánto queda abierto.", "FILTER SUSTAIN", "How open it stays." } },
        // ------------------------------------------------------------------ ARP 2600
        { "arp2600:vco#_wave", { "FORMA", "Forma de onda de este VCO.", "WAVE", "This VCO's waveform." } },
        { "arp2600:vco#_tune", { "AFINACIÓN", "Afinación en semitonos contra la nota: 7 = quinta, -12 = octava abajo.", "TUNE", "Tuning in semitones from the note: 7 = fifth, -12 = octave down." } },
        { "arp2600:vco2_pw",   { "ANCHO", "Ancho del pulso del VCO 2.", "WIDTH", "VCO 2 pulse width." } },
        { "arp2600:vco3_lf",   { "VCO 3 COMO LFO", "Convierte al VCO 3 en LFO: deja de sonar y modula por cable.", "VCO 3 AS LFO", "Turns VCO 3 into an LFO: it stops sounding and modulates through cables." } },
        { "arp2600:vco3_lfrate",{ "VELOCIDAD DEL LFO", "Velocidad del VCO 3 cuando es LFO.", "LFO RATE", "VCO 3 speed when it's an LFO." } },
        { "arp2600:noise_color",{ "COLOR DEL RUIDO", "Blanco = brillante; rosa = más grave.", "NOISE COLOR", "White = bright; pink = darker." } },
        { "arp2600:filter_mode",{ "MODO DEL FILTRO", "Pasabajos (clásico), pasabanda (nasal) o pasaaltos (fino).", "FILTER MODE", "Low-pass (classic), band-pass (nasal) or high-pass (thin)." } },
        { "arp2600:fenv",      { "ADSR > FILTRO", "Cuánto abre el filtro la envolvente ADSR.", "ADSR > FILTER", "How much the ADSR opens the filter." } },
        { "arp2600:ar_attack", { "AR: ATAQUE", "Segunda envolvente (ataque-liberación), para usar por cable.", "AR: ATTACK", "Second envelope (attack-release), to use through cables." } },
        { "arp2600:ar_release",{ "AR: LIBERACIÓN", "Liberación de la envolvente AR.", "AR: RELEASE", "AR envelope release." } },
        { "arp2600:sh_rate",   { "RELOJ DEL S&H", "Cada cuánto toma un valor al azar el sample & hold. Conectalo al filtro con un cable: sonido de robot.",
                                 "S&H CLOCK", "How often sample & hold grabs a random value. Patch it to the filter: robot sound." } },
        { "arp2600:c#_src",    { "ORIGEN DEL CABLE", "De dónde sale la modulación (flechas para cambiar).", "CABLE SOURCE", "Where the modulation comes from (arrows to change)." } },
        { "arp2600:c#_dst",    { "DESTINO DEL CABLE", "Qué modula el cable.", "CABLE DESTINATION", "What the cable modulates." } },
        { "arp2600:c#_amt",    { "CANTIDAD", "Cuánto modula. En el centro el cable está desconectado; abajo invierte.", "AMOUNT", "How much it modulates. Centered = unplugged; below center inverts." } },
        // ------------------------------------------------------------------ VCS3
        { "vcs3:osc1_shape",  { "FORMA", "Deforma el seno del oscilador 1: de redondo a más agudo.", "SHAPE", "Warps oscillator 1's sine: from round to sharper." } },
        { "vcs3:osc1_sine",   { "SENO", "Volumen del seno del oscilador 1.", "SINE", "Oscillator 1 sine level." } },
        { "vcs3:osc1_saw",    { "SIERRA", "Volumen de la sierra del oscilador 1.", "SAW", "Oscillator 1 saw level." } },
        { "vcs3:osc#_tune",   { "AFINACIÓN", "Afinación en semitonos contra la nota.", "TUNE", "Tuning in semitones from the note." } },
        { "vcs3:osc2_pw",     { "ANCHO", "Ancho del pulso del oscilador 2.", "WIDTH", "Oscillator 2 pulse width." } },
        { "vcs3:osc#_square", { "CUADRADA", "Volumen de la onda cuadrada.", "SQUARE", "Square wave level." } },
        { "vcs3:osc#_tri",    { "TRIÁNGULO", "Volumen del triángulo.", "TRIANGLE", "Triangle level." } },
        { "vcs3:osc3_lf",     { "OSCILADOR 3 COMO LFO", "Encendido: el oscilador 3 es lento y sirve para modular por la matriz.", "OSC 3 AS LFO", "On: oscillator 3 runs slow and modulates through the matrix." } },
        { "vcs3:osc3_rate",   { "VELOCIDAD", "Velocidad del oscilador 3 en modo LFO.", "RATE", "Oscillator 3 speed in LFO mode." } },
        { "vcs3:noise_color", { "COLOR DEL RUIDO", "Más oscuro o más brillante.", "NOISE COLOR", "Darker or brighter." } },
        { "vcs3:res",         { "RESPUESTA", "Resonancia del filtro. Al máximo autooscila y silba solo.", "RESPONSE", "Filter resonance. At max it self-oscillates and whistles." } },
        { "vcs3:env_attack",  { "ATAQUE", "Subida del trapezoide.", "ATTACK", "Trapezoid rise." } },
        { "vcs3:env_on",      { "ENCENDIDO", "Tiempo arriba del trapezoide.", "ON TIME", "Time the trapezoid stays up." } },
        { "vcs3:env_decay",   { "CAÍDA", "Bajada del trapezoide.", "DECAY", "Trapezoid fall." } },
        { "vcs3:env_off",     { "APAGADO", "Pausa antes de repetir (modo REPETIR).", "OFF TIME", "Pause before repeating (REPEAT mode)." } },
        { "vcs3:env_trig",    { "DISPARO", "TECLA = una vez por nota; REPETIR = se repite mientras apretás (ritmos).", "TRIGGER", "KEY = once per note; REPEAT = loops while you hold (rhythms)." } },
        { "vcs3:env_vca",     { "TRAPEZOIDE > VCA", "Cuánto controla el volumen el trapezoide (si no, prendido/apagado).", "TRAPEZOID > VCA", "How much the trapezoid controls volume (otherwise on/off)." } },
        { "vcs3:env_filter",  { "TRAPEZOIDE > FILTRO", "Cuánto abre el filtro el trapezoide.", "TRAPEZOID > FILTER", "How much the trapezoid opens the filter." } },
        { "vcs3:reverb",      { "RESORTE", "Reverb de resorte metálica del VCS3.", "SPRING", "The VCS3's metallic spring reverb." } },
        { "vcs3:depth_pitch", { "PROFUNDIDAD: TONO", "Cuánto mueven los pines de TONO la afinación.", "DEPTH: PITCH", "How much the PITCH pins move the tuning." } },
        { "vcs3:depth_filter",{ "PROFUNDIDAD: CORTE", "Cuánto mueven los pines de CORTE el filtro.", "DEPTH: CUTOFF", "How much the CUTOFF pins move the filter." } },
        { "vcs3:depth_vca",   { "PROFUNDIDAD: VCA", "Cuánto mueven los pines de VCA el volumen.", "DEPTH: VCA", "How much the VCA pins move the volume." } },
        { "ui:joystick",      { "JOYSTICK", "Arrastrá: X e Y son fuentes en la matriz (por defecto X mueve el filtro e Y la afinación del oscilador 1).",
                                "JOYSTICK", "Drag: X and Y are sources in the matrix (by default X moves the filter and Y oscillator 1's pitch)." } },
    };
    return t;
}

String normalise (const String& key)
{
    // op3_decay -> op#_decay, l2_lpf -> l#_lpf, c4_amt -> c#_amt, osc2_wave -> osc#_wave, s1_wave -> s#_wave, mix2 -> mix#
    String out;
    for (int i = 0; i < key.length(); ++i)
    {
        const auto c = key[i];
        const bool digit = CharacterFunctions::isDigit (c);
        const bool afterPrefix = i > 0 && (out.endsWith ("op") || out.endsWith ("osc") || out.endsWith ("vco") || out.endsWith ("mix")
                                 || ((out == "l" || out == "s" || out == "c") && i == 1));
        out += (digit && afterPrefix) ? juce_wchar ('#') : c;
    }
    return out;
}

String format (const Text& t)
{
    return i18n::pick (t.titleEs, t.titleEn) + "\n" + i18n::pick (t.es, t.en);
}

String engineDescription (int e)
{
    static const char* es[] = { "Minimoog: el bajo y lead analógico más clásico.", "Prophet-5: polifónico de los 80, brass y campanas con poly-mod.",
        "Jupiter-8: pads y brass enormes, arpegiador.", "Juno-60/106: pads con chorus y bajos con sub.", "CS-80: dos capas, expresivo, bandas de sonido.",
        "DX7: FM, pianos eléctricos, campanas, bajos punzantes.", "M1: muestras PCM de los 90, pianos, órganos y coros.",
        "ARP 2600: semi-modular con cables, efectos y ruidos.", "VCS3: el más raro: matriz de pines, joystick, sonidos experimentales." };
    static const char* en[] = { "Minimoog: the classic analog bass and lead.", "Prophet-5: 80s polysynth, brass and poly-mod bells.",
        "Jupiter-8: huge pads and brass, arpeggiator.", "Juno-60/106: chorus pads and sub basses.", "CS-80: two layers, expressive, film scores.",
        "DX7: FM, electric pianos, bells, punchy basses.", "M1: 90s PCM samples, pianos, organs and choirs.",
        "ARP 2600: semi-modular with cables, effects and noises.", "VCS3: the weirdest: pin matrix, joystick, experimental sounds." };
    return String::fromUTF8 (i18n::english() ? en[e] : es[e]);
}
}

String tooltip (const String& key)
{
    if (key.isEmpty()) return {};
    const auto& t = table();
    auto find = [&] (const String& k) -> const Text* { auto it = t.find (k); return it != t.end() ? &it->second : nullptr; };

    // personajes del tablero
    if (key.startsWith ("ui:character_"))
    {
        const int e = key.getTrailingIntValue();
        const auto& info = engines()[(size_t) jlimit (0, numEngines - 1, e)];
        return String::fromUTF8 (info.character) + " / " + info.technical + "\n" + engineDescription (e) + "\n"
             + i18n::pick ("Click: elegirlo y verlo en acción. En DISEÑO aparecen sus controles.", "Click: select it and see it in action. Its controls show up in DESIGN.");
    }
    // pines del VCS3
    if (key.startsWith ("pin:"))
    {
        const auto parts = StringArray::fromTokens (key.substring (4), ":", "");
        const int r = parts[0].getIntValue(), c = parts[1].getIntValue();
        const String src = i18n::tr (dsp::VCS3Engine::pinSources()[r]), dst = i18n::tr (dsp::VCS3Engine::pinDestinations()[c]);
        static const char* whatEs[] = { "entra como audio al filtro", "sale directo, sin filtro", "modula la afinación del oscilador 1",
                                        "modula la afinación del oscilador 2", "modula la afinación del oscilador 3", "abre y cierra el filtro", "modula el volumen" };
        static const char* whatEn[] = { "goes into the filter as audio", "goes straight out, unfiltered", "modulates oscillator 1's pitch",
                                        "modulates oscillator 2's pitch", "modulates oscillator 3's pitch", "opens and closes the filter", "modulates the volume" };
        return src + " > " + dst + "\n" + src + " " + String::fromUTF8 (i18n::english() ? whatEn[c] : whatEs[c]) + ". "
             + i18n::pick ("Click para poner o sacar el pin.", "Click to insert or remove the pin.");
    }

    if (const auto* x = find (key)) return format (*x);
    if (key.startsWith ("vcs3_pin_")) return tooltip ("pin:" + key.substring (9, 10) + ":" + key.substring (10, 11));
    if (key == "vcs3_joy_x" || key == "vcs3_joy_y") return format (*find ("ui:joystick"));

    // parametros de motores
    for (int e = 0; e < numEngines; ++e)
    {
        const String prefix = String (engines()[(size_t) e].id) + "_";
        if (key == ids::engineOn (e))    return format (*find ("ui:engine_on"));
        if (key == ids::engineLevel (e)) return format (*find ("ui:engine_level"));
        if (! key.startsWith (prefix)) continue;
        const String rest = key.substring (prefix.length());
        const String eng (engines()[(size_t) e].id);
        for (auto k : { eng + ":" + rest, eng + ":" + normalise (rest), normalise (rest), rest })
            if (const auto* x = find (k)) return format (*x);
        // generico por la ultima palabra (a_attack -> attack, l1_res -> res)
        const String last = rest.fromLastOccurrenceOf ("_", false, false);
        if (const auto* x = find (last)) return format (*x);
        if (rest.contains ("lfo_rate") || rest.endsWith ("rate")) return format (*find ("lfo_rate"));
        if (rest.contains ("level") || rest.startsWith ("mix")) return format (*find ("level"));
    }
    return {};
}

//============================================================================== recorridos
const std::vector<Step>& tour (const String& screen)
{
    static const std::map<String, std::vector<Step>> tours {
        { "play", {
            { "rect:0,0,1024,1536", "BIENVENIDO A FLIPPER", "Un sintetizador de nueve motores con forma de máquina de pinball. Te muestro dónde está cada cosa en un minuto. Podés salir cuando quieras y volver con el botón ?.",
                                    "WELCOME TO FLIPPER", "A nine-engine synthesizer shaped like a pinball machine. Let me show you around in a minute. You can exit anytime and come back with the ? button." },
            { "rect:150,180,700,980", "LOS PERSONAJES SON LOS MOTORES", "Cada personaje es un sintetizador clásico (su nombre está en la placa). Hacé click en uno para elegirlo: reacciona, y sus controles aparecen en DISEÑO.",
                                      "THE CHARACTERS ARE THE ENGINES", "Each character is a classic synthesizer (its name is on the plate). Click one to select it: it reacts, and its controls appear in DESIGN." },
            { "rect:826,244,184,474", "MEZCLA DE MOTORES", "La luz prende o apaga cada motor y la cápsula amarilla es su volumen. Podés tener los nueve sonando a la vez.",
                                      "ENGINE MIX", "The light turns each engine on or off and the yellow capsule is its volume. All nine can play at once." },
            { "rect:858,740,152,435", "PERSONAJE Y CARÁCTER", "Muestra el personaje elegido. Tocá su retrato y pedile un sonido: te enseña a armarlo paso a paso. La perilla de abajo es la saturación de todo el plugin: de LIMPIO a ROTO.",
                                      "CHARACTER", "Shows the selected character. Click its portrait and ask for a sound: it teaches you to build it step by step. The knob below is the saturation of the whole plugin: from CLEAN to BROKEN." },
            { "rect:24,1320,790,148", "LAS OCHO MACROS", "Moldean el sonido de todos los motores a la vez: graves, brillo, movimiento, textura, estéreo, reverb, ataque y cola. Pasá el mouse por cualquiera para ver qué hace.",
                                      "THE EIGHT MACROS", "They shape every engine at once: lows, brightness, motion, texture, stereo, reverb, attack and tail. Hover any of them to see what it does." },
            { "rect:808,1320,200,148", "GENERAR, VARIACIONES, FAVORITO", "GENERAR crea un sonido nuevo (sin internet). VARIACIONES te da cuatro versiones del actual. FAVORITO lo guarda.",
                                       "GENERATE, VARIATIONS, FAVORITE", "GENERATE makes a new sound (no internet). VARIATIONS gives you four versions of the current one. FAVORITE saves it." },
            { "rect:20,260,160,290", "JUGAR, DISEÑO Y LAB", "JUGAR es esta pantalla. DISEÑO muestra todos los controles del motor elegido, con la forma de su máquina original. LAB tiene presets, generador avanzado, deshacer e idioma.",
                                     "PLAY, DESIGN AND LAB", "PLAY is this screen. DESIGN shows every control of the selected engine, shaped like its original machine. LAB has presets, the advanced generator, undo and language." },
            { "rect:950,16,60,60", "AYUDA SIEMPRE A MANO", "Tocá una nota en tu teclado MIDI y empezá. Este botón ? abre el recorrido de la pantalla donde estés, y pasando el mouse por cualquier control ves qué hace.",
                                   "HELP IS ALWAYS THERE", "Play a note on your MIDI keyboard and start. This ? button opens the tour of the screen you're on, and hovering any control tells you what it does." },
        } },
        { "lab", {
            { "rect:204,296,600,64", "BUSCAR Y FILTRAR", "Buscá por nombre o tag, y filtrá por categoría, motor, carácter o solo favoritos.",
                                     "SEARCH AND FILTER", "Search by name or tag, and filter by category, engine, character or favorites only." },
            { "rect:204,400,600,310", "LISTA DE PRESETS", "Un click carga el preset. F = de fábrica. La ★ lo marca como favorito. Si no te gusta, DESHACER lo vuelve atrás.",
                                      "PRESET LIST", "One click loads a preset. F = factory. The ★ marks it as favorite. Don't like it? UNDO takes it back." },
            { "rect:204,708,600,52", "GUARDAR", "Poné un nombre, elegí la categoría, agregá tags y guardá. Tus presets quedan en tu compu.",
                                     "SAVE", "Type a name, pick a category, add tags and save. Your presets stay on your computer." },
            { "rect:566,356,240,36", "COMPARAR A / B", "Dos memorias para comparar ideas. COPIAR A/B pasa el sonido actual a la otra.",
                                     "COMPARE A / B", "Two slots to compare ideas. COPY A/B copies the current sound to the other." },
            { "rect:204,780,600,190", "GENERADOR", "Elegí el modo (CLÁSICO, ESTILO o HÍBRIDO), la intensidad y generá. Cada sonido tiene una semilla: con el mismo número volvés al mismo sonido. DESHACER y REHACER guardan 100 pasos.",
                                      "GENERATOR", "Pick the mode (CLASSIC, STYLE or HYBRID), the amount and generate. Every sound has a seed: the same number brings back the same sound. UNDO and REDO keep 100 steps." },
            { "rect:204,1010,600,200", "BLOQUEOS", "Bloqueá motores, macros o carácter para que no cambien al generar. Ideal para quedarte con algo que te gusta y buscar el resto.",
                                       "LOCKS", "Lock engines, macros or character so they don't change when generating. Great to keep something you like and search for the rest." },
            { "rect:620,206,196,92", "OPCIONES E IDIOMA", "Animaciones, sonidos de pinball y el idioma (ES / EN).",
                                     "OPTIONS AND LANGUAGE", "Animations, pinball sounds and language (ES / EN)." },
        } },
        { "design0", {
            { "band:BANCO DE OSCILADORES", "OSCILADORES", "Tres osciladores como el Minimoog: RANGO es la octava, FORMA DE ONDA el timbre, FRECUENCIA desafina el 2 y el 3. Un poco de desafinación = sonido gordo.",
                                           "OSCILLATORS", "Three oscillators like the Minimoog: RANGE is the octave, WAVEFORM the tone, FREQUENCY detunes 2 and 3. A little detune = fat sound." },
            { "band:MEZCLADOR", "MEZCLADOR", "El volumen de cada oscilador y del ruido antes del filtro.", "MIXER", "The volume of each oscillator and the noise before the filter." },
            { "band:FILTRO", "FILTRO", "El corazón del sonido Moog. CORTE oscurece o abre, ÉNFASIS resuena, CONTORNO decide cuánto se abre en cada nota.",
                             "FILTER", "The heart of the Moog sound. CUTOFF darkens or opens, EMPHASIS resonates, CONTOUR decides how much it opens on each note." },
            { "band:CONTORNO DEL FILTRO", "CONTORNOS", "Arriba, cómo se mueve el filtro; abajo, cómo se mueve el volumen. Para un bajo con golpe: ataque corto, caída corta, sostén bajo en el filtro.",
                                          "CONTOURS", "Top: how the filter moves; bottom: how the volume moves. For a punchy bass: short attack, short decay, low sustain on the filter." },
        } },
        { "design1", {
            { "band:OSCILADOR A", "OSCILADOR A", "Prendé sierra, pulso o los dos. ANCHO cambia el timbre del pulso.", "OSCILLATOR A", "Turn on saw, pulse or both. WIDTH changes the pulse tone." },
            { "band:OSCILADOR B", "OSCILADOR B", "Afinalo contra el A con SEMITONOS y FINO. SYNC lo usa para rasgar el A.", "OSCILLATOR B", "Tune it against A with SEMITONES and FINE. SYNC uses it to tear A." },
            { "band:POLY-MOD", "POLY-MOD", "El secreto del Prophet: la envolvente del filtro y el oscilador B modulan la frecuencia de A o el filtro. Campanas, metales y barridos.",
                               "POLY-MOD", "The Prophet's secret: the filter envelope and oscillator B modulate A's frequency or the filter. Bells, brass and sweeps." },
            { "band:FILTRO 24 DB", "FILTRO", "Filtro de 24 dB: corte, resonancia, cuánto lo abre su envolvente y el seguimiento de teclado.",
                                   "FILTER", "24 dB filter: cutoff, resonance, how much its envelope opens it and keyboard tracking." },
            { "band:VOCES", "UNÍSONO", "Prendé UNÍSONO para un lead o bajo enorme y regulá cuánto se desafinan las copias.", "UNISON", "Turn on UNISON for a huge lead or bass and set how much the copies detune." },
        } },
        { "design2", {
            { "band:VCO-1", "LOS DOS VCO", "Elegí forma y pies de cada VCO. El VCO-2 también puede ser ruido.", "THE TWO VCOS", "Pick wave and feet for each VCO. VCO-2 can also be noise." },
            { "band:VCO MOD", "CROSS MOD Y SYNC", "CROSS MOD hace que el VCO-2 module al VCO-1 (metálico). SYNC rasga. VCO 1 / 2 los mezcla.",
                              "CROSS MOD AND SYNC", "CROSS MOD lets VCO-2 modulate VCO-1 (metallic). SYNC tears. VCO 1 / 2 blends them." },
            { "band:VCF", "FILTROS", "Un pasaaltos (HPF) y el filtro principal de 12 o 24 dB, movido por la ENV-1.", "FILTERS", "A high-pass (HPF) and the main 12 or 24 dB filter, driven by ENV-1." },
            { "band:ENV-1 (FILTRO)", "DOS ENVOLVENTES", "ENV-1 mueve el filtro y ENV-2 el volumen.", "TWO ENVELOPES", "ENV-1 moves the filter and ENV-2 the volume." },
            { "band:ARPEGIADOR (TODOS LOS MOTORES)", "ARPEGIADOR", "Prendelo y mantené un acorde: toca las notas una por una al tempo de tu DAW. Toca todos los motores encendidos.",
                                                     "ARPEGGIATOR", "Turn it on and hold a chord: it plays the notes one by one at your DAW's tempo. It plays every engine that's on." },
            { "band:TECLADO", "ZONA DEL TECLADO", "Con GRAVES o AGUDOS y el split, PITER suena solo en una parte del teclado: bajo abajo y otro motor arriba.",
                              "KEY ZONE", "With LOW or HIGH and the split, PITER plays only on part of the keyboard: bass below and another engine on top." },
        } },
        { "design3", {
            { "band:DCO", "EL DCO", "Un solo oscilador, estable como el Juno. PWM en LFO hace que el pulso respire.", "THE DCO", "A single oscillator, stable like the Juno. PWM on LFO makes the pulse breathe." },
            { "band:FUENTES", "FUENTES", "Sierra, pulso, el SUB (una octava abajo) y ruido. Sierra + sub es la base de casi todo.",
                              "SOURCES", "Saw, pulse, SUB (one octave down) and noise. Saw + sub is the base of almost everything." },
            { "band:VCF", "FILTRO", "Corte, resonancia, envolvente, LFO y teclado.", "FILTER", "Cutoff, resonance, envelope, LFO and keyboard." },
            { "band:CHORUS", "CHORUS", "La mitad del sonido Juno. Probá I y II en un pad y vas a escuchar la diferencia.",
                             "CHORUS", "Half of the Juno sound. Try I and II on a pad and you'll hear the difference." },
            { "band:ARPEGIADOR (TODOS LOS MOTORES)", "ARPEGIADOR", "El mismo arpegiador del Jupiter: toca todos los motores encendidos.",
                                                     "ARPEGGIATOR", "Same arpeggiator as the Jupiter: it plays every engine that's on." },
        } },
        { "design4", {
            { "rect:236,308,536,38", "DOS CANALES", "El CS-80 tiene dos sintes completos en capa. Con estos botones elegís cuál editar.",
                                     "TWO CHANNELS", "The CS-80 has two full synths layered. These buttons choose which one you edit." },
            { "band:OSCILADOR", "OSCILADOR Y FILTROS", "Cada canal tiene su oscilador (sierra, pulso, ruido) y dos filtros: pasaaltos y pasabajos.",
                                "OSCILLATOR AND FILTERS", "Each channel has its oscillator (saw, pulse, noise) and two filters: high-pass and low-pass." },
            { "band:MEZCLA", "MEZCLA Y DESAFINACIÓN", "Balanceá los canales y desafiná el II contra el I: así sale el sonido enorme del CS-80.",
                             "MIX AND DETUNE", "Balance the channels and detune II against I: that's the huge CS-80 sound." },
            { "band:AFTERTOUCH", "EXPRESIÓN", "Si tu teclado tiene aftertouch, apretar más fuerte abre el sonido y agrega vibrato.",
                                 "EXPRESSION", "If your keyboard has aftertouch, pressing harder opens the sound and adds vibrato." },
            { "rect:236,936,536,80", "LA CINTA", "Apoyá y deslizá para doblar el tono, como el ribbon original. Al soltar vuelve a su lugar.",
                                     "THE RIBBON", "Press and slide to bend the pitch, like the original ribbon. It springs back when released." },
        } },
        { "design5", {
            { "rect:236,312,330,220", "EL ALGORITMO", "Cómo se conectan los 6 operadores. Los de abajo (P) suenan; los de arriba los modulan y dan el brillo. Cambiá con las flechas y hacé click en un operador para editarlo.",
                                      "THE ALGORITHM", "How the 6 operators connect. The bottom ones (C) make the sound; the ones above modulate them and add brightness. Change it with the arrows and click an operator to edit it." },
            { "rect:236,558,540,40", "ELEGIR OPERADOR", "Cada botón edita un operador. P = portador, M = modulador.", "SELECT OPERATOR", "Each button edits one operator. C = carrier, M = modulator." },
            { "band:FRECUENCIA", "RATIO Y NIVEL", "En un modulador, el RATIO define el timbre (1 cálido, 2-3 brillante, 14 tine de piano) y el NIVEL cuánto brillo agrega.",
                                 "RATIO AND LEVEL", "On a modulator, RATIO sets the tone (1 warm, 2-3 bright, 14 piano tine) and LEVEL how much brightness it adds." },
            { "band:ENVOLVENTE DEL OPERADOR", "ENVOLVENTE POR OPERADOR", "Cada operador tiene la suya: un modulador con caída corta da el golpe del piano eléctrico.",
                                              "PER-OPERATOR ENVELOPE", "Each operator has its own: a modulator with a short decay gives the electric piano its bite." },
            { "band:NIVEL DE LOS 6 OPERADORES", "VISTA RÁPIDA", "Los niveles de los seis operadores juntos para mezclar rápido.", "QUICK VIEW", "All six operator levels together for quick mixing." },
        } },
        { "design6", {
            { "band:FUENTE 1", "DOS FUENTES PCM", "Elegí una muestra en cada fuente (piano, órgano, coro, pad…). Son muestras propias generadas por síntesis.",
                               "TWO PCM SOURCES", "Pick a sample for each source (piano, organ, choir, pad…). They are original samples made by synthesis." },
            { "band:FUENTE 2", "MEZCLAR", "Piano + pad, órgano + coro: la mezcla de dos muestras era el truco del M1. Desafiná unos cents para abrir.",
                               "LAYERING", "Piano + pad, organ + choir: layering two samples was the M1's trick. Detune a few cents to widen it." },
            { "band:FILTRO DIGITAL", "FILTRO", "Filtro digital sin resonancia, como el original. VELOCIDAD hace que tocar fuerte suene más brillante.",
                                     "FILTER", "Digital filter without resonance, like the original. VELOCITY makes harder playing brighter." },
            { "band:EFECTOS", "EFECTOS", "Chorus, delay o los dos, solo para este motor.", "EFFECTS", "Chorus, delay or both, for this engine only." },
        } },
        { "design7", {
            { "band:VCO 1", "TRES VCO", "Tres osciladores. El VCO 3 puede ser LFO para modular por cable.", "THREE VCOS", "Three oscillators. VCO 3 can be an LFO to modulate through cables." },
            { "band:FILTRO", "FILTRO MULTIMODO", "Pasabajos, pasabanda o pasaaltos, con resonancia y envolvente.", "MULTIMODE FILTER", "Low-pass, band-pass or high-pass, with resonance and envelope." },
            { "band:ADSR", "ENVOLVENTES Y S&H", "ADSR para volumen y filtro, AR para usar por cable, y el reloj del sample & hold.",
                           "ENVELOPES AND S&H", "ADSR for volume and filter, AR to use through cables, and the sample & hold clock." },
            { "band:PATCH BAY (4 CABLES)", "PATCH BAY", "Cuatro cables: elegí origen, destino y cantidad. En el centro, el cable está desenchufado. Probá S&H > FILTRO con resonancia alta.",
                                           "PATCH BAY", "Four cables: pick source, destination and amount. Centered = unplugged. Try S&H > FILTER with high resonance." },
        } },
        { "design8", {
            { "band:OSCILADOR 1", "OSCILADORES", "Tres osciladores. El 3 suele usarse lento, como LFO.", "OSCILLATORS", "Three oscillators. The 3rd is usually slow, as an LFO." },
            { "band:TRAPEZOIDE", "TRAPEZOIDE", "La envolvente del VCS3: sube, se queda, baja. En REPETIR se repite sola y arma ritmos.",
                                 "TRAPEZOID", "The VCS3 envelope: rises, holds, falls. On REPEAT it loops on its own and makes rhythms." },
            { "band:MATRIZ DE PINES (FUENTE > DESTINO)", "LA MATRIZ DE PINES", "Cada fila es una fuente y cada columna un destino. Poné un pin para conectarlos: OSC 3 > CORTE = filtro que se mueve; OSC 2 > TONO 1 = FM.",
                                                          "THE PIN MATRIX", "Each row is a source and each column a destination. Insert a pin to connect them: OSC 3 > CUTOFF = moving filter; OSC 2 > PITCH 1 = FM." },
            { "band:CONTROL", "JOYSTICK", "Arrastralo mientras tocás: X e Y son fuentes en la matriz.", "JOYSTICK", "Drag it while you play: X and Y are sources in the matrix." },
            { "band:RESORTE", "RESORTE", "Reverb de resorte metálica, parte del sonido raro del VCS3.", "SPRING", "Metallic spring reverb, part of the VCS3's strange sound." },
        } },
    };
    static const std::vector<Step> empty;
    auto it = tours.find (screen);
    return it != tours.end() ? it->second : empty;
}
}
