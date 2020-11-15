# OUTPUT_FILE = rts
# COMPILER = g++
# SDL_LIB = -L/usr/lib -lSDL2 -Wl,-rpath=/usr/lib
# SDL_INCLUDE = -I/usr/include
# COMPILER_FLAGS = -Wall -c -std=c++11 $(SDL_INCLUDE)
# LINKER_FLAGS = $(SDL_LIB)
# all: $(OUTPUT_FILE)
# $(OUTPUT_FILE): main.o
# 	$(COMPILER) $< $(LINKER_FLAGS) -o $@
# main.o: main.cpp
# 	$(COMPILER) $(COMPILER_FLAGS) $< -o $@
# clean:
# 	rm *.o && rm $(OUTPUT_FILE)



# SDL_LIB = /usr/lib
# SDL_INCLUDE = /usr/include
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
