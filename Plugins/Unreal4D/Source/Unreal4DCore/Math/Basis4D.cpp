#include "Basis4D.h"

#include "VectorFuncs4D.h"

// Interpolation.

FMatrix UBasis4D::Basis4D_Lerp(const FMatrix& InFrom, const FMatrix& InTo, const double InWeight)
{
	return Basis4D_FromVectors(
		FMath::Lerp(
			FVector4(InFrom.M[0][0], InFrom.M[0][1], InFrom.M[0][2], InFrom.M[0][3]),
			FVector4(InTo.M[0][0], InTo.M[0][1], InTo.M[0][2], InTo.M[0][3]),
			InWeight),
		FMath::Lerp(
			FVector4(InFrom.M[1][0], InFrom.M[1][1], InFrom.M[1][2], InFrom.M[1][3]),
			FVector4(InTo.M[1][0], InTo.M[1][1], InTo.M[1][2], InTo.M[1][3]),
			InWeight),
		FMath::Lerp(
			FVector4(InFrom.M[2][0], InFrom.M[2][1], InFrom.M[2][2], InFrom.M[2][3]),
			FVector4(InTo.M[2][0], InTo.M[2][1], InTo.M[2][2], InTo.M[2][3]),
			InWeight),
		FMath::Lerp(
			FVector4(InFrom.M[3][0], InFrom.M[3][1], InFrom.M[3][2], InFrom.M[3][3]),
			FVector4(InTo.M[3][0], InTo.M[3][1], InTo.M[3][2], InTo.M[3][3]),
			InWeight));
}

// Transformation methods.

FMatrix UBasis4D::Basis4D_TransformTo(const FMatrix& InFrom, const FMatrix& InTo)
{
	return Basis4D_Compose(InTo, Basis4D_Inverse(InFrom));
}

FVector4 UBasis4D::Basis4D_InverseTransform(const FMatrix& InBasis, const FVector4& InVector)
{
	return Basis4D_Inverse(InBasis).TransformFVector4(InVector);
}

FVector4 UBasis4D::Basis4D_TransposeTransform(const FMatrix& InBasis, const FVector4& InVector)
{
	return FVector4(
			Dot4(FVector4(InBasis.M[0][0], InBasis.M[0][1], InBasis.M[0][2], InBasis.M[0][3]), InVector),
			Dot4(FVector4(InBasis.M[1][0], InBasis.M[1][1], InBasis.M[1][2], InBasis.M[1][3]), InVector),
			Dot4(FVector4(InBasis.M[2][0], InBasis.M[2][1], InBasis.M[2][2], InBasis.M[2][3]), InVector),
			Dot4(FVector4(InBasis.M[3][0], InBasis.M[3][1], InBasis.M[3][2], InBasis.M[3][3]), InVector));
}

TArray<FVector4> UBasis4D::Basis4D_TransformMany(const FMatrix& InBasis, const TArray<FVector4>& InVectors)
{
	TArray<FVector4> array;
	array.SetNum(InVectors.Num());
	for (int32 i = 0; i < InVectors.Num(); ++i)
	{
		array[i] = InBasis.TransformFVector4(InVectors[i]);
	}
	return array;
}

// Rotation methods.

FMatrix UBasis4D::Basis4D_RotatePlaneGlobal(const FMatrix& InBasis, const FVector4& InPlaneFrom, const FVector4& InPlaneTo, const double InAngle)
{
	return Basis4D_FromVectors(UVectorFuncs4D::Vector4_RotateInPlane(FVector4(InBasis.M[0][0], InBasis.M[0][1], InBasis.M[0][2], InBasis.M[0][3]), InPlaneFrom, InPlaneTo, InAngle),
			UVectorFuncs4D::Vector4_RotateInPlane(FVector4(InBasis.M[1][0], InBasis.M[1][1], InBasis.M[1][2], InBasis.M[1][3]), InPlaneFrom, InPlaneTo, InAngle),
			UVectorFuncs4D::Vector4_RotateInPlane(FVector4(InBasis.M[2][0], InBasis.M[2][1], InBasis.M[2][2], InBasis.M[2][3]), InPlaneFrom, InPlaneTo, InAngle),
			UVectorFuncs4D::Vector4_RotateInPlane(FVector4(InBasis.M[3][0], InBasis.M[3][1], InBasis.M[3][2], InBasis.M[3][3]), InPlaneFrom, InPlaneTo, InAngle));
}

FMatrix UBasis4D::Basis4D_RotatePlaneLocal(const FMatrix& InBasis, const FVector4& InLocalPlaneFrom, const FVector4& InLocalPlaneTo, const double InAngle)
{
	const FVector4 globalPlaneFrom = InBasis.TransformFVector4(InLocalPlaneFrom);
	const FVector4 globalPlaneTo = InBasis.TransformFVector4(InLocalPlaneTo);
	return Basis4D_RotatePlaneGlobal(InBasis, globalPlaneFrom, globalPlaneTo, InAngle);
}

FMatrix UBasis4D::Basis4D_Inverse(const FMatrix& InBasis)
{
	const FMatrix adj = Basis4D_Adjugate(InBasis);
	const double det =
			InBasis.M[0][0] * adj.M[0][0] +
			InBasis.M[0][1] * adj.M[1][0] +
			InBasis.M[0][2] * adj.M[2][0] +
			InBasis.M[0][3] * adj.M[3][0];
	// We want to only fail for values *really* close to zero.
	// A scale of 0.00001 (1e-5) in 4D results in a 1e-20 determinant.
	if (FMath::IsNearlyZero(det, 1e-20) || !FMath::IsFinite(det))
	{
		UE_LOG(LogUnreal4DCore, Warning, TEXT("Basis4D_Inverse: Cannot invert matrix with zero or near-zero determinant (%f). Returning identity matrix."), det);
		ensure(false);
		return FMatrix::Identity;
	}
	return Basis4D_ScaledUniform(adj, 1.0 / det);
}

FMatrix UBasis4D::Basis4D_Adjugate(const FMatrix& InBasis)
{
	// This method accumulates a lot of floating-point error, but doing
	// all calculations in "at least doubles" (double_t) helps a ton.
	// Canonical 2x2 minors.
	const double_t dxy_zw = (double_t)InBasis.M[2][0] * (double_t)InBasis.M[3][1] - (double_t)InBasis.M[3][0] * (double_t)InBasis.M[2][1];
	const double_t dxz_zw = (double_t)InBasis.M[2][0] * (double_t)InBasis.M[3][2] - (double_t)InBasis.M[3][0] * (double_t)InBasis.M[2][2];
	const double_t dxw_zw = (double_t)InBasis.M[2][0] * (double_t)InBasis.M[3][3] - (double_t)InBasis.M[3][0] * (double_t)InBasis.M[2][3];
	const double_t dyz_zw = (double_t)InBasis.M[2][1] * (double_t)InBasis.M[3][2] - (double_t)InBasis.M[3][1] * (double_t)InBasis.M[2][2];
	const double_t dyw_zw = (double_t)InBasis.M[2][1] * (double_t)InBasis.M[3][3] - (double_t)InBasis.M[3][1] * (double_t)InBasis.M[2][3];
	const double_t dzw_zw = (double_t)InBasis.M[2][2] * (double_t)InBasis.M[3][3] - (double_t)InBasis.M[3][2] * (double_t)InBasis.M[2][3];
	const double_t dxy_yw = (double_t)InBasis.M[1][0] * (double_t)InBasis.M[3][1] - (double_t)InBasis.M[3][0] * (double_t)InBasis.M[1][1];
	const double_t dxz_yw = (double_t)InBasis.M[1][0] * (double_t)InBasis.M[3][2] - (double_t)InBasis.M[3][0] * (double_t)InBasis.M[1][2];
	const double_t dxw_yw = (double_t)InBasis.M[1][0] * (double_t)InBasis.M[3][3] - (double_t)InBasis.M[3][0] * (double_t)InBasis.M[1][3];
	const double_t dyz_yw = (double_t)InBasis.M[1][1] * (double_t)InBasis.M[3][2] - (double_t)InBasis.M[3][1] * (double_t)InBasis.M[1][2];
	const double_t dyw_yw = (double_t)InBasis.M[1][1] * (double_t)InBasis.M[3][3] - (double_t)InBasis.M[3][1] * (double_t)InBasis.M[1][3];
	const double_t dzw_yw = (double_t)InBasis.M[1][2] * (double_t)InBasis.M[3][3] - (double_t)InBasis.M[3][2] * (double_t)InBasis.M[1][3];
	const double_t dxy_yz = (double_t)InBasis.M[1][0] * (double_t)InBasis.M[2][1] - (double_t)InBasis.M[2][0] * (double_t)InBasis.M[1][1];
	const double_t dxz_yz = (double_t)InBasis.M[1][0] * (double_t)InBasis.M[2][2] - (double_t)InBasis.M[2][0] * (double_t)InBasis.M[1][2];
	const double_t dxw_yz = (double_t)InBasis.M[1][0] * (double_t)InBasis.M[2][3] - (double_t)InBasis.M[2][0] * (double_t)InBasis.M[1][3];
	const double_t dyz_yz = (double_t)InBasis.M[1][1] * (double_t)InBasis.M[2][2] - (double_t)InBasis.M[2][1] * (double_t)InBasis.M[1][2];
	const double_t dyw_yz = (double_t)InBasis.M[1][1] * (double_t)InBasis.M[2][3] - (double_t)InBasis.M[2][1] * (double_t)InBasis.M[1][3];
	const double_t dzw_yz = (double_t)InBasis.M[1][2] * (double_t)InBasis.M[2][3] - (double_t)InBasis.M[2][2] * (double_t)InBasis.M[1][3];
	// Adjugate entries.
	const double xx = +(double_t)InBasis.M[1][1] * dzw_zw - (double_t)InBasis.M[1][2] * dyw_zw + (double_t)InBasis.M[1][3] * dyz_zw;
	const double xy = -(double_t)InBasis.M[0][3] * dyz_zw + (double_t)InBasis.M[0][2] * dyw_zw - (double_t)InBasis.M[0][1] * dzw_zw;
	const double xz = +(double_t)InBasis.M[0][1] * dzw_yw - (double_t)InBasis.M[0][2] * dyw_yw + (double_t)InBasis.M[0][3] * dyz_yw;
	const double xw = -(double_t)InBasis.M[0][3] * dyz_yz + (double_t)InBasis.M[0][2] * dyw_yz - (double_t)InBasis.M[0][1] * dzw_yz;
	const double yx = -(double_t)InBasis.M[1][3] * dxz_zw + (double_t)InBasis.M[1][2] * dxw_zw - (double_t)InBasis.M[1][0] * dzw_zw;
	const double yy = +(double_t)InBasis.M[0][0] * dzw_zw - (double_t)InBasis.M[0][2] * dxw_zw + (double_t)InBasis.M[0][3] * dxz_zw;
	const double yw = +(double_t)InBasis.M[0][0] * dzw_yz - (double_t)InBasis.M[0][2] * dxw_yz + (double_t)InBasis.M[0][3] * dxz_yz;
	const double yz = -(double_t)InBasis.M[0][3] * dxz_yw + (double_t)InBasis.M[0][2] * dxw_yw - (double_t)InBasis.M[0][0] * dzw_yw;
	const double zx = +(double_t)InBasis.M[1][0] * dyw_zw - (double_t)InBasis.M[1][1] * dxw_zw + (double_t)InBasis.M[1][3] * dxy_zw;
	const double zy = -(double_t)InBasis.M[0][3] * dxy_zw + (double_t)InBasis.M[0][1] * dxw_zw - (double_t)InBasis.M[0][0] * dyw_zw;
	const double zz = +(double_t)InBasis.M[0][0] * dyw_yw - (double_t)InBasis.M[0][1] * dxw_yw + (double_t)InBasis.M[0][3] * dxy_yw;
	const double zw = -(double_t)InBasis.M[0][3] * dxy_yz + (double_t)InBasis.M[0][1] * dxw_yz - (double_t)InBasis.M[0][0] * dyw_yz;
	const double wx = -(double_t)InBasis.M[1][2] * dxy_zw + (double_t)InBasis.M[1][1] * dxz_zw - (double_t)InBasis.M[1][0] * dyz_zw;
	const double wy = +(double_t)InBasis.M[0][0] * dyz_zw - (double_t)InBasis.M[0][1] * dxz_zw + (double_t)InBasis.M[0][2] * dxy_zw;
	const double wz = -(double_t)InBasis.M[0][2] * dxy_yw + (double_t)InBasis.M[0][1] * dxz_yw - (double_t)InBasis.M[0][0] * dyz_yw;
	const double ww = +(double_t)InBasis.M[0][0] * dyz_yz - (double_t)InBasis.M[0][1] * dxz_yz + (double_t)InBasis.M[0][2] * dxy_yz;
	return Basis4D_FromNumbers(xx, xy, xz, xw, yx, yy, yz, yw, zx, zy, zz, zw, wx, wy, wz, ww);
}

// Scale methods.
// NOTE: These functions follow Unreal's FMatrix conventions, where matrix
// rows are the basis vectors, and the matrix is stored in row-major order.
// Therefore, local scaling is on the rows, and global scaling is on the columns.

FMatrix UBasis4D::Basis4D_ScaledGlobal(const FMatrix& InBasis, const FVector4& InScale)
{
	FMatrix basis = InBasis;
	basis.M[0][0] *= InScale.X;
	basis.M[1][0] *= InScale.X;
	basis.M[2][0] *= InScale.X;
	basis.M[3][0] *= InScale.X;
	basis.M[0][1] *= InScale.Y;
	basis.M[1][1] *= InScale.Y;
	basis.M[2][1] *= InScale.Y;
	basis.M[3][1] *= InScale.Y;
	basis.M[0][2] *= InScale.Z;
	basis.M[1][2] *= InScale.Z;
	basis.M[2][2] *= InScale.Z;
	basis.M[3][2] *= InScale.Z;
	basis.M[0][3] *= InScale.W;
	basis.M[1][3] *= InScale.W;
	basis.M[2][3] *= InScale.W;
	basis.M[3][3] *= InScale.W;
	return basis;
}

FMatrix UBasis4D::Basis4D_ScaledLocal(const FMatrix& InBasis, const FVector4& InScale)
{
	FMatrix basis = InBasis;
	basis.M[0][0] *= InScale.X;
	basis.M[0][1] *= InScale.X;
	basis.M[0][2] *= InScale.X;
	basis.M[0][3] *= InScale.X;
	basis.M[1][0] *= InScale.Y;
	basis.M[1][1] *= InScale.Y;
	basis.M[1][2] *= InScale.Y;
	basis.M[1][3] *= InScale.Y;
	basis.M[2][0] *= InScale.Z;
	basis.M[2][1] *= InScale.Z;
	basis.M[2][2] *= InScale.Z;
	basis.M[2][3] *= InScale.Z;
	basis.M[3][0] *= InScale.W;
	basis.M[3][1] *= InScale.W;
	basis.M[3][2] *= InScale.W;
	basis.M[3][3] *= InScale.W;
	return basis;
}

FMatrix UBasis4D::Basis4D_ScaledUniform(const FMatrix& InBasis, const double InScale)
{
	FMatrix basis = InBasis;
	basis.M[0][0] *= InScale;
	basis.M[0][1] *= InScale;
	basis.M[0][2] *= InScale;
	basis.M[0][3] *= InScale;
	basis.M[1][0] *= InScale;
	basis.M[1][1] *= InScale;
	basis.M[1][2] *= InScale;
	basis.M[1][3] *= InScale;
	basis.M[2][0] *= InScale;
	basis.M[2][1] *= InScale;
	basis.M[2][2] *= InScale;
	basis.M[2][3] *= InScale;
	basis.M[3][0] *= InScale;
	basis.M[3][1] *= InScale;
	basis.M[3][2] *= InScale;
	basis.M[3][3] *= InScale;
	return basis;
}

FVector4 UBasis4D::Basis4D_GetGlobalScaleAbs(const FMatrix& InBasis)
{
	return FVector4(
			Basis4D_GetColumn(InBasis, 0).Size(),
			Basis4D_GetColumn(InBasis, 1).Size(),
			Basis4D_GetColumn(InBasis, 2).Size(),
			Basis4D_GetColumn(InBasis, 3).Size());
}

FVector4 UBasis4D::Basis4D_GetScale(const FMatrix& InBasis)
{
	FVector4 scale = Basis4D_GetScaleAbs(InBasis);
	if (InBasis.Determinant() < 0.0)
	{
		// In order to have a negative scale, we need to flip the handedness.
		// This means we need to flip an odd number of axes. In 2D we flip Y,
		// in 3D we flip all axes, for 4D we will just flip the W axis.
		scale.W = -scale.W;
	}
	return scale;
}

FVector4 UBasis4D::Basis4D_GetScaleAbs(const FMatrix& InBasis)
{
	return FVector4(
			Basis4D_GetRow(InBasis, 0).Size(),
			Basis4D_GetRow(InBasis, 1).Size(),
			Basis4D_GetRow(InBasis, 2).Size(),
			Basis4D_GetRow(InBasis, 3).Size());
}

FMatrix UBasis4D::Basis4D_WithScale(const FMatrix& InBasis, const FVector4& InScale)
{
	// Prevent scaling to zero to avoid breaking things.
	const FVector4 newScale = FVector4(
			FMath::IsNearlyZero(InScale.X, UNREAL4D_COMPARISON_EPSILON) ? UNREAL4D_COMPARISON_EPSILON : InScale.X,
			FMath::IsNearlyZero(InScale.Y, UNREAL4D_COMPARISON_EPSILON) ? UNREAL4D_COMPARISON_EPSILON : InScale.Y,
			FMath::IsNearlyZero(InScale.Z, UNREAL4D_COMPARISON_EPSILON) ? UNREAL4D_COMPARISON_EPSILON : InScale.Z,
			FMath::IsNearlyZero(InScale.W, UNREAL4D_COMPARISON_EPSILON) ? UNREAL4D_COMPARISON_EPSILON : InScale.W);
	const FVector4 deltaScale = newScale / Basis4D_GetScale(InBasis);
	return Basis4D_ScaledLocal(InBasis, deltaScale);
}

double UBasis4D::Basis4D_GetUniformScale(const FMatrix& InBasis)
{
	const double det = InBasis.Determinant();
	// Take the fourth root of the determinant. For example, if we have a
	// scale of (2, 2, 2, 2), the determinant will be 16, and the fourth root
	// will be 2. If the determinant is negative, scale of (2, 2, 2, -2),
	// the determinant will be -16, and we want to return -2.
	if (det < 0.0)
	{
		return -FMath::Sqrt(FMath::Sqrt(-det));
	}
	else
	{
		return FMath::Sqrt(FMath::Sqrt(det));
	}
}

FMatrix UBasis4D::Basis4D_WithUniformScale(const FMatrix& InBasis, const double InUniformScale)
{
	const double det = InBasis.Determinant();
	const double absScale = FMath::Abs(InUniformScale);
	const FVector4 x = absScale * UVectorFuncs4D::Vector4_Normalized(
		FVector4(InBasis.M[0][0], InBasis.M[0][1], InBasis.M[0][2], InBasis.M[0][3]));
	const FVector4 y = absScale * UVectorFuncs4D::Vector4_Normalized(
		FVector4(InBasis.M[1][0], InBasis.M[1][1], InBasis.M[1][2], InBasis.M[1][3]));
	const FVector4 z = absScale * UVectorFuncs4D::Vector4_Normalized(
		FVector4(InBasis.M[2][0], InBasis.M[2][1], InBasis.M[2][2], InBasis.M[2][3]));
	const FVector4 wNormalized = UVectorFuncs4D::Vector4_Normalized(
		FVector4(InBasis.M[3][0], InBasis.M[3][1], InBasis.M[3][2], InBasis.M[3][3]));
	// The canonical negative scale is only on the W axis.
	// Check if the signs match. We only want positive/negative, not zero.
	if ((det < 0.0) == (InUniformScale < 0.0))
	{
		// If the determinant and the scale have the same sign, we can just multiply the W axis by the scale.
		return Basis4D_FromVectors(x, y, z, absScale * wNormalized);
	}
	// If the determinant and the scale have different signs, we need to flip the W axis.
	return Basis4D_FromVectors(x, y, z, -absScale * wNormalized);
}

// Validation methods.

FMatrix UBasis4D::Basis4D_Conformalized(const FMatrix& InBasis)
{
	const double scale = Basis4D_GetUniformScale(InBasis);
	return Basis4D_Orthonormalized(InBasis) * scale;
}

FMatrix UBasis4D::Basis4D_Normalized(const FMatrix& InBasis)
{
	return Basis4D_FromVectors(
		UVectorFuncs4D::Vector4_Normalized(FVector4(InBasis.M[0][0], InBasis.M[0][1], InBasis.M[0][2], InBasis.M[0][3])),
		UVectorFuncs4D::Vector4_Normalized(FVector4(InBasis.M[1][0], InBasis.M[1][1], InBasis.M[1][2], InBasis.M[1][3])),
		UVectorFuncs4D::Vector4_Normalized(FVector4(InBasis.M[2][0], InBasis.M[2][1], InBasis.M[2][2], InBasis.M[2][3])),
		UVectorFuncs4D::Vector4_Normalized(FVector4(InBasis.M[3][0], InBasis.M[3][1], InBasis.M[3][2], InBasis.M[3][3])));
}

FMatrix UBasis4D::Basis4D_Orthonormalized(const FMatrix& InBasis)
{
	// Gram-Schmidt Process, now in 4D.
	// https://en.wikipedia.org/wiki/Gram-Schmidt_process
	const FVector4 x = UVectorFuncs4D::Vector4_Normalized(
		FVector4(InBasis.M[0][0], InBasis.M[0][1], InBasis.M[0][2], InBasis.M[0][3]));
	FVector4 y = FVector4(InBasis.M[1][0], InBasis.M[1][1], InBasis.M[1][2], InBasis.M[1][3]);
	y = y - x * Dot4(x, y);
	y = UVectorFuncs4D::Vector4_Normalized(y);
	FVector4 z = FVector4(InBasis.M[2][0], InBasis.M[2][1], InBasis.M[2][2], InBasis.M[2][3]);
	z = z - x * Dot4(x, z) - y * Dot4(y, z);
	z = UVectorFuncs4D::Vector4_Normalized(z);
	FVector4 w = FVector4(InBasis.M[3][0], InBasis.M[3][1], InBasis.M[3][2], InBasis.M[3][3]);
	w = w - x * Dot4(x, w) - y * Dot4(y, w) - z * Dot4(z, w);
	w = UVectorFuncs4D::Vector4_Normalized(w);
	return Basis4D_FromVectors(x, y, z, w);
}

FMatrix UBasis4D::Basis4D_Orthogonalized(const FMatrix& InBasis)
{
	const FMatrix orthonorm = Basis4D_Orthonormalized(InBasis);
	const FVector4 scale = Basis4D_GetScaleAbs(InBasis);
	return Basis4D_ScaledLocal(orthonorm, scale);
}

// Returns true if the basis vectors are orthogonal (perpendicular), so it has no skew or shear, and can be decomposed into rotation and scale.
// See https://en.wikipedia.org/wiki/Orthogonal_basis
bool UBasis4D::Basis4D_IsOrthogonal(const FMatrix& InBasis)
{
	const FVector4 x = FVector4(InBasis.M[0][0], InBasis.M[0][1], InBasis.M[0][2], InBasis.M[0][3]);
	const FVector4 y = FVector4(InBasis.M[1][0], InBasis.M[1][1], InBasis.M[1][2], InBasis.M[1][3]);
	const FVector4 z = FVector4(InBasis.M[2][0], InBasis.M[2][1], InBasis.M[2][2], InBasis.M[2][3]);
	const FVector4 w = FVector4(InBasis.M[3][0], InBasis.M[3][1], InBasis.M[3][2], InBasis.M[3][3]);
	return FMath::IsNearlyZero(Dot4(x, y), UNREAL4D_COMPARISON_EPSILON) &&
	       FMath::IsNearlyZero(Dot4(x, z), UNREAL4D_COMPARISON_EPSILON) &&
	       FMath::IsNearlyZero(Dot4(y, z), UNREAL4D_COMPARISON_EPSILON) &&
	       FMath::IsNearlyZero(Dot4(x, w), UNREAL4D_COMPARISON_EPSILON) &&
	       FMath::IsNearlyZero(Dot4(y, w), UNREAL4D_COMPARISON_EPSILON) &&
	       FMath::IsNearlyZero(Dot4(z, w), UNREAL4D_COMPARISON_EPSILON);
}

// Returns true if the basis vectors are orthonormal (orthogonal and normalized), so it has no scale, skew, or shear,
// but it may have a flip (scale of -1). Use `Basis4D_IsRotation` if flips are not desired.
// See https://en.wikipedia.org/wiki/Orthonormal_basis
bool UBasis4D::Basis4D_IsOrthonormal(const FMatrix& InBasis)
{
	const FVector4 x = FVector4(InBasis.M[0][0], InBasis.M[0][1], InBasis.M[0][2], InBasis.M[0][3]);
	const FVector4 y = FVector4(InBasis.M[1][0], InBasis.M[1][1], InBasis.M[1][2], InBasis.M[1][3]);
	const FVector4 z = FVector4(InBasis.M[2][0], InBasis.M[2][1], InBasis.M[2][2], InBasis.M[2][3]);
	const FVector4 w = FVector4(InBasis.M[3][0], InBasis.M[3][1], InBasis.M[3][2], InBasis.M[3][3]);
	const bool isUnscaled = FMath::IsNearlyEqual(x.SizeSquared(), 1.0, UNREAL4D_COMPARISON_EPSILON) &&
	                        FMath::IsNearlyEqual(y.SizeSquared(), 1.0, UNREAL4D_COMPARISON_EPSILON) &&
	                        FMath::IsNearlyEqual(z.SizeSquared(), 1.0, UNREAL4D_COMPARISON_EPSILON) &&
	                        FMath::IsNearlyEqual(w.SizeSquared(), 1.0, UNREAL4D_COMPARISON_EPSILON);
	const bool isOrthogonal = FMath::IsNearlyZero(Dot4(x, y), UNREAL4D_COMPARISON_EPSILON) &&
	                          FMath::IsNearlyZero(Dot4(x, z), UNREAL4D_COMPARISON_EPSILON) &&
	                          FMath::IsNearlyZero(Dot4(y, z), UNREAL4D_COMPARISON_EPSILON) &&
	                          FMath::IsNearlyZero(Dot4(x, w), UNREAL4D_COMPARISON_EPSILON) &&
	                          FMath::IsNearlyZero(Dot4(y, w), UNREAL4D_COMPARISON_EPSILON) &&
	                          FMath::IsNearlyZero(Dot4(z, w), UNREAL4D_COMPARISON_EPSILON);
	return isUnscaled && isOrthogonal;
}

// Returns true if the basis is conformal (orthogonal, uniform scale, preserves angles and distance ratios).
// Conformal matrices are important for correct behavior in physics simulations and many other calculations.
// See https://en.wikipedia.org/wiki/Conformal_linear_transformation
bool UBasis4D::Basis4D_IsConformal(const FMatrix& InBasis)
{
	const FVector4 x = FVector4(InBasis.M[0][0], InBasis.M[0][1], InBasis.M[0][2], InBasis.M[0][3]);
	const FVector4 y = FVector4(InBasis.M[1][0], InBasis.M[1][1], InBasis.M[1][2], InBasis.M[1][3]);
	const FVector4 z = FVector4(InBasis.M[2][0], InBasis.M[2][1], InBasis.M[2][2], InBasis.M[2][3]);
	const FVector4 w = FVector4(InBasis.M[3][0], InBasis.M[3][1], InBasis.M[3][2], InBasis.M[3][3]);
	const double xLenSq = x.SizeSquared();
	const bool isUniformScale = FMath::IsNearlyEqual(xLenSq, y.SizeSquared(), UNREAL4D_COMPARISON_EPSILON) &&
	                            FMath::IsNearlyEqual(xLenSq, z.SizeSquared(), UNREAL4D_COMPARISON_EPSILON) &&
	                            FMath::IsNearlyEqual(xLenSq, w.SizeSquared(), UNREAL4D_COMPARISON_EPSILON);
	const bool isOrthogonal = FMath::IsNearlyZero(Dot4(x, y), UNREAL4D_COMPARISON_EPSILON) &&
	                          FMath::IsNearlyZero(Dot4(x, z), UNREAL4D_COMPARISON_EPSILON) &&
	                          FMath::IsNearlyZero(Dot4(y, z), UNREAL4D_COMPARISON_EPSILON) &&
	                          FMath::IsNearlyZero(Dot4(x, w), UNREAL4D_COMPARISON_EPSILON) &&
	                          FMath::IsNearlyZero(Dot4(y, w), UNREAL4D_COMPARISON_EPSILON) &&
	                          FMath::IsNearlyZero(Dot4(z, w), UNREAL4D_COMPARISON_EPSILON);
	return isUniformScale && isOrthogonal;
}

// Returns true if the basis is diagonal (only has scale or flip, no rotation, skew, or shear).
bool UBasis4D::Basis4D_IsDiagonal(const FMatrix& InBasis)
{
	return (
			FMath::IsNearlyZero(InBasis.M[0][1], UNREAL4D_COMPARISON_EPSILON) && FMath::IsNearlyZero(InBasis.M[1][0], UNREAL4D_COMPARISON_EPSILON) &&
			FMath::IsNearlyZero(InBasis.M[0][2], UNREAL4D_COMPARISON_EPSILON) && FMath::IsNearlyZero(InBasis.M[2][0], UNREAL4D_COMPARISON_EPSILON) &&
			FMath::IsNearlyZero(InBasis.M[0][3], UNREAL4D_COMPARISON_EPSILON) && FMath::IsNearlyZero(InBasis.M[3][0], UNREAL4D_COMPARISON_EPSILON) &&
			FMath::IsNearlyZero(InBasis.M[1][2], UNREAL4D_COMPARISON_EPSILON) && FMath::IsNearlyZero(InBasis.M[2][1], UNREAL4D_COMPARISON_EPSILON) &&
			FMath::IsNearlyZero(InBasis.M[1][3], UNREAL4D_COMPARISON_EPSILON) && FMath::IsNearlyZero(InBasis.M[3][1], UNREAL4D_COMPARISON_EPSILON) &&
			FMath::IsNearlyZero(InBasis.M[2][3], UNREAL4D_COMPARISON_EPSILON) && FMath::IsNearlyZero(InBasis.M[3][2], UNREAL4D_COMPARISON_EPSILON));
}

// Returns true if the basis is a pure rotation (no scale, flip, skew, or shear).
bool UBasis4D::Basis4D_IsRotation(const FMatrix& InBasis)
{
	return Basis4D_IsConformal(InBasis) && FMath::IsNearlyEqual(InBasis.Determinant(), 1.0, UNREAL4D_COMPARISON_EPSILON);
}

// Helper setters/getters.
// NOTE: The FMatrix versions of these are only for 3D matrices, returning 3D FVector.
// NOTE: These names follow Unreal's conventions, where matrix rows are the basis vectors.
FVector4 UBasis4D::Basis4D_GetColumn(const FMatrix& InBasis, const int32 InIndex)
{
	switch (InIndex)
	{
		case 0: return FVector4(InBasis.M[0][0], InBasis.M[1][0], InBasis.M[2][0], InBasis.M[3][0]);
		case 1: return FVector4(InBasis.M[0][1], InBasis.M[1][1], InBasis.M[2][1], InBasis.M[3][1]);
		case 2: return FVector4(InBasis.M[0][2], InBasis.M[1][2], InBasis.M[2][2], InBasis.M[3][2]);
		case 3: return FVector4(InBasis.M[0][3], InBasis.M[1][3], InBasis.M[2][3], InBasis.M[3][3]);
		default:
		{
			UE_LOG(LogUnreal4DCore, Error, TEXT("UBasis4D::GetColumn: Invalid column index %d"), InIndex);
			return FVector4::Zero();
		}
	}
}

void UBasis4D::Basis4D_SetColumn(FMatrix& InBasis, const int32 InIndex, const FVector4& InValue)
{
	switch (InIndex)
	{
		case 0:
		{
			InBasis.M[0][0] = InValue.X;
			InBasis.M[1][0] = InValue.Y;
			InBasis.M[2][0] = InValue.Z;
			InBasis.M[3][0] = InValue.W;
		} break;
		case 1:
		{
			InBasis.M[0][1] = InValue.X;
			InBasis.M[1][1] = InValue.Y;
			InBasis.M[2][1] = InValue.Z;
			InBasis.M[3][1] = InValue.W;
		} break;
		case 2:
		{
			InBasis.M[0][2] = InValue.X;
			InBasis.M[1][2] = InValue.Y;
			InBasis.M[2][2] = InValue.Z;
			InBasis.M[3][2] = InValue.W;
		} break;
		case 3:
		{
			InBasis.M[0][3] = InValue.X;
			InBasis.M[1][3] = InValue.Y;
			InBasis.M[2][3] = InValue.Z;
			InBasis.M[3][3] = InValue.W;
		} break;
		default:
		{
			UE_LOG(LogUnreal4DCore, Error, TEXT("UBasis4D::SetColumn: Invalid column index %d"), InIndex);
		} break;
	}
}

FVector4 UBasis4D::Basis4D_GetRow(const FMatrix& InBasis, const int32 InIndex)
{
	switch (InIndex)
	{
		case 0: return FVector4(InBasis.M[0][0], InBasis.M[0][1], InBasis.M[0][2], InBasis.M[0][3]);
		case 1: return FVector4(InBasis.M[1][0], InBasis.M[1][1], InBasis.M[1][2], InBasis.M[1][3]);
		case 2: return FVector4(InBasis.M[2][0], InBasis.M[2][1], InBasis.M[2][2], InBasis.M[2][3]);
		case 3: return FVector4(InBasis.M[3][0], InBasis.M[3][1], InBasis.M[3][2], InBasis.M[3][3]);
		default:
		{
			UE_LOG(LogUnreal4DCore, Error, TEXT("UBasis4D::GetRow: Invalid row index %d"), InIndex);
			return FVector4::Zero();
		}
	}
}

void UBasis4D::Basis4D_SetRow(FMatrix& InBasis, const int32 InIndex, const FVector4& InRow)
{
	switch (InIndex)
	{
		case 0:
		{
			InBasis.M[0][0] = InRow.X;
			InBasis.M[0][1] = InRow.Y;
			InBasis.M[0][2] = InRow.Z;
			InBasis.M[0][3] = InRow.W;
		} break;
		case 1:
		{
			InBasis.M[1][0] = InRow.X;
			InBasis.M[1][1] = InRow.Y;
			InBasis.M[1][2] = InRow.Z;
			InBasis.M[1][3] = InRow.W;
		} break;
		case 2:
		{
			InBasis.M[2][0] = InRow.X;
			InBasis.M[2][1] = InRow.Y;
			InBasis.M[2][2] = InRow.Z;
			InBasis.M[2][3] = InRow.W;
		} break;
		case 3:
		{
			InBasis.M[3][0] = InRow.X;
			InBasis.M[3][1] = InRow.Y;
			InBasis.M[3][2] = InRow.Z;
			InBasis.M[3][3] = InRow.W;
		} break;
		default:
		{
			UE_LOG(LogUnreal4DCore, Error, TEXT("UBasis4D::SetRow: Invalid row index %d"), InIndex);
		} break;
	}
}

// Conversion.

FMatrix UBasis4D::Basis4D_From3D(const FMatrix& InFrom3D)
{
	return Basis4D_FromVectors(
			FVector4(InFrom3D.M[0][0], InFrom3D.M[0][1], InFrom3D.M[0][2], 0.0),
			FVector4(InFrom3D.M[1][0], InFrom3D.M[1][1], InFrom3D.M[1][2], 0.0),
			FVector4(InFrom3D.M[2][0], InFrom3D.M[2][1], InFrom3D.M[2][2], 0.0),
			FVector4(0.0, 0.0, 0.0, 1.0));
}

FMatrix UBasis4D::Basis4D_To3D(const FMatrix& InBasis, const bool bOrthonormalized)
{
	const FMatrix ret = Basis4D_FromVectors(
		FVector4(InBasis.M[0][0], InBasis.M[0][1], InBasis.M[0][2], 0.0),
		FVector4(InBasis.M[1][0], InBasis.M[1][1], InBasis.M[1][2], 0.0),
		FVector4(InBasis.M[2][0], InBasis.M[2][1], InBasis.M[2][2], 0.0),
		FVector4(0.0, 0.0, 0.0, 1.0));
	if (bOrthonormalized)
	{
		return UBasis4D::Basis4D_Orthonormalized(ret);
	}
	return ret;
}

FMatrix UBasis4D::Basis4D_FromArray(const TArray<double>& InFromArray)
{
	check(InFromArray.Num() >= 16);
	return Basis4D_FromNumbers(InFromArray[0], InFromArray[1], InFromArray[2], InFromArray[3],
			InFromArray[4], InFromArray[5], InFromArray[6], InFromArray[7],
			InFromArray[8], InFromArray[9], InFromArray[10], InFromArray[11],
			InFromArray[12], InFromArray[13], InFromArray[14], InFromArray[15]);
}

TArray<double> UBasis4D::Basis4D_ToArray(const FMatrix& InBasis)
{
	TArray<double> arr;
	arr.SetNum(16);
	arr[0] = InBasis.M[0][0];
	arr[1] = InBasis.M[0][1];
	arr[2] = InBasis.M[0][2];
	arr[3] = InBasis.M[0][3];
	arr[4] = InBasis.M[1][0];
	arr[5] = InBasis.M[1][1];
	arr[6] = InBasis.M[1][2];
	arr[7] = InBasis.M[1][3];
	arr[8] = InBasis.M[2][0];
	arr[9] = InBasis.M[2][1];
	arr[10] = InBasis.M[2][2];
	arr[11] = InBasis.M[2][3];
	arr[12] = InBasis.M[3][0];
	arr[13] = InBasis.M[3][1];
	arr[14] = InBasis.M[3][2];
	arr[15] = InBasis.M[3][3];
	return arr;
}

FMatrix UBasis4D::Basis4D_FromJsonArray(const TArray<TSharedPtr<FJsonValue>>& InJsonArray)
{
	check(InJsonArray.Num() >= 16);
	return Basis4D_FromNumbers(InJsonArray[0]->AsNumber(), InJsonArray[1]->AsNumber(), InJsonArray[2]->AsNumber(), InJsonArray[3]->AsNumber(),
			InJsonArray[4]->AsNumber(), InJsonArray[5]->AsNumber(), InJsonArray[6]->AsNumber(), InJsonArray[7]->AsNumber(),
			InJsonArray[8]->AsNumber(), InJsonArray[9]->AsNumber(), InJsonArray[10]->AsNumber(), InJsonArray[11]->AsNumber(),
			InJsonArray[12]->AsNumber(), InJsonArray[13]->AsNumber(), InJsonArray[14]->AsNumber(), InJsonArray[15]->AsNumber());
}

TArray<TSharedPtr<FJsonValue>> UBasis4D::Basis4D_ToJsonArray(const FMatrix& InBasis)
{
	TArray<TSharedPtr<FJsonValue>> ret;
	ret.SetNum(16);
	for (int32 i = 0; i < 4; i++)
	{
		for (int32 j = 0; j < 4; j++)
		{
			ret[i * 4 + j] = MakeShared<FJsonValueNumber>(InBasis.M[i][j]);
		}
	}
	return ret;
}

// Constructors.

FMatrix UBasis4D::Basis4D_FromScale(const FVector4& InScale)
{
	return Basis4D_FromVectors(
			FVector4(InScale.X, 0.0, 0.0, 0.0),
			FVector4(0.0, InScale.Y, 0.0, 0.0),
			FVector4(0.0, 0.0, InScale.Z, 0.0),
			FVector4(0.0, 0.0, 0.0, InScale.W));
}

FMatrix UBasis4D::Basis4D_FromScaleUniform(const double InScale)
{
	if (InScale < 0.0)
	{
		// For negative uniform scales, only the W axis is flipped.
		return Basis4D_FromVectors(
				FVector4(-InScale, 0.0, 0.0, 0.0),
				FVector4(0.0, -InScale, 0.0, 0.0),
				FVector4(0.0, 0.0, -InScale, 0.0),
				FVector4(0.0, 0.0, 0.0, InScale));
	}
	return Basis4D_FromVectors(
			FVector4(InScale, 0.0, 0.0, 0.0),
			FVector4(0.0, InScale, 0.0, 0.0),
			FVector4(0.0, 0.0, InScale, 0.0),
			FVector4(0.0, 0.0, 0.0, InScale));
}

FMatrix UBasis4D::Basis4D_FromXY(const double InAngle)
{
	double sinAngle = FMath::Sin(InAngle);
	double cosAngle = FMath::Cos(InAngle);
	FMatrix basis = FMatrix::Identity;
	basis.M[0][0] = cosAngle;
	basis.M[0][1] = sinAngle;
	basis.M[1][0] = -sinAngle;
	basis.M[1][1] = cosAngle;
	return basis;
}

FMatrix UBasis4D::Basis4D_FromYZ(const double InAngle)
{
	double sinAngle = FMath::Sin(InAngle);
	double cosAngle = FMath::Cos(InAngle);
	FMatrix basis = FMatrix::Identity;
	basis.M[1][1] = cosAngle;
	basis.M[1][2] = sinAngle;
	basis.M[2][1] = -sinAngle;
	basis.M[2][2] = cosAngle;
	return basis;
}

FMatrix UBasis4D::Basis4D_FromZX(const double InAngle)
{
	double sinAngle = FMath::Sin(InAngle);
	double cosAngle = FMath::Cos(InAngle);
	FMatrix basis = FMatrix::Identity;
	basis.M[2][2] = cosAngle;
	basis.M[2][0] = sinAngle;
	basis.M[0][2] = -sinAngle;
	basis.M[0][0] = cosAngle;
	return basis;
}

FMatrix UBasis4D::Basis4D_FromXW(const double InAngle)
{
	double sinAngle = FMath::Sin(InAngle);
	double cosAngle = FMath::Cos(InAngle);
	FMatrix basis = FMatrix::Identity;
	basis.M[0][0] = cosAngle;
	basis.M[0][3] = sinAngle;
	basis.M[3][0] = -sinAngle;
	basis.M[3][3] = cosAngle;
	return basis;
}

FMatrix UBasis4D::Basis4D_FromWY(const double InAngle)
{
	double sinAngle = FMath::Sin(InAngle);
	double cosAngle = FMath::Cos(InAngle);
	FMatrix basis = FMatrix::Identity;
	basis.M[3][3] = cosAngle;
	basis.M[3][1] = sinAngle;
	basis.M[1][3] = -sinAngle;
	basis.M[1][1] = cosAngle;
	return basis;
}

FMatrix UBasis4D::Basis4D_FromZW(const double InAngle)
{
	double sinAngle = FMath::Sin(InAngle);
	double cosAngle = FMath::Cos(InAngle);
	FMatrix basis = FMatrix::Identity;
	basis.M[2][2] = cosAngle;
	basis.M[2][3] = sinAngle;
	basis.M[3][2] = -sinAngle;
	basis.M[3][3] = cosAngle;
	return basis;
}

FMatrix UBasis4D::Basis4D_FromSwapRotation(const int32 InRotFrom, const int32 InRotTo)
{
	FMatrix ret = FMatrix::Identity;
	check(InRotFrom >= 0 && InRotFrom <= 3 && InRotTo >= 0 && InRotTo <= 3 && InRotFrom != InRotTo);
	// Set the rotation values as if the rotation was 90 degrees.
	ret.M[InRotFrom][InRotFrom] = 0.0;
	ret.M[InRotTo][InRotTo] = 0.0;
	ret.M[InRotFrom][InRotTo] = 1.0;
	ret.M[InRotTo][InRotFrom] = -1.0;
	return ret;
}

FMatrix UBasis4D::Basis4D_LookingAt(const FVector4& InTarget, const FVector4& InPerp, const FVector4& InUp)
{
	check(!FMath::IsNearlyZero(InTarget.Size(), UNREAL4D_COMPARISON_EPSILON));
	check(!FMath::IsNearlyZero(InUp.Size(), UNREAL4D_COMPARISON_EPSILON));
	check(!FMath::IsNearlyZero(InPerp.Size(), UNREAL4D_COMPARISON_EPSILON));
	// The highest priority is the forward direction (X).
	const FVector4 x = UVectorFuncs4D::Vector4_Normalized(InTarget);
	// Next, we determine the right direction (Y).
	FVector4 y = UVectorFuncs4D::Vector4_Perpendicular(x, InUp, InPerp);
	if (FMath::IsNearlyZero(y.Size(), UNREAL4D_COMPARISON_EPSILON))
	{
		y = FVector4(0.0, 1.0, 0.0, 0.0);
	}
	else
	{
		y = UVectorFuncs4D::Vector4_Normalized(y);
	}
	// Third, we determine the perpendicular direction (W).
	FVector4 w = UVectorFuncs4D::Vector4_Perpendicular(x, y, InUp);
	if (FMath::IsNearlyZero(w.Size(), UNREAL4D_COMPARISON_EPSILON))
	{
		w = FVector4(0.0, 0.0, 0.0, 1.0);
	}
	else
	{
		w = UVectorFuncs4D::Vector4_Normalized(w);
	}
	// Determine the up direction last (Z).
	FVector4 z = UVectorFuncs4D::Vector4_Perpendicular(y, x, w);
	if (FMath::IsNearlyZero(z.Size(), UNREAL4D_COMPARISON_EPSILON))
	{
		z = FVector4(0.0, 0.0, 1.0, 0.0);
	}
	else
	{
		z = UVectorFuncs4D::Vector4_Normalized(z);
	}
	const FMatrix basis = Basis4D_FromVectors(x, y, z, w);
	check(FMath::IsNearlyEqual(basis.Determinant(), 1.0, UNREAL4D_COMPARISON_EPSILON));
	return basis;
}
