#pragma once

#include "../Unreal4DDefines.h"

#include "Containers/Array.h"

#include "Math4D.generated.h"

struct FTransform4D;

// 4D math functions needed for Unreal 4D that don't fit in any other file.
// Prefer using UGeometry4D, UVectorFuncs4D, etc if those are appropriate.
// Some helpers are intentionally C++ only, but some are exposed to Blueprints.
UCLASS(meta = (ScriptName = "Math4D"))
class UNREAL4DCORE_API UMath4D : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static double Float8ToDouble(const uint8 InFloat8);
	static double Float16ToDouble(const uint16 InFloat16);
	static uint8 DoubleToFloat8(const double InDouble);
	static uint16 DoubleToFloat16(const double InDouble);

	static FLinearColor ColorLightened(const FLinearColor InColor, const float InAmount);

	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math")
	static double Vector2_Angle(const FVector2f& InVector);
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math")
	static double Vector2_AngleToPoint(const FVector2f& InFrom, const FVector2f& InTo);

	static int32 FindCommonInt32(const TArray<int32>& InA, const TArray<int32>& InB, int32& OutAIndex, int32& OutBIndex);

	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math")
	static double SafeDivide(const double InA, const double InB, const double InFallback = 0.0);
};
