using UnrealBuildTool;

public class Unreal4DExampleEditorTarget : TargetRules
{
	public Unreal4DExampleEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
#if UE_6_0_OR_LATER
		DefaultBuildSettings = BuildSettingsVersion.V9;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal6_0;
		// UE6 enables this optional debugger by default, pulling in unused CommonUI input routing.
		DisablePlugins.Add("InputDebugging");
#elif UE_5_9_OR_LATER
		DefaultBuildSettings = BuildSettingsVersion.V8;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_9;
#elif UE_5_8_OR_LATER
		DefaultBuildSettings = BuildSettingsVersion.V7;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_8;
#else
		DefaultBuildSettings = BuildSettingsVersion.V6;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_7;
#endif
		ExtraModuleNames.Add("Unreal4DExample");
	}
}
