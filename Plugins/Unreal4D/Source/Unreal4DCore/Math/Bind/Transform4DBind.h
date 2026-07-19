#pragma once

#include "../Transform4D.h"

#include "Transform4DBind.generated.h"

UCLASS(meta = (ScriptMixin = "FTransform4D"))
class UNREAL4DCORE_API UTransform4DLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// Misc methods.
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Transform", meta = (ScriptName = "Determinant"))
	static double Transform4D_Determinant(const FTransform4D& InTransform);

	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Transform", meta = (ScriptName = "Lerp"))
	static FTransform4D Transform4D_Lerp(const FTransform4D& InFrom, const FTransform4D& InTo, const double InWeight);

	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Transform", meta = (ScriptName = "IsEqualApprox"))
	static bool Transform4D_IsEqualApprox(const FTransform4D& InTransform, const FTransform4D& InOther);

	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Transform", meta = (ScriptName = "TranslatedLocal"))
	static FTransform4D Transform4D_TranslatedLocal(const FTransform4D& InTransform, const FVector4& InTranslation);

	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Transform", meta = (ScriptName = "Identity"))
	static FTransform4D Transform4D_Identity();

	// Transformation methods.
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Transform", meta = (ScriptName = "Compose"))
	static FTransform4D Transform4D_Compose(const FTransform4D& InParent, const FTransform4D& InChild);

	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Transform", meta = (ScriptName = "TransformTo"))
	static FTransform4D Transform4D_TransformTo(const FTransform4D& InFrom, const FTransform4D& InTo);

	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Transform", meta = (ScriptName = "TransformPosition"))
	static FVector4 Transform4D_TransformPosition(const FTransform4D& InTransform, const FVector4& InVector);

	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Transform", meta = (ScriptName = "TransformPositionMany"))
	static TArray<FVector4> Transform4D_TransformPositionMany(const FTransform4D& InTransform, const TArray<FVector4>& InVectors);

	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Transform", meta = (ScriptName = "TransformPositionInv"))
	static FVector4 Transform4D_InverseTransformPosition(const FTransform4D& InTransform, const FVector4& InVector);

	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Transform", meta = (ScriptName = "TransformPositionInvMany"))
	static TArray<FVector4> Transform4D_InverseTransformPositionMany(const FTransform4D& InTransform, const TArray<FVector4>& InVectors);

	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Transform", meta = (ScriptName = "TransformPositionTransposed"))
	static FVector4 Transform4D_TransposeTransformPosition(const FTransform4D& InTransform, const FVector4& InVector);

	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Transform", meta = (ScriptName = "TransformPositionTransposedMany"))
	static TArray<FVector4> Transform4D_TransposeTransformPositionMany(const FTransform4D& InTransform, const TArray<FVector4>& InVectors);

	// Inversion methods.
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Transform", meta = (ScriptName = "Inverse"))
	static FTransform4D Transform4D_Inverse(const FTransform4D& InTransform);

	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Transform", meta = (ScriptName = "InverseBasis"))
	static FTransform4D Transform4D_InverseBasis(const FTransform4D& InTransform);

	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Transform", meta = (ScriptName = "InverseTransposed"))
	static FTransform4D Transform4D_InverseTransposed(const FTransform4D& InTransform);

	// Rotation methods.
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Transform", meta = (ScriptName = "RotatedGlobal"))
	static FTransform4D Transform4D_RotatedGlobal(const FTransform4D& InTransform, const FEuler4D& InEuler);

	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Transform", meta = (ScriptName = "RotatedLocal"))
	static FTransform4D Transform4D_RotatedLocal(const FTransform4D& InTransform, const FEuler4D& InEuler);

	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Transform", meta = (ScriptName = "GetRotation"))
	static FEuler4D Transform4D_GetRotation(const FTransform4D& InTransform);

	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Transform", meta = (ScriptName = "SetRotation"))
	static FTransform4D Transform4D_SetRotation(const FTransform4D& InTransform, const FEuler4D& InEuler);

	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Transform", meta = (ScriptName = "GetRotationDegrees"))
	static FEuler4D Transform4D_GetRotationDegrees(const FTransform4D& InTransform);

	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Transform", meta = (ScriptName = "SetRotationDegrees"))
	static FTransform4D Transform4D_SetRotationDegrees(const FTransform4D& InTransform, const FEuler4D& InEuler);

	// Scale methods.
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Transform", meta = (ScriptName = "ScaledGlobal"))
	static FTransform4D Transform4D_ScaledGlobal(const FTransform4D& InTransform, const FVector4& InScale);

	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Transform", meta = (ScriptName = "ScaledLocal"))
	static FTransform4D Transform4D_ScaledLocal(const FTransform4D& InTransform, const FVector4& InScale);

	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Transform", meta = (ScriptName = "ScaledUniformGlobal"))
	static FTransform4D Transform4D_ScaledUniformGlobal(const FTransform4D& InTransform, double InScale);

	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Transform", meta = (ScriptName = "ScaledUniformLocal"))
	static FTransform4D Transform4D_ScaledUniformLocal(const FTransform4D& InTransform, double InScale);

	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Transform", meta = (ScriptName = "GetScale"))
	static FVector4 Transform4D_GetScale(const FTransform4D& InTransform);

	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Transform", meta = (ScriptName = "GetScaleAbs"))
	static FVector4 Transform4D_GetScaleAbs(const FTransform4D& InTransform);

	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Transform", meta = (ScriptName = "SetScale"))
	static FTransform4D Transform4D_SetScale(const FTransform4D& InTransform, const FVector4& InScale);

	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Transform", meta = (ScriptName = "GetUniformScale"))
	static double Transform4D_GetUniformScale(const FTransform4D& InTransform);

	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Transform", meta = (ScriptName = "SetUniformScale"))
	static FTransform4D Transform4D_SetUniformScale(const FTransform4D& InTransform, double InUniformScale);

	// Validation methods.
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Transform", meta = (ScriptName = "Conformalized"))
	static FTransform4D Transform4D_Conformalized(const FTransform4D& InTransform);

	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Transform", meta = (ScriptName = "Normalized"))
	static FTransform4D Transform4D_Normalized(const FTransform4D& InTransform);

	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Transform", meta = (ScriptName = "Orthonormalized"))
	static FTransform4D Transform4D_Orthonormalized(const FTransform4D& InTransform);

	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Transform", meta = (ScriptName = "Orthogonalized"))
	static FTransform4D Transform4D_Orthogonalized(const FTransform4D& InTransform);

	// Helper setters/getters.
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Transform", meta = (ScriptName = "GetBasis"))
	static FMatrix Transform4D_GetBasis(const FTransform4D& InTransform);

	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Transform", meta = (ScriptName = "SetBasis"))
	static FTransform4D Transform4D_SetBasis(const FTransform4D& InTransform, const FMatrix& InBasis);

	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Transform", meta = (ScriptName = "GetOrigin"))
	static FVector4 Transform4D_GetOrigin(const FTransform4D& InTransform);

	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Transform", meta = (ScriptName = "SetOrigin"))
	static FTransform4D Transform4D_SetOrigin(const FTransform4D& InTransform, const FVector4& InOrigin);

	// Conversion methods.
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Transform", meta = (ScriptName = "From3D"))
	static FTransform4D Transform4D_From3D(const FTransform& InTransform);

	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Transform", meta = (ScriptName = "To3D"))
	static FTransform Transform4D_To3D(const FTransform4D& InTransform, bool bOrthonormalized = false);

	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Transform", meta = (ScriptName = "FromArray"))
	static FTransform4D Transform4D_FromArray(const TArray<double>& InArray);

	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Transform", meta = (ScriptName = "ToArray"))
	static TArray<double> Transform4D_ToArray(const FTransform4D& InTransform);

	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Transform", meta = (ScriptName = "ToString"))
	static FString Transform4D_ToString(const FTransform4D& InTransform);
};
