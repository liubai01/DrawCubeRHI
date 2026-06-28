# DrawCubeRHI

Minimal Unreal Engine 5.5 RHI cube rendering demo.

Note: the original learning HTML that inspired this repo was written against UE 5.6 source notes, but this standalone demo project targets UE 5.5.

This demo bypasses UE's normal mesh/material rendering path. It records a small offscreen RHI draw list on the Render Thread, writes a cube into a `UTextureRenderTarget2D`, and lets UMG display that render target as a UI texture.

## Core Idea

```text
Game Thread
  -> URhiCubeRenderComponent::DrawCube()
  -> ENQUEUE_RENDER_COMMAND

Render Thread
  -> GetRenderTargetTexture()
  -> VertexBuffer / IndexBuffer
  -> VertexDeclaration
  -> Global Shader VS / PS
  -> Transition ColorRT / DepthRT
  -> BeginRenderPass
  -> Graphics PSO
  -> SetShaderParameters
  -> SetStreamSource
  -> DrawIndexedPrimitive
  -> EndRenderPass
  -> Transition ColorRT to SRV
```

## File Map

```text
DrawCubeRHI.uproject

Source/
  DrawCubeRHI/
    DrawCubeRHI.Build.cs
    Private/
      DrawCubeRHIModule.cpp
      RhiCubeRenderComponent.cpp
    Public/
      RhiCubeRenderComponent.h

  DrawCubeRHIShaders/
    DrawCubeRHIShaders.Build.cs
    Private/
      DrawCubeRHIShadersModule.cpp
      DrawCubeRHIShaders.cpp
    Public/
      DrawCubeRHIShaders.h

Shaders/
  Private/
    DrawCubeRHI.usf
```

## Why Two Modules?

`DrawCubeRHIShaders` loads at `PostConfigInit` so Global Shader types are registered early enough for UE's shader system.

`DrawCubeRHI` loads at `Default` and contains normal gameplay-facing code such as `UCLASS` and `USceneComponent`.

This avoids loading the whole game module too early while still making shader registration safe.

## Usage

1. Open the project in Unreal Engine 5.5.
2. Add `URhiCubeRenderComponent` to an Actor.
3. Create a `UTextureRenderTarget2D` asset.
4. Assign it to `OutputRenderTarget`.
5. Display the render target in UMG through an `Image` brush or a UI material.
6. Enable `bAnimate` to redraw the cube every tick.

## Learning Focus

- `ENQUEUE_RENDER_COMMAND`
- `FGlobalShader`
- `.usf` shader registration
- RHI vertex/index buffers
- `FVertexDeclarationElementList`
- `FRHIRenderPassInfo`
- resource `Transition`
- `FGraphicsPipelineStateInitializer`
- `SetGraphicsPipelineState`
- `DrawIndexedPrimitive`
