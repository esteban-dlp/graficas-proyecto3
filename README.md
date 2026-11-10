# Proyecto 3: Space Travel

**Esteban de la Peña — 24171**

Proyecto en C++17 con SDL2. La base incluye un framebuffer propio y un renderer 3D por software que transforma y rasteriza una esfera con prueba de profundidad y un shader de fragmentos calculado en CPU.

## Requisitos

- Compilador C++17
- SDL2 y `pkg-config`
- `make`

En macOS con Homebrew, instala lo necesario con `brew install sdl2 pkg-config`.

## Compilar y ejecutar

```sh
make
./space_travel
```

La ventana muestra una esfera 3D iluminada y una nave facetada que puedes pilotear con teclado y cámara en tercera persona. Presiona `Esc` o cierra la ventana para salir.

## Renderer implementado

- Framebuffer RGBA y rasterización de triángulos por CPU
- Malla de esfera generada con triángulos
- Transformaciones de modelo, vista y proyección en matrices propias
- Interpolación de normales y prueba de profundidad por píxel
- Shader de fragmentos procedural con iluminación y bandas animadas
- Actualización del tiempo con el contador de alto rendimiento de SDL
- Nave de malla poligonal propia, dibujada con el mismo z-buffer del renderer
- Movimiento de la cámara y la nave mediante WASD, espacio y Shift
- Giro de cámara con flechas o con ratón tras pulsar Tab; libera el ratón con la tecla de acento grave (`)

Todavía no incluye órbitas, colisiones ni límites de proximidad al planeta.
