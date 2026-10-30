# Particle Simulation

A small OpenGL particle simulation with compute shaders, rendered as point sprites. The project uses CMake and relies on the vendored GLFW, GLAD, GLM, and ImGui files in the `external/` directory.

## Requirements

- CMake 3.20 or newer
- A C++ compiler with C++17 support
- Windows build environment for the bundled MinGW GLFW libraries
- OpenGL 4.3-capable GPU/driver

## Build

From the project root:

```powershell
cmake -S . -B build
cmake --build build
```

This creates the executable in `build/particles.exe`.

If you are using an IDE such as CLion, you can also configure the project normally with CMake and build it from there.

## Run

Run the executable from the build directory so the shader paths resolve correctly:

```powershell
cd build
.\particles.exe
```

The program loads shaders from the sibling `shaders/` directory, so running from the repository root may also work depending on your working directory and launch configuration. The project is written to expect the shaders to be reachable via relative paths such as `../shaders/...`.

## Notes

- The app uses ImGui for the control panel.
- Particles spawn gradually at startup and after reset; adjust the delay duration in the control panel.
- The compute shaders are located under `shaders/compute/`.
- The render shader is located under `shaders/render/`.
- If the program fails to start, confirm that `glfw3.dll` is present next to the executable and that your system supports OpenGL 4.3.
