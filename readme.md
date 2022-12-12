
# Darius Engine

## First Look

<img src="Docs/overview.png" width="100%">

## Features
- Resource Management
    - Default: PBR Material, Texture (DDS, TGA), Static and Skeletal Mesh (FBX), Animation (FBX), Vertex Batch Resources, and Physics Material
    - Dynamic Resource Type Registration (You can define your own types)
- Renderer
    - Frustum Culling
    - Physics-Based Rendering (PBR) Materials
        - Diffuse, Normal, Roughness, Metallic, and Emissive Textures Mapping
        - Diffuse Albedo, Emissive, Metallic, and Roughness Components
        - Opaque and Transparent
    - Basic Lights (Point, Cone, and Directional)
    - Skybox
    - PIX Debugging Integration
- Debug
    - Debug Draws
- Scene Management
    - Saving & Loading
    - Dynamic Entity Component System
- Physics
    - Dynamic, Static, and Kinematic Actors
    - Physics Material
    - Collision Detection
    - Ray Casting
- Editor
    - Resource Monitor
    - Ghost and Orbit Cameras
    - Gizmos (Translation, Rotation, and Scale)
    - Simulation (Run, Stop, Pause, and Step)
    - Profiler Graph
    - Game Object Property Manipulation trough GUI (including components)
    - Resource Property Manipulation trough GUI (including saving & loading)
- Job System and CPU Multi-Core Utilization
- CPU & GPU Profiling
    - Framerate Metrics (Last, Max, Avg)
    - Framerate Graphs
    - CPU Frame Snapshot and Flame Graph
- Scene Graph and Hierarchical Transform Math
- And so much more!

## Installing

1. Clone the repository and its submodules:

    `git clone --recursive https://github.com/MohammadMDSA/Darius.git`

2. Install dependencies
    1. Install Boost and have `Boost_ROOT` environment variable pointing to where you've installed Boost.
    2. Install FBX SDK.
        1. Download FBX SDK 2020 from [FBX SDK website](https://autodesk.com/fbx) and install it.
        2. Have `FBXSDK_ROOT` system environment variable pointing to where you've installed FBX SDK.
3. Configure and Build using CMake and *MSVC* compiler. (Other compilers *may* work but not tested)

4. Run `DariusEngine` executable to run Darius Editor with demo project.