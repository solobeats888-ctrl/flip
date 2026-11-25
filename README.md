# FLIPPER — Viszeversza

Sintetizador híbrido de nueve motores con la interfaz de una máquina de pinball vintage.
La imagen `Reference/flipper_reference_aprobada.png` es la autoridad visual: no se rediseña.

## Estado

Funciona de verdad:
- Interfaz JUGAR escalable (proporción 2:3, de 512×768 a 1024×1536; recuerda el tamaño).
- Los nueve personajes se seleccionan con un click: placa iluminada con lamparitas y foco sobre el personaje. Las placas del tablero dicen el nombre del sintetizador; el nombre del personaje aparece solo en el panel PERSONAJE, con su retrato.
- MEZCLA DE MOTORES: LED = activar/desactivar, cápsula = nivel (arrastrar; doble click = 70 %). Un motor apagado queda en sombra en el playfield, sin desaparecer.
- 8 macros + CHARACTER: parámetros reales del host (automatizables, se guardan con el proyecto). Doble click = valor por defecto; al pasar el mouse se ve el valor.
- GENERAR / VARIACIONES usan el modo, estilo y bloqueos elegidos en LAB.
- FAVORITO guarda el sonido como preset marcado con ★ (se ve en LAB).
- JUGAR / DISEÑO / LAB cambian de modo y el sonido se mantiene.
- LEDs de actividad cuando entra MIDI (notas sostenidas).

LAB (fase 2):
- Presets: búsqueda por nombre o tag; filtros por categoría, motor y carácter; solo favoritos;
  un click carga (escuchar), la ★ marca favorito; guardar con nombre, categoría y tags.
  Carpeta: `~/Library/Application Support/Viszeversza/FLIPPER/Presets/` (Mac) y
  `%APPDATA%\Viszeversza\FLIPPER\Presets\` (Windows).
- A/B: dos sonidos para comparar, y COPIAR A/B.
- Generador: CLÁSICO (solo el motor elegido en el tablero), ESTILO (OSCURO, BRILLANTE,
  AMBIENTE, AGRESIVO, LO-FI), HÍBRIDO (2 a 4 motores). Semilla visible y reutilizable
  (misma semilla = mismo resultado), MUTAR con intensidad, SORPRESA.
- DESHACER / REHACER (100 pasos), incluye perillas, mezcla, generador y presets.
- BLOQUEOS: cada motor, MACROS y CARÁCTER quedan fijos al generar, mutar y variar.

Personajes vivos (animaciones):
- Click en un personaje: sale la bola metálica (la del arte original) desde abajo, pega, el
  personaje hace recoil (squash & stretch), activa su poder, aparece el bonus y vuelve a reposo.
- Poderes:
  - MINIMOOG (la bestia): atrapa la bola, muerde, sacude la cabeza y escupe monedas. +4000
  - PROPHET-5 (el profeta): brillo en los lentes, se eleva, explosión astral, humo. +5000
  - JUPITER-8 (el orbital): gira, anillos orbitales, la bola orbita, explosión circular. +4000
  - JUNO-60/106 (la wacha): tira la pastilla en parábola, la cabeza la sigue, la atrapa con la lengua, rebote canchero, estrellas y arcoíris. +5500 BONUS!
  - CS-80 (el ojo): carga energía en el iris, rayo, flash y distorsión. +6000 EXTRA!
  - DX7 (el fumón): pitada con brasa, exhala humo con onda expansiva, la gorra vuela y vuelve. +3500
  - M1 (el robot): LEDs que parpadean, sobrecarga, vibración, glitch, arcos eléctricos. +5000
  - ARP 2600 (el eterno): el cráneo se separa, rebota y gira, partículas doradas y huesos, se vuelve a armar. +5500
  - VCS3 (el espectro): distorsión oscura, absorbe la bola, vórtice de vinilos y humo, la devuelve. +5000 MULTIPLIER!
- Cooldown de 0,9 s por personaje: los clicks repetidos no rompen la animación; cada personaje es independiente.
- Reposo: solo el personaje elegido respira (humo, LEDs o iris según quién sea).
- LAB: interruptores ANIMACIONES (encendido) y SONIDOS DEL TABLERO (apagado por defecto).
  Los sonidos se sintetizan en el plugin (sin archivos), nunca en loop.
- Cómo está hecho: el cuerpo de cada personaje es una capa recortada en tiempo real del arte
  aprobado (mismo dibujo); cráneo del ARP, gorra del DX7 y cabeza de la JUNO son recortes del
  mismo arte. Objetos y efectos salen de `Reference/animacion/` (`tools/extract_fx.py`) y se
  guardan en `Assets/objects/` y `Assets/effects/{ball,sparks,smoke,lightning,rainbow}/`.
  Lengua, brazos y mandíbula no son capas separadas: el movimiento se resuelve con
  transformaciones, piezas recortadas y efectos por código.
- La bola sigue una trayectoria simulada (todavía no hay física de pinball); cada personaje tiene
  su punto de impacto y dirección, listo para conectarlo a la física después.

Sonido (los 9 motores):
- Base compartida: 12 voces, velocidad, pitch bend (±2 st), rueda de modulación, sustain, aftertouch
  (polifónico y de canal). Cada motor suma sus voces en su propio bus (para sus efectos). Los
  motores apagados no procesan; no hay límite de motores encendidos a la vez.
- Macros conectadas al sonido de los 9 motores; CHARACTER = saturación (LIMPIO → ROTO).
- Arpegiador (ARRIBA, ABAJO, ARRIBA-ABAJO, AZAR; 1/4 a 1/32 con el tempo del DAW; 1-3 octavas;
  gate). Se maneja desde JUPITER-8 y JUNO, y toca todos los motores encendidos.
- MINIMOOG (KROKO): 3 osc, mezclador, filtro escalera 24 dB, 2 contornos, glide.
- PROPHET-5 (PROFETA): osc A y B con formas sumables, sync, poly-mod (env y osc B a frecuencia A
  y filtro), LFO, 24 dB, unísono, glide.
- JUPITER-8 (PITER): VCO-1/2 con pies, cross mod, sync, PWM, HPF + VCF 12/24 dB, ENV-1 y ENV-2,
  LFO de 5 formas, zona de teclado (TODO / GRAVES / AGUDOS con split, para tocar en capas).
- JUNO-60/106 (NO JUNA): DCO con sierra, pulso, sub y ruido, PWM manual o LFO, HPF de 4 posiciones,
  VCF 24 dB, una envolvente, VCA ENV/GATE, LFO con retardo, chorus I / II / I+II.
- CS-80 (ILLUMINARY): dos canales con osc, pasaaltos y pasabajos y su envolvente, balance y
  desafinación, modulador en anillo, aftertouch a brillo y vibrato, portamento/glissando y cinta.
- DX7 (TRAP PROD): 6 operadores FM, 32 algoritmos (con diagrama), envolvente por operador, ratio,
  fino, desafinación, velocidad, feedback, LFO.
- M1 (ARTURITO): 2 fuentes PCM con 11 multimuestras PROPIAS generadas por síntesis (no usa samples
  de Korg), filtro digital, envolventes, LFO, efectos (chorus, delay).
- ARP 2600 (CALAVERA): 3 VCO (VCO 3 también como LFO), ruido blanco/rosa, anillo, filtro
  pasabajos/pasabanda/pasaaltos, ADSR + AR, sample & hold, portamento y patch bay de 4 cables.
- VCS3 (EL QUE LA MATA): 3 osciladores, ruido, anillo, filtro con autooscilación, trapezoide
  (tecla o repetir), matriz de pines 8×7, joystick, reverb de resorte.

DISEÑO: cada motor con la forma de controles de su máquina original y el color de su personaje
(Model D, perillas Prophet, faders y botones del Jupiter y del Juno, palancas de colores del CS-80,
botones de membrana y LCD del DX7, perillas de goma y LCD del M1, faders y patch bay del ARP,
perillas de colores, matriz y joystick del VCS3), con una línea de ayuda abajo.

Presets: 40 de fábrica (4 o 5 por motor + 3 híbridos), sonidos originales que recrean la función
de los clásicos de cada máquina. Se marcan con F en LAB y no se pueden borrar; también pueden ser
favoritos. El generador tiene reglas para los 9 motores.

Ayuda e idioma (0.9.1):
- Ayuda al pasar el mouse: cada perilla, fader, botón, pin, cable y personaje explica qué hace,
  con un consejo de uso (391 controles, en castellano e inglés).
- Botón ? (arriba a la derecha): recorrido guiado de la pantalla donde estés: JUGAR, LAB y la
  pantalla DISEÑO de cada uno de los 9 motores (58 pasos). Ilumina cada zona y la explica; se maneja
  con ANTERIOR / SIGUIENTE / SALIR o las flechas del teclado. El de JUGAR se abre solo la primera vez.
- Idioma: selector ES / EN en LAB; traduce toda la interfaz, la ayuda y los recorridos. La primera
  vez usa el idioma del sistema. Se guarda por usuario (no por proyecto).

MAESTRO (0.9.2):
- Tocando el retrato del panel PERSONAJE (o el globo de diálogo) se abre el maestro: el personaje
  pregunta qué sonido querés hacer. Se escribe (castellano o inglés) o se toca un ejemplo.
- Buscador sin internet: compara las palabras con las de cada receta (Source/Teach/Lessons.cpp).
  Si nombrás un motor o personaje ("campana con el dx7") prioriza sus recetas. Palabras como
  oscuro, brillante, sucio, roto, limpio, gordo, fino, largo, corto, suave, golpe, ancho, espacio
  o movimiento agregan pasos al final con las macros o con CHARACTER.
- 26 recetas en los 9 motores (3 de primeros pasos: filtro, envolvente, desafinar), 202 pasos.
- Lección: pone el motor en su sonido base (lo anterior queda en DESHACER), pasa a DISEÑO, ilumina
  la perilla, explica qué hace y por qué, muestra el valor actual y el buscado, y espera a que el
  usuario la mueva. Solo la zona iluminada recibe el mouse. MOSTRAME la mueve por él.
  En DX7 y CS-80 cambia solo de operador o de capa cuando hace falta.
- Para sumar recetas: agregar una entrada en lessons() con pasos { control, condiciones, textos ES/EN }.
  tools/tests verifica que cada paso apunte a un control visible, que no arranque cumplido, que
  MOSTRAME lo cumpla y que el sonido final suene.

Por defecto arranca solo KROKO encendido (antes eran los 9 a la vez y sonaba todo junto).

Todavía NO (no se muestra como si funcionara):
- Modulación cruzada entre motores (fase 4). El patch bay del ARP y la matriz del VCS3 ya funcionan, cada uno dentro de su motor.
- Faltan presets de fábrica para llegar a la meta de 500 (hay 40).
- Serial y ventana de activación (como los demás plugins de Viszeversza).

## Assets

`tools/prepare_assets.py` genera todo desde la imagen aprobada:
- Fondo sin los textos en inglés (los textos en español los dibuja JUCE, así se pueden traducir).
- Perillas, LEDs y retratos recortados de la misma imagen.

Retratos (`Assets/Characters/`): recortes de la imagen aprobada. Son los definitivos.

Tipografía: Bebas Neue (licencia OFL, `Assets/Fonts/OFL-BebasNeue.txt`).

## Instaladores (GitHub)

En el repo `solobeats888-ctrl/flip`: Actions > Build FLIPPER (Mac + Windows) > Run workflow.
Compila, arma `.pkg` (Mac, con desinstalador) y `.exe` (Windows), los instala y desinstala de
prueba, valida el Audio Unit y publica un Release con `FLIPPER-Mac.zip` y `FLIPPER-Windows.zip`.

## Compilar

    cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
    cmake --build build --target Flipper_VST3 Flipper_AU

Captura de la interfaz sin DAW (revisión visual):

    cmake -S . -B build -DFLIPPER_BUILD_SNAPSHOT=ON
    cmake --build build --target FlipperSnapshot
    ./build/FlipperSnapshot_artefacts/Release/FlipperSnapshot captura.png 1024 8 0
    # argumentos: salida ancho motor(0-8) modo(0 jugar, 1 diseño, 2 lab) apagados(ej. 1,6)
    cmake --build build --target FlipperTests && ./build/FlipperTests_artefacts/Release/FlipperTests
