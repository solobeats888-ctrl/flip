#include "Lessons.h"
#include <map>
#include "../Help/I18n.h"

namespace flipper::teach
{
using namespace juce;

/*
    Valores en unidades reales (Hz, segundos, 0..1, indice de opcion, 0/1 en los SI/NO).
    Cada paso arranca NO cumplido desde el sonido base del motor (lo verifica tools/tests).
*/
const std::vector<Lesson>& lessons()
{
    static const std::vector<Lesson> list {
    // =================================================================== PRIMEROS PASOS (KROKO)
    { "basics_filter", 0, "QUÉ HACE UN FILTRO", "WHAT A FILTER DOES",
      "filtro filter corte cutoff resonancia resonance emphasis enfasis aprender basico basics empezar principiante",
      "Mantené una nota y escuchá cómo el filtro saca o deja pasar los agudos.",
      "Hold a note and hear how the filter removes or lets through the highs.",
      {
        { "", { { "@note", 0, 0, 0 } },
          "MANTENÉ UNA NOTA", "Tocá una nota grave y dejala apretada (o activá el sostén). Vamos a moverle el filtro mientras suena.",
          "HOLD A NOTE", "Play a low note and keep it held (or use the sustain pedal). We'll move the filter while it plays." },
        { "minimoog_cutoff", { { "minimoog_cutoff", 20, 260, 150 } },
          "CERRÁ EL FILTRO", "Girá CORTE todo a la izquierda. Suena apagado, como música detrás de una pared: el filtro se está comiendo los agudos.",
          "CLOSE THE FILTER", "Turn CUTOFF all the way left. It sounds muffled, like music behind a wall: the filter is eating the highs." },
        { "minimoog_cutoff", { { "minimoog_cutoff", 6000, 18000, 9000 } },
          "ABRILO", "Ahora llevalo bien a la derecha. Vuelve el brillo: el filtro deja pasar casi todo.",
          "OPEN IT", "Now take it far right. The brightness comes back: the filter lets almost everything through." },
        { "minimoog_emphasis", { { "minimoog_emphasis", 0.7f, 1.0f, 0.8f } },
          "RESONANCIA", "Subí ÉNFASIS. Realza justo la zona del corte: aparece un silbido. Así suenan los bajos ácidos y los barridos.",
          "RESONANCE", "Turn up EMPHASIS. It boosts right at the cutoff: a whistle appears. That's the sound of acid basses and sweeps." },
        { "minimoog_cutoff", { { "minimoog_cutoff", 400, 1600, 900 } },
          "BARRÉ CON RESONANCIA", "Con el énfasis alto, bajá CORTE hasta el medio despacio, mientras suena. Ese 'wiiu' es el barrido de filtro.",
          "SWEEP WITH RESONANCE", "With emphasis high, slowly bring CUTOFF down to the middle while it plays. That 'weeoo' is a filter sweep." },
        { "minimoog_emphasis", { { "minimoog_emphasis", 0.15f, 0.45f, 0.3f } },
          "VOLVÉ A ALGO MUSICAL", "Bajá ÉNFASIS a un tercio. Regla: corte para el brillo, resonancia para el carácter. Los dos juntos hacen casi todo el sonido.",
          "BACK TO SOMETHING MUSICAL", "Bring EMPHASIS down to a third. Rule: cutoff for brightness, resonance for character. Together they make most of the sound." },
      }, true },

    { "basics_env", 0, "LA ENVOLVENTE (ADSR)", "THE ENVELOPE (ADSR)",
      "envolvente envelope adsr ataque attack caida decay sosten sustain liberacion release cola aprender basico basics principiante",
      "Ataque, caída, sostén y liberación: cómo nace y cómo muere cada nota.",
      "Attack, decay, sustain and release: how every note is born and how it dies.",
      {
        { "minimoog_a_attack", { { "minimoog_a_attack", 0.4f, 3.0f, 0.8f } },
          "ATAQUE LARGO", "En CONTORNO DE VOLUMEN subí TIEMPO DE ATAQUE. Tocá: la nota entra despacio, como un pad o unas cuerdas.",
          "LONG ATTACK", "In LOUDNESS CONTOUR turn up ATTACK TIME. Play: the note fades in slowly, like a pad or strings." },
        { "minimoog_a_attack", { { "minimoog_a_attack", 0.001f, 0.012f, 0.003f } },
          "ATAQUE CORTO", "Ahora llevalo a cero. La nota golpea de una: así son los bajos, los plucks y las percusiones.",
          "SHORT ATTACK", "Now take it to zero. The note hits right away: that's basses, plucks and percussion." },
        { "minimoog_a_sustain", { { "minimoog_a_sustain", 0.0f, 0.06f, 0.0f }, { "minimoog_a_decay", 0.12f, 0.6f, 0.25f } },
          "SOSTÉN EN CERO", "Bajá NIVEL DE SOSTÉN a cero y dejá la CAÍDA corta (un cuarto). Aunque mantengas la tecla, el sonido se apaga solo: un pluck.",
          "SUSTAIN AT ZERO", "Bring SUSTAIN LEVEL to zero and keep DECAY short (a quarter). Even if you hold the key, the sound dies by itself: a pluck." },
        { "minimoog_a_release", { { "minimoog_a_release", 1.0f, 5.0f, 1.8f } },
          "LA COLA", "Subí LIBERACIÓN. Ahora, al soltar la tecla, la nota queda sonando un rato. Corta = seco; larga = ambiente.",
          "THE TAIL", "Turn up RELEASE. Now when you let go, the note keeps ringing. Short = dry; long = ambient." },
        { "band:CONTORNO DEL FILTRO", {},
          "EL MISMO TRUCO EN EL FILTRO", "Arriba está la misma envolvente pero para el filtro: en vez de mover el volumen, abre y cierra el brillo en cada nota. Es lo que le da el 'golpe' a un bajo.",
          "SAME TRICK ON THE FILTER", "Above is the same envelope for the filter: instead of volume, it opens and closes the brightness on each note. That's what gives a bass its 'punch'." },
      }, true },

    { "basics_detune", 0, "DESAFINAR PARA ENGORDAR", "DETUNE TO GET FAT",
      "desafinar desafinado detune gordo fat grueso ancho chorus unison unisono aprender basico basics",
      "Por qué dos osciladores apenas desafinados suenan enormes.",
      "Why two slightly detuned oscillators sound huge.",
      {
        { "minimoog_osc2_detune", { { "minimoog_osc2_detune", -0.02f, 0.02f, 0.0f }, { "minimoog_mix3", 0.0f, 0.05f, 0.0f } },
          "PRIMERO, AFINADOS IGUAL", "Poné FRECUENCIA del oscilador 2 justo en 0 y bajá el volumen del OSC 3 en el MEZCLADOR. Tocá: suena correcto, pero finito.",
          "FIRST, PERFECTLY IN TUNE", "Set oscillator 2 FREQUENCY right at 0 and turn OSC 3 down in the MIXER. Play: it sounds fine, but thin." },
        { "minimoog_osc2_detune", { { "minimoog_osc2_detune", 0.12f, 0.35f, 0.2f } },
          "AHORA, APENAS DESAFINADO", "Mové FRECUENCIA del 2 un poquito a la derecha. Escuchá cómo el sonido 'late' y se abre: las dos ondas se cruzan todo el tiempo.",
          "NOW SLIGHTLY DETUNED", "Move oscillator 2 FREQUENCY a little to the right. Hear how it 'beats' and opens up: the two waves keep crossing each other." },
        { "minimoog_osc3_detune", { { "minimoog_osc3_detune", -0.35f, -0.12f, -0.2f }, { "minimoog_mix3", 0.4f, 1.0f, 0.6f } },
          "EL TERCERO, PARA EL OTRO LADO", "Subí OSC 3 en el mezclador y desafinalo un poco hacia la izquierda. Uno arriba y otro abajo: el truco del Minimoog para sonar gordo.",
          "THE THIRD ONE, THE OTHER WAY", "Turn OSC 3 up in the mixer and detune it slightly to the left. One up, one down: the Minimoog trick to sound fat." },
        { "", {},
          "LA REGLA", "Poquito = gordo. Mucho = desafinado (sirve para efectos). El mismo truco existe como UNÍSONO en PROFETA y como desafinación del canal II en ILLUMINARY.",
          "THE RULE", "A little = fat. A lot = out of tune (good for effects). The same trick exists as UNISON on PROFETA and as channel II detune on ILLUMINARY." },
      }, true },

    // =================================================================== KROKO (MINIMOOG)
    { "mm_bass", 0, "BAJO MOOG GORDO", "FAT MOOG BASS",
      "bajo bass moog gordo grave low funk funky analogico pesado bajos",
      "El bajo clásico: tres osciladores graves, filtro cerrado y un golpe de contorno.",
      "The classic bass: three low oscillators, a closed filter and a contour punch.",
      {
        { "band:BANCO DE OSCILADORES", { { "minimoog_osc1_range", 1, 1, 1 }, { "minimoog_osc2_range", 1, 1, 1 } },
          "UNA OCTAVA ABAJO", "Poné RANGO del oscilador 1 y del 2 en 16'. Los pies son octavas, como en un órgano: más pies, más grave.",
          "ONE OCTAVE DOWN", "Set RANGE of oscillators 1 and 2 to 16'. Feet are octaves, like on an organ: more feet, lower pitch." },
        { "minimoog_osc3_range", { { "minimoog_osc3_range", 0, 0, 0 }, { "minimoog_mix3", 0.55f, 1.0f, 0.7f } },
          "EL SUB", "El oscilador 3 a 32' y subilo en el MEZCLADOR. Es el piso del bajo: lo sentís más de lo que lo escuchás.",
          "THE SUB", "Oscillator 3 to 32' and turn it up in the MIXER. It's the floor of the bass: you feel it more than you hear it." },
        { "minimoog_cutoff", { { "minimoog_cutoff", 180, 600, 380 } },
          "CERRÁ EL FILTRO", "Bajá FRECUENCIA DE CORTE a la zona de 200-600 Hz. Un bajo no necesita agudos: los deja para el resto del beat.",
          "CLOSE THE FILTER", "Bring CUTOFF FREQUENCY down to about 200-600 Hz. A bass doesn't need highs: it leaves them for the rest of the beat." },
        { "minimoog_contour", { { "minimoog_contour", 0.6f, 1.0f, 0.72f } },
          "CANTIDAD DE CONTORNO", "Subí CONTORNO. Ahora el filtro se abre en cada nota y se vuelve a cerrar: eso es el 'golpe'.",
          "CONTOUR AMOUNT", "Turn up CONTOUR. Now the filter opens on every note and closes again: that's the 'punch'." },
        { "band:CONTORNO DEL FILTRO", { { "minimoog_f_decay", 0.07f, 0.26f, 0.16f }, { "minimoog_f_sustain", 0.0f, 0.15f, 0.05f } },
          "GOLPE CORTO", "En CONTORNO DEL FILTRO: CAÍDA corta (un cuarto de vuelta) y SOSTÉN casi en cero. El brillo dura un instante y queda solo el cuerpo.",
          "SHORT PUNCH", "In FILTER CONTOUR: short DECAY (a quarter turn) and SUSTAIN almost at zero. The brightness lasts an instant and only the body stays." },
        { "", { { "@note", 0, 0, 0 } },
          "TOCALO", "Tocá un patrón grave. Si querés que muerda más, subí ÉNFASIS o SATURACIÓN.",
          "PLAY IT", "Play a low pattern. Want more bite? Turn up EMPHASIS or OVERLOAD." },
      } },

    { "mm_808", 0, "BAJO 808", "808 BASS",
      "808 bajo bass sub subgrave trap drill boom kick grave deslizar glide slide bajos",
      "El bajo largo del trap: una onda casi pura que cae sola, con glide y saturación.",
      "The long trap bass: an almost pure wave that decays on its own, with glide and saturation.",
      {
        { "minimoog_osc1_wave", { { "minimoog_osc1_wave", 0, 0, 0 } },
          "UNA ONDA SUAVE", "FORMA DE ONDA del oscilador 1 en triángulo (el primer ícono). Un 808 es casi un seno: grave y redondo.",
          "A SMOOTH WAVE", "Oscillator 1 WAVEFORM to triangle (the first icon). An 808 is almost a sine: low and round." },
        { "band:MEZCLADOR", { { "minimoog_mix2", 0.0f, 0.05f, 0.0f }, { "minimoog_mix3", 0.0f, 0.05f, 0.0f } },
          "UN SOLO OSCILADOR", "En el MEZCLADOR bajá OSC 2 y OSC 3 a cero. Más osciladores ensucian el sub.",
          "A SINGLE OSCILLATOR", "In the MIXER turn OSC 2 and OSC 3 down to zero. More oscillators muddy the sub." },
        { "minimoog_osc1_range", { { "minimoog_osc1_range", 0, 1, 1 } },
          "BIEN ABAJO", "RANGO del oscilador 1 en 16' (o 32' si querés que tiemble).",
          "WAY DOWN", "Oscillator 1 RANGE to 16' (or 32' if you want it to rumble)." },
        { "band:CONTORNO DE VOLUMEN", { { "minimoog_a_sustain", 0.0f, 0.08f, 0.0f }, { "minimoog_a_decay", 0.8f, 3.5f, 1.4f } },
          "QUE CAIGA SOLO", "En CONTORNO DE VOLUMEN: SOSTÉN en cero y CAÍDA larga (más de la mitad). El 808 no se sostiene: pega y se va apagando.",
          "LET IT DECAY", "In LOUDNESS CONTOUR: SUSTAIN at zero and long DECAY (past halfway). An 808 doesn't sustain: it hits and fades out." },
        { "minimoog_glide", { { "minimoog_glide", 0.04f, 0.3f, 0.09f } },
          "EL DESLIZAMIENTO", "Subí GLIDE un poco. Tocá una nota y, sin soltarla, otra: el bajo se desliza. Ese es el slide del trap.",
          "THE SLIDE", "Turn GLIDE up a bit. Play a note and, without letting go, another: the bass slides. That's the trap slide." },
        { "minimoog_drive", { { "minimoog_drive", 0.45f, 1.0f, 0.6f } },
          "SATURACIÓN", "Subí SATURACIÓN. Le agrega armónicos: así el 808 se escucha también en un celular o en parlantes chicos.",
          "OVERLOAD", "Turn up OVERLOAD. It adds harmonics: that way the 808 can be heard on a phone or small speakers too." },
        { "", { { "@note", 0, 0, 0 } },
          "PROBALO GRAVE", "Tocá entre C1 y C2. Si retumba demasiado, bajá CUERPO en las macros.",
          "TRY IT LOW", "Play between C1 and C2. If it booms too much, turn BODY down in the macros." },
      } },

    { "mm_lead", 0, "LEAD MOOG CON GLIDE", "MOOG LEAD WITH GLIDE",
      "lead leads melodia punta solo moog glide portamento cantante g-funk gfunk silbido whistle",
      "Un lead que canta: osciladores desafinados, resonancia y glide.",
      "A singing lead: detuned oscillators, resonance and glide.",
      {
        { "minimoog_osc2_detune", { { "minimoog_osc2_detune", 0.14f, 0.4f, 0.22f } },
          "UN POCO DE MOVIMIENTO", "FRECUENCIA del oscilador 2 un poquito a la derecha. Desafinado apenas, el lead se ensancha.",
          "A LITTLE MOTION", "Oscillator 2 FREQUENCY a touch to the right. Barely detuned, the lead widens." },
        { "minimoog_cutoff", { { "minimoog_cutoff", 1800, 6000, 2600 } },
          "MÁS BRILLO", "Abrí FRECUENCIA DE CORTE a la zona de 2-5 kHz: el lead tiene que cortar por encima del beat.",
          "MORE BRIGHTNESS", "Open CUTOFF FREQUENCY to around 2-5 kHz: a lead has to cut above the beat." },
        { "minimoog_emphasis", { { "minimoog_emphasis", 0.45f, 0.75f, 0.55f } },
          "QUE CANTE", "Subí ÉNFASIS a la mitad. Le da ese tono nasal que canta.",
          "MAKE IT SING", "Turn EMPHASIS up to the middle. It gives that nasal singing tone." },
        { "minimoog_kbd", { { "minimoog_kbd", 0.6f, 1.0f, 0.8f } },
          "SEGUIMIENTO DE TECLADO", "Subí TECLADO. El filtro se abre en las notas agudas, así no suenan apagadas cuando subís.",
          "KEYBOARD TRACKING", "Turn up KEYBOARD. The filter opens on high notes, so they don't sound dull as you go up." },
        { "minimoog_glide", { { "minimoog_glide", 0.05f, 0.3f, 0.12f } },
          "GLIDE", "Subí GLIDE. Tocá ligado (sin soltar entre notas) y la melodía se desliza de nota a nota.",
          "GLIDE", "Turn up GLIDE. Play legato (without releasing between notes) and the melody slides from note to note." },
        { "", { { "@note", 0, 0, 0 } },
          "TOCÁ UNA MELODÍA", "Probá arriba de C4. Para un lead estilo g-funk: subí MOVIMIENTO en las macros para el vibrato.",
          "PLAY A MELODY", "Try above C4. For a g-funk style lead: turn up MOVEMENT in the macros for vibrato." },
      } },

    // =================================================================== PROFETA (PROPHET-5)
    { "p5_brass", 1, "METALES DE LOS 80", "80S BRASS",
      "metales brass vientos trompeta trompetas stab stabs 80s ochentas sinte synth prophet",
      "Dos sierras desafinadas y un filtro que se abre con un pequeño retraso: el 'wah' de los metales.",
      "Two detuned saws and a filter that opens with a little delay: the brass 'wah'.",
      {
        { "prophet5_b_fine", { { "prophet5_b_fine", 0.12f, 0.35f, 0.16f } },
          "DOS SIERRAS", "Los dos osciladores ya están en sierra. Subí FINO del B un poquito: la desafinación hace el cuerpo de una sección de metales.",
          "TWO SAWS", "Both oscillators are already on saw. Turn B's FINE up a touch: the detune makes the body of a brass section." },
        { "prophet5_cutoff", { { "prophet5_cutoff", 350, 1000, 650 } },
          "FILTRO CERRADO", "Bajá CORTE. Va a abrirlo la envolvente en cada acorde.",
          "CLOSED FILTER", "Bring CUTOFF down. The envelope will open it on every chord." },
        { "band:ENVOLVENTE DEL FILTRO", { { "prophet5_f_attack", 0.04f, 0.25f, 0.08f }, { "prophet5_f_sustain", 0.45f, 0.8f, 0.55f } },
          "EL WAH", "En ENVOLVENTE DEL FILTRO: ATAQUE un poco lento y SOSTÉN a la mitad. El brillo entra un instante después de la nota, como cuando sopla un trompetista.",
          "THE WAH", "In FILTER ENVELOPE: a slightly slow ATTACK and SUSTAIN at half. The brightness comes in an instant after the note, like a horn player blowing." },
        { "prophet5_fenv", { { "prophet5_fenv", 0.6f, 1.0f, 0.7f } },
          "CUÁNTO SE ABRE", "Subí ENV en FILTRO 24 DB: es cuánto abre esa envolvente.",
          "HOW MUCH IT OPENS", "Turn up ENV in FILTER 24 DB: that's how much the envelope opens it." },
        { "band:ENVOLVENTE DE VOLUMEN", { { "prophet5_a_attack", 0.02f, 0.09f, 0.04f }, { "prophet5_a_release", 0.12f, 0.35f, 0.25f } },
          "ENTRADA Y SALIDA", "En ENVOLVENTE DE VOLUMEN: un pelito de ATAQUE y LIBERACIÓN corta. Stabs secos.",
          "IN AND OUT", "In AMP ENVELOPE: a hair of ATTACK and a short RELEASE. Dry stabs." },
        { "", { { "@note", 0, 0, 0 } },
          "TOCÁ ACORDES", "Acordes cortos, rítmicos. Para más grande: prendé UNÍSONO.",
          "PLAY CHORDS", "Short, rhythmic chords. Want it bigger? Turn on UNISON." },
      } },

    { "p5_bell", 1, "CAMPANA POLY-MOD", "POLY-MOD BELL",
      "campana campanas bell bells metalico metallic polymod poly-mod campanita gong glass cristal prophet",
      "El oscilador B modula la frecuencia del A: aparecen armónicos de campana.",
      "Oscillator B modulates A's frequency: bell harmonics appear.",
      {
        { "band:OSCILADOR B", { { "prophet5_b_saw", 0, 0, 0 }, { "prophet5_b_tri", 1, 1, 1 } },
          "UN B SUAVE", "En OSCILADOR B apagá SIERRA y prendé TRIÁNGULO. Va a ser el modulador: cuanto más suave, más limpia la campana.",
          "A SOFT B", "In OSCILLATOR B turn SAW off and TRIANGLE on. It will be the modulator: the softer it is, the cleaner the bell." },
        { "prophet5_b_semi", { { "prophet5_b_semi", 14, 24, 19 } },
          "LEJOS DEL A", "Subí SEMITONOS del B a 19 (octava y quinta). La distancia entre los dos define qué tan metálico suena.",
          "FAR FROM A", "Raise B's SEMITONES to 19 (octave plus a fifth). The distance between them defines how metallic it sounds." },
        { "prophet5_mix_b", { { "prophet5_mix_b", 0.0f, 0.15f, 0.05f } },
          "QUE NO SE ESCUCHE", "Bajá OSC B en el MEZCLADOR. No lo queremos escuchar: solo que trabaje sobre el A.",
          "KEEP IT SILENT", "Turn OSC B down in the MIXER. We don't want to hear it: only to have it work on A." },
        { "prophet5_pm_oscb", { { "prophet5_pm_oscb", 0.3f, 0.7f, 0.45f } },
          "POLY-MOD", "En POLY-MOD subí OSC B (con > FREC A prendido). El B ahora mueve la frecuencia del A muy rápido: eso es FM, y suena a campana.",
          "POLY-MOD", "In POLY-MOD turn up OSC B (with > FREQ A on). B now moves A's frequency very fast: that's FM, and it sounds like a bell." },
        { "prophet5_cutoff", { { "prophet5_cutoff", 4000, 18000, 7000 } },
          "FILTRO ABIERTO", "Abrí CORTE: la campana necesita todos sus armónicos.",
          "OPEN FILTER", "Open CUTOFF: the bell needs all its harmonics." },
        { "band:ENVOLVENTE DE VOLUMEN", { { "prophet5_a_sustain", 0.0f, 0.06f, 0.0f }, { "prophet5_a_decay", 1.2f, 5.0f, 2.0f }, { "prophet5_a_release", 0.9f, 4.0f, 1.5f } },
          "QUE SUENE Y SE APAGUE", "SOSTÉN en cero, CAÍDA y LIBERACIÓN largas. Una campana se golpea y se apaga sola.",
          "STRIKE AND FADE", "SUSTAIN at zero, long DECAY and RELEASE. A bell is struck and fades by itself." },
        { "", { { "@note", 0, 0, 0 } },
          "TOCALA ARRIBA", "Suena mejor en notas agudas. Movés SEMITONOS del B y cambia el metal: probá 12, 17, 24.",
          "PLAY IT HIGH", "It sounds best on high notes. Move B's SEMITONES and the metal changes: try 12, 17, 24." },
      } },

    { "p5_sync", 1, "LEAD SYNC RASGADO", "TEARING SYNC LEAD",
      "sync lead leads rasgado agresivo aggressive sincronizado punta melodia electro rock prophet",
      "El sync obliga al A a reiniciarse con el B; si después barrés el A, rasga.",
      "Sync forces A to restart with B; sweep A afterwards and it tears.",
      {
        { "prophet5_sync", { { "prophet5_sync", 1, 1, 1 } },
          "PRENDÉ SYNC", "En OSCILADOR B prendé SYNC. El A queda atado al B: cada vez que el B empieza su ciclo, el A vuelve a cero.",
          "TURN ON SYNC", "In OSCILLATOR B turn SYNC on. A is now tied to B: every time B starts its cycle, A resets." },
        { "prophet5_mix_b", { { "prophet5_mix_b", 0.0f, 0.15f, 0.05f } },
          "EL B ES EL RELOJ", "Bajá OSC B en el MEZCLADOR. El B marca el tono pero lo que escuchamos es el A rasgado.",
          "B IS THE CLOCK", "Turn OSC B down in the MIXER. B sets the pitch, but what we hear is the torn A." },
        { "prophet5_pm_env", { { "prophet5_pm_env", 0.3f, 0.75f, 0.45f } },
          "EL BARRIDO", "En POLY-MOD subí ENV FILTRO. La envolvente sube la frecuencia del A en cada nota: el clásico 'ñiaau' del sync.",
          "THE SWEEP", "In POLY-MOD turn up FILTER ENV. The envelope raises A's frequency on every note: the classic sync 'nyaow'." },
        { "prophet5_cutoff", { { "prophet5_cutoff", 3000, 12000, 4500 } },
          "QUE CORTE", "Abrí CORTE: el sync vive en los agudos.",
          "LET IT CUT", "Open CUTOFF: sync lives in the highs." },
        { "prophet5_unison", { { "prophet5_unison", 1, 1, 1 } },
          "UNÍSONO", "En VOCES prendé UNÍSONO: varias copias desafinadas tocando la misma nota. Enorme.",
          "UNISON", "In VOICES turn UNISON on: several detuned copies playing the same note. Huge." },
        { "prophet5_glide", { { "prophet5_glide", 0.03f, 0.25f, 0.07f } },
          "UN POCO DE GLIDE", "Un toque de GLIDE para que la melodía se arrastre entre notas.",
          "A BIT OF GLIDE", "A touch of GLIDE so the melody drags between notes." },
        { "", { { "@note", 0, 0, 0 } },
          "TOCÁ", "Notas sueltas y fuertes. Si querés más rasgado, subí ENV FILTRO en POLY-MOD.",
          "PLAY", "Single strong notes. Want more tearing? Turn up FILTER ENV in POLY-MOD." },
      } },

    // =================================================================== PITER (JUPITER-8)
    { "j8_pad", 2, "PAD ENORME", "HUGE PAD",
      "pad pads colchon atmosfera atmosferico ambient cuerdas strings fondo jupiter lento suave",
      "Dos VCO desafinados, envolventes lentas y un LFO que mueve el filtro.",
      "Two detuned VCOs, slow envelopes and an LFO moving the filter.",
      {
        { "jupiter8_vco2_fine", { { "jupiter8_vco2_fine", 0.15f, 0.4f, 0.2f } },
          "DESAFINÁ EL VCO-2", "Subí FINO del VCO-2. Dos sierras desafinadas = el colchón ancho del Jupiter.",
          "DETUNE VCO-2", "Turn up VCO-2 FINE. Two detuned saws = the wide Jupiter cushion." },
        { "band:ENV-2 (VCA)", { { "jupiter8_e2_attack", 0.5f, 3.0f, 0.9f }, { "jupiter8_e2_release", 1.5f, 6.0f, 2.2f } },
          "ENTRA Y SE VA LENTO", "En ENV-2 (VCA): A (ataque) a la mitad y R (liberación) larga. El pad aparece despacio y se queda flotando al soltar.",
          "SLOW IN, SLOW OUT", "In ENV-2 (VCA): A (attack) at half and a long R (release). The pad fades in and floats after you let go." },
        { "band:VCF", { { "jupiter8_cutoff", 700, 1800, 1300 }, { "jupiter8_env_amt", 0.1f, 0.32f, 0.2f } },
          "FILTRO SUAVE", "En VCF: CORTE un poco más abajo y ENV-1 bajo. Un pad no tiene que golpear.",
          "SOFT FILTER", "In VCF: CUTOFF a little lower and ENV-1 low. A pad shouldn't punch." },
        { "jupiter8_hpf", { { "jupiter8_hpf", 80, 400, 150 } },
          "LIMPIÁ LOS GRAVES", "Subí HPF (el pasaaltos). Saca graves para que el pad no se pelee con el bajo y el 808.",
          "CLEAN THE LOWS", "Turn up HPF (the high-pass). It removes lows so the pad doesn't fight the bass and the 808." },
        { "band:LFO", { { "jupiter8_lfo_vcf", 0.08f, 0.35f, 0.15f }, { "jupiter8_lfo_rate", 0.1f, 1.5f, 0.5f } },
          "QUE RESPIRE", "En LFO: VELOC. bien lenta y > VCF un poquito. El filtro se mueve solo y el pad respira.",
          "MAKE IT BREATHE", "In LFO: very slow RATE and a little > VCF. The filter moves by itself and the pad breathes." },
        { "", { { "@note", 0, 0, 0 } },
          "TOCÁ UN ACORDE LARGO", "Dejalo sonar. Subí ESPACIO y ANCHURA en las macros para que se vuelva enorme.",
          "PLAY A LONG CHORD", "Let it ring. Turn up SPACE and WIDTH in the macros to make it massive." },
      } },

    { "j8_arp", 2, "ARPEGIO", "ARPEGGIO",
      "arpegio arpegios arpegiador arp arpeggio arpeggiator secuencia sequence trance synthwave jupiter",
      "El arpegiador toca tu acorde nota por nota, al tempo del DAW.",
      "The arpeggiator plays your chord note by note, at the DAW tempo.",
      {
        { "arp_on", { { "arp_on", 1, 1, 1 } },
          "PRENDÉ EL ARPEGIADOR", "En ARPEGIADOR prendé ARP. Toca todos los motores encendidos, no solo PITER.",
          "TURN ON THE ARPEGGIATOR", "In ARPEGGIATOR turn ARP on. It plays every engine that's on, not just PITER." },
        { "band:ARPEGIADOR (TODOS LOS MOTORES)", { { "arp_mode", 2, 2, 2 }, { "arp_oct", 1, 1, 1 } },
          "EL DIBUJO", "MODO en ARRIBA-ABAJO y OCTAVAS en 2. Sube y baja por dos octavas.",
          "THE PATTERN", "MODE to UP-DOWN and OCTAVES to 2. It goes up and down across two octaves." },
        { "band:ENV-2 (VCA)", { { "jupiter8_e2_decay", 0.12f, 0.4f, 0.25f }, { "jupiter8_e2_sustain", 0.0f, 0.25f, 0.1f } },
          "NOTAS CORTAS", "En ENV-2 (VCA): D (caída) corta y S (sostén) bajo. Cada nota del arpegio se separa de la otra.",
          "SHORT NOTES", "In ENV-2 (VCA): short D (decay) and low S (sustain). Every note of the arpeggio stands apart." },
        { "band:ENV-1 (FILTRO)", { { "jupiter8_e1_decay", 0.08f, 0.35f, 0.2f } },
          "UN GOLPE DE FILTRO POR NOTA", "En ENV-1 (FILTRO) bajá D. El filtro se abre y cierra rápido en cada paso.",
          "A FILTER HIT PER NOTE", "In ENV-1 (FILTER) bring D down. The filter opens and closes fast on every step." },
        { "jupiter8_env_amt", { { "jupiter8_env_amt", 0.55f, 0.9f, 0.65f } },
          "MÁS GOLPE", "En VCF subí ENV-1: cuánto se abre en cada nota.",
          "MORE PUNCH", "In VCF turn up ENV-1: how much it opens on each note." },
        { "arp_gate", { { "arp_gate", 0.15f, 0.45f, 0.35f } },
          "GATE", "Bajá GATE: notas más cortas, más staccato.",
          "GATE", "Bring GATE down: shorter, more staccato notes." },
        { "", { { "@note", 0, 0, 0 } },
          "MANTENÉ UN ACORDE", "Apretá un acorde y dejalo: el arpegio sigue el tempo del proyecto. Probá VALOR en 1/8T para un groove de tresillo.",
          "HOLD A CHORD", "Hold a chord: the arpeggio follows the project tempo. Try RATE at 1/8T for a triplet groove." },
      } },

    // =================================================================== NO JUNA (JUNO)
    { "ju_pad", 3, "PAD CON CHORUS", "CHORUS PAD",
      "pad pads chorus juno colchon 80s ochentas synthwave suave lento atmosfera",
      "El sonido Juno: un solo oscilador y el chorus que lo hace enorme.",
      "The Juno sound: a single oscillator and the chorus that makes it huge.",
      {
        { "juno_sub", { { "juno_sub", 0.0f, 0.12f, 0.0f } },
          "SACÁ EL SUB", "En FUENTES bajá SUB. En un pad el sub embarra los graves.",
          "REMOVE THE SUB", "In SOURCES turn SUB down. In a pad the sub muddies the low end." },
        { "band:ENV", { { "juno_attack", 0.35f, 2.5f, 0.7f }, { "juno_release", 1.2f, 5.0f, 2.0f } },
          "LENTO", "En ENV: A a la mitad y R larga. Que entre y se vaya suave.",
          "SLOW", "In ENV: A at half and a long R. Let it fade in and out softly." },
        { "juno_cutoff", { { "juno_cutoff", 900, 2100, 1500 } },
          "UN POCO MÁS OSCURO", "Bajá CORTE en el VCF. Más cálido, menos chillón.",
          "A BIT DARKER", "Bring CUTOFF down in the VCF. Warmer, less shrill." },
        { "juno_chorus", { { "juno_chorus", 2, 2, 2 } },
          "CHORUS II", "Poné CHORUS en II. Es más rápido y profundo que el I: escuchá cómo se abre en el estéreo.",
          "CHORUS II", "Set CHORUS to II. It's faster and deeper than I: hear how it opens in stereo." },
        { "", { { "@note", 0, 0, 0 } },
          "ACORDES", "Tocá acordes largos y compará I, II y I+II. El chorus es la mitad del sonido Juno.",
          "CHORDS", "Play long chords and compare I, II and I+II. The chorus is half of the Juno sound." },
      } },

    { "ju_pwm", 3, "PULSO QUE RESPIRA (PWM)", "BREATHING PULSE (PWM)",
      "pwm pulso pulse ancho width respira respirar strings cuerdas juno pad movimiento",
      "Mover el ancho del pulso con el LFO: el truco de las cuerdas del Juno.",
      "Moving the pulse width with the LFO: the Juno string trick.",
      {
        { "juno_saw", { { "juno_saw", 0, 0, 0 } },
          "SOLO PULSO", "En FUENTES apagá SIERRA. Queda solo el pulso, para escucharlo bien.",
          "PULSE ONLY", "In SOURCES turn SAW off. Only the pulse is left, so you hear it clearly." },
        { "juno_pw", { { "juno_pw", 0.55f, 0.95f, 0.7f } },
          "CUÁNTO SE MUEVE", "Subí ANCHO en DCO. Con PWM en LFO, esto es cuánto se mueve el ancho del pulso.",
          "HOW MUCH IT MOVES", "Turn up WIDTH in DCO. With PWM on LFO, this is how much the pulse width moves." },
        { "juno_lfo_rate", { { "juno_lfo_rate", 0.2f, 1.4f, 0.6f } },
          "LENTO", "Bajá VELOC. del LFO. El pulso se abre y se cierra despacio: suena como varias cuerdas tocando juntas.",
          "SLOW", "Bring the LFO RATE down. The pulse opens and closes slowly: it sounds like several strings playing together." },
        { "band:ENV", { { "juno_attack", 0.2f, 1.5f, 0.4f }, { "juno_release", 0.9f, 4.0f, 1.4f } },
          "ENTRADA DE CUERDAS", "En ENV: A un poco lento y R larga.",
          "STRING ENTRANCE", "In ENV: a slightly slow A and a long R." },
        { "", { { "@note", 0, 0, 0 } },
          "TOCÁ", "Acordes en el medio del teclado. Con CHORUS en I+II se vuelve un ensamble.",
          "PLAY", "Chords in the middle of the keyboard. With CHORUS on I+II it becomes an ensemble." },
      } },

    { "ju_bass", 3, "BAJO CON SUB", "SUB BASS",
      "bajo bass sub house deep juno grave bajos analogico",
      "Sierra + sub una octava abajo: la base de mil bajos de house.",
      "Saw + sub one octave down: the base of a thousand house basses.",
      {
        { "juno_range", { { "juno_range", 0, 0, 0 } },
          "16 PIES", "En DCO poné PIES en 16'. Una octava abajo.",
          "16 FEET", "In DCO set FEET to 16'. One octave down." },
        { "band:FUENTES", { { "juno_pulse", 0, 0, 0 }, { "juno_sub", 0.7f, 1.0f, 0.8f } },
          "SIERRA + SUB", "En FUENTES apagá PULSO y subí SUB. El sub es una onda cuadrada una octava más abajo: el peso.",
          "SAW + SUB", "In SOURCES turn PULSE off and turn SUB up. The sub is a square wave one octave lower: the weight." },
        { "band:VCF", { { "juno_cutoff", 250, 800, 500 }, { "juno_res", 0.2f, 0.45f, 0.3f } },
          "FILTRO", "En VCF: CORTE abajo y un poco de RESON.",
          "FILTER", "In VCF: CUTOFF down and a little RESON." },
        { "band:ENV", { { "juno_decay", 0.15f, 0.5f, 0.3f }, { "juno_sustain", 0.25f, 0.55f, 0.45f } },
          "GOLPE", "En ENV: D corta y S a la mitad. La envolvente mueve filtro y volumen juntos.",
          "PUNCH", "In ENV: short D and S at half. The envelope moves filter and volume together." },
        { "juno_env_amt", { { "juno_env_amt", 0.5f, 0.85f, 0.6f } },
          "CUÁNTO GOLPEA", "Subí ENV en el VCF.",
          "HOW HARD IT HITS", "Turn up ENV in the VCF." },
        { "juno_chorus", { { "juno_chorus", 0, 0, 0 } },
          "SIN CHORUS", "CHORUS en OFF. En un bajo, el chorus desparrama los graves por el estéreo y pierde pegada.",
          "NO CHORUS", "CHORUS to OFF. On a bass, the chorus spreads the lows across the stereo field and loses punch." },
        { "", { { "@note", 0, 0, 0 } },
          "TOCÁ UN GROOVE", "Notas cortas en la zona de C1-C2.",
          "PLAY A GROOVE", "Short notes around C1-C2." },
      } },

    { "ju_pluck", 3, "PLUCK", "PLUCK",
      "pluck plucks pizzicato pellizco punteado marimba arpa harp corto juno melodia",
      "Un sonido que se golpea y muere solo: el filtro hace el trabajo.",
      "A sound that is struck and dies by itself: the filter does the work.",
      {
        { "band:ENV", { { "juno_sustain", 0.0f, 0.05f, 0.0f }, { "juno_decay", 0.12f, 0.45f, 0.25f } },
          "QUE SE APAGUE SOLO", "En ENV: S en cero y D corta. Aunque mantengas la tecla, la nota se apaga.",
          "LET IT DIE", "In ENV: S at zero and a short D. Even if you hold the key, the note dies." },
        { "juno_release", { { "juno_release", 0.08f, 0.35f, 0.2f } },
          "SIN COLA", "Bajá R: al soltar, se corta.",
          "NO TAIL", "Bring R down: it stops when you let go." },
        { "band:VCF", { { "juno_cutoff", 300, 1000, 600 }, { "juno_env_amt", 0.55f, 0.95f, 0.7f } },
          "EL PELLIZCO", "En VCF: CORTE cerrado y ENV alto. El filtro se abre de golpe al principio y se cierra: eso es el 'plic'.",
          "THE PLUCK", "In VCF: closed CUTOFF and high ENV. The filter snaps open at the start and closes: that's the 'plink'." },
        { "juno_res", { { "juno_res", 0.25f, 0.55f, 0.35f } },
          "UN POCO DE RESONANCIA", "Subí RESON. un toque: el pluck se vuelve más brillante y elástico.",
          "A LITTLE RESONANCE", "Turn RESON up a touch: the pluck becomes brighter and bouncier." },
        { "", { { "@note", 0, 0, 0 } },
          "MELODÍAS RÁPIDAS", "Ideal para melodías y arpegios. Con ESPACIO en las macros queda como en un salón.",
          "FAST MELODIES", "Great for melodies and arpeggios. With SPACE in the macros it sounds like a hall." },
      } },

    // =================================================================== ILLUMINARY (CS-80)
    { "cs_blade", 4, "METALES DE PELÍCULA", "CINEMATIC BRASS",
      "pelicula cine cinematic cinematografico blade runner vangelis epico epic metales brass cs80 cs-80 expresivo aftertouch",
      "Dos capas desafinadas, ataques lentos y vibrato al apretar la tecla.",
      "Two detuned layers, slow attacks and vibrato when you press harder.",
      {
        { "cs80_detune", { { "cs80_detune", 0.12f, 0.35f, 0.18f } },
          "DESAFINÁ LA CAPA II", "En MEZCLA subí DESAF. II. Las dos capas se separan y el sonido se vuelve enorme.",
          "DETUNE LAYER II", "In MIX turn up DETUNE II. The two layers drift apart and the sound gets huge." },
        { "cs80_l1_attack", { { "cs80_l1_attack", 0.25f, 0.9f, 0.4f }, { "cs80_l1_fenv", 0.55f, 0.9f, 0.65f } },
          "CAPA I: QUE CREZCA", "En la capa I: ATAQUE más lento y ENV del filtro más alto. El metal crece en vez de golpear.",
          "LAYER I: LET IT SWELL", "On layer I: slower ATTACK and higher filter ENV. The brass swells instead of hitting." },
        { "cs80_l2_lpf", { { "cs80_l2_lpf", 1300, 2800, 2000 }, { "cs80_l2_release", 2.4f, 7.0f, 3.0f } },
          "CAPA II: MÁS OSCURA Y LARGA", "Tocá la pestaña II. Bajá PASABAJOS y alargá LIBERACIÓN: la capa II queda de fondo, sosteniendo.",
          "LAYER II: DARKER AND LONGER", "Click tab II. Lower LOW-PASS and lengthen RELEASE: layer II stays in the back, holding." },
        { "cs80_at_vib", { { "cs80_at_vib", 0.55f, 1.0f, 0.65f } },
          "EXPRESIÓN", "En AFTERTOUCH subí VIBRATO. Si tu teclado tiene aftertouch, apretá más fuerte una nota ya tocada: vibra como un instrumento de verdad.",
          "EXPRESSION", "In AFTERTOUCH turn up VIBRATO. If your keyboard has aftertouch, press harder on a held note: it vibrates like a real instrument." },
        { "", { { "@note", 0, 0, 0 } },
          "ACORDES ÉPICOS", "Acordes graves y lentos. Subí ESPACIO al máximo para el cine.",
          "EPIC CHORDS", "Slow low chords. Turn SPACE all the way up for the movie vibe." },
      } },

    // =================================================================== TRAP PROD (DX7)
    { "dx_ep", 5, "PIANO ELÉCTRICO FM", "FM ELECTRIC PIANO",
      "piano electrico rhodes wurli wurlitzer epiano keys teclas rnb r&b soul neo lofi fm dx7 dx",
      "Tres pares de operadores: cada modulador le da brillo a su portador.",
      "Three operator pairs: each modulator adds brightness to its carrier.",
      {
        { "dx7_algorithm", {},
          "EL ALGORITMO", "TRAP PROD arranca en el algoritmo 5: tres parejas. Los operadores de abajo (1, 3, 5) son los que escuchás; los de arriba (2, 4, 6) les cambian el timbre.",
          "THE ALGORITHM", "TRAP PROD starts on algorithm 5: three pairs. The bottom operators (1, 3, 5) are the ones you hear; the top ones (2, 4, 6) change their tone." },
        { "dx7_op2_level", { { "dx7_op2_level", 0.6f, 0.85f, 0.66f } },
          "MÁS GOLPE", "En NIVEL DE LOS 6 OPERADORES subí OP 2. Es el modulador del cuerpo: más nivel = más brillo y más ataque.",
          "MORE PUNCH", "In LEVEL OF THE 6 OPERATORS turn up OP 2. It's the body's modulator: more level = more brightness and attack." },
        { "dx7_op4_level", { { "dx7_op4_level", 0.4f, 0.7f, 0.45f } },
          "EL 'TINE'", "Subí OP 4. Está en ratio 14: agrega ese tintineo metálico del Rhodes al tocar fuerte.",
          "THE TINE", "Turn up OP 4. It's at ratio 14: it adds the metallic Rhodes tine when you play hard." },
        { "dx7_op2_decay", { { "dx7_op2_decay", 0.3f, 0.95f, 0.6f } },
          "QUE EL BRILLO CAIGA", "Elegí OP 2 arriba y bajá su CAÍDA. El brillo se apaga antes que la nota: así es un piano eléctrico.",
          "LET THE BRIGHTNESS FALL", "Select OP 2 above and lower its DECAY. The brightness fades before the note: that's an electric piano." },
        { "dx7_lfo_amp", { { "dx7_lfo_amp", 0.15f, 0.45f, 0.25f } },
          "TRÉMOLO", "En FEEDBACK / LFO subí LFO VOLUMEN: el volumen ondula, como el trémolo de los Rhodes.",
          "TREMOLO", "In FEEDBACK / LFO turn up LFO AMP: the volume wobbles, like a Rhodes tremolo." },
        { "", { { "@note", 0, 0, 0 } },
          "ACORDES SOUL", "Acordes con séptima y novena. Tocar suave o fuerte cambia el brillo.",
          "SOUL CHORDS", "Seventh and ninth chords. Playing soft or hard changes the brightness." },
      } },

    { "dx_bell", 5, "CAMPANA FM", "FM BELL",
      "campana campanas bell bells fm glass vidrio cristal celesta metalico dx7 dx navidad",
      "Un modulador con ratio no entero crea armónicos de campana.",
      "A modulator with a non-whole ratio creates bell harmonics.",
      {
        { "band:NIVEL DE LOS 6 OPERADORES", { { "dx7_op3_level", 0.0f, 0.05f, 0.0f }, { "dx7_op5_level", 0.0f, 0.05f, 0.0f } },
          "UNA SOLA PAREJA", "En NIVEL DE LOS 6 OPERADORES bajá OP 3 y OP 5 a cero. Nos quedamos solo con la pareja 2 → 1.",
          "A SINGLE PAIR", "In LEVEL OF THE 6 OPERATORS turn OP 3 and OP 5 down to zero. We keep only the 2 → 1 pair." },
        { "dx7_op2_coarse", { { "dx7_op2_coarse", 3, 3, 3 }, { "dx7_op2_fine", 0.4f, 0.6f, 0.5f } },
          "RATIO 3.5", "Elegí OP 2 y poné RATIO en 3 y FINO a la mitad (3.5). Un ratio entero suena armónico; uno 'roto' suena a metal y a campana.",
          "RATIO 3.5", "Select OP 2 and set RATIO to 3 and FINE to half (3.5). A whole ratio sounds harmonic; a 'broken' one sounds like metal and bells." },
        { "dx7_op2_level", { { "dx7_op2_level", 0.6f, 0.85f, 0.7f } },
          "CUÁNTA CAMPANA", "Subí OP 2. Más nivel del modulador = más armónicos.",
          "HOW MUCH BELL", "Turn up OP 2. More modulator level = more harmonics." },
        { "dx7_op1_decay", { { "dx7_op1_decay", 3.0f, 8.0f, 4.0f } },
          "QUE QUEDE SONANDO", "Elegí OP 1 y alargá su CAÍDA. La campana sigue sonando después del golpe.",
          "LET IT RING", "Select OP 1 and lengthen its DECAY. The bell keeps ringing after the strike." },
        { "dx7_feedback", { { "dx7_feedback", 0.0f, 0.1f, 0.0f } },
          "SIN FEEDBACK", "Bajá FEEDBACK a cero: la campana queda limpia, de cristal.",
          "NO FEEDBACK", "Bring FEEDBACK to zero: the bell stays clean, like glass." },
        { "", { { "@note", 0, 0, 0 } },
          "ARRIBA DEL TECLADO", "Notas agudas, pocas. Probá otros ratios del OP 2: 1.41, 2.5, 7.",
          "HIGH ON THE KEYBOARD", "High notes, sparse. Try other OP 2 ratios: 1.41, 2.5, 7." },
      } },

    { "dx_bass", 5, "BAJO FM", "FM BASS",
      "bajo bass fm digital slap 80s ochentas dx7 dx funk pop bajos",
      "Un bajo digital con golpe: modulador con caída corta y feedback.",
      "A punchy digital bass: a modulator with short decay and feedback.",
      {
        { "band:NIVEL DE LOS 6 OPERADORES", { { "dx7_op3_level", 0.0f, 0.05f, 0.0f }, { "dx7_op5_level", 0.0f, 0.05f, 0.0f } },
          "UNA PAREJA", "Bajá OP 3 y OP 5 a cero. Un bajo no necesita tres capas.",
          "ONE PAIR", "Turn OP 3 and OP 5 down to zero. A bass doesn't need three layers." },
        { "dx7_op2_level", { { "dx7_op2_level", 0.7f, 0.95f, 0.8f } },
          "MODULADOR FUERTE", "Subí OP 2 bastante: es el que le da el ataque.",
          "STRONG MODULATOR", "Turn OP 2 well up: it's what gives the attack." },
        { "dx7_op2_decay", { { "dx7_op2_decay", 0.1f, 0.45f, 0.25f } },
          "EL CHASQUIDO", "Elegí OP 2 y bajá su CAÍDA: el brillo dura un instante, como un dedo golpeando la cuerda.",
          "THE SNAP", "Select OP 2 and lower its DECAY: the brightness lasts an instant, like a finger slapping the string." },
        { "dx7_op1_decay", { { "dx7_op1_decay", 0.5f, 1.6f, 1.0f } },
          "NOTA MÁS CORTA", "Elegí OP 1 y acortá su CAÍDA: el bajo no queda sonando de más.",
          "SHORTER NOTE", "Select OP 1 and shorten its DECAY: the bass doesn't ring on." },
        { "dx7_feedback", { { "dx7_feedback", 0.5f, 0.85f, 0.6f } },
          "RUGOSIDAD", "Subí FEEDBACK: el operador se modula a sí mismo y suena más áspero, casi como una sierra.",
          "GRIT", "Turn up FEEDBACK: the operator modulates itself and gets rougher, almost like a saw." },
        { "", { { "@note", 0, 0, 0 } },
          "TOCÁ GRAVE", "Notas cortas entre C1 y C3. Tocar fuerte = más chasquido.",
          "PLAY LOW", "Short notes between C1 and C3. Playing hard = more snap." },
      } },

    // =================================================================== ARTURITO (M1)
    { "m1_piano", 6, "PIANO HOUSE", "HOUSE PIANO",
      "piano house 90s noventas m1 korg acordes stabs dance club gospel",
      "El piano de los 90: una muestra, poco sostén y algo de chorus.",
      "The 90s piano: one sample, little sustain and some chorus.",
      {
        { "m1_s2_level", { { "m1_s2_level", 0.0f, 0.05f, 0.0f } },
          "SOLO EL PIANO", "En FUENTE 2 bajá NIVEL a cero. La FUENTE 1 ya es PIANO.",
          "JUST THE PIANO", "In SOURCE 2 turn LEVEL to zero. SOURCE 1 is already PIANO." },
        { "m1_vel_filter", { { "m1_vel_filter", 0.6f, 1.0f, 0.7f } },
          "QUE RESPONDA", "En FILTRO DIGITAL subí VELOCIDAD: tocar fuerte suena más brillante, como un piano de verdad.",
          "MAKE IT RESPOND", "In DIGITAL FILTER turn up VELOCITY: playing hard sounds brighter, like a real piano." },
        { "band:ENVOLVENTE DE VOLUMEN", { { "m1_a_sustain", 0.15f, 0.5f, 0.35f }, { "m1_a_release", 0.25f, 0.7f, 0.45f } },
          "STABS", "En ENVOLVENTE DE VOLUMEN: SOSTÉN bajo y LIBERACIÓN corta. Los acordes de house son cortos y rítmicos.",
          "STABS", "In AMP ENVELOPE: low SUSTAIN and short RELEASE. House chords are short and rhythmic." },
        { "m1_fx_mix", { { "m1_fx_mix", 0.08f, 0.25f, 0.18f } },
          "UN TOQUE DE CHORUS", "En EFECTOS bajá MEZCLA: el chorus apenas, para que no se lave.",
          "A TOUCH OF CHORUS", "In EFFECTS lower MIX: just a bit of chorus so it doesn't wash out." },
        { "", { { "@note", 0, 0, 0 } },
          "ACORDES CON SÉPTIMA", "Acordes menores con séptima en contratiempo. Eso es house.",
          "SEVENTH CHORDS", "Minor seventh chords on the offbeat. That's house." },
      } },

    { "m1_organ", 6, "ÓRGANO HOUSE", "HOUSE ORGAN",
      "organo organ house gospel iglesia church 90s m1",
      "Órgano sostenido, sin cola, con chorus como un Leslie.",
      "Sustained organ, no tail, with chorus like a Leslie.",
      {
        { "m1_s1_wave", { { "m1_s1_wave", 2, 2, 2 } },
          "LA MUESTRA", "En FUENTE 1 cambiá MUESTRA a ÓRGANO con las flechas.",
          "THE SAMPLE", "In SOURCE 1 change SAMPLE to ORGAN with the arrows." },
        { "m1_s2_level", { { "m1_s2_level", 0.0f, 0.05f, 0.0f } },
          "UNA SOLA", "Bajá NIVEL de la FUENTE 2 a cero.",
          "JUST ONE", "Turn SOURCE 2 LEVEL down to zero." },
        { "band:ENVOLVENTE DE VOLUMEN", { { "m1_a_sustain", 0.9f, 1.0f, 1.0f }, { "m1_a_release", 0.02f, 0.2f, 0.08f } },
          "COMO UN ÓRGANO", "SOSTÉN al máximo y LIBERACIÓN casi cero: un órgano suena mientras apretás y se corta al soltar.",
          "LIKE AN ORGAN", "SUSTAIN all the way up and RELEASE almost zero: an organ sounds while you hold and stops when you let go." },
        { "m1_fx_mix", { { "m1_fx_mix", 0.45f, 0.8f, 0.55f } },
          "EL LESLIE", "En EFECTOS subí MEZCLA del chorus: imita el parlante giratorio de los órganos.",
          "THE LESLIE", "In EFFECTS turn up the chorus MIX: it imitates the rotating speaker of organs." },
        { "", { { "@note", 0, 0, 0 } },
          "TOCÁ", "Acordes ligados, o una línea de bajo con el órgano (el clásico de los 90).",
          "PLAY", "Legato chords, or a bass line with the organ (the 90s classic)." },
      } },

    { "m1_pad", 6, "PAD UNIVERSO", "UNIVERSE PAD",
      "pad pads coro choir voces voices universo universe m1 atmosfera ambient colchon celestial",
      "Dos muestras mezcladas y desafinadas: el truco del M1.",
      "Two blended, detuned samples: the M1 trick.",
      {
        { "m1_s1_wave", { { "m1_s1_wave", 4, 4, 4 } },
          "CORO", "En FUENTE 1 elegí CORO.",
          "CHOIR", "In SOURCE 1 pick CHOIR." },
        { "m1_s2_level", { { "m1_s2_level", 0.55f, 0.9f, 0.65f } },
          "MÁS PAD", "La FUENTE 2 ya es PAD UNIVERSO: subile el NIVEL. Coro + pad era la mezcla de fábrica más famosa.",
          "MORE PAD", "SOURCE 2 is already UNIVERSE PAD: turn its LEVEL up. Choir + pad was the most famous factory blend." },
        { "m1_s2_fine", { { "m1_s2_fine", 10, 25, 14 } },
          "DESAFINÁ UNOS CENTS", "Subí CENTS de la FUENTE 2. Las dos muestras se separan y se abre el sonido.",
          "DETUNE A FEW CENTS", "Raise SOURCE 2 CENTS. The two samples drift apart and the sound opens up." },
        { "band:ENVOLVENTE DE VOLUMEN", { { "m1_a_attack", 0.4f, 2.5f, 0.8f }, { "m1_a_release", 1.5f, 5.0f, 2.5f } },
          "LENTO", "ATAQUE a la mitad y LIBERACIÓN larga.",
          "SLOW", "ATTACK at half and a long RELEASE." },
        { "m1_fx_type", { { "m1_fx_type", 3, 3, 3 } },
          "CHORUS + DELAY", "En EFECTOS elegí CHORUS+DELAY. Los ecos llenan el espacio entre acordes.",
          "CHORUS + DELAY", "In EFFECTS pick CHORUS+DELAY. The echoes fill the space between chords." },
        { "", { { "@note", 0, 0, 0 } },
          "ACORDES LARGOS", "Dejalos sonar. Bajá BRILLO en las macros si está muy chillón.",
          "LONG CHORDS", "Let them ring. Lower BRIGHTNESS in the macros if it's too shrill." },
      } },

    // =================================================================== CALAVERA (ARP 2600)
    { "arp_acid", 7, "BAJO ÁCIDO", "ACID BASS",
      "acido acid 303 squelch bajo bass acid-house techno resonancia chillido",
      "Un oscilador, mucha resonancia y una envolvente corta: el chillido ácido.",
      "One oscillator, lots of resonance and a short envelope: the acid squelch.",
      {
        { "arp2600_vco2_level", { { "arp2600_vco2_level", 0.0f, 0.05f, 0.0f } },
          "UN SOLO VCO", "En VCO 2 bajá NIVEL a cero. El ácido es una sola onda.",
          "ONE VCO", "In VCO 2 turn LEVEL down to zero. Acid is a single wave." },
        { "band:FILTRO", { { "arp2600_res", 0.7f, 0.93f, 0.8f }, { "arp2600_cutoff", 250, 800, 500 } },
          "RESONANCIA ALTA", "En FILTRO: RESON. bien alta y CORTE abajo. Ya debería silbar.",
          "HIGH RESONANCE", "In FILTER: RESON well up and CUTOFF low. It should already whistle." },
        { "arp2600_fenv", { { "arp2600_fenv", 0.6f, 1.0f, 0.75f } },
          "EL ADSR AL FILTRO", "Subí ADSR en FILTRO: la envolvente abre el filtro en cada nota.",
          "ADSR TO FILTER", "Turn up ADSR in FILTER: the envelope opens the filter on every note." },
        { "band:ADSR", { { "arp2600_decay", 0.08f, 0.3f, 0.18f }, { "arp2600_sustain", 0.0f, 0.15f, 0.05f } },
          "EL SQUELCH", "En ADSR: CAÍDA corta y SOSTÉN casi cero. El filtro se abre de golpe y cierra: 'yiuc'.",
          "THE SQUELCH", "In ADSR: short DECAY and SUSTAIN almost zero. The filter snaps open and shut: 'squelch'." },
        { "arp2600_porta", { { "arp2600_porta", 0.03f, 0.15f, 0.07f } },
          "EL SLIDE", "En PORTAMENTO subí TIEMPO un poco. Las notas ligadas se deslizan, como en la 303.",
          "THE SLIDE", "In PORTAMENTO turn TIME up a little. Legato notes slide, like on the 303." },
        { "", { { "@note", 0, 0, 0 } },
          "PATRÓN DE 16AVOS", "Tocá un patrón rápido y, mientras suena, movés CORTE. Eso es acid house.",
          "16TH-NOTE PATTERN", "Play a fast pattern and move CUTOFF while it runs. That's acid house." },
      } },

    { "arp_sh", 7, "EFECTO SAMPLE & HOLD", "SAMPLE & HOLD EFFECT",
      "sample hold s&h sh efecto efectos fx robot random aleatorio azar scifi sci-fi computadora blips",
      "Un cable lleva valores al azar al filtro: el sonido de computadora de las películas.",
      "A cable sends random values to the filter: the movie computer sound.",
      {
        { "arp2600_c1_amt", { { "arp2600_c1_amt", 0.5f, 1.0f, 0.7f } },
          "ENCHUFÁ EL CABLE", "En PATCH BAY el CABLE 1 ya va de S&H a FILTRO. Subí su CANTIDAD: el filtro empieza a saltar a valores al azar.",
          "PLUG THE CABLE", "In PATCH BAY, CABLE 1 already goes from S&H to FILTER. Turn up its AMOUNT: the filter starts jumping to random values." },
        { "arp2600_res", { { "arp2600_res", 0.6f, 0.9f, 0.7f } },
          "QUE SE NOTE", "Subí RESON. en FILTRO: cada salto se escucha como un 'blip'.",
          "MAKE IT OBVIOUS", "Turn up RESON in FILTER: every jump sounds like a 'blip'." },
        { "arp2600_sh_rate", { { "arp2600_sh_rate", 2, 6, 4 } },
          "LA VELOCIDAD", "En S&H bajá RELOJ: cuántos saltos por segundo.",
          "THE SPEED", "In S&H lower CLOCK: how many jumps per second." },
        { "arp2600_sustain", { { "arp2600_sustain", 0.8f, 1.0f, 0.9f } },
          "NOTA SOSTENIDA", "En ADSR subí SOSTÉN para que la nota se mantenga y se escuchen los saltos.",
          "HELD NOTE", "In ADSR turn up SUSTAIN so the note holds and you hear the jumps." },
        { "", { { "@note", 0, 0, 0 } },
          "MANTENÉ UNA NOTA", "Dejá una nota apretada. Probá enchufar el S&H a VCO 1 en otro cable: notas al azar.",
          "HOLD A NOTE", "Keep a note held. Try plugging S&H into VCO 1 on another cable: random notes." },
      } },

    // =================================================================== EL QUE LA MATA (VCS3)
    { "vcs_laser", 8, "EFECTO ESPACIAL", "SPACE EFFECT",
      "efecto efectos fx espacial space laser ovni ufo scifi sci-fi raro weird riser transicion vcs3 vcs",
      "La matriz de pines: un oscilador lento modulando el tono de otro.",
      "The pin matrix: a slow oscillator modulating another one's pitch.",
      {
        { "band:MATRIZ DE PINES (FUENTE > DESTINO)", { { "vcs3_pin_24", 1, 1, 1 } },
          "PONÉ UN PIN", "En la MATRIZ, fila OSC 3 y columna TONO 1: poné el pin. Ahora el oscilador 3 mueve el tono del 1.",
          "INSERT A PIN", "In the MATRIX, row OSC 3 and column PITCH 1: insert the pin. Oscillator 3 now moves the pitch of oscillator 1." },
        { "band:OSCILADOR 3", { { "vcs3_osc3_rate", 2, 9, 4 } },
          "MÁS RÁPIDO", "En OSCILADOR 3 subí VELOC.: el tono sube y baja varias veces por segundo.",
          "FASTER", "In OSCILLATOR 3 turn up RATE: the pitch goes up and down several times per second." },
        { "vcs3_depth_pitch", { { "vcs3_depth_pitch", 0.4f, 0.85f, 0.5f } },
          "CUÁNTO", "En PROFUNDIDAD DE LA MATRIZ subí TONO: cuánto mueven los pines el tono.",
          "HOW MUCH", "In MATRIX DEPTH turn up PITCH: how much the pins move the pitch." },
        { "vcs3_env_trig", { { "vcs3_env_trig", 1, 1, 1 } },
          "QUE SE REPITA", "En TRAPEZOIDE poné DISPARO en REPETIR: la envolvente se dispara sola una y otra vez.",
          "MAKE IT REPEAT", "In TRAPEZOID set TRIGGER to REPEAT: the envelope fires by itself again and again." },
        { "vcs3_reverb", { { "vcs3_reverb", 0.4f, 0.85f, 0.5f } },
          "EL RESORTE", "Subí REVERB del RESORTE: el eco metálico del VCS3.",
          "THE SPRING", "Turn up the SPRING REVERB: the VCS3's metallic echo." },
        { "", { { "@note", 0, 0, 0 } },
          "MANTENÉ UNA NOTA", "Mientras suena, arrastrá el JOYSTICK. Ideal para transiciones e intros.",
          "HOLD A NOTE", "While it plays, drag the JOYSTICK. Great for transitions and intros." },
      } },

    { "vcs_wind", 8, "VIENTO Y OLAS", "WIND AND WAVES",
      "viento wind ruido noise ola olas mar sea ocean textura texture riser barrido sweep vcs3 vcs ambiente",
      "Ruido por un filtro resonante que un LFO mueve despacio.",
      "Noise through a resonant filter slowly moved by an LFO.",
      {
        { "band:MATRIZ DE PINES (FUENTE > DESTINO)", { { "vcs3_pin_00", 0, 0, 0 }, { "vcs3_pin_10", 0, 0, 0 }, { "vcs3_pin_30", 1, 1, 1 } },
          "SOLO RUIDO", "En la MATRIZ sacá los pines de OSC 1 y OSC 2 en FILTRO, y poné RUIDO > FILTRO. Ahora entra solo el ruido.",
          "NOISE ONLY", "In the MATRIX remove the OSC 1 and OSC 2 pins on FILTER, and insert NOISE > FILTER. Only noise goes in now." },
        { "vcs3_res", { { "vcs3_res", 0.6f, 0.9f, 0.7f } },
          "QUE SILBE", "En FILTRO subí RESPUESTA: el ruido toma un tono, como el viento.",
          "MAKE IT WHISTLE", "In FILTER turn up RESPONSE: the noise takes on a pitch, like wind." },
        { "vcs3_depth_filter", { { "vcs3_depth_filter", 0.55f, 0.9f, 0.65f } },
          "LA OLA", "El OSC 3 lento ya está enchufado a CORTE. En PROFUNDIDAD DE LA MATRIZ subí CORTE: el filtro sube y baja como una ola.",
          "THE WAVE", "The slow OSC 3 is already patched to CUTOFF. In MATRIX DEPTH turn up CUTOFF: the filter rises and falls like a wave." },
        { "band:TRAPEZOIDE", { { "vcs3_env_attack", 0.5f, 3.0f, 1.0f }, { "vcs3_env_decay", 1.0f, 5.0f, 2.0f } },
          "QUE CREZCA Y SE VAYA", "En TRAPEZOIDE: ATAQUE y CAÍDA largos.",
          "SWELL AND FADE", "In TRAPEZOID: long ATTACK and DECAY." },
        { "", { { "@note", 0, 0, 0 } },
          "MANTENÉ UNA NOTA", "Cambiá VELOC. del OSC 3 para olas más rápidas o más lentas. Con ESPACIO alto, queda de película.",
          "HOLD A NOTE", "Change OSC 3 RATE for faster or slower waves. With SPACE high, it's cinematic." },
      } },
    };
    return list;
}

const Lesson* find (const String& id)
{
    for (auto& l : lessons()) if (id == l.id) return &l;
    return nullptr;
}

String greeting (int engine)
{
    static const char* es[] = {
        "Soy KROKO. Bajos gordos y leads que muerden: eso es lo mío.",
        "PROFETA acá. Metales, campanas y leads rasgados.",
        "Soy PITER: pads enormes y arpegios.",
        "NO JUNA. Chorus, pads que respiran y bajos con sub.",
        "ILLUMINARY. Metales de película y sonidos que se expresan.",
        "TRAP PROD. FM: pianos eléctricos, campanas y bajos digitales.",
        "ARTURITO. Pianos, órganos y pads de los 90.",
        "CALAVERA. Cables, ácido y efectos raros.",
        "EL QUE LA MATA. Ruido, pines y efectos del espacio.",
    };
    static const char* en[] = {
        "I'm KROKO. Fat basses and leads that bite: that's my thing.",
        "PROFETA here. Brass, bells and tearing leads.",
        "I'm PITER: huge pads and arpeggios.",
        "NO JUNA. Chorus, breathing pads and sub basses.",
        "ILLUMINARY. Movie brass and expressive sounds.",
        "TRAP PROD. FM: electric pianos, bells and digital basses.",
        "ARTURITO. 90s pianos, organs and pads.",
        "CALAVERA. Cables, acid and weird effects.",
        "EL QUE LA MATA. Noise, pins and space effects.",
    };
    engine = jlimit (0, 8, engine);
    return String::fromUTF8 (i18n::english() ? en[engine] : es[engine]);
}

const std::vector<Modifier>& modifiers()
{
    static const std::vector<Modifier> list {
        { "oscuro oscura dark darker apagado opaco grave calido warm", "oscuro", "dark",
          { "macro_brightness", { { "macro_brightness", 0.0f, 0.36f, 0.25f } },
            "MÁS OSCURO", "Pediste oscuro: bajá BRILLO en las macros. Cierra los filtros de todos los motores a la vez.",
            "DARKER", "You asked for dark: lower BRIGHTNESS in the macros. It closes every engine's filter at once." } },
        { "brillante brilloso bright brighter claro agudo aire airy", "brillante", "bright",
          { "macro_brightness", { { "macro_brightness", 0.65f, 1.0f, 0.75f } },
            "MÁS BRILLANTE", "Pediste brillante: subí BRILLO en las macros.",
            "BRIGHTER", "You asked for bright: raise BRIGHTNESS in the macros." } },
        { "sucio sucia dirty saturado saturated distorsion distorsionado distorted crunchy crudo gritty", "sucio", "dirty",
          { "character", { { "character", 0.5f, 0.74f, 0.62f } },
            "MÁS SUCIO", "Pediste sucio: llevá CHARACTER (en el panel PERSONAJE) a SUCIO. Satura todo el plugin.",
            "DIRTIER", "You asked for dirty: turn CHARACTER (in the CHARACTER panel) to DIRTY. It saturates the whole plugin." } },
        { "roto rota broken lofi lo-fi bitcrush destruido", "roto", "broken",
          { "character", { { "character", 0.76f, 1.0f, 0.85f } },
            "ROTO", "Pediste roto / lo-fi: llevá CHARACTER a ROTO. Satura fuerte y además reduce bits.",
            "BROKEN", "You asked for broken / lo-fi: turn CHARACTER to BROKEN. Heavy saturation plus bit reduction." } },
        { "limpio limpia clean puro pure", "limpio", "clean",
          { "character", { { "character", 0.0f, 0.24f, 0.1f } },
            "LIMPIO", "Pediste limpio: llevá CHARACTER a LIMPIO.",
            "CLEAN", "You asked for clean: turn CHARACTER to CLEAN." } },
        { "gordo gorda fat pesado pesada heavy grueso thick", "gordo", "fat",
          { "macro_body", { { "macro_body", 0.68f, 1.0f, 0.75f } },
            "MÁS GORDO", "Pediste gordo: subí CUERPO en las macros. Suma graves a todo.",
            "FATTER", "You asked for fat: raise BODY in the macros. It adds low end to everything." } },
        { "fino fina thin liviano light", "fino", "thin",
          { "macro_body", { { "macro_body", 0.0f, 0.32f, 0.25f } },
            "MÁS FINO", "Pediste fino: bajá CUERPO en las macros.",
            "THINNER", "You asked for thin: lower BODY in the macros." } },
        { "largo larga long cola colas tail sostenido", "largo", "long",
          { "macro_release", { { "macro_release", 0.68f, 1.0f, 0.75f } },
            "MÁS LARGO", "Pediste largo: subí LIBERACIÓN en las macros. Alarga la cola de todos los motores.",
            "LONGER", "You asked for long: raise RELEASE in the macros. It lengthens every engine's tail." } },
        { "corto corta short staccato seco cortito", "corto", "short",
          { "macro_release", { { "macro_release", 0.0f, 0.32f, 0.25f } },
            "MÁS CORTO", "Pediste corto: bajá LIBERACIÓN en las macros.",
            "SHORTER", "You asked for short: lower RELEASE in the macros." } },
        { "suave soft lento slow tranquilo smooth", "suave", "soft",
          { "macro_attack", { { "macro_attack", 0.66f, 1.0f, 0.72f } },
            "MÁS SUAVE", "Pediste suave: subí ATAQUE en las macros. Las notas entran más despacio.",
            "SOFTER", "You asked for soft: raise ATTACK in the macros. Notes fade in more slowly." } },
        { "golpe pegada punch punchy agresivo aggressive", "con golpe", "punchy",
          { "macro_attack", { { "macro_attack", 0.0f, 0.32f, 0.2f } },
            "MÁS GOLPE", "Pediste golpe: bajá ATAQUE en las macros. Las notas entran de una.",
            "MORE PUNCH", "You asked for punch: lower ATTACK in the macros. Notes hit instantly." } },
        { "ancho ancha wide estereo stereo abierto", "ancho", "wide",
          { "macro_width", { { "macro_width", 0.7f, 1.0f, 0.8f } },
            "MÁS ANCHO", "Pediste ancho: subí ANCHURA en las macros.",
            "WIDER", "You asked for wide: raise WIDTH in the macros." } },
        { "reverb espacio space ambiente espacial hall salon eco", "con espacio", "spacious",
          { "macro_space", { { "macro_space", 0.6f, 1.0f, 0.7f } },
            "MÁS ESPACIO", "Pediste espacio: subí ESPACIO en las macros (la reverb de todo el plugin).",
            "MORE SPACE", "You asked for space: raise SPACE in the macros (the whole plugin's reverb)." } },
        { "vibrato wobble movimiento moving mueve ondulante", "con movimiento", "moving",
          { "macro_movement", { { "macro_movement", 0.6f, 1.0f, 0.65f } },
            "MÁS MOVIMIENTO", "Pediste movimiento: subí MOVIMIENTO en las macros (vibrato y filtro con LFO).",
            "MORE MOTION", "You asked for motion: raise MOVEMENT in the macros (vibrato and LFO filter)." } },
    };
    return list;
}

//============================================================================== buscador
String normalise (const String& s)
{
    static const std::map<juce_wchar, juce_wchar> accents {
        { 0xe1, 'a' }, { 0xe9, 'e' }, { 0xed, 'i' }, { 0xf3, 'o' }, { 0xfa, 'u' }, { 0xfc, 'u' }, { 0xf1, 'n' },
        { 0xc1, 'a' }, { 0xc9, 'e' }, { 0xcd, 'i' }, { 0xd3, 'o' }, { 0xda, 'u' }, { 0xdc, 'u' }, { 0xd1, 'n' },
    };
    String out;
    for (auto c : s)
    {
        if (auto it = accents.find (c); it != accents.end()) c = it->second;
        c = CharacterFunctions::toLowerCase (c);
        out += (CharacterFunctions::isLetterOrDigit (c) || c == '&') ? c : (juce_wchar) ' ';
    }
    return out;
}

static StringArray tokens (const String& s)
{
    auto t = StringArray::fromTokens (normalise (s), " ", "");
    t.removeEmptyStrings();
    return t;
}

static String stem (const String& w) { return (w.length() > 4 && w.endsWithChar ('s')) ? w.dropLastCharacters (1) : w; }

static bool matches (const String& query, const String& key)
{
    const auto q = stem (query), k = stem (key);
    if (q == k) return true;
    return q.length() >= 4 && k.length() >= 4 && (k.startsWith (q) || q.startsWith (k));
}

// palabras que nombran un motor o un personaje (dan preferencia a sus recetas)
static int engineFor (const String& w)
{
    static const std::map<String, int> names {
        { "minimoog", 0 }, { "moog", 0 }, { "kroko", 0 },
        { "prophet", 1 }, { "profeta", 1 }, { "sequential", 1 },
        { "jupiter", 2 }, { "piter", 2 },
        { "juno", 3 }, { "juna", 3 },
        { "cs80", 4 }, { "illuminary", 4 }, { "yamaha", 4 },
        { "dx7", 5 }, { "dx", 5 },
        { "m1", 6 }, { "arturito", 6 }, { "korg", 6 },
        { "2600", 7 }, { "calavera", 7 },
        { "vcs3", 8 }, { "vcs", 8 }, { "mata", 8 },
    };
    auto it = names.find (w);
    return it != names.end() ? it->second : -1;
}

// palabras que no dicen nada del sonido
static bool isFiller (const String& w)
{
    static const StringArray f { "un", "una", "uno", "el", "la", "los", "las", "de", "del", "con", "sin", "para", "por", "y", "o", "que",
                                 "como", "hago", "hacer", "quiero", "quisiera", "me", "mi", "algo", "tipo", "estilo", "sonido", "sonidos",
                                 "a", "an", "the", "of", "with", "for", "and", "or", "how", "make", "want", "i", "my", "some", "sound", "sounds",
                                 "style", "like", "un", "mas", "more", "muy", "very", "bien", "ensename", "teach", "me", "en", "in" };
    return f.contains (w);
}

Result search (const String& text)
{
    Result r;
    const auto words = tokens (text);

    // modificadores (no cuentan para elegir receta)
    StringArray soundWords, modWords;   // las palabras de modificador tambien cuentan un poco (ej. "efecto espacial")
    std::vector<int> engineBoost;
    for (auto& w : words)
    {
        if (isFiller (w)) continue;
        if (const int e = engineFor (w); e >= 0) { engineBoost.push_back (e); continue; }
        bool isMod = false;
        for (auto& m : modifiers())
            for (auto& k : StringArray::fromTokens (m.words, " ", ""))
                if (matches (w, k))
                {
                    isMod = true;
                    if (std::find (r.modifiers.begin(), r.modifiers.end(), &m) == r.modifiers.end()) r.modifiers.push_back (&m);
                }
        if (isMod) modWords.add (w); else soundWords.add (w);
    }

    // los modificadores que se pisan (oscuro y brillante...) se quedan con el ultimo
    for (size_t i = 0; i < r.modifiers.size(); ++i)
        for (size_t j = i + 1; j < r.modifiers.size(); ++j)
            if (String (r.modifiers[i]->step.target) == r.modifiers[j]->step.target)
            { r.modifiers.erase (r.modifiers.begin() + (long) i); --i; break; }

    std::vector<std::pair<int, const Lesson*>> scored;
    for (auto& l : lessons())
    {
        const auto keys = tokens (String::fromUTF8 (l.keywords) + " " + String::fromUTF8 (l.titleEs) + " " + String::fromUTF8 (l.titleEn));
        int score = 0;
        for (auto& w : soundWords)
        {
            int best = 0;
            for (auto& k : keys)
                if (matches (w, k)) best = jmax (best, stem (w) == stem (k) ? 3 : 2);
            score += best;
        }
        if (score > 0)
            for (auto& w : modWords)
                for (auto& k : keys)
                    if (matches (w, k)) { ++score; break; }
        const bool boosted = std::find (engineBoost.begin(), engineBoost.end(), l.engine) != engineBoost.end();
        if (score > 0 && boosted) score += 4;
        if (score > 0) scored.push_back ({ score, &l });
    }
    // solo dijo un motor ("kroko"): todas las recetas de ese motor
    if (scored.empty() && soundWords.isEmpty() && ! engineBoost.empty())
        for (auto& l : lessons())
            if (std::find (engineBoost.begin(), engineBoost.end(), l.engine) != engineBoost.end())
                scored.push_back ({ l.basics ? 1 : 2, &l });

    std::stable_sort (scored.begin(), scored.end(), [] (auto& a, auto& b) { return a.first > b.first; });
    for (auto& s : scored) r.lessons.push_back (s.second);
    r.understood = ! r.lessons.empty();
    return r;
}
}
