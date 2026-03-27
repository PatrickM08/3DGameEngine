# ProtoPlay Engine

## Build Instructions

### Requirements
- CMake 3.20 or higher
- Ninja build system
- A C++20 compatible compiler (GCC, MSVC, or Clang)
- Windows (currently the only supported platform)

### Building the Engine
```
git clone <repo-url>
cd 3DGameEngine/3DGameEngine
cmake -B build -G Ninja
cmake --build build
```

The engine executable, game DLL, and mesh converter will be built to `build/bin/`.

### Running the Engine
```
cd build/bin
./engine
```

### Controls
- **Q** — Toggle editor
- **Escape** — Quit
- **B** — Reload gameplay DLL (hot reload)

### Mesh Converter
Convert OBJ files to the engine's binary mesh format:
```
cd build/bin
./mesh_converter input.obj output.mesh
```

Place the output `.mesh` files in the `resources/` directory.

### Hot Reloading
Edit gameplay code in `src/game.cpp`, `src/movement_system.cpp`, or `src/collision_system.cpp`. Then rebuild only the gameplay DLL:
```
cmake --build build --target game
```

Press **B** in the engine window to reload the new code. The game state is preserved across reloads.

### Project Structure
```
3DGameEngine/
    src/                  # Engine and gameplay source
        application.cpp   # Host executable, main loop, input
        ecs.cpp           # ECS state, init, timing
        game.cpp          # Gameplay systems (hot-reloadable DLL)
        movement_system.cpp
        collision_system.cpp
        render_system.cpp
        asset_manager.cpp
        camera.cpp
        editor.cpp
        events.cpp
        window.cpp
        text.cpp
        serialization.cpp
    vendor/               # Third-party dependencies
        glfw/             # Windowing (built from source)
        glad/             # OpenGL loader
        glm-master/       # Math library
        imgui/            # Debug UI
        stb/              # Image loading
    resources/            # Textures, fonts, binary meshes
    MeshBinaryConverter/  # OBJ to binary mesh tool
    CMakeLists.txt
```
