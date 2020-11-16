COMPILER = g++
OUTPUT_FILE = run

ifeq ($(os),linux)
	SDL_LIB = ./lib/SDL2_linux/lib
	SDL_INCLUDE = ./lib/SDL2_linux/include
	COMPILER_FLAGS = -Wall -c -std=c++11 -I$(SDL_INCLUDE)
	LINKER_FLAGS = -L$(SDL_LIB) -lSDL2 -lSDL2_ttf -Wl,-rpath=$(SDL_LIB)
else
	SDL_LIB = ./lib/SDL2_windows/lib
	SDL_INCLUDE = ./lib/SDL2_windows/include
	SDL_TTF_LIB = ./lib/SDL2_ttf/lib
	SDL_TTF_INCLUDE = ./lib/SDL2_ttf/include
	COMPILER_FLAGS = -Wall -c -std=c++11 -I$(SDL_INCLUDE) -I$(SDL_INCLUDE)/SDL2 $(SDL_TTF_INCLUDE)
	LINKER_FLAGS = -lmingw32 -mwindows -mconsole $(SDL_LIB) -lSDL2main -lSDL2 $(SDL_TTF_LIB) -llibSDL2_ttf
endif

all: ./bin/$(OUTPUT_FILE)

./bin/$(OUTPUT_FILE): ./bin/main.o
	$(COMPILER) $< $(LINKER_FLAGS) -o $@

./bin/main.o: ./src/main.cpp
	$(COMPILER) $(COMPILER_FLAGS) $< -o $@

clean:
	ifeq ($(os),linux)
		rm ./bin/*.o && rm ./bin/$(OUTPUT_FILE)
	else
		del ./bin/*.o && del ./bin/$(OUTPUT_FILE)
