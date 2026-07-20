#pragma once

#include "../../Math/Transform4D.h"

#include "Camera/CameraComponent.h"

#include "Camera4DComponent.generated.h"

UENUM(BlueprintType, Category = "Unreal 4D|Component|Camera")
enum class ECamera4DDepthFadeMode : uint8
{
	Disabled = 0,
	Distance = 1,
	XYZ_Only = 2,
	Z_Only = 3,
};

UENUM(BlueprintType, Category = "Unreal 4D|Component|Camera")
enum class ECamera4DPerpFadeMode : uint8
{
	Disabled = 0,
	Transparency = 1,
	HueShift = 2,
	Transparency_HueShift = 3,
};

UCLASS(BlueprintType, Category = "Unreal 4D|Component|Camera", meta=(BlueprintSpawnableComponent))
class UCamera4DComponent : public UCameraComponent
{
	GENERATED_BODY()

private:
	bool _isCurrent = false;

protected:
	virtual void OnRegister() override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera 4D", meta=(DisplayPriority=1))
	ECamera4DDepthFadeMode DepthFadeMode = ECamera4DDepthFadeMode::Disabled;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera 4D", meta=(DisplayPriority=2))
	double DepthFadeStart = 25.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera 4D", meta=(DisplayPriority=3))
	ECamera4DPerpFadeMode PerpFadeMode = ECamera4DPerpFadeMode::Disabled;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera 4D", meta=(DisplayPriority=4))
	FLinearColor PerpFadeColorNegative = FLinearColor(0.0f, 0.5f, 1.0f);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera 4D", meta=(DisplayPriority=5))
	FLinearColor PerpFadeColorPositive = FLinearColor(1.0f, 0.5f, 0.0f);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera 4D", meta=(DisplayPriority=6))
	double PerpFadeDistance = 5.0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera 4D", meta=(DisplayPriority=7))
	double PerpFadeSlope = 1.0;

	double GetFocalLength() const;
	void SetFocalLength(const double InFocalLength);
	FTransform4D GetGlobalTransform4D() const;

	// Current camera functions.
	bool IsCurrent() const;
	void SetCurrent(bool bEnable);
	void ClearCurrent(bool bEnableNext = true);
	void MakeCurrent();

	// Projection functions.
	bool IsPositionBehind(const FVector4& InGlobalPosition) const;
	FVector4 ViewportToWorldRayOrigin(const FVector2D& InViewportPosition) const;
	FVector4 ViewportToWorldRayDirection(const FVector2D& InViewportPosition) const;
	FVector2D WorldToViewportLocalNormal(const FVector4& InLocalPosition) const;
	FVector2D WorldToViewport(const FVector4& InGlobalPosition) const;
};
