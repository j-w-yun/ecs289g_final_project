# Package manager: apt
# apt-get install libsdl2-2.0
# apt-get install libsdl2-dev
# apt-get install libsdl2-ttf-2.0-0
# apt-get install libghc-sdl2-ttf-dev

# Package manager: pacman
# pacman -S sdl2

COMPILER = g++
OUTPUT_FILE = run.exe


SDL_LIB = C:\SDL2\SDL2-2.0.12\x86_64-w64-mingw32\lib 
SDL_INCLUDE = C:\SDL2\SDL2-2.0.12\x86_64-w64-mingw32\include
SDL_TTF_LIB = C:\SDL2\SDL2_ttf-2.0.15\x86_64-w64-mingw32\lib
SDL_TTF_INCLUDE = C:\SDL2\SDL2_ttf-2.0.15\x86_64-w64-mingw32\include
COMPILER_FLAGS = -Wall -c -std=c++11 -I$(SDL_INCLUDE) -I$(SDL_INCLUDE)\SDL2 -I$(SDL_TTF_INCLUDE)
LINKER_FLAGS = -lmingw32 -mwindows -mconsole -L$(SDL_LIB) -lSDL2main -lSDL2 -L$(SDL_TTF_LIB) -llibSDL2_ttf
#CLEAN = del .\bin\*.o && del .\bin\$(OUTPUT_FILE) 
CLEAN = rm bin/*.o && rm bin/$(OUTPUT_FILE)



all: .\bin\$(OUTPUT_FILE)

.\bin\$(OUTPUT_FILE): .\bin\main.o
	$(COMPILER) $< $(LINKER_FLAGS) -o $@

.\bin\main.o: .\src\main.cpp
	$(COMPILER) $(COMPILER_FLAGS) $< -o $@

clean:
	$(CLEAN)
