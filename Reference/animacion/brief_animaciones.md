# VOLTEADOR — UPDATE VISUAL + SISTEMA DE PERSONAJES VIVOS

## CONTEXTO

La interfaz base de VOLTEADOR ya está desarrollada. No quiero empezar de cero ni perder el diseño que ya existe. Trabajá sobre el proyecto actual y evolucioná la experiencia.

Voy a adjuntar una imagen de referencia con la dirección artística de los personajes, animaciones, efectos y poderes. Usala como referencia de estilo y concepto, no como una imagen para colocar directamente en el plugin.

El objetivo es convertir VOLTEADOR en una máquina de pinball psicodélica, donde cada personaje tenga personalidad, orientación coherente, animaciones únicas, efectos y recompensas.

---

# 1. CONSERVAR LA INTERFAZ EXISTENTE

Mantener la estructura actual:

- Tablero central de pinball.
- Navegación izquierda: JUGAR, DISEÑO, LAB.
- Panel derecho: mezcla de motores.
- Selector de personaje.
- Control SUCIO.
- Knobs inferiores: CUERPO, BRILLO, MOVIMIENTO, TEXTURA, ANCHURA, ESPACIO, ATAQUE y LIBERACIÓN.
- Botones GENERAR, VARIACIONES y FAVORITO.

Conservar la estética:
- Arcade vintage.
- Rock psicodélico.
- Comic art underground.
- Metal envejecido.
- Negro, dorado, bronce, rojo y ámbar.
- Texturas desgastadas.
- Tipografía retro agresiva.
- Ilustración detallada.

No modernizar ni minimalizar el diseño. No reemplazar el tablero por una web genérica.

---

# 2. REORIENTAR Y REORGANIZAR LOS PERSONAJES

Actualmente varios personajes tienen una orientación visual repetitiva o miran hacia afuera. Corregirlo.

### Regla principal:

- Personajes ubicados a la izquierda: deben mirar hacia el centro/derecha.
- Personajes ubicados a la derecha: deben mirar hacia el centro/izquierda.
- Personajes centrales: frontal, pose dominante o perspectiva que funcione como eje.

La nariz, ojos, rostro, torso y pose deben seguir la orientación.

Quiero una composición simétrica en intención, pero orgánica en ejecución. No hacer que todos tengan la misma pose.

Si se usan sprites espejados:
- No espejar texto.
- No espejar logos.
- No generar accesorios duplicados.
- Revisar anatomía.
- No invertir elementos con marcas identificables.
- Mantener las poses orientadas hacia el centro del tablero.

La máquina debe sentirse equilibrada visualmente.

---

# 3. CREAR ASSETS INDIVIDUALES REALES

No usar screenshots del plugin como assets.

Crear una carpeta independiente por personaje y assets separados para:
- Cuerpo.
- Idle.
- Impacto.
- Poder.
- Bonus.
- Accesorios animables.
- Efectos.

Formato preferido:
- PNG.
- Transparencia real.
- Alta resolución.
- Sin fondos.
- Sin rótulos.
- Sin texto.
- Sin marcos.
- Sin scores dibujados.
- Sin objetos duplicados.

Los accesorios que se muevan deben ser capas independientes.

Ejemplos:
- Juno: pastilla, lengua, brazo, partículas.
- DX7: gorra, porro, humo.
- Prophet: lentes, humo, energía.
- CS-80: ojo, rayos.
- M1: LEDs, piezas robóticas.

No usar una única imagen para simular movimientos que necesitan elementos separados.

Si la generación de assets no puede garantizar consistencia entre frames, utilizar el personaje base como imagen y animar accesorios, transformaciones, partículas, deformaciones y efectos mediante código.

No inventar que existen sprites terminados si solamente hay ilustraciones de referencia.

---

# 4. SISTEMA DE ANIMACIÓN

Cada personaje debe ser clickeable y reaccionar de manera independiente.

Secuencia general:

1. La bola sale disparada hacia el personaje.
2. Impacto.
3. Recoil del cuerpo.
4. Activación del poder especial.
5. Efectos visuales.
6. Bonus de puntuación.
7. Reset al estado idle.

Duraciones orientativas:
- Lanzamiento: 100–250 ms.
- Impacto: 100–200 ms.
- Poder: 300–800 ms.
- Bonus: 500–1000 ms.
- Reset: transición progresiva.

Las animaciones deben sentirse arcade, elásticas, contundentes y satisfactorias.

Implementar:
- Squash & stretch.
- Rotación.
- Rebote.
- Shake.
- Flash.
- Glow.
- Partículas.
- Humo.
- Ondas de impacto.
- Score flotante.

No animar todo el tablero a la vez.

---

# 5. PODER ÚNICO DE CADA PERSONAJE

Cada personaje debe tener su propio evento, animación, efectos y recompensa.

## PROPHET-5 — EL PROFETA

- Recoil al impacto.
- Brillo intenso en los lentes.
- Levanta el dedo o realiza una pose de profeta.
- Explosión astral.
- Ondas cósmicas.
- Humo.
- Bonus: +5000.

Corregir accesorios duplicados. Un solo par de auriculares y un solo porro si forma parte del diseño.

## MINIMOOG — LA BESTIA

- Abre la mandíbula.
- Atrapa la bola.
- Hace una mordida cartoon.
- Sacude la cabeza.
- Escupe monedas y partículas.
- Bonus: +4000.

## JUPITER-8 — EL ORBITAL

- Giro o sacudida del personaje.
- Anillos orbitales.
- La bola orbita alrededor del personaje.
- Explosión circular.
- Bonus: +4000.

## JUNO-60/106 — LA WACHA

Una de las animaciones principales.

Secuencia:
1. Impacto.
2. Recoil.
3. Tira una pastilla al aire.
4. La pastilla realiza una parábola.
5. La cabeza sigue el movimiento.
6. Abre la boca.
7. Extiende la lengua.
8. Atrapa la pastilla con la lengua.
9. Pose canchera.
10. Explosión de estrellas, color y bonus.
11. Reset.

La pastilla, la lengua y el brazo deben ser elementos animables independientes siempre que sea viable.

La animación debe ser cartoon, psicodélica y exagerada, no realista.

Bonus: +5500.

## CS-80 — EL OJO

- El ojo se abre.
- Se ilumina el iris.
- Carga energía.
- Dispara un rayo.
- Distorsión cósmica.
- Flash.
- Bonus: +6000.

## DX7 — EL FUMÓN

Animación característica:

1. Impacto.
2. Se lleva el porro a la boca.
3. Da una pitada.
4. Expulsa humo.
5. El humo genera una onda expansiva.
6. La gorra sale volando hacia atrás.
7. La gorra rota en el aire.
8. Reacción graciosa del personaje.
9. La gorra vuelve o se recoloca al reset.
10. Bonus.

El porro, el humo y la gorra deben ser independientes del cuerpo para poder animarlos por separado.

Bonus: +3500.

## M1 — EL ROBOT

- Impacto.
- LEDs parpadean.
- Overload.
- Vibración de la carcasa.
- Descarga de luz.
- Glitch breve.
- Partículas eléctricas.
- Bonus: +5000.

## ARP 2600 — EL ETERNO

- Recoil.
- El cráneo se separa del cuerpo en una animación cartoon.
- Rebota y gira.
- Partículas doradas.
- Se vuelve a ensamblar.
- Bonus: +5500.

## VCS3 — EL ESPECTRO

- Distorsión oscura.
- Absorbe la bola.
- Vórtice de vinilos y humo.
- La bola reaparece con multiplicador.
- Bonus: +5000.

---

# 6. SISTEMA DE BONUS

Cada poder debe mostrar una recompensa.

Ejemplos:
- +3500
- +4000
- +5000
- +5500
- +6000
- BONUS!
- EXTRA!
- MULTIPLIER!

Estética:
- Tipografía arcade.
- Dorado o amarillo.
- Outline negro/rojo.
- Bounce.
- Scale.
- Glow.
- Partículas.

El score aparece cerca del personaje y desaparece automáticamente.

No debe tapar permanentemente los controles ni otros elementos.

---

# 7. ESTADOS POR PERSONAJE

Implementar una estructura reutilizable con:

- IDLE: movimiento sutil.
- HIT: impacto.
- POWER: habilidad especial.
- BONUS: puntuación y efectos.
- RESET: regreso al estado original.

Cada personaje tiene su propia máquina de estados.

Los idle deben ser sutiles:
- Respiración.
- Movimiento de cabeza.
- Humo.
- Parpadeo.
- LEDs.
- Pequeñas vibraciones.

No generar movimiento excesivo cuando el usuario no está interactuando.

---

# 8. FÍSICA Y CLICKS

Si la bola todavía no tiene física real, simular su trayectoria hacia el personaje cuando se haga click.

Preparar la arquitectura para conectar posteriormente la animación con la física real de pinball.

Cada personaje debe tener:
- Hitbox independiente.
- Punto de impacto.
- Dirección de la bola.
- Recoil.
- Cooldown.
- Estado de animación.
- Bonus.
- Reset.

Un personaje no debe activar a los demás.

Evitar que los clicks repetidos rompan las animaciones. Permitir reactivación después del cooldown y controlar las superposiciones.

---

# 9. SONIDO

Agregar soporte opcional para:
- Impacto.
- Rebote metálico.
- Activación de poder.
- Bonus.
- Sonido particular por personaje.

Agregar un control para apagar y encender los sonidos.

No reproducir audio invasivo ni automáticamente en loop.

---

# 10. ORGANIZACIÓN DE ARCHIVOS

Mantener una estructura modular similar a:

assets/
  characters/
    prophet5/
      idle.png
      hit.png
      power/
      fx/
      manifest.json
    minimoog/
    jupiter8/
    juno60106/
    cs80/
    dx7/
    m1/
    arp2600/
    vcs3/
  effects/
    sparks/
    smoke/
    lightning/
    bonus/
    ball/

No es obligatorio que cada animación sea un spritesheet. Se pueden utilizar PNG individuales, SVG, capas animables, CSS, canvas o la arquitectura más adecuada para el proyecto actual.

Mantener nombres descriptivos y consistentes.

---

# 11. RENDIMIENTO Y CALIDAD

- Animaciones fluidas.
- No bloquear la generación de sonidos.
- Evitar acumulación de partículas.
- Limpiar efectos al finalizar.
- Evitar estados congelados.
- Evitar re-renderizados innecesarios.
- No romper controles.
- Respetar el responsive existente.
- Mantener un código modular y reutilizable.
- Evitar dependencias innecesarias.

---

# 12. IMPLEMENTACIÓN Y VALIDACIÓN

Implementá directamente sobre el proyecto actual.

No me devuelvas solamente un plan ni una descripción de lo que harías.

Primero revisá la estructura existente, identificá cómo están renderizados los personajes y cómo se maneja el estado de la UI, y después integrá las animaciones sin destruir lo que ya funciona.

Validá:
- Orientación de cada personaje.
- Simetría de la composición.
- Hitboxes.
- Click individual.
- Animaciones.
- Bonus.
- Cooldowns.
- Reset.
- Sonido.
- Responsive.
- Assets.
- Rendimiento.

No reemplazar los personajes por capturas de pantalla ni utilizar la hoja de referencia como imagen única dentro del plugin.

## RESULTADO ESPERADO

Quiero que VOLTEADOR parezca una máquina pinball psicodélica viva.

Los personajes deben estar orientados hacia el centro, tener personalidades distintas, reaccionar al golpe de la bola, activar poderes exclusivos, generar efectos y otorgar bonus.

Conservar la identidad visual actual y transformar la interfaz de una buena maqueta a una experiencia interactiva real.

Implementá los cambios, revisá el resultado y entregame el proyecto funcional con los archivos modificados y los assets organizados.