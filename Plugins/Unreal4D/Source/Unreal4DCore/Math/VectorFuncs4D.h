#pragma once

#include "../Unreal4DDefines.h"

#include "Dom/JsonValue.h"

#include "VectorFuncs4D.generated.h"

extern UNREAL4DCORE_API const FVector4 FVector4_Zero;
extern UNREAL4DCORE_API const FVector4 FVector4_One;
extern UNREAL4DCORE_API const FVector4 FVector4_Forward;
extern UNREAL4DCORE_API const FVector4 FVector4_Back;
extern UNREAL4DCORE_API const FVector4 FVector4_Right;
extern UNREAL4DCORE_API const FVector4 FVector4_Left;
extern UNREAL4DCORE_API const FVector4 FVector4_Up;
extern UNREAL4DCORE_API const FVector4 FVector4_Down;
extern UNREAL4DCORE_API const FVector4 FVector4_Ana;
extern UNREAL4DCORE_API const FVector4 FVector4_Kata;

// Axis enumeration for 4D vectors.
UENUM(BlueprintType)
enum class EAxis4D : uint8
{
	X UMETA(DisplayName = "X"),
	Y UMETA(DisplayName = "Y"),
	Z UMETA(DisplayName = "Z"),
	W UMETA(DisplayName = "W"),
};

// Stateless static class for 4D vector math functions.
UCLASS(meta = (ScriptMixin = "FVector4"))
class UNREAL4DCORE_API UVectorFuncs4D : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// Cosmetic functions.
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Vector", meta = (ScriptName = "AxisColor"))
	static FLinearColor Vector4_AxisColor(const EAxis4D InAxis);
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Vector", meta = (ScriptName = "AxisLetter"))
	static FString Vector4_AxisLetter(const EAxis4D InAxis);

	// Non-bindable helpers.
	static void SortAxesMaxToMin(const FVector4& InVector, int32 (&InOutAxes)[4]);

	// Vector math.
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Vector", meta = (ScriptName = "Abs"))
	static FVector4 Vector4_Abs(const FVector4& InVector);
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Vector", meta = (ScriptName = "AngleTo"))
	static double Vector4_AngleTo(const FVector4& InFrom, const FVector4& InTo);
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Vector", meta = (ScriptName = "Average"))
	static FVector4 Vector4_Average(const TArray<FVector4>& InVectors);
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Vector", meta = (ScriptName = "Bounce"))
	static FVector4 Vector4_Bounce(const FVector4& InVector, const FVector4& InNormal);
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Vector", meta = (ScriptName = "BounceRatio"))
	static FVector4 Vector4_BounceRatio(const FVector4& InVector, const FVector4& InNormal, const double InBounceRatio);
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Vector", meta = (ScriptName = "Cross"))
	static double Vector4_Cross(const FVector4& InA, const FVector4& InB);
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Vector", meta = (ScriptName = "Distance"))
	static double Vector4_Distance(const FVector4& InA, const FVector4& InB);
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Vector", meta = (ScriptName = "DistanceSquared"))
	static double Vector4_DistanceSquared(const FVector4& InA, const FVector4& InB);
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Vector", meta = (ScriptName = "Dot"))
	static double Vector4_Dot(const FVector4& InA, const FVector4& InB);
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Vector", meta = (ScriptName = "IsUniform"))
	static bool Vector4_IsUniform(const FVector4& InVector);
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Vector", meta = (ScriptName = "Lerp"))
	static FVector4 Vector4_Lerp(const FVector4& InFrom, const FVector4& InTo, const double InWeight);
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Vector", meta = (ScriptName = "LimitLength"))
	static FVector4 Vector4_LimitLength(const FVector4& InVector, const double InLen = 1.0);
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Vector", meta = (ScriptName = "LimitLengthTaxicab"))
	static FVector4 Vector4_LimitLengthTaxicab(const FVector4& InVector, const double InLen = 1.0);
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Vector", meta = (ScriptName = "Max"))
	static FVector4 Vector4_Max(const FVector4& InA, const FVector4& InB);
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Vector", meta = (ScriptName = "MaxAxisIndex"))
	static EAxis4D Vector4_MaxAxisIndex(const FVector4& InVector);
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Vector", meta = (ScriptName = "MoveToward"))
	static FVector4 Vector4_MoveToward(const FVector4& InFrom, const FVector4& InTo, const double InDelta);
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Vector", meta = (ScriptName = "Normalized"))
	static FVector4 Vector4_Normalized(const FVector4& InVector);
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Vector", meta = (ScriptName = "OrthogonalFromTwo"))
	static FVector4 Vector4_OrthogonalFromTwo(const FVector4& InWhat, const FVector4& InA, const FVector4& InB);
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Vector", meta = (ScriptName = "Perpendicular"))
	static FVector4 Vector4_Perpendicular(const FVector4& InA, const FVector4& InB, const FVector4& InC);
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Vector", meta = (ScriptName = "Project"))
	static FVector4 Vector4_Project(const FVector4& InVector, const FVector4& InOnNormal);
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Vector", meta = (ScriptName = "Reflect"))
	static FVector4 Vector4_Reflect(const FVector4& InVector, const FVector4& InNormal);
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Vector", meta = (ScriptName = "RotateInPlane"))
	static FVector4 Vector4_RotateInPlane(const FVector4& InVector, const FVector4& InPlaneFrom, const FVector4& InPlaneTo, const double InAngle);
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Vector", meta = (ScriptName = "SafeReciprocal"))
	static FVector4 Vector4_SafeReciprocal(const FVector4& InVector);
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Vector", meta = (ScriptName = "Sign"))
	static FVector4 Vector4_Sign(const FVector4& InVector);
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Vector", meta = (ScriptName = "Slide"))
	static FVector4 Vector4_Slide(const FVector4& InVector, const FVector4& InNormal);
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Vector", meta = (ScriptName = "Snap"))
	static FVector4 Vector4_Snap(const FVector4& InVector, const double InSnapInterval);

	// Generation.
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Vector", meta = (ScriptName = "RandomInRadius"))
	static FVector4 Vector4_RandomInRadius(const double InRadius = 1.0);
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Vector", meta = (ScriptName = "RandomInRange", AutoCreateRefTerm = "InFrom,InTo"))
	static FVector4 Vector4_RandomInRange(const FVector4& InFrom, const FVector4& InTo);

	// Conversion.
	UFUNCTION(Category = "Unreal 4D|Math|Vector", BlueprintCallable, BlueprintPure = true, meta = (ScriptName = "FromColor"))
	static FVector4 Vector4_FromColor(const FLinearColor& InColor) { return FVector4(InColor.R, InColor.G, InColor.B, InColor.A); }
	UFUNCTION(Category = "Unreal 4D|Math|Vector", BlueprintCallable, BlueprintPure = true, meta = (ScriptName = "ToColor"))
	static FLinearColor Vector4_ToColor(const FVector4& InVector) { return FLinearColor(InVector.X, InVector.Y, InVector.Z, InVector.W); }
	UFUNCTION(Category = "Unreal 4D|Math|Vector", BlueprintCallable, BlueprintPure = true, meta = (ScriptName = "From3D"))
	static FVector4 Vector4_From3D(const FVector& InVector, const double InW = 0.0) { return FVector4(InVector.X, InVector.Y, InVector.Z, InW); }
	UFUNCTION(Category = "Unreal 4D|Math|Vector", BlueprintCallable, BlueprintPure = true, meta = (ScriptName = "To3D"))
	static FVector Vector4_To3D(const FVector4& InVector) { return FVector(InVector.X, InVector.Y, InVector.Z); }
	UFUNCTION(Category = "Unreal 4D|Math|Vector", BlueprintCallable, BlueprintPure = true, meta = (ScriptName = "FromArray"))
	static FVector4 Vector4_FromArray(const TArray<double>& InArray);
	UFUNCTION(Category = "Unreal 4D|Math|Vector", BlueprintCallable, BlueprintPure = false, meta = (ScriptName = "ToArray"))
	static TArray<double> Vector4_ToArray(const FVector4& InVector);
	// Note: These cannot be bound using UFUNCTION because Blueprint does not support `TArray<TSharedPtr<T>>`.
	static FVector4 Vector4_FromJsonArray(const TArray<TSharedPtr<FJsonValue>>& InJsonArray);
	static TArray<TSharedPtr<FJsonValue>> Vector4_ToJsonArray(const FVector4& InVector);
};

// Inline conversion helpers for internal C++ use.
inline FVector4 FromColor(const FLinearColor& InColor)
{
	return FVector4(InColor.R, InColor.G, InColor.B, InColor.A);
}

inline FVector4 From3D(const FVector& InVector, const double InW = 0.0)
{
	return FVector4(InVector.X, InVector.Y, InVector.Z, InW);
}
