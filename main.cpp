// estudiante: esteban de la pena | carnet: 24171
#include <SDL.h>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <vector>

struct Color {
    std::uint8_t r;
    std::uint8_t g;
    std::uint8_t b;
    std::uint8_t a;
};

class Framebuffer {
public:
    Framebuffer(int ancho, int alto)
        : ancho(ancho), alto(alto), pixeles(static_cast<std::size_t>(ancho * alto * 4), 0) {}

    void poner_pixel(int x, int y, Color color) {
        if (x < 0 || x >= ancho || y < 0 || y >= alto) return;
        const std::size_t i = static_cast<std::size_t>((y * ancho + x) * 4);
        pixeles[i] = color.r;
        pixeles[i + 1] = color.g;
        pixeles[i + 2] = color.b;
        pixeles[i + 3] = color.a;
    }

    void limpiar(Color color) {
        for (int y = 0; y < alto; ++y) {
            for (int x = 0; x < ancho; ++x) poner_pixel(x, y, color);
        }
    }

    int ancho;
    int alto;
    std::vector<std::uint8_t> pixeles;
};

void dibujar_linea(Framebuffer& buffer, int x0, int y0, int x1, int y1, Color color) {
    const int dx = std::abs(x1 - x0);
    const int sx = x0 < x1 ? 1 : -1;
    const int dy = -std::abs(y1 - y0);
    const int sy = y0 < y1 ? 1 : -1;
    int error = dx + dy;

    while (true) {
        buffer.poner_pixel(x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        const int doble = 2 * error;
        if (doble >= dy) {
            error += dy;
            x0 += sx;
        }
        if (doble <= dx) {
            error += dx;
            y0 += sy;
        }
    }
}

void dibujar_triangulo(Framebuffer& buffer, int ax, int ay, int bx, int by, int cx, int cy, Color color) {
    dibujar_linea(buffer, ax, ay, bx, by, color);
    dibujar_linea(buffer, bx, by, cx, cy, color);
    dibujar_linea(buffer, cx, cy, ax, ay, color);
}

void dibujar_escena_minima(Framebuffer& buffer) {
    const Color fondo{5, 9, 26, 255};
    const Color estrella{245, 196, 94, 255};
    const Color planeta{65, 151, 220, 255};
    const Color nave{225, 230, 239, 255};
    buffer.limpiar(fondo);

    for (int i = 0; i < 90; ++i) {
        const int x = (i * 197 + 31) % buffer.ancho;
        const int y = (i * 113 + 47) % buffer.alto;
        const std::uint8_t brillo = static_cast<std::uint8_t>(100 + (i * 37) % 120);
        buffer.poner_pixel(x, y, {brillo, brillo, static_cast<std::uint8_t>(std::min(255, brillo + 25)), 255});
    }

    const int cx = buffer.ancho / 2;
    const int cy = buffer.alto / 2;
    dibujar_triangulo(buffer, cx - 220, cy + 100, cx - 145, cy - 45, cx - 70, cy + 100, estrella);
    dibujar_triangulo(buffer, cx + 50, cy + 35, cx + 155, cy - 85, cx + 260, cy + 35, planeta);
    dibujar_triangulo(buffer, cx + 50, cy + 35, cx + 260, cy + 35, cx + 155, cy + 100, {30, 93, 151, 255});
    dibujar_triangulo(buffer, cx - 18, cy + 65, cx, cy + 5, cx + 18, cy + 65, nave);
    dibujar_triangulo(buffer, cx - 18, cy + 65, cx + 18, cy + 65, cx, cy + 76, {112, 181, 224, 255});
}

int main() {
    constexpr int ancho = 960;
    constexpr int alto = 640;
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "No se pudo iniciar SDL: " << SDL_GetError() << '\n';
        return 1;
    }

    SDL_Window* ventana = SDL_CreateWindow("Space Travel - Fase 1", SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED, ancho, alto, SDL_WINDOW_SHOWN);
    if (!ventana) {
        std::cerr << "No se pudo crear la ventana: " << SDL_GetError() << '\n';
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* presentador = SDL_CreateRenderer(ventana, -1, SDL_RENDERER_SOFTWARE);
    if (!presentador) {
        std::cerr << "No se pudo crear el presentador: " << SDL_GetError() << '\n';
        SDL_DestroyWindow(ventana);
        SDL_Quit();
        return 1;
    }

    Framebuffer framebuffer(ancho, alto);
    dibujar_escena_minima(framebuffer);
    SDL_Texture* textura = SDL_CreateTexture(presentador, SDL_PIXELFORMAT_RGBA32,
        SDL_TEXTUREACCESS_STREAMING, ancho, alto);
    if (!textura) {
        std::cerr << "No se pudo crear la textura: " << SDL_GetError() << '\n';
        SDL_DestroyRenderer(presentador);
        SDL_DestroyWindow(ventana);
        SDL_Quit();
        return 1;
    }

    bool salir = false;
    while (!salir) {
        SDL_Event evento;
        while (SDL_PollEvent(&evento)) {
            if (evento.type == SDL_QUIT) salir = true;
            if (evento.type == SDL_KEYDOWN && evento.key.keysym.sym == SDLK_ESCAPE) salir = true;
        }
        SDL_UpdateTexture(textura, nullptr, framebuffer.pixeles.data(), ancho * 4);
        SDL_RenderClear(presentador);
        SDL_RenderCopy(presentador, textura, nullptr, nullptr);
        SDL_RenderPresent(presentador);
        SDL_Delay(16);
    }

    SDL_DestroyTexture(textura);
    SDL_DestroyRenderer(presentador);
    SDL_DestroyWindow(ventana);
    SDL_Quit();
    return 0;
}
