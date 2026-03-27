\# ProtoPlay Engine



\## Build Instructions



\### Requirements

\- CMake 3.20 or higher

\- Ninja build system

\- A C++20 compatible compiler (GCC, MSVC, or Clang)

\- Windows (currently the only supported platform)



\### Building the Engine

```

git clone <repo-url>

cd 3DGameEngine/3DGameEngine

cmake -B build -G Ninja

cmake --build build

```



The engine executable, game DLL, and mesh converter will be built to `build/bin/`.



\### Running the Engine

```

cd build/bin

./engine

```



\### Controls

\- \*\*Q\*\* — Toggle editor

\- \*\*Escape\*\* — Quit

\- \*\*B\*\* — Reload gameplay DLL (hot reload)



\### Mesh Converter

Convert OBJ files to the engine's binary mesh format:

```

cd build/bin

./mesh\_converter input.obj output.mesh

```



Place the output `.mesh` files in the `resources/` directory.



\### Hot Reloading

Edit gameplay code in `src/game.cpp`, `src/movement\_system.cpp`, or `src/collision\_system.cpp`. Then rebuild only the gameplay DLL:

```

cmake --build build --target game

```



Press \*\*B\*\* in the engine window to reload the new code. The game state is preserved across reloads.



\### Project Structure

```

3DGameEngine/

&#x20;   src/                  # Engine and gameplay source

&#x20;       application.cpp   # Host executable, main loop, input

&#x20;       ecs.cpp           # ECS state, init, timing

&#x20;       game.cpp          # Gameplay systems (hot-reloadable DLL)

&#x20;       movement\_system.cpp

&#x20;       collision\_system.cpp

&#x20;       render\_system.cpp

&#x20;       asset\_manager.cpp

&#x20;       camera.cpp

&#x20;       editor.cpp

&#x20;       events.cpp

&#x20;       window.cpp

&#x20;       text.cpp

&#x20;       serialization.cpp

&#x20;   vendor/               # Third-party dependencies

&#x20;       glfw/             # Windowing (built from source)

&#x20;       glad/             # OpenGL loader

&#x20;       glm-master/       # Math library

&#x20;       imgui/            # Debug UI

&#x20;       stb/              # Image loading

&#x20;   resources/            # Textures, fonts, binary meshes

&#x20;   MeshBinaryConverter/  # OBJ to binary mesh tool

&#x20;   CMakeLists.txt

```

