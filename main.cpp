// estudiante: esteban de la pena | carnet: 24171
#include <SDL.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <vector>

struct Vector3 {
    float x;
    float y;
    float z;
};

struct Color {
    std::uint8_t r;
    std::uint8_t g;
    std::uint8_t b;
    std::uint8_t a;
};

struct Vertice {
    Vector3 posicion;
    Vector3 normal;
};

struct Triangulo {
    std::array<Vertice, 3> vertices;
};

struct PuntoPantalla {
    float x;
    float y;
    float z;
    Vector3 normal;
};

struct Camara {
    Vector3 posicion;
    float yaw;
    float pitch;
};

Vector3 operator+(Vector3 a, Vector3 b) { return {a.x + b.x, a.y + b.y, a.z + b.z}; }
Vector3 operator-(Vector3 a, Vector3 b) { return {a.x - b.x, a.y - b.y, a.z - b.z}; }
Vector3 operator*(Vector3 a, float s) { return {a.x * s, a.y * s, a.z * s}; }
float producto(Vector3 a, Vector3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
Vector3 normalizar(Vector3 v) {
    const float largo = std::sqrt(producto(v, v));
    return largo > 0.00001f ? v * (1.0f / largo) : Vector3{0, 0, 0};
}

Vector3 direccion_camara(const Camara& camara) {
    const float cp = std::cos(camara.pitch);
    return normalizar({std::sin(camara.yaw) * cp, std::sin(camara.pitch), std::cos(camara.yaw) * cp});
}

struct Matriz4 {
    float m[4][4]{};

    static Matriz4 identidad() {
        Matriz4 r{};
        for (int i = 0; i < 4; ++i) r.m[i][i] = 1.0f;
        return r;
    }
};

Matriz4 multiplicar(const Matriz4& a, const Matriz4& b) {
    Matriz4 r{};
    for (int fila = 0; fila < 4; ++fila)
        for (int col = 0; col < 4; ++col)
            for (int k = 0; k < 4; ++k) r.m[fila][col] += a.m[fila][k] * b.m[k][col];
    return r;
}

Vector3 transformar(const Matriz4& m, Vector3 v) {
    const float x = m.m[0][0] * v.x + m.m[0][1] * v.y + m.m[0][2] * v.z + m.m[0][3];
    const float y = m.m[1][0] * v.x + m.m[1][1] * v.y + m.m[1][2] * v.z + m.m[1][3];
    const float z = m.m[2][0] * v.x + m.m[2][1] * v.y + m.m[2][2] * v.z + m.m[2][3];
    const float w = m.m[3][0] * v.x + m.m[3][1] * v.y + m.m[3][2] * v.z + m.m[3][3];
    if (std::abs(w) < 0.00001f) return {x, y, z};
    return {x / w, y / w, z / w};
}

Matriz4 trasladar(float x, float y, float z) {
    Matriz4 r = Matriz4::identidad();
    r.m[0][3] = x;
    r.m[1][3] = y;
    r.m[2][3] = z;
    return r;
}

Matriz4 escalar(float x, float y, float z) {
    Matriz4 r = Matriz4::identidad();
    r.m[0][0] = x;
    r.m[1][1] = y;
    r.m[2][2] = z;
    return r;
}

Matriz4 girar_y(float angulo) {
    Matriz4 r = Matriz4::identidad();
    const float c = std::cos(angulo), s = std::sin(angulo);
    r.m[0][0] = c;
    r.m[0][2] = s;
    r.m[2][0] = -s;
    r.m[2][2] = c;
    return r;
}

Matriz4 girar_x(float angulo) {
    Matriz4 r = Matriz4::identidad();
    const float c = std::cos(angulo), s = std::sin(angulo);
    r.m[1][1] = c;
    r.m[1][2] = -s;
    r.m[2][1] = s;
    r.m[2][2] = c;
    return r;
}

Matriz4 vista(Vector3 ojo, Vector3 objetivo, Vector3 arriba) {
    const Vector3 hacia = normalizar(objetivo - ojo);
    const Vector3 derecha = normalizar({hacia.y * arriba.z - hacia.z * arriba.y,
                                       hacia.z * arriba.x - hacia.x * arriba.z,
                                       hacia.x * arriba.y - hacia.y * arriba.x});
    const Vector3 arriba_vista = {derecha.y * hacia.z - derecha.z * hacia.y,
                                  derecha.z * hacia.x - derecha.x * hacia.z,
                                  derecha.x * hacia.y - derecha.y * hacia.x};
    Matriz4 r = Matriz4::identidad();
    r.m[0][0] = derecha.x; r.m[0][1] = derecha.y; r.m[0][2] = derecha.z; r.m[0][3] = -producto(derecha, ojo);
    r.m[1][0] = arriba_vista.x; r.m[1][1] = arriba_vista.y; r.m[1][2] = arriba_vista.z; r.m[1][3] = -producto(arriba_vista, ojo);
    r.m[2][0] = hacia.x; r.m[2][1] = hacia.y; r.m[2][2] = hacia.z; r.m[2][3] = -producto(hacia, ojo);
    return r;
}

Matriz4 proyeccion(float fov, float aspecto, float cerca, float lejos) {
    Matriz4 r{};
    const float f = 1.0f / std::tan(fov * 0.5f);
    r.m[0][0] = f / aspecto;
    r.m[1][1] = f;
    r.m[2][2] = (lejos + cerca) / (lejos - cerca);
    r.m[2][3] = (-2.0f * lejos * cerca) / (lejos - cerca);
    r.m[3][2] = 1.0f;
    return r;
}

class Framebuffer {
public:
    Framebuffer(int ancho, int alto)
        : ancho(ancho), alto(alto), pixeles(static_cast<std::size_t>(ancho * alto * 4), 0),
          profundidad(static_cast<std::size_t>(ancho * alto), 1.0f) {}

    void limpiar(Color color) {
        for (int y = 0; y < alto; ++y) {
            for (int x = 0; x < ancho; ++x) poner_pixel(x, y, color);
        }
        std::fill(profundidad.begin(), profundidad.end(), 1.0f);
    }

    void poner_pixel(int x, int y, Color color) {
        if (x < 0 || x >= ancho || y < 0 || y >= alto) return;
        const std::size_t i = static_cast<std::size_t>((y * ancho + x) * 4);
        pixeles[i] = color.r;
        pixeles[i + 1] = color.g;
        pixeles[i + 2] = color.b;
        pixeles[i + 3] = color.a;
    }

    int ancho;
    int alto;
    std::vector<std::uint8_t> pixeles;
    std::vector<float> profundidad;
};

Color shader_fragmento(Vector3 normal, float luz_animada) {
    const Vector3 luz = normalizar({-0.45f, 0.72f, 0.9f});
    const float difusa = std::max(0.0f, producto(normal, luz));
    const float banda = 0.5f + 0.5f * std::sin(normal.y * 18.0f + normal.x * 7.0f + luz_animada);
    const float iluminacion = 0.18f + 0.82f * difusa;
    const float borde = std::pow(1.0f - std::max(0.0f, normal.z), 3.0f);
    const float rojo = std::clamp((0.08f + banda * 0.22f + borde * 0.35f) * iluminacion, 0.0f, 1.0f);
    const float verde = std::clamp((0.20f + banda * 0.48f + borde * 0.55f) * iluminacion, 0.0f, 1.0f);
    const float azul = std::clamp((0.42f + banda * 0.52f + borde * 0.65f) * iluminacion, 0.0f, 1.0f);
    return {static_cast<std::uint8_t>(rojo * 255), static_cast<std::uint8_t>(verde * 255),
            static_cast<std::uint8_t>(azul * 255), 255};
}

Color shader_nave(Vector3 normal) {
    const float luz = 0.35f + 0.65f * std::max(0.0f, producto(normal, normalizar({-0.5f, 0.7f, 0.8f})));
    const float franja = normal.y > 0.18f ? 1.0f : 0.0f;
    const float r = (0.24f + 0.68f * franja) * luz;
    const float g = (0.62f + 0.26f * franja) * luz;
    const float b = (0.82f + 0.16f * franja) * luz;
    return {static_cast<std::uint8_t>(r * 255), static_cast<std::uint8_t>(g * 255),
            static_cast<std::uint8_t>(b * 255), 255};
}

void dibujar_triangulo(Framebuffer& buffer, PuntoPantalla a, PuntoPantalla b, PuntoPantalla c,
                       float tiempo, bool es_nave) {
    const float area = (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
    if (std::abs(area) < 0.0001f) return;
    const int min_x = std::max(0, static_cast<int>(std::floor(std::min({a.x, b.x, c.x}))));
    const int max_x = std::min(buffer.ancho - 1, static_cast<int>(std::ceil(std::max({a.x, b.x, c.x}))));
    const int min_y = std::max(0, static_cast<int>(std::floor(std::min({a.y, b.y, c.y}))));
    const int max_y = std::min(buffer.alto - 1, static_cast<int>(std::ceil(std::max({a.y, b.y, c.y}))));

    for (int y = min_y; y <= max_y; ++y) {
        for (int x = min_x; x <= max_x; ++x) {
            const float px = x + 0.5f, py = y + 0.5f;
            const float wa = ((b.x - px) * (c.y - py) - (b.y - py) * (c.x - px)) / area;
            const float wb = ((c.x - px) * (a.y - py) - (c.y - py) * (a.x - px)) / area;
            const float wc = 1.0f - wa - wb;
            if (wa < 0 || wb < 0 || wc < 0) continue;
            const float z = wa * a.z + wb * b.z + wc * c.z;
            const std::size_t i = static_cast<std::size_t>(y * buffer.ancho + x);
            if (z < 0.0f || z > 1.0f || z >= buffer.profundidad[i]) continue;
            buffer.profundidad[i] = z;
            const Vector3 n = normalizar(a.normal * wa + b.normal * wb + c.normal * wc);
            buffer.poner_pixel(x, y, es_nave ? shader_nave(n) : shader_fragmento(n, tiempo));
        }
    }
}

std::vector<Triangulo> crear_esfera(int segmentos, int anillos) {
    std::vector<Triangulo> malla;
    for (int y = 0; y < anillos; ++y) {
        const float v0 = static_cast<float>(y) / anillos;
        const float v1 = static_cast<float>(y + 1) / anillos;
        const float lat0 = (v0 - 0.5f) * 3.14159265f;
        const float lat1 = (v1 - 0.5f) * 3.14159265f;
        for (int x = 0; x < segmentos; ++x) {
            const float u0 = static_cast<float>(x) / segmentos;
            const float u1 = static_cast<float>(x + 1) / segmentos;
            const float lon0 = u0 * 6.2831853f;
            const float lon1 = u1 * 6.2831853f;
            auto punto = [](float lat, float lon) {
                Vector3 n{std::cos(lat) * std::cos(lon), std::sin(lat), std::cos(lat) * std::sin(lon)};
                return Vertice{n, n};
            };
            const Vertice a = punto(lat0, lon0), b = punto(lat1, lon0);
            const Vertice c = punto(lat1, lon1), d = punto(lat0, lon1);
            malla.push_back({{{a, b, c}}});
            malla.push_back({{{a, c, d}}});
        }
    }
    return malla;
}

std::vector<Triangulo> crear_nave() {
    const Vector3 punta{0.0f, 0.0f, 0.48f};
    const Vector3 cola{0.0f, 0.0f, -0.36f};
    const Vector3 ala_izq{-0.34f, 0.0f, -0.22f};
    const Vector3 ala_der{0.34f, 0.0f, -0.22f};
    const Vector3 lomo{0.0f, 0.16f, -0.16f};
    const Vector3 vientre{0.0f, -0.12f, -0.20f};
    const Vector3 cima_izq{-0.18f, 0.04f, -0.24f};
    const Vector3 cima_der{0.18f, 0.04f, -0.24f};
    std::vector<Triangulo> malla;
    auto agregar = [&malla](Vector3 a, Vector3 b, Vector3 c) {
        const Vector3 n = normalizar({(b.y-a.y)*(c.z-a.z)-(b.z-a.z)*(c.y-a.y),
                                      (b.z-a.z)*(c.x-a.x)-(b.x-a.x)*(c.z-a.z),
                                      (b.x-a.x)*(c.y-a.y)-(b.y-a.y)*(c.x-a.x)});
        malla.push_back({{{{a,n},{b,n},{c,n}}}});
    };
    agregar(punta, ala_izq, lomo);
    agregar(punta, lomo, ala_der);
    agregar(punta, ala_der, vientre);
    agregar(punta, vientre, ala_izq);
    agregar(ala_izq, cola, lomo);
    agregar(cola, ala_der, lomo);
    agregar(ala_izq, vientre, cola);
    agregar(vientre, ala_der, cola);
    agregar(lomo, cima_izq, cima_der);
    agregar(lomo, cima_der, ala_der);
    agregar(cima_izq, ala_izq, lomo);
    return malla;
}

PuntoPantalla proyectar(Vector3 posicion, Vector3 normal, const Matriz4& mvp, int ancho, int alto) {
    const Vector3 clip = transformar(mvp, posicion);
    return {(clip.x * 0.5f + 0.5f) * ancho, (1.0f - (clip.y * 0.5f + 0.5f)) * alto,
            clip.z * 0.5f + 0.5f, normal};
}

void dibujar_malla(Framebuffer& buffer, const std::vector<Triangulo>& malla,
                    const Matriz4& modelo, const Matriz4& vp, float tiempo, bool es_nave = false) {
    const Matriz4 mvp = multiplicar(vp, modelo);
    for (const Triangulo& t : malla) {
        PuntoPantalla p[3];
        for (int i = 0; i < 3; ++i) {
            const Vector3 mundo = transformar(modelo, t.vertices[i].posicion);
            const Vector3 normal = normalizar(transformar(modelo, t.vertices[i].normal));
            p[i] = proyectar(mundo, normal, vp, buffer.ancho, buffer.alto);
        }
        dibujar_triangulo(buffer, p[0], p[1], p[2], tiempo, es_nave);
    }
    (void)mvp;
}

void dibujar_escena(Framebuffer& buffer, const std::vector<Triangulo>& esfera,
                    const std::vector<Triangulo>& nave, const Camara& camara, float tiempo) {
    buffer.limpiar({5, 9, 26, 255});
    for (int i = 0; i < 120; ++i) {
        const int x = (i * 197 + 31) % buffer.ancho;
        const int y = (i * 113 + 47) % buffer.alto;
        const std::uint8_t brillo = static_cast<std::uint8_t>(75 + (i * 37) % 130);
        buffer.poner_pixel(x, y, {brillo, brillo, static_cast<std::uint8_t>(std::min(255, brillo + 35)), 255});
    }
    const Vector3 frente = direccion_camara(camara);
    const Vector3 objetivo = camara.posicion + frente;
    const Matriz4 v = vista(camara.posicion, objetivo, {0, 1, 0});
    const Matriz4 p = proyeccion(0.92f, static_cast<float>(buffer.ancho) / buffer.alto, 0.1f, 30.0f);
    const Matriz4 vp = multiplicar(p, v);
    const Matriz4 modelo_planeta = multiplicar(trasladar(0, 0, 3.4f),
        multiplicar(girar_y(tiempo * 0.25f), escalar(1.05f, 1.05f, 1.05f)));
    dibujar_malla(buffer, esfera, modelo_planeta, vp, tiempo);

    const Matriz4 orientacion = multiplicar(girar_y(camara.yaw), girar_x(-camara.pitch));
    const Matriz4 modelo_nave = multiplicar(trasladar(camara.posicion.x + frente.x * 0.85f,
        camara.posicion.y + frente.y * 0.85f, camara.posicion.z + frente.z * 0.85f),
        multiplicar(orientacion, escalar(0.48f, 0.48f, 0.48f)));
    dibujar_malla(buffer, nave, modelo_nave, vp, tiempo, true);
}

int main() {
    constexpr int ancho = 960;
    constexpr int alto = 640;
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "No se pudo iniciar SDL: " << SDL_GetError() << '\n';
        return 1;
    }
    SDL_Window* ventana = SDL_CreateWindow("Space Travel - Nave y camara", SDL_WINDOWPOS_CENTERED,
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
    const std::vector<Triangulo> esfera = crear_esfera(36, 20);
    const std::vector<Triangulo> nave = crear_nave();
    SDL_Texture* textura = SDL_CreateTexture(presentador, SDL_PIXELFORMAT_RGBA32,
        SDL_TEXTUREACCESS_STREAMING, ancho, alto);
    if (!textura) {
        std::cerr << "No se pudo crear la textura: " << SDL_GetError() << '\n';
        SDL_DestroyRenderer(presentador);
        SDL_DestroyWindow(ventana);
        SDL_Quit();
        return 1;
    }

    Camara camara{{0.0f, 0.0f, -3.4f}, 0.0f, 0.0f};
    bool salir = false;
    const std::uint64_t inicio = SDL_GetPerformanceCounter();
    const double frecuencia = static_cast<double>(SDL_GetPerformanceFrequency());
    std::cout << "Controles: W/S avanzar y retroceder, A/D desplazarse lateralmente, espacio/shift subir y bajar, flechas mirar, Esc salir\n";
    while (!salir) {
        SDL_Event evento;
        while (SDL_PollEvent(&evento)) {
            if (evento.type == SDL_QUIT || (evento.type == SDL_KEYDOWN && evento.key.keysym.sym == SDLK_ESCAPE)) salir = true;
            if (evento.type == SDL_KEYDOWN && evento.key.keysym.sym == SDLK_TAB) SDL_SetRelativeMouseMode(SDL_TRUE);
            if (evento.type == SDL_KEYDOWN && evento.key.keysym.sym == SDLK_BACKQUOTE) SDL_SetRelativeMouseMode(SDL_FALSE);
            if (evento.type == SDL_MOUSEMOTION && SDL_GetRelativeMouseMode()) {
                camara.yaw += evento.motion.xrel * 0.003f;
                camara.pitch -= evento.motion.yrel * 0.003f;
                camara.pitch = std::clamp(camara.pitch, -1.35f, 1.35f);
            }
            if (evento.type == SDL_KEYDOWN && evento.key.repeat == 0) {
                if (evento.key.keysym.sym == SDLK_LEFT) camara.yaw -= 0.06f;
                if (evento.key.keysym.sym == SDLK_RIGHT) camara.yaw += 0.06f;
                if (evento.key.keysym.sym == SDLK_UP) camara.pitch = std::clamp(camara.pitch + 0.05f, -1.35f, 1.35f);
                if (evento.key.keysym.sym == SDLK_DOWN) camara.pitch = std::clamp(camara.pitch - 0.05f, -1.35f, 1.35f);
            }
        }
        const std::uint64_t ahora = SDL_GetPerformanceCounter();
        const float tiempo = static_cast<float>((ahora - inicio) / frecuencia);
        static std::uint64_t anterior = ahora;
        const float delta = std::clamp(static_cast<float>(ahora - anterior) / static_cast<float>(frecuencia), 0.0f, 0.05f);
        anterior = ahora;
        const Uint8* teclas = SDL_GetKeyboardState(nullptr);
        const Vector3 frente = direccion_camara(camara);
        const Vector3 derecha = normalizar({frente.z, 0.0f, -frente.x});
        const float velocidad = 2.2f * delta;
        if (teclas[SDL_SCANCODE_W]) camara.posicion = camara.posicion + frente * velocidad;
        if (teclas[SDL_SCANCODE_S]) camara.posicion = camara.posicion - frente * velocidad;
        if (teclas[SDL_SCANCODE_A]) camara.posicion = camara.posicion - derecha * velocidad;
        if (teclas[SDL_SCANCODE_D]) camara.posicion = camara.posicion + derecha * velocidad;
        if (teclas[SDL_SCANCODE_SPACE]) camara.posicion.y += velocidad;
        if (teclas[SDL_SCANCODE_LSHIFT] || teclas[SDL_SCANCODE_RSHIFT]) camara.posicion.y -= velocidad;

        dibujar_escena(framebuffer, esfera, nave, camara, tiempo);
        SDL_UpdateTexture(textura, nullptr, framebuffer.pixeles.data(), ancho * 4);
        SDL_RenderClear(presentador);
        SDL_RenderCopy(presentador, textura, nullptr, nullptr);
        SDL_RenderPresent(presentador);
        SDL_Delay(8);
    }

    SDL_DestroyTexture(textura);
    SDL_DestroyRenderer(presentador);
    SDL_DestroyWindow(ventana);
    SDL_Quit();
    return 0;
}
