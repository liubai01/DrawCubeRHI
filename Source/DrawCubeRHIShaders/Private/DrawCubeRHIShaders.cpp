#include "DrawCubeRHIShaders.h"

IMPLEMENT_GLOBAL_SHADER(FDrawCubeRHIVS, "/DrawCubeRHI/Private/DrawCubeRHI.usf", "MainVS", SF_Vertex);
IMPLEMENT_GLOBAL_SHADER(FDrawCubeRHIPS, "/DrawCubeRHI/Private/DrawCubeRHI.usf", "MainPS", SF_Pixel);

