#include "Transform4DBind.h"

// Misc methods.

double UTransform4DLibrary::Transform4D_Determinant(const FTransform4D& InTransform)
{
	return InTransform.Determinant();
}

FTransform4D UTransform4DLibrary::Transform4D_Lerp(const FTransform4D& InFrom, const FTransform4D& InTo, const double InWeight)
{
	return InFrom.Lerp(InTo, InWeight);
}

bool UTransform4DLibrary::Transform4D_IsEqualApprox(const FTransform4D& InTransform, const FTransform4D& InOther)
{
	return InTransform.IsEqualApprox(InOther);
}

FTransform4D UTransform4DLibrary::Transform4D_TranslatedLocal(const FTransform4D& InTransform, const FVector4& InTranslation)
{
	return InTransform.TranslatedLocal(InTranslation);
}

FTransform4D UTransform4DLibrary::Transform4D_Identity()
{
	return FTransform4D::Identity();
}

// Transformation methods.

FTransform4D UTransform4DLibrary::Transform4D_Compose(const FTransform4D& InParent, const FTransform4D& InChild)
{
	return InParent.Compose(InChild);
}

FTransform4D UTransform4DLibrary::Transform4D_TransformTo(const FTransform4D& InFrom, const FTransform4D& InTo)
{
	return InFrom.TransformTo(InTo);
}

FVector4 UTransform4DLibrary::Transform4D_TransformPosition(const FTransform4D& InTransform, const FVector4& InVector)
{
	return InTransform.TransformPosition(InVector);
}

TArray<FVector4> UTransform4DLibrary::Transform4D_TransformPositionMany(const FTransform4D& InTransform, const TArray<FVector4>& InVectors)
{
	return InTransform.TransformPositionMany(InVectors);
}

FVector4 UTransform4DLibrary::Transform4D_InverseTransformPosition(const FTransform4D& InTransform, const FVector4& InVector)
{
	return InTransform.InverseTransformPosition(InVector);
}

TArray<FVector4> UTransform4DLibrary::Transform4D_InverseTransformPositionMany(const FTransform4D& InTransform, const TArray<FVector4>& InVectors)
{
	return InTransform.InverseTransformPositionMany(InVectors);
}

FVector4 UTransform4DLibrary::Transform4D_TransposeTransformPosition(const FTransform4D& InTransform, const FVector4& InVector)
{
	return InTransform.TransposeTransformPosition(InVector);
}

TArray<FVector4> UTransform4DLibrary::Transform4D_TransposeTransformPositionMany(const FTransform4D& InTransform, const TArray<FVector4>& InVectors)
{
	return InTransform.TransposeTransformPositionMany(InVectors);
}

// Inversion methods.

FTransform4D UTransform4DLibrary::Transform4D_Inverse(const FTransform4D& InTransform)
{
	return InTransform.Inverse();
}

FTransform4D UTransform4DLibrary::Transform4D_InverseBasis(const FTransform4D& InTransform)
{
	return InTransform.InverseBasis();
}

FTransform4D UTransform4DLibrary::Transform4D_InverseTransposed(const FTransform4D& InTransform)
{
	return InTransform.InverseTransposed();
}

// Rotation methods.

FTransform4D UTransform4DLibrary::Transform4D_RotatedGlobal(const FTransform4D& InTransform, const FEuler4D& InEuler)
{
	return InTransform.RotatedGlobal(InEuler);
}

FTransform4D UTransform4DLibrary::Transform4D_RotatedLocal(const FTransform4D& InTransform, const FEuler4D& InEuler)
{
	return InTransform.RotatedLocal(InEuler);
}

FEuler4D UTransform4DLibrary::Transform4D_GetRotation(const FTransform4D& InTransform)
{
	return InTransform.GetRotation();
}

FTransform4D UTransform4DLibrary::Transform4D_SetRotation(const FTransform4D& InTransform, const FEuler4D& InEuler)
{
	FTransform4D Result = InTransform;
	Result.SetRotation(InEuler);
	return Result;
}

FEuler4D UTransform4DLibrary::Transform4D_GetRotationDegrees(const FTransform4D& InTransform)
{
	return InTransform.GetRotationDegrees();
}

FTransform4D UTransform4DLibrary::Transform4D_SetRotationDegrees(const FTransform4D& InTransform, const FEuler4D& InEuler)
{
	FTransform4D Result = InTransform;
	Result.SetRotationDegrees(InEuler);
	return Result;
}

// Scale methods.

FTransform4D UTransform4DLibrary::Transform4D_ScaledGlobal(const FTransform4D& InTransform, const FVector4& InScale)
{
	return InTransform.ScaledGlobal(InScale);
}

FTransform4D UTransform4DLibrary::Transform4D_ScaledLocal(const FTransform4D& InTransform, const FVector4& InScale)
{
	return InTransform.ScaledLocal(InScale);
}

FTransform4D UTransform4DLibrary::Transform4D_ScaledUniformGlobal(const FTransform4D& InTransform, double InScale)
{
	return InTransform.ScaledUniformGlobal(InScale);
}

FTransform4D UTransform4DLibrary::Transform4D_ScaledUniformLocal(const FTransform4D& InTransform, double InScale)
{
	return InTransform.ScaledUniformLocal(InScale);
}

FVector4 UTransform4DLibrary::Transform4D_GetScale(const FTransform4D& InTransform)
{
	return InTransform.GetScale();
}

FVector4 UTransform4DLibrary::Transform4D_GetScaleAbs(const FTransform4D& InTransform)
{
	return InTransform.GetScaleAbs();
}

FTransform4D UTransform4DLibrary::Transform4D_SetScale(const FTransform4D& InTransform, const FVector4& InScale)
{
	FTransform4D Result = InTransform;
	Result.SetScale(InScale);
	return Result;
}

double UTransform4DLibrary::Transform4D_GetUniformScale(const FTransform4D& InTransform)
{
	return InTransform.GetUniformScale();
}

FTransform4D UTransform4DLibrary::Transform4D_SetUniformScale(const FTransform4D& InTransform, double InUniformScale)
{
	FTransform4D Result = InTransform;
	Result.SetUniformScale(InUniformScale);
	return Result;
}

// Validation methods.

FTransform4D UTransform4DLibrary::Transform4D_Conformalized(const FTransform4D& InTransform)
{
	return InTransform.Conformalized();
}

FTransform4D UTransform4DLibrary::Transform4D_Normalized(const FTransform4D& InTransform)
{
	return InTransform.Normalized();
}

FTransform4D UTransform4DLibrary::Transform4D_Orthonormalized(const FTransform4D& InTransform)
{
	return InTransform.Orthonormalized();
}

FTransform4D UTransform4DLibrary::Transform4D_Orthogonalized(const FTransform4D& InTransform)
{
	return InTransform.Orthogonalized();
}

// Helper setters/getters.

FMatrix UTransform4DLibrary::Transform4D_GetBasis(const FTransform4D& InTransform)
{
	return InTransform.GetBasis();
}

FTransform4D UTransform4DLibrary::Transform4D_SetBasis(const FTransform4D& InTransform, const FMatrix& InBasis)
{
	FTransform4D Result = InTransform;
	Result.SetBasis(InBasis);
	return Result;
}

FVector4 UTransform4DLibrary::Transform4D_GetOrigin(const FTransform4D& InTransform)
{
	return InTransform.GetOrigin();
}

FTransform4D UTransform4DLibrary::Transform4D_SetOrigin(const FTransform4D& InTransform, const FVector4& InOrigin)
{
	FTransform4D Result = InTransform;
	Result.SetOrigin(InOrigin);
	return Result;
}

// Conversion methods.

FTransform4D UTransform4DLibrary::Transform4D_From3D(const FTransform& InTransform)
{
	return FTransform4D::From3D(InTransform);
}

FTransform UTransform4DLibrary::Transform4D_To3D(const FTransform4D& InTransform, bool bOrthonormalized)
{
	return InTransform.To3D(bOrthonormalized);
}

FTransform4D UTransform4DLibrary::Transform4D_FromArray(const TArray<double>& InArray)
{
	return FTransform4D::FromArray(InArray);
}

TArray<double> UTransform4DLibrary::Transform4D_ToArray(const FTransform4D& InTransform)
{
	return InTransform.ToArray();
}

FString UTransform4DLibrary::Transform4D_ToString(const FTransform4D& InTransform)
{
	return InTransform.ToString();
}
