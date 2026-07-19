#include "Euler4DBind.h"

// Misc methods.

FEuler4D UEuler4DLibrary::Euler4D_Compose(const FEuler4D& InParent, const FEuler4D& InChild)
{
	return InParent.Compose(InChild);
}

bool UEuler4DLibrary::Euler4D_IsEqualApprox(const FEuler4D& InEuler, const FEuler4D& InOther)
{
	return InEuler.IsEqualApprox(InOther);
}

FEuler4D UEuler4DLibrary::Euler4D_Lerp(const FEuler4D& InFrom, const FEuler4D& InTo, const double InWeight)
{
	return InFrom.Lerp(InTo, InWeight);
}

FMatrix UEuler4DLibrary::Euler4D_RotateBasis(const FEuler4D& InEuler, const FMatrix& InBasis)
{
	return InEuler.RotateBasis(InBasis);
}

FVector4 UEuler4DLibrary::Euler4D_RotatePoint(const FEuler4D& InEuler, const FVector4& InPoint)
{
	return InEuler.RotatePoint(InPoint);
}

FEuler4D UEuler4DLibrary::Euler4D_RotationTo(const FEuler4D& InFrom, const FEuler4D& InTo)
{
	return InFrom.RotationTo(InTo);
}

FEuler4D UEuler4DLibrary::Euler4D_Snapped(const FEuler4D& InEuler, double InStep)
{
	return InEuler.Snapped(InStep);
}

FEuler4D UEuler4DLibrary::Euler4D_Wrapped(const FEuler4D& InEuler)
{
	return InEuler.Wrapped();
}

FEuler4D UEuler4DLibrary::Euler4D_Zero()
{
	return FEuler4D::Zero();
}

// Radians/degrees.

FEuler4D UEuler4DLibrary::Euler4D_DegToRad(const FEuler4D& InEuler)
{
	return InEuler.DegToRad();
}

FEuler4D UEuler4DLibrary::Euler4D_RadToDeg(const FEuler4D& InEuler)
{
	return InEuler.RadToDeg();
}

// Component with-style setters.

FEuler4D UEuler4DLibrary::Euler4D_WithYZ(const FEuler4D& InEuler, double InYZ)
{
	return InEuler.WithYZ(InYZ);
}

FEuler4D UEuler4DLibrary::Euler4D_WithZX(const FEuler4D& InEuler, double InZX)
{
	return InEuler.WithZX(InZX);
}

FEuler4D UEuler4DLibrary::Euler4D_WithXY(const FEuler4D& InEuler, double InXY)
{
	return InEuler.WithXY(InXY);
}

FEuler4D UEuler4DLibrary::Euler4D_WithXW(const FEuler4D& InEuler, double InXW)
{
	return InEuler.WithXW(InXW);
}

FEuler4D UEuler4DLibrary::Euler4D_WithWY(const FEuler4D& InEuler, double InWY)
{
	return InEuler.WithWY(InWY);
}

FEuler4D UEuler4DLibrary::Euler4D_WithZW(const FEuler4D& InEuler, double InZW)
{
	return InEuler.WithZW(InZW);
}

// Conversion methods.

FEuler4D UEuler4DLibrary::Euler4D_From3D(const FVector& InVector)
{
	return FEuler4D::From3D(InVector);
}

FVector UEuler4DLibrary::Euler4D_To3D(const FEuler4D& InEuler)
{
	return InEuler.To3D();
}

FEuler4D UEuler4DLibrary::Euler4D_FromArray(const TArray<double>& InArray)
{
	return FEuler4D::FromArray(InArray);
}

TArray<double> UEuler4DLibrary::Euler4D_ToArray(const FEuler4D& InEuler)
{
	return InEuler.ToArray();
}

FEuler4D UEuler4DLibrary::Euler4D_DegreesFromBasis(const FMatrix& InBasis, bool bUseSpecialCases)
{
	return FEuler4D::DegreesFromBasis(InBasis, bUseSpecialCases);
}

FMatrix UEuler4DLibrary::Euler4D_DegreesToBasis(const FEuler4D& InEuler)
{
	return InEuler.DegreesToBasis();
}

FEuler4D UEuler4DLibrary::Euler4D_RadiansFromBasis(const FMatrix& InBasis, bool bUseSpecialCases)
{
	return FEuler4D::RadiansFromBasis(InBasis, bUseSpecialCases);
}

FMatrix UEuler4DLibrary::Euler4D_RadiansToBasis(const FEuler4D& InEuler)
{
	return InEuler.RadiansToBasis();
}
