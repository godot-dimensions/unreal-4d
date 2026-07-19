using UnrealBuildTool;

public class Unreal4DExampleTarget : TargetRules
{
	public Unreal4DExampleTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V6;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_7;
		ExtraModuleNames.Add("Unreal4DExample");
	}
}
