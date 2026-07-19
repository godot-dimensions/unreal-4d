using UnrealBuildTool;

public class Unreal4DEditor : ModuleRules
{
	public Unreal4DEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"AssetTools",
			"Chaos",
			"Core",
			"CoreUObject",
			"EditorFramework",
			"Engine",
			"GizmoEdMode",
			"InputCore",
			"InteractiveToolsFramework",
			"Slate",
			"SlateCore",
			"Unreal4DCore",
			"UnrealEd",
		});
	}
}
