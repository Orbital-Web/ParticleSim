# Particle Simulator
A Particle Simulator written in C++ and [SFML](https://www.sfml-dev.org/), and [Thread Pools](https://github.com/bshoshany/thread-pool).
The simulator has been greatly inspired by [this video](https://www.youtube.com/watch?v=9IULfQH7E90&t=1s).


## Features
- Simulation using verlet integration method.
- Customizable radius, mass, coef of restitutiton, starting position, velocity, and acceleration of particles.
- Customizable display size, sub step count, and fps (physics behavior is dependent of fps).
- Space partitioning
- Multi-threading

The space partitioning and threading is still incomplete as it is still pretty inefficient and lags with a large number of particles. 


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
