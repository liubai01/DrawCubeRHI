#include "RhiCubeRenderComponent.h"

#include "ClearQuad.h"
#include "DrawCubeRHIShaders.h"
#include "Engine/TextureRenderTarget2D.h"
#include "PipelineStateCache.h"
#include "RHICommandList.h"
#include "RHIResourceUtils.h"
#include "RHIStaticStates.h"
#include "ShaderParameterStruct.h"
#include "TextureResource.h"

DEFINE_LOG_CATEGORY_STATIC(LogRhiCubeRenderComponent, Log, All);

namespace
{
	struct FRhiCubeVertex
	{
		FVector4f Position;
		FVector4f Color;
	};

	FVector4f ProjectCubeVertex(const FVector3f& LocalPosition, float AngleRadians, float AspectRatio)
	{
		const float YawSin = FMath::Sin(AngleRadians);
		const float YawCos = FMath::Cos(AngleRadians);
		const float PitchSin = FMath::Sin(AngleRadians * 0.55f);
		const float PitchCos = FMath::Cos(AngleRadians * 0.55f);

		FVector3f Rotated;
		Rotated.X = LocalPosition.X * YawCos - LocalPosition.Y * YawSin;
		Rotated.Y = LocalPosition.X * YawSin + LocalPosition.Y * YawCos;
		Rotated.Z = LocalPosition.Z;

		FVector3f Pitched;
		Pitched.X = Rotated.X;
		Pitched.Y = Rotated.Y * PitchCos - Rotated.Z * PitchSin;
		Pitched.Z = Rotated.Y * PitchSin + Rotated.Z * PitchCos;

		const float CameraDistance = 2.4f;
		const float ViewZ = CameraDistance + Pitched.Z;
		const float Perspective = 1.0f / FMath::Max(ViewZ, 0.1f);
		const float SafeAspectRatio = FMath::Max(AspectRatio, 0.01f);

		const float ClipX = Pitched.X * Perspective / SafeAspectRatio;
		const float ClipY = Pitched.Y * Perspective;
		const float ClipZ = 1.0f - FMath::Clamp((ViewZ - 0.5f) / 4.0f, 0.0f, 1.0f);

		return FVector4f(ClipX, ClipY, ClipZ, 1.0f);
	}

	TArray<FRhiCubeVertex> BuildCubeVertices(float HalfExtent, float AngleRadians, float AspectRatio)
	{
		const float H = FMath::Max(HalfExtent, 0.01f);

		TArray<FRhiCubeVertex> Vertices;
		Vertices.SetNumUninitialized(8);

		for (uint32 Z = 0; Z < 2; ++Z)
		{
			for (uint32 Y = 0; Y < 2; ++Y)
			{
				for (uint32 X = 0; X < 2; ++X)
				{
					const int32 Index = X * 4 + Y * 2 + Z;
					const FVector3f LocalPosition(
						X ? -H : H,
						Y ? -H : H,
						Z ? -H : H);

					Vertices[Index].Position = ProjectCubeVertex(LocalPosition, AngleRadians, AspectRatio);
					Vertices[Index].Color = FVector4f(float(X), float(Y), float(Z), 1.0f);
				}
			}
		}

		return Vertices;
	}

	TArray<uint16> BuildCubeIndices()
	{
		return {
			0, 2, 3, 0, 3, 1,
			4, 5, 7, 4, 7, 6,
			0, 1, 5, 0, 5, 4,
			2, 6, 7, 2, 7, 3,
			0, 4, 6, 0, 6, 2,
			1, 3, 7, 1, 7, 5,
		};
	}

	void DrawCube_RenderThread(FRHICommandListImmediate& RHICmdList, FTextureRenderTargetResource* OutputResource, float HalfExtent, float AngleRadians, FLinearColor BackgroundColor)
	{
		check(IsInRenderingThread());

		if (!OutputResource)
		{
			return;
		}

		FTextureRHIRef ColorRT = OutputResource->GetRenderTargetTexture();
		if (!ColorRT.IsValid())
		{
			return;
		}

		FIntPoint Size = OutputResource->GetSizeXY();
		Size.X = FMath::Max(Size.X, 16);
		Size.Y = FMath::Max(Size.Y, 16);

		const float AspectRatio = float(Size.X) / float(Size.Y);
		const TArray<FRhiCubeVertex> Vertices = BuildCubeVertices(HalfExtent, AngleRadians, AspectRatio);
		const TArray<uint16> Indices = BuildCubeIndices();

		FBufferRHIRef VertexBuffer = UE::RHIResourceUtils::CreateVertexBufferFromArray(
			RHICmdList,
			TEXT("DrawCubeRHI.VertexBuffer"),
			EBufferUsageFlags::Static,
			MakeConstArrayView(Vertices));

		FBufferRHIRef IndexBuffer = UE::RHIResourceUtils::CreateIndexBufferFromArray(
			RHICmdList,
			TEXT("DrawCubeRHI.IndexBuffer"),
			EBufferUsageFlags::Static,
			MakeConstArrayView(Indices));

		FVertexDeclarationElementList Elements;
		const uint16 Stride = sizeof(FRhiCubeVertex);
		Elements.Add(FVertexElement(0, STRUCT_OFFSET(FRhiCubeVertex, Position), VET_Float4, 0, Stride));
		Elements.Add(FVertexElement(0, STRUCT_OFFSET(FRhiCubeVertex, Color), VET_Float4, 1, Stride));
		FVertexDeclarationRHIRef VertexDeclaration = PipelineStateCache::GetOrCreateVertexDeclaration(Elements);

		auto* ShaderMap = GetGlobalShaderMap(GMaxRHIFeatureLevel);
		TShaderMapRef<FDrawCubeRHIVS> VertexShader(ShaderMap);
		TShaderMapRef<FDrawCubeRHIPS> PixelShader(ShaderMap);

		const FRHITextureCreateDesc DepthDesc = FRHITextureCreateDesc::Create2D(
				TEXT("DrawCubeRHI.DepthRT"),
				Size.X,
				Size.Y,
				PF_DepthStencil)
			.SetFlags(ETextureCreateFlags::DepthStencilTargetable)
			.SetClearValue(FClearValueBinding::DepthFar);
		FTextureRHIRef DepthRT = RHICreateTexture(DepthDesc);

		RHICmdList.Transition(FRHITransitionInfo(ColorRT, ERHIAccess::Unknown, ERHIAccess::RTV));
		RHICmdList.Transition(FRHITransitionInfo(DepthRT, ERHIAccess::Unknown, ERHIAccess::DSVWrite));

		FRHIRenderPassInfo RenderPassInfo(
			ColorRT,
			ERenderTargetActions::DontLoad_Store,
			DepthRT,
			EDepthStencilTargetActions::ClearDepthStencil_DontStoreDepthStencil);

		RHICmdList.BeginRenderPass(RenderPassInfo, TEXT("DrawCubeRHI"));
		{
			RHICmdList.SetViewport(0, 0, 0.0f, float(Size.X), float(Size.Y), 1.0f);
			DrawClearQuad(RHICmdList, BackgroundColor);

			FGraphicsPipelineStateInitializer GraphicsPSOInit;
			RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
			GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
			GraphicsPSOInit.RasterizerState = TStaticRasterizerState<FM_Solid, CM_None>::GetRHI();
			GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<true, CF_DepthNearOrEqual>::GetRHI();
			GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = VertexDeclaration;
			GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
			GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
			GraphicsPSOInit.PrimitiveType = PT_TriangleList;

			SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit, 0);

			FDrawCubeRHIVS::FParameters VSParameters;
			VSParameters.LocalToClip = FMatrix44f::Identity;
			SetShaderParameters(RHICmdList, VertexShader, VertexShader.GetVertexShader(), VSParameters);

			RHICmdList.SetStreamSource(0, VertexBuffer, 0);
			RHICmdList.DrawIndexedPrimitive(IndexBuffer, 0, 0, Vertices.Num(), 0, Indices.Num() / 3, 1);
		}
		RHICmdList.EndRenderPass();

		RHICmdList.Transition(FRHITransitionInfo(ColorRT, ERHIAccess::RTV, ERHIAccess::SRVMask));
	}
}

URhiCubeRenderComponent::URhiCubeRenderComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void URhiCubeRenderComponent::BeginPlay()
{
	Super::BeginPlay();

	if (bDrawOnBeginPlay)
	{
		DrawCube();
	}
}

void URhiCubeRenderComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bAnimate)
	{
		return;
	}

	ElapsedSeconds += DeltaTime;
	DrawCube();
}

void URhiCubeRenderComponent::DrawCube()
{
	if (!OutputRenderTarget)
	{
		UE_LOG(LogRhiCubeRenderComponent, Warning, TEXT("DrawCube skipped: OutputRenderTarget is not assigned."));
		return;
	}

	FTextureRenderTargetResource* OutputResource = OutputRenderTarget->GameThread_GetRenderTargetResource();
	const float HalfExtent = CubeHalfExtent;
	const float AngleRadians = FMath::DegreesToRadians(ElapsedSeconds * RotationSpeedDegreesPerSecond);
	const FLinearColor ClearColor = BackgroundColor;

	OutputRenderTarget->ClearColor = ClearColor;

	ENQUEUE_RENDER_COMMAND(DrawCubeRHICommand)(
		[OutputResource, HalfExtent, AngleRadians, ClearColor](FRHICommandListImmediate& RHICmdList)
		{
			DrawCube_RenderThread(RHICmdList, OutputResource, HalfExtent, AngleRadians, ClearColor);
		});
}

