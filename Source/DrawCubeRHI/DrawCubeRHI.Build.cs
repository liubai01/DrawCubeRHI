using UnrealBuildTool;

public class DrawCubeRHI : ModuleRules
{
	public DrawCubeRHI(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"RHI",
			"RenderCore",
			"DrawCubeRHIShaders"
		});
	}
}

