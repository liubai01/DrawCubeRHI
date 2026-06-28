using UnrealBuildTool;

public class DrawCubeRHITarget : TargetRules
{
	public DrawCubeRHITarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_5;
		ExtraModuleNames.Add("DrawCubeRHI");
	}
}

