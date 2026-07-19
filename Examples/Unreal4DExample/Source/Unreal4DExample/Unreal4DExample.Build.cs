using UnrealBuildTool;

public class Unreal4DExample : ModuleRules
{
	public Unreal4DExample(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"Unreal4DCore"
		});
	}
}
