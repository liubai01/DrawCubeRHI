#pragma once

#include "CoreMinimal.h"
#include "GlobalShader.h"
#include "ShaderParameterStruct.h"

class FDrawCubeRHIVS : public FGlobalShader
{
	DECLARE_EXPORTED_GLOBAL_SHADER(FDrawCubeRHIVS, DRAWCUBERHISHADERS_API);
	SHADER_USE_PARAMETER_STRUCT(FDrawCubeRHIVS, FGlobalShader);

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER(FMatrix44f, LocalToClip)
	END_SHADER_PARAMETER_STRUCT()

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters&)
	{
		return true;
	}
};

class FDrawCubeRHIPS : public FGlobalShader
{
	DECLARE_EXPORTED_GLOBAL_SHADER(FDrawCubeRHIPS, DRAWCUBERHISHADERS_API);
	SHADER_USE_PARAMETER_STRUCT(FDrawCubeRHIPS, FGlobalShader);

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
	END_SHADER_PARAMETER_STRUCT()

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters&)
	{
		return true;
	}
};

