# Proyecto 3: Space Travel

**Esteban de la Peña — 24171**

Proyecto en C++17 con SDL2. Esta primera fase deja lista una ventana y un framebuffer de software para construir el renderer del proyecto.

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

La ventana muestra una escena 2D provisional dibujada en un framebuffer propio. Presiona `Esc` o cierra la ventana para salir.

## Alcance de esta fase

- Ventana SDL2 y ciclo de eventos
- Framebuffer RGBA y dibujo de píxeles y líneas en CPU
- Escena provisional para comprobar la presentación del framebuffer

Esta fase no implementa todavía geometría 3D, transformaciones, profundidad, shaders, controles de vuelo ni órbitas. Se incorporarán en fases posteriores.
