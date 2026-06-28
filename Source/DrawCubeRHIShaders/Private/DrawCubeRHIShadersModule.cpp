#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"
#include "ShaderCore.h"

class FDrawCubeRHIShadersModule : public IModuleInterface
{
public:
	virtual void StartupModule() override
	{
		const FString ShaderDir = FPaths::Combine(FPaths::ProjectDir(), TEXT("Shaders"));
		AddShaderSourceDirectoryMapping(TEXT("/DrawCubeRHI"), ShaderDir);
	}
};

IMPLEMENT_MODULE(FDrawCubeRHIShadersModule, DrawCubeRHIShaders)

