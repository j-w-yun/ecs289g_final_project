# Hal’s Pals - ECS 289G Final Project - Declarative RTS

## Getting Started

### Compiled binary (Windows)

You can run the compiled binary by clicking `.\run.bat` on Windows.

### Instructions on Linux

1. Prerequisites: use the package manager in your system to get the following packages. (apt shown as example here)

```
sudo apt-get install g++
sudo apt-get install libsdl2-2.0
sudo apt-get install libsdl2-dev
sudo apt-get install libsdl2-ttf-2.0-0
sudo apt-get install libghc-sdl2-ttf-dev
```

2. Enter the project directory, build & run project
```
$ ./build.sh
```

3. To re-run the project, you can use this command
```
$ ./bin/run
```

### Instructions on Windows (MinGW)

1. Get MinGW32 here if you do not have it in your system.
https://plug-mirror.rcac.purdue.edu/osdn//mingw/68260/mingw-get-setup.exe

2. Choose and install basic packages

3. Open up command line window, enter the project directory, build & run project:
```
> build.bat
```

4. To re-run the project, you can use this command
```
> .\bin\run.exe
```

### Instructions on Windows (Visual Studio 2019 Community)

1. Make sure you choose "C++ for game development" when you install/modify VS2019

2. Open {Project directory}\vs2019\build\build.sln

3. Build and run the project with Release|x86 configuration

### Turning on/off features (rebuild after modification)

1. Change renderer: enable this macro to use SDL2 renderer. Disable to use OpenGL renderer. (main.c)

   ```
   #define USE_SDL2_RENDERER
   ```

2. Change terrain rendering when using OpenGL renderer (map.cpp)

   ```
   #define USE_PERLIN_NOISE_SHADER 1
   ```

---

## How to play

### Objective

The goal of the game is to eliminate all of the enemy’s factories so they can no longer produce units. You can command your units to plan and execute high level strategies and control a strong commander unit to join the fight.

### Controls

**Mouse Move, Arrow Keys**: Pan map

**Mouse Scroll, Page Up, Page Down**: Zoom map

**Mouse Left Click**:  Organize attack group

**Mouse Right Click**: Organize defense group

**E**: Select commander

**W, A, S, D**: Move commander

**N, M**: Increase or decrease group size

**K**: Disband all groups

**Spacebar**: Generate new map texture
