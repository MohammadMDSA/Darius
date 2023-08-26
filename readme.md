
# Darius Engine

[![Codacy Badge](https://app.codacy.com/project/badge/Grade/ac33ce5f8bae433698ab70c31a7d6fbf)](https://app.codacy.com/gh/MohammadMDSA/Darius/dashboard?utm_source=gh&utm_medium=referral&utm_content=&utm_campaign=Badge_grade)

## First Look

<img src="Docs/overview.png" width="100%">

---

## Features

- Core
    - Runtime Type Reflection
    - Code Generation
        - Auto Type Reflection Registration
        - Auto Serialization and Deserialization
- Resource Management
    - Default: PBR Material, Texture (DDS, TGA), Static and Skeletal Mesh (FBX), Animation (FBX), Vertex Batch Resources, and Physics Material
    - Dynamic Resource Type Registration (You can define your own types)
- Renderer
    - DirectX 12 Vendor
    - Anti-Aliasing 
        - TAA
    - Post Processing
        - HDR Tone Mapping
        - Bloom
    - Physics-Based Rendering (PBR) Materials
        - Diffuse, Normal, Roughness, Metallic, and Emissive Textures Mapping
        - Diffuse Albedo, Emissive, Metallic, and Roughness Components
        - Opaque, Transparent
        - Multiple Sub-Meshes
    - Lighting
        - Hammon Model
        - Shadow Mapping
    - Texture Filtering
    - Skybox
    - Ray Tracing Renderer
        - Static Mesh
        - Terrain
    - Rasterization Renderer
        - Static Mesh
        - Skeletal Mesh
        - Terrain
        - Billboard
    - Optimizations
        - Frustum Culling
        - Separate Z Pass
        - Pipeline Caching
    - PIX Debugging Integration
- Debug
    - Debug Draws
- Scene Management
    - Saving & Loading
    - Entity Component System Architecture
    - Scene Graph and Hierarchical Transform Math
    - Prefab Creation
    - FBX Scene Loading
    - Game Object Instantiation
- Physics
    - Dynamic, Static, and Kinematic Actors
    - Physics Material
    - Collision Handling
        - Box, and Sphere Colliders
    - Scene Query
        - Ray Casting
- Editor
    - Resource Monitor
    - Ghost and Orbit Cameras
    - Gizmos (Translation, Rotation, and Scale)
    - Simulation (Run, Stop, Pause, and Step)
    - Profiler Graph
    - Game Object Property Manipulation through GUI (including components)
    - Resource Property Manipulation through GUI (including saving & loading)
- Job System and CPU Multi-Core Utilization
- CPU & GPU Profiling
    - Framerate Metrics (Last, Max, Avg)
    - Framerate Graphs
    - CPU Frame Snapshot and Flame Graph
- And so much more!

---

### PBR Material
<img width="49%" display="inline-block" src="Docs/MetallicRoughness.png">
<img width="49%" display="inline-block" src="Docs/PBR.png">

<br>

### Lighting
![](Docs/Lighting.png)

<br>

### Mesh Rendering (Skinned and Static)
![](Docs/MeshRendering.png)

<br>

### Skinned Mesh Animation
![](Docs/SkinnedMeshAnimation.png)

---

## Known Braking Issues
- Prefab and FBX resources must be loaded. Either manually or through the usage of its contained resources.
---

## Installing

1. Clone the repository and its submodules:

    `git clone --recursive https://github.com/MohammadMDSA/Darius.git`

2. Install dependencies
    1. Install Boost and have `Boost_ROOT` environment variable pointing to where you've installed Boost.
    2. Install FBX SDK.
        1. Download FBX SDK 2020 from [FBX SDK website](https://autodesk.com/fbx) and install it.
        2. Have `FBXSDK_ROOT` system environment variable pointing to where you've installed FBX SDK.
    3. Install Pix and have `WinPixEventRuntime_DIR` enviroment variable pointing to its install directory. It must contain `Include/` and `bin/` as its subdirectories
3. Configure and Build using CMake and *MSVC* compiler. (Other compilers *may* work but have not been tested)

4. Run `Darius` executable to run Darius Editor with demo project.
