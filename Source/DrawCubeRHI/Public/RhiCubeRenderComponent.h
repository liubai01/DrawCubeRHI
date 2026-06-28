#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "RhiCubeRenderComponent.generated.h"

class UTextureRenderTarget2D;

UCLASS(ClassGroup=(Rendering), meta=(BlueprintSpawnableComponent))
class DRAWCUBERHI_API URhiCubeRenderComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	URhiCubeRenderComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RHI Cube")
	bool bDrawOnBeginPlay = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RHI Cube")
	UTextureRenderTarget2D* OutputRenderTarget = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RHI Cube")
	FLinearColor BackgroundColor = FLinearColor::Transparent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RHI Cube", meta = (ClampMin = "0.01", UIMin = "0.01"))
	float CubeHalfExtent = 0.45f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RHI Cube|Animation")
	bool bAnimate = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RHI Cube|Animation", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float RotationSpeedDegreesPerSecond = 45.0f;

	UFUNCTION(BlueprintCallable, Category = "RHI Cube")
	void DrawCube();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	float ElapsedSeconds = 0.0f;
};

