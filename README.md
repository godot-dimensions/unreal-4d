# Unreal 4D

Unreal 4D adds four-dimensional math, components, physics, rendering, and editor tooling to Unreal Engine for making 4D games.

## Origins

Unreal 4D is developed by Input Output Group (IOG): https://www.iog.io/

The code in Unreal 4D was originally developed for Quantum Hosky, a 4D game project: https://quantumhosky.iog.io/

IOG has since pivoted the reusable engine-level technology, releasing it as open source under the MIT license. Much of the code in Unreal 4D is based on Godot 4D, which is published as public domain under The Unlicense.

## Related technologies

- Godot 4D: https://github.com/godot-dimensions/godot-4d
  - Open source 4D module for making 4D games in Godot Engine.

- G4MF: https://github.com/godot-dimensions/g4mf
  - Open standard glTF-like 4D model format for the interchange of 4D models between 4D software.

- Unreal Engine: https://www.unrealengine.com/
  - Proprietary game engine for making 3D games, which Unreal 4D extends to support 4D games.
  - Usage of Unreal 4D requires obtaining a copy of the Unreal Engine source code: https://dev.epicgames.com/documentation/unreal-engine/downloading-source-code-in-unreal-engine
    - Which requires agreeing to the Unreal Engine EULA: https://www.unrealengine.com/en-US/eula

## Installation and usage

### Prerequisites

- Git: https://git-scm.com/install/
- A clone of this repository.
- A clone of the Unreal Engine source repository.
  - Check out the `5.7` branch for Unreal Engine 5.7, which is the currently tested version.
  - Other engine versions may work, but are untested and unsupported.
- On Windows:
  - Visual Studio: https://visualstudio.microsoft.com/
    - Use Visual Studio 2022 for Unreal Engine 5.x.
    - Use Visual Studio 2026 for Unreal Engine 6.x.
  - In the Visual Studio Installer, install:
    - The "Desktop development with C++" workload.
    - The "Game development with C++" workload.
    - Unreal Engine workload components.
- On macOS and Linux, install the platform build dependencies required by your Unreal Engine source checkout.
- The macOS and Linux scripts are new and have not yet been validated on those platforms.

### Compile on Windows

1. Follow the Unreal Engine instructions to generate project files and build the engine.
    - Do a clean build of the engine before installing Unreal 4D, just to make sure everything is working.
2. Apply the appropriate engine patch in [`EnginePatches/`](EnginePatches/) to your clone of the Unreal Engine source code.
    - Choose the patch that matches your engine version.
3. Recompile the engine after applying the patch.
4. Navigate to [`Plugins/Unreal4D/`](Plugins/Unreal4D/) and run one of the build scripts, such as `Build-Windows.bat` or `Build-Windows.ps1`.
    - This generates build artifacts in `Plugins/Unreal4D/BuildArtifacts` which can be used in Unreal projects as a plugin dependency.
5. Navigate to [`Examples/Unreal4DExample/`](Examples/Unreal4DExample/) and run one of the build scripts, such as `Build-Windows.bat` or `Build-Windows.ps1`.
    - This generates a Visual Studio solution (`Unreal4DExample.sln`) and VS Code configuration files, then builds the example.
6. Open the generated solution in Visual Studio.
7. Set "Unreal4DExample" as the startup project.
8. Build and run the project to start the Unreal editor.

Alternatively, run `Run-Windows.bat` from this repository's root to build and open the example in one step. Pass `-SkipBuild` to launch an existing editor build without rebuilding.

The Windows `.bat` and `.ps1` scripts do the same thing. The `.bat` script calls the `.ps1` script, for convenience of double-clicking from Windows Explorer. Otherwise, the `.ps1` script should be ran directly, such as from a terminal, or from a build script.

See the `README.md` files in each subdirectory for more details.

If you do not wish to use the example project, you can skip steps 5 to 8.

### Compile on macOS or Linux

1. Follow the Unreal Engine instructions to set up and build the engine for your platform.
2. Apply the appropriate engine patch in [`EnginePatches/`](EnginePatches/) to your Unreal Engine source checkout.
3. Recompile the engine after applying the patch.
4. From this repository's root, run `Build-macOS.command` on macOS or `Build-Linux.sh` on Linux.
    - The root script packages the plugin, generates VS Code metadata, and builds the example project.
    - `Build-macOS.command` can be double-clicked in Finder.
    - `Build-Unix.sh` contains the shared implementation and can be invoked directly from a terminal or another build script.
5. To build and open the example in the Unreal Editor in one step, run `Run-macOS.command` on macOS or `Run-Linux.sh` on Linux.
  - The run scripts accept the same arguments as their corresponding build scripts.
  - Pass `--skip-build` to open the existing editor build without rebuilding.
  - `Run-macOS.command` can be double-clicked in Finder.

The platform-specific scripts automatically select `Mac` or `Linux`. Pass `--engine-root` or set `UNREAL_ENGINE_ROOT` when the Unreal Engine checkout is located somewhere other than the adjacent `UnrealEngine5` directory.

You can also run the scripts in [`Plugins/Unreal4D/`](Plugins/Unreal4D/) and [`Examples/Unreal4DExample/`](Examples/Unreal4DExample/) independently. See their README files for supported arguments.

### Show Unreal 4D classes in the Content Browser

By default, Unreal Engine hides classes from plugins in the Content Browser. To show Unreal 4D classes:

- Click the gear icon at the top-right of the Content Browser.
- Check the box that says "Plugin Content" in the section below the "SHOW" section.
- Expand the new "Plugins" folder in the left panel of the Content Browser.

You will now be able to see Unreal 4D classes in the Content Browser.

### Develop with Visual Studio Code

The example project's build scripts generate Visual Studio Code (VS Code) configuration files. The Windows script also generates Visual Studio solution files.

The platform C++ toolchain is still required for compiling Unreal Engine projects, but Visual Studio Code can be used as the code editor.

1. Generate and compile everything, as described above.
2. Open this repository's root folder, `unreal-4d`, in Visual Studio Code.
3. Install the recommended Microsoft C/C++ and C# Dev Kit extensions when prompted.

The build script asks UnrealBuildTool to generate the C++ compile database and C# projects. It then creates an ignored, machine-specific `.vscode/` directory at the repository root so IntelliSense also works when a graphical Git client opens this folder directly. Rerun the script after adding, deleting, or renaming source files, changing module dependencies, switching engines, switching branches, etc.

UnrealBuildTool also generates `Examples/Unreal4DExample/Unreal4DExample.code-workspace`. Both opening the repository's root folder and this workspace will work. This workspace will show both the example project and the engine source code.

### Use Unreal 4D from another project

When using Unreal 4D in another project, consider changing the RHI rendering driver to `Vulkan`. The `DirectX 12` driver causes system crashes when rendering 4D objects on some AMD graphics cards.

#### Copy or submodule the plugin into the project

Place the complete plugin directory at:

```text
YourProject/Plugins/Unreal4D/
```

Then enable `Unreal4D` in the project's `.uproject` file or through the Unreal Editor plugin browser. This is the most self-contained arrangement for a pinned dependency.

#### Reference a shared checkout

Alternatively, add the directory containing the plugin to `AdditionalPluginDirectories` in the `.uproject`. Paths are relative to the project descriptor:

```json
"AdditionalPluginDirectories": [
  "../unreal-4d/Plugins"
],
"Plugins": [
  {
    "Name": "Unreal4D",
    "Enabled": true
  }
]
```

Add `Unreal4DCore` to any project module that includes its public headers. Add `Unreal4DRendering` only when directly consuming rendering APIs; ordinary users should enter through Core.

## Folder struccture

### Repository folder overview

- `EnginePatches/` contains patches that need to be applied to the Unreal Engine source code for Unreal 4D to work.
- `Examples/Unreal4DExample/` contains a minimal example project that uses the plugin.
  - `Content/` contains example assets for testing the plugin.
- `Plugins/Unreal4D/` contains the Unreal 4D plugin itself.
  - `Source/` contains the plugin source code (the bulk of the repository).
    - `Unreal4DCore/` contains runtime-facing 4D types and systems.
    - `Unreal4DRendering/` owns renderer integration and plugin shaders.
    - `Unreal4DEditor/` contains editor-only tools and integration.

Generated IDE projects, build products, caches, saved editor state, and packaged plugin output are intentionally gitignored and not committed.

### Expected directory layout

You should have a clone of Unreal Engine next to a clone of this repository, like this:

```text
Unreal4D/
├── UnrealEngine5/ (if using Unreal Engine 5.x)
├── UnrealEngine6/ (if using Unreal Engine 6.x)
└── unreal-4d/ (this repository)
```

The build scripts expect this layout by default.

If you desire a different directory layout, you can pass `-EngineRoot` to a script, or set the `UNREAL_ENGINE_ROOT` environment variable when your engine source code is elsewhere. Command-line arguments take precedence over the environment variable.

## License

Unreal 4D is available under the [MIT License](LICENSE.txt).
