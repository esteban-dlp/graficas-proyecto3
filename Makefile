# estudiante: esteban de la pena | carnet: 24171
CXX = c++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra -pedantic $(shell pkg-config --cflags sdl2)
LDLIBS = $(shell pkg-config --libs sdl2)

space_travel: main.cpp
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDLIBS)

run: space_travel
	./space_travel

limpiar:
	rm -f space_travel

.PHONY: run limpiar
