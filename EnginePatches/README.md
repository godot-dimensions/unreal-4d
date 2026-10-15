# Unreal 4D engine patches

Several of the features in Unreal 4D require patches to the Unreal Engine source code. This directory contains the patches that are applied to the engine source code. Unreal 4D will not compile correctly without a patched engine.

You can find patches in the following subdirectories:

- `UE5-main` - Patches for Unreal Engine 5 main branch.
- `UE6-main` - Patches for Unreal Engine 6 main branch.
- `UE5.7` - Patches for the Unreal Engine 5.7 branch.
- `UE5.8` - Patches for the Unreal Engine 5.8 branch.

If you have access to the EpicGames organization on GitHub:

- You can find Git branches with these patches pre-applied here: https://github.com/aaronfranke/UnrealEngine/branches/all?query=unreal4d%2Fue

- Please thumbs up aaronfranke's pull requests, which propose adding some of these patches upstream to the Unreal Engine source code, when those patches are also applicable to the base engine for 3D games. https://github.com/EpicGames/UnrealEngine/pulls/aaronfranke

## License

The Unreal Engine End User License Agreement (EULA) explicitly allows sharing snippets of engine code for the purpose of patch files:

> ii. Distributing Engine Code for Public Discussion
>
> You are permitted to post snippets of Engine Code, up to 30 lines of code in length, online in public forums for the sole purpose of discussing the content of the snippet or Distribute such snippets in connection with supporting patches and plug-ins for the Licensed Technology, so long as it is not for the purpose of enabling third parties without a license to the Engine Code to use or modify any Engine Code or to aggregate, recombine, or reconstruct any larger portion of the Engine Code.

Source: https://www.unrealengine.com/eula/unreal

These patch files are provided publicly under the terms of this license, but are otherwise as freely licensed as possible. You are free to use, modify, and distribute these patches as you see fit.

Note that the patches are not useful to you unless you have a copy of the Unreal Engine source code and agree to the Unreal Engine EULA, so you must read and understand the Unreal Engine EULA before using these patches anyway.
