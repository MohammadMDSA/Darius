
# Darius Engine

## First Look

<img src="images/overview.png" width="50%"><p>This is how the editor looks like. You can select game objects from Scene Graph window and change its transform properties and mesh trough Details window. You also can change object position via the gizmo which is located on object position.</p>

<p>You can explore the scene with ghost camera by holding right-click and via W/A/S/D.</p>

## Installing

1. Clone the repository and its submodules:

    `git clone --recursive https://github.com/MohammadMDSA/Darius.git`

2. Install dependencies
    1. Install Boost and have `Boost_ROOT` environment variable pointing to where you've installed Boost.
    2. Install FBX SDK.
        1. Download FBX SDK 2020 from [FBX SDK website](https://autodesk.com/fbx) and install it.
        2. Have `FBXSDK_ROOT` system environment variable pointing to where you've installed FBX SDK.
3. Configure and Build using CMake and *MSVC* compiler. (Other compilers *may* work but not tested)

4. Run `DariusEngine` executable to run Darius Editor.