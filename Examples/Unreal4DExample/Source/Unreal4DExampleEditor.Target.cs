using UnrealBuildTool;

public class Unreal4DExampleEditorTarget : TargetRules
{
	public Unreal4DExampleEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V6;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_7;
		ExtraModuleNames.Add("Unreal4DExample");
	}
}
