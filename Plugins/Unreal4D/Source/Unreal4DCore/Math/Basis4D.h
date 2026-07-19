#pragma once

#include "../Unreal4DDefines.h"

#include "Dom/JsonValue.h"

#include "Basis4D.generated.h"

// Stateless static class for 4D basis matrix functions.
// Most code in Unreal's FMatrix is designed for 3D use cases,
// this class explicitly comments when FMatrix's built-in stuff can be used.
//
// List of functions in FMatrix that work correctly for 4D basis math:
// - ComputeHash: Use as-is.
// - ContainsNaN: Use as-is.
// - Determinant: Use as-is.
// - Equals: Use with the tolerance set to UNREAL4D_COMPARISON_EPSILON for best results.
// - GetTransposed: Use as-is.
// - Identity and SetIdentity: Use as-is to get the identity basis.
// - Inverse4x4: Works but prefer Basis4D_Inverse. Do not use the 3D Inverse and InverseFast.
// - Operators: Beware of the multiplication order, but otherwise these work fine.
// - Serialize and SerializeFromMismatchedTag: Use as-is.
// - ToString, DebugPrint, ErrorEnsure: Use as-is.
// - TransformFVector4: Use as-is.
//
// List of functions in FMatrix that expect 3D and cannot be used for 4D math:
// - Constructor with vectors: Ambiguous with TPlane and TVector, so use Basis4D_FromVectors instead.
// - GetColumn, SetColumn: Use the helper functions in this class instead.
// - GetMinimumAxisScale, GetMaximumAxisScale: Only for 3D, no replacement provided.
// - GetUnitAxis, GetUnitAxes, SetAxis, SetAxes: Only for 3D, no replacement provided.
// - Inverse and InverseFast: Prefer Basis4D_Inverse, or Inverse4x4 works too (not a member function).
// - InverseTransformPosition and InverseTransformVector: Use Basis4D_InverseTransform instead. Use FTransform4D if translation is needed.
// - MakeFrustumPlane and GetFrustum(Near/Far/Left/Right/Top/Bottom)Plane: Only for 3D, no replacement provided.
// - Mirror: Only for 3D, no replacement provided.
// - RemoveScaling, GetMatrixWithoutScale, ExtractScaling, GetScaleVector, GetScaledAxis, GetScaledAxes, ApplyScale: Use the Basis4D_Scale* functions instead.
// - RemoveTranslation, ConcatTranslation, ScaleTranslation, GetOrigin, SetOrigin: Not applicable as we are using the entire FMatrix only as a basis, use FTransform4D if translation is needed.
// - RotDeterminant: Use Determinant instead.
// - TBasisVectorMatrix, TLookFromMatrix, TLookAtMatrix: Use the Basis4D_LookingAt function if only rotation is needed, but Unreal's functions also include the translation, and if that is needed, use FTransform4D instead.
// - To3x4MatrixTranspose: Only for 3D, no replacement provided.
// - ToQuat: Only for 3D, no replacement provided.
// - TransformBy, TransformByUsingAdjoint: Only for 3D, no replacement provided.
// - TransformPosition and TransformVector: Use FMatrix::TransformFVector4 instead.
// - TransposeAdjoint: Only for 3D, no replacement provided.
UCLASS(meta = (ScriptMixin = "FMatrix"))
class UNREAL4DCORE_API UBasis4D : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// Misc functions.
	// For the determinant, use FMatrix::Determinant. Do not use FMatrix::RotDeterminant.
	// For approximate equality, use FMatrix::Equals with UNREAL4D_COMPARISON_EPSILON as a replacement for IsEqualApprox.
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Basis", meta = (ScriptName = "Lerp"))
	static FMatrix Basis4D_Lerp(const FMatrix& InFrom, const FMatrix& InTo, const double InWeight);

	// Transformation functions.
	// Note that Unreal's FMatrix multiplication order is child * parent,
	// the Basis4D_Compose helper can be used to make this clearer.
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Basis", meta = (ScriptName = "Compose"))
	static FORCEINLINE FMatrix Basis4D_Compose(const FMatrix& InParent, const FMatrix& InChild)
	{
		// NOTE: Unreal's FMatrix uses child * parent order for multiplication.
		return InChild * InParent;
	}
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Basis", meta = (ScriptName = "TransformTo"))
	static FMatrix Basis4D_TransformTo(const FMatrix& InFrom, const FMatrix& InTo);

	// For transforming a Vector4 in 4D space, use FMatrix::TransformFVector4.
	// For inversely transforming a Vector4 in 4D space, you need these helpers.
	// TransposeTransform is faster if you know the basis is orthonormal,
	// but InverseTransform works with scaled and sheared basis matrices.
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Basis", meta = (ScriptName = "InverseTransform"))
	static FVector4 Basis4D_InverseTransform(const FMatrix& InBasis, const FVector4& InVector);
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Basis", meta = (ScriptName = "TransposeTransform"))
	static FVector4 Basis4D_TransposeTransform(const FMatrix& InBasis, const FVector4& InVector);
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Basis", meta = (ScriptName = "TransformMany"))
	static TArray<FVector4> Basis4D_TransformMany(const FMatrix& InBasis, const TArray<FVector4>& InVectors);

	// Rotation methods.
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Basis", meta = (ScriptName = "RotatePlaneGlobal"))
	static FMatrix Basis4D_RotatePlaneGlobal(const FMatrix& InBasis, const FVector4& InPlaneFrom, const FVector4& InPlaneTo, const double InAngle);
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Basis", meta = (ScriptName = "RotatePlaneLocal"))
	static FMatrix Basis4D_RotatePlaneLocal(const FMatrix& InBasis, const FVector4& InLocalPlaneFrom, const FVector4& InLocalPlaneTo, const double InAngle);

	// Inversion methods.
	// For the transpose, use FMatrix::GetTransposed.
	// For the inverse, prefer Basis4D_Inverse, but Inverse4x4 also works. Do not use FMatrix::Inverse or FMatrix::InverseFast.
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Basis", meta = (ScriptName = "Inverse"))
	static FMatrix Basis4D_Inverse(const FMatrix& InBasis);
	// We can skip exposing this one because it's very low-level.
	static FMatrix Basis4D_Adjugate(const FMatrix& InBasis);

	// Scale methods.
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Basis", meta = (ScriptName = "ScaledGlobal"))
	static FMatrix Basis4D_ScaledGlobal(const FMatrix& InBasis, const FVector4& InScale);
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Basis", meta = (ScriptName = "ScaledLocal"))
	static FMatrix Basis4D_ScaledLocal(const FMatrix& InBasis, const FVector4& InScale);
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Basis", meta = (ScriptName = "ScaledUniform"))
	static FMatrix Basis4D_ScaledUniform(const FMatrix& InBasis, const double InScale);
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Basis", meta = (ScriptName = "GetGlobalScaleAbs"))
	static FVector4 Basis4D_GetGlobalScaleAbs(const FMatrix& InBasis);
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Basis", meta = (ScriptName = "GetScale"))
	static FVector4 Basis4D_GetScale(const FMatrix& InBasis);
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Basis", meta = (ScriptName = "GetScaleAbs"))
	static FVector4 Basis4D_GetScaleAbs(const FMatrix& InBasis);
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Basis", meta = (ScriptName = "WithScale"))
	static FMatrix Basis4D_WithScale(const FMatrix& InBasis, const FVector4& InScale);
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Basis", meta = (ScriptName = "GetUniformScale"))
	static double Basis4D_GetUniformScale(const FMatrix& InBasis);
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Basis", meta = (ScriptName = "WithUniformScale"))
	static FMatrix Basis4D_WithUniformScale(const FMatrix& InBasis, const double InUniformScale);

	// Validation methods.
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Basis", meta = (ScriptName = "Conformalized"))
	static FMatrix Basis4D_Conformalized(const FMatrix& InBasis);
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Basis", meta = (ScriptName = "Normalized"))
	static FMatrix Basis4D_Normalized(const FMatrix& InBasis);
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Basis", meta = (ScriptName = "Orthonormalized"))
	static FMatrix Basis4D_Orthonormalized(const FMatrix& InBasis);
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Basis", meta = (ScriptName = "Orthogonalized"))
	static FMatrix Basis4D_Orthogonalized(const FMatrix& InBasis);
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Basis", meta = (ScriptName = "IsConformal"))
	static bool Basis4D_IsConformal(const FMatrix& InBasis);
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Basis", meta = (ScriptName = "IsDiagonal"))
	static bool Basis4D_IsDiagonal(const FMatrix& InBasis);
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Basis", meta = (ScriptName = "IsOrthogonal"))
	static bool Basis4D_IsOrthogonal(const FMatrix& InBasis);
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Basis", meta = (ScriptName = "IsOrthonormal"))
	static bool Basis4D_IsOrthonormal(const FMatrix& InBasis);
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Basis", meta = (ScriptName = "IsRotation"))
	static bool Basis4D_IsRotation(const FMatrix& InBasis);

	// Helper setters/getters.
	// NOTE: The FMatrix versions of these are only for 3D matrices, returning 3D FVector.
	// NOTE: These names follow Unreal's conventions, where matrix rows are the basis vectors.
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Basis", meta = (ScriptName = "GetColumn"))
	static FVector4 Basis4D_GetColumn(const FMatrix& InBasis, const int32 InIndex);
	static void Basis4D_SetColumn(FMatrix& InBasis, const int32 InIndex, const FVector4& InValue);
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Basis", meta = (ScriptName = "GetRow"))
	static FVector4 Basis4D_GetRow(const FMatrix& InBasis, const int32 InIndex);
	static void Basis4D_SetRow(FMatrix& InBasis, const int32 InIndex, const FVector4& InRow);

	// Conversion.
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Basis", meta = (ScriptName = "From3D"))
	static FMatrix Basis4D_From3D(const FMatrix& InFrom3D);
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Basis", meta = (ScriptName = "To3D"))
	static FMatrix Basis4D_To3D(const FMatrix& InBasis, const bool bInOrthonormalized = false);
	static FMatrix Basis4D_FromArray(const TArray<double>& InFromArray);
	static TArray<double> Basis4D_ToArray(const FMatrix& InBasis);
	// Note: These cannot be bound using UFUNCTION because Blueprint does not support `TArray<TSharedPtr<T>>`.
	static FMatrix Basis4D_FromJsonArray(const TArray<TSharedPtr<FJsonValue>>& InJsonArray);
	static TArray<TSharedPtr<FJsonValue>> Basis4D_ToJsonArray(const FMatrix& InBasis);

	// Constructor-like factory functions.
	// To construct the identity basis, use FMatrix::Identity.
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Basis", meta = (ScriptName = "FromScale"))
	static FMatrix Basis4D_FromScale(const FVector4& InScale);
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Basis", meta = (ScriptName = "FromScaleUniform"))
	static FMatrix Basis4D_FromScaleUniform(const double InScale);
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Basis", meta = (ScriptName = "FromYZ"))
	static FMatrix Basis4D_FromYZ(const double InYZ);
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Basis", meta = (ScriptName = "FromZX"))
	static FMatrix Basis4D_FromZX(const double InZX);
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Basis", meta = (ScriptName = "FromXY"))
	static FMatrix Basis4D_FromXY(const double InXY);
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Basis", meta = (ScriptName = "FromXW"))
	static FMatrix Basis4D_FromXW(const double InXW);
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Basis", meta = (ScriptName = "FromWY"))
	static FMatrix Basis4D_FromWY(const double InWY);
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Basis", meta = (ScriptName = "FromZW"))
	static FMatrix Basis4D_FromZW(const double InZW);
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Basis", meta = (ScriptName = "FromSwapRotation"))
	static FMatrix Basis4D_FromSwapRotation(const int32 InRotFrom, const int32 InRotTo);
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Basis", meta = (ScriptName = "LookingAt", CPP_Default_InUp = "(X=0.0,Y=0.0,Z=1.0,W=0.0)"))
	static FMatrix Basis4D_LookingAt(const FVector4& InTarget, const FVector4& InPerp, const FVector4& InUp = FVector4(0.0, 0.0, 1.0, 0.0));

	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Basis", meta = (ScriptName = "FromVectors"))
	static FORCEINLINE FMatrix Basis4D_FromVectors(
		const FVector4& InX,
		const FVector4& InY,
		const FVector4& InZ,
		const FVector4& InW)
	{
		return FMatrix(UE::Math::TPlane<double>(InX), UE::Math::TPlane<double>(InY), UE::Math::TPlane<double>(InZ), UE::Math::TPlane<double>(InW));
	}

	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Math|Basis", meta = (ScriptName = "FromNumbers"))
	static FORCEINLINE FMatrix Basis4D_FromNumbers(
		const double InXX, const double InXY, const double InXZ, const double InXW,
		const double InYX, const double InYY, const double InYZ, const double InYW,
		const double InZX, const double InZY, const double InZZ, const double InZW,
		const double InWX, const double InWY, const double InWZ, const double InWW)
	{
		return FMatrix(
			UE::Math::TPlane<double>(InXX, InXY, InXZ, InXW),
			UE::Math::TPlane<double>(InYX, InYY, InYZ, InYW),
			UE::Math::TPlane<double>(InZX, InZY, InZZ, InZW),
			UE::Math::TPlane<double>(InWX, InWY, InWZ, InWW)
		);
	}
};
