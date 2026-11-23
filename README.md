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

## Getting Started (macOS)
1. Clone the repository
2. Install Xcode Command Line Tools (`xcode-select --install`)
3. Download SFML-2.6.0 source from https://www.sfml-dev.org/download/sfml/2.6.0/ and add it to the root directory
4. Build SFML:
    ```
    cd SFML-2.6.0 && mkdir -p build && cd build
    cmake .. -DSFML_STATIC_LIBRARIES=TRUE -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_C_FLAGS="-Wno-unused-command-line-argument" \
      -DCMAKE_CXX_FLAGS="-Wno-unused-command-line-argument" \
      -DCMAKE_OBJC_FLAGS="-Wno-unused-command-line-argument"
    make -j$(sysctl -n hw.ncpu)
    cd ../..
    ```
5. Compile and run `main` with the following commands
    ```
    g++ -std=c++17 -c main.cpp -ISFML-2.6.0/include -DSFML_STATIC
    g++ -o main main.o -ISFML-2.6.0/include -LSFML-2.6.0/build/lib \
      -lsfml-graphics -lsfml-window -lsfml-system -framework OpenGL \
      -Wl,-rpath,SFML-2.6.0/build/lib
    ./main
    ```
