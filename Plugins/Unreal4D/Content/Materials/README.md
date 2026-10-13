# Unreal4D Mesh Projection Material

Project Settings → Plugins → Unreal 4D Rendering controls the default projection material. A mesh's `ProjectionMaterial` overrides this setting; clearing the project default uses Unreal's default surface material. Editor changes refresh existing projections.

The built-in material is a public plugin asset at `/Unreal4D/Materials/MAT_Mesh4DProjection_Default.MAT_Mesh4DProjection_Default`. Enable **Show Plugin Content** in the Content Browser to find it, or load this path from code. Projects can duplicate it into their own Content folder and select the copy in the settings. Replacement materials should preserve custom primitive data indices 0–2 for projection RGB and 3 for opacity. Indices 4 and 5 are reserved by the projection vertex factory for scale and distance.

The setting is available from C++ through `URenderingSettings4D::Get()->GetDefaultProjectionMaterial()` (include `Unreal4DCore/Render/RenderingSettings4D.h`), and from Blueprints through **Get** on Rendering Settings 4D, followed by **Get Default Projection Material**. The configurable asset path is a soft reference; the settings object loads and retains a strong reference to the selected material after configuration is applied. Project overrides are saved in `DefaultGame.ini` under `[/Script/Unreal4DCore.RenderingSettings4D]`.
