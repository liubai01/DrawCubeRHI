using UnrealBuildTool;

public class DrawCubeRHIShaders : ModuleRules
{
	public DrawCubeRHIShaders(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new[]
		{
			"Core",
			"RHI",
			"RenderCore"
		});
	}
}

