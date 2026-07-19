#pragma once

#include "../Unreal4DDefines.h"

#include "Containers/Array.h"
//#include "Math/UnrealMathUtility.h"

struct FTransform4D;

// 4D math functions needed for Unreal 4D that don't fit in any other file.
// Prefer using UGeometry4D, UVectorFuncs4D, etc if those are appropriate.
// These functions are NOT exposed to Blueprints.
class UNREAL4DCORE_API UMath4D
{
public:
	static double Float8ToDouble(const uint8 InFloat8);
	static double Float16ToDouble(const uint16 InFloat16);
	static uint8 DoubleToFloat8(const double InDouble);
	static uint16 DoubleToFloat16(const double InDouble);

	static FLinearColor ColorLightened(const FLinearColor InColor, const float InAmount);

	static double Vector2_Angle(const FVector2f& InVector);
	static double Vector2_AngleToPoint(const FVector2f& InFrom, const FVector2f& InTo);

	static int32 FindCommonInt32(const TArray<int32>& InA, const TArray<int32>& InB, int32& OutAIndex, int32& OutBIndex);

	FORCEINLINE static double SafeDivide(const double InA, const double InB, const double InFallback = 0.0)
	{
		if (FMath::IsNearlyZero(InB))
		{
			return InFallback;
		}
		return InA / InB;
	}
};
