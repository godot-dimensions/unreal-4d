using UnrealBuildTool;

public class Unreal4DCore : ModuleRules
{
	public Unreal4DCore(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Chaos",
			"ChaosCore",
			"Core",
			"CoreUObject",
			"DeveloperSettings",
			"Engine",
			"EnhancedInput",
			"GameplayTags",
			"InputCore",
			"Json",
			"PhysicsCore",
			"UMG",
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Unreal4DRendering"
		});
	}
}
