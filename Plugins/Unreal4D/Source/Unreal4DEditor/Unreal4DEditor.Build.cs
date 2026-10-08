using UnrealBuildTool;

public class Unreal4DEditor : ModuleRules
{
	public Unreal4DEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"AssetRegistry",
			"AssetTools",
			"Chaos",
			"Core",
			"CoreUObject",
			"EditorFramework",
			"Engine",
			"GizmoEdMode",
			"InputCore",
			"InteractiveToolsFramework",
			"Kismet",
			"LevelEditor",
			"PropertyEditor",
			"Slate",
			"SlateCore",
			"TypedElementFramework",
			"Unreal4DCore",
			"UnrealEd",
		});
		// Access Kismet private headers for SSCSEditorViewport (needed for Blueprint gizmo integration).
		PrivateIncludePaths.Add(System.IO.Path.Combine(EngineDirectory, "Source", "Editor", "Kismet", "Private"));
	}
}
