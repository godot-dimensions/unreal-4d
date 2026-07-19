#pragma once

#include "../Euler4D.h"

#include "Euler4DBind.generated.h"

UCLASS(meta = (ScriptMixin = "FEuler4D"))
class UNREAL4DCORE_API UEuler4DLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// Misc methods.
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Euler", meta = (ScriptName = "Compose"))
	static FEuler4D Euler4D_Compose(const FEuler4D& InParent, const FEuler4D& InChild);

	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Euler", meta = (ScriptName = "IsEqualApprox"))
	static bool Euler4D_IsEqualApprox(const FEuler4D& InEuler, const FEuler4D& InOther);

	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Euler", meta = (ScriptName = "Lerp"))
	static FEuler4D Euler4D_Lerp(const FEuler4D& InFrom, const FEuler4D& InTo, const double InWeight);

	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Euler", meta = (ScriptName = "RotateBasis"))
	static FMatrix Euler4D_RotateBasis(const FEuler4D& InEuler, const FMatrix& InBasis);

	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Euler", meta = (ScriptName = "RotatePoint"))
	static FVector4 Euler4D_RotatePoint(const FEuler4D& InEuler, const FVector4& InPoint);

	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Euler", meta = (ScriptName = "RotationTo"))
	static FEuler4D Euler4D_RotationTo(const FEuler4D& InFrom, const FEuler4D& InTo);

	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Euler", meta = (ScriptName = "Snapped"))
	static FEuler4D Euler4D_Snapped(const FEuler4D& InEuler, double InStep);

	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Euler", meta = (ScriptName = "Wrapped"))
	static FEuler4D Euler4D_Wrapped(const FEuler4D& InEuler);

	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Euler", meta = (ScriptName = "Zero"))
	static FEuler4D Euler4D_Zero();

	// Radians/degrees.
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Euler", meta = (ScriptName = "DegToRad"))
	static FEuler4D Euler4D_DegToRad(const FEuler4D& InEuler);

	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Euler", meta = (ScriptName = "RadToDeg"))
	static FEuler4D Euler4D_RadToDeg(const FEuler4D& InEuler);

	// Component with-style setters.
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Euler", meta = (ScriptName = "WithYZ"))
	static FEuler4D Euler4D_WithYZ(const FEuler4D& InEuler, double InYZ);

	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Euler", meta = (ScriptName = "WithZX"))
	static FEuler4D Euler4D_WithZX(const FEuler4D& InEuler, double InZX);

	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Euler", meta = (ScriptName = "WithXY"))
	static FEuler4D Euler4D_WithXY(const FEuler4D& InEuler, double InXY);

	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Euler", meta = (ScriptName = "WithXW"))
	static FEuler4D Euler4D_WithXW(const FEuler4D& InEuler, double InXW);

	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Euler", meta = (ScriptName = "WithWY"))
	static FEuler4D Euler4D_WithWY(const FEuler4D& InEuler, double InWY);

	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Euler", meta = (ScriptName = "WithZW"))
	static FEuler4D Euler4D_WithZW(const FEuler4D& InEuler, double InZW);

	// Conversion methods.
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Euler", meta = (ScriptName = "From3D"))
	static FEuler4D Euler4D_From3D(const FVector& InVector);

	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Euler", meta = (ScriptName = "To3D"))
	static FVector Euler4D_To3D(const FEuler4D& InEuler);

	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Euler", meta = (ScriptName = "FromArray"))
	static FEuler4D Euler4D_FromArray(const TArray<double>& InArray);

	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Euler", meta = (ScriptName = "ToArray"))
	static TArray<double> Euler4D_ToArray(const FEuler4D& InEuler);

	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Euler", meta = (ScriptName = "DegreesFromBasis"))
	static FEuler4D Euler4D_DegreesFromBasis(const FMatrix& InBasis, bool bUseSpecialCases = true);

	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Euler", meta = (ScriptName = "DegreesToBasis"))
	static FMatrix Euler4D_DegreesToBasis(const FEuler4D& InEuler);

	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Euler", meta = (ScriptName = "RadiansFromBasis"))
	static FEuler4D Euler4D_RadiansFromBasis(const FMatrix& InBasis, bool bUseSpecialCases = true);

	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Euler", meta = (ScriptName = "RadiansToBasis"))
	static FMatrix Euler4D_RadiansToBasis(const FEuler4D& InEuler);
};
