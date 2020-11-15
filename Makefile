# SDL_LIB = ./lib/SDL2_windows/lib
# SDL_INCLUDE = ./lib/SDL2_windows/include
SDL_LIB = ./SDL2_linux/lib
SDL_INCLUDE = ./SDL2_linux/include

OUTPUT_FILE = run
COMPILER = g++

COMPILER_FLAGS = -Wall -c -std=c++11 -I$(SDL_INCLUDE)
LINKER_FLAGS = -L$(SDL_LIB) -lSDL2 -Wl,-rpath=$(SDL_LIB)

all: ./bin/$(OUTPUT_FILE)

./bin/$(OUTPUT_FILE): ./bin/main.o
	$(COMPILER) $< $(LINKER_FLAGS) -o $@

./bin/main.o: ./src/main.cpp
	$(COMPILER) $(COMPILER_FLAGS) $< -o $@

clean:
	rm ./bin/*.o && rm ./bin/$(OUTPUT_FILE)
