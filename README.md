# Particle Simulator
A Particle Simulator written in C++ and [SFML](https://www.sfml-dev.org/).


## Features
- Simulation using verlet integration method.
- Customizable radius, mass, coef of restitutiton, starting position, velocity, and acceleration of particles.
- Customizable display size, sub step count, and fps (physics behavior is dependent of fps).

In the future, I will be adding space partitioning, multi-threading, and hopefully make the simulation independent of fps. 


## Getting Started (Windows)
1. Clone the repository
2. Install g++
3. Download SFML-2.6.0 (GCC MinGW) from https://www.sfml-dev.org/download/sfml/2.6.0/ and add it to the root directory
4. Compile and run `main.exe` with the following commands
    ```
    g++ -c main.cpp -ISFML-2.6.0/include -DSFML_STATIC
    g++ -o main main.o -LSFML-2.6.0/lib -lsfml-graphics-s -lsfml-window-s -lsfml-system-s -lopengl32 -lwinmm -lgdi32
    del main.o
    main.exe
    ```
