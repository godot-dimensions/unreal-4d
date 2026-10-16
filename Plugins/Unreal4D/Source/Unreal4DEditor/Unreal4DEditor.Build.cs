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
			"BlueprintGraph",
			"Chaos",
			"Core",
			"CoreUObject",
			"EditorFramework",
			"EditorStyle",
			"Engine",
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
#if UE_6_0_OR_LATER
		PrivateDependencyModuleNames.Add("EditorViewport");
#endif
		// Access Kismet private headers for SSCSEditorViewport (needed for Blueprint gizmo integration).
		PrivateIncludePaths.Add(System.IO.Path.Combine(EngineDirectory, "Source", "Editor", "Kismet", "Private"));
	}
}
