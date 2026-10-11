#include "Bivector4D.h"

#include "QuaternionPair.h"
#include "Basis4D.h"

#include "GenericPlatform/GenericPlatformMath.h"
#include "Math/UnrealMath.h"

FMatrix FBivector4D::ToMatrix() const
{
	return UBasis4D::Basis4D_FromNumbers(
			0.0, -XY, -XZ, -XW,
			+XY, 0.0, -YZ, -YW,
			+XZ, +YZ, 0.0, -ZW,
			+XW, +YW, +ZW, 0.0);
}

double FBivector4D::Dot(const FBivector4D& InBivector) const
{
	return XY * InBivector.XY + XZ * InBivector.XZ + XW * InBivector.XW + YZ * InBivector.YZ + YW * InBivector.YW + ZW * InBivector.ZW;
}

FBivector4D FBivector4D::Dual() const
{
	return FBivector4D(-ZW, YW, -YZ, -XW, XZ, -XY);
}

FQuaternionPair FBivector4D::Exp() const
{
	FQuat left  = FQuat(-YZ + XW, +XZ + YW, -XY + ZW, 0.0) * 0.5;
	FQuat right = FQuat(+YZ + XW, -XZ + YW, +XY + ZW, 0.0) * 0.5;
	return FQuaternionPair(left.Exp(), right.Exp());
}

FBivector4D FBivector4D::BiplanarComponent() const
{
	FBivector4D plane = MainBiplane();
	return plane * (plane | *this);
}

FBivector4D FBivector4D::MainBiplane() const
{
	FBivector4D dual = Dual();
	FBivector4D selfDualPart = *this + dual;
	FBivector4D antiSelfDualPart = *this - dual;
	selfDualPart.Normalize();
	selfDualPart /= UE_DOUBLE_SQRT_2;
	antiSelfDualPart.Normalize();
	antiSelfDualPart /= UE_DOUBLE_SQRT_2;
	if (selfDualPart.SizeSquared() < 0.4)
	{
		// The only way it can have a size significantly different from 1/sqrt(2) is if it failed to normalize.
		// In that case, the result is at least partially arbitrary.
		selfDualPart = FBivector4D(0.5, 0.0, 0.0, 0.0, 0.0, -0.5);
	}
	if (antiSelfDualPart.SizeSquared() < 0.4)
	{
		antiSelfDualPart = FBivector4D(0.5, 0.0, 0.0, 0.0, 0.0, 0.5);
	}
	return selfDualPart + antiSelfDualPart;
}

FVector4 FBivector4D::LargestAxis() const {
	FBivector4D biplane = MainBiplane();
	FVector4 candidate = biplane * FVector4(1, 0, 0, 0);
	if (candidate.SizeSquared() < 0.01) {
		candidate = biplane * FVector4(0, 1, 0, 0);
		if (candidate.SizeSquared() < 0.01) {
			candidate = biplane * FVector4(0, 0, 1, 0);
			// A bivector of size 1 should not be able to be so nearly perpendicular to three axes at once.
			ensure(candidate.SizeSquared() >= 0.01);
		}
	}
	// The GetSafeNormal method isn't right because it only uses the first 3 coordinates. Candidate is known to be non-zero.
	return candidate / candidate.Size();
}

void FBivector4D::Eigenvalues(double& OutLargerEigenvalue, double& OutSmallerEigenvalue) const {
	// Dual-part lengths avoid cancellation in size squared minus chirality.
	const FBivector4D dual = Dual();
	const double selfDualSize = (*this + dual).Size();
	const double antiSelfDualSize = (*this - dual).Size();
	OutLargerEigenvalue = (selfDualSize + antiSelfDualSize) / (2.0 * UE_DOUBLE_SQRT_2);
	OutSmallerEigenvalue = FMath::Abs(selfDualSize - antiSelfDualSize) / (2.0 * UE_DOUBLE_SQRT_2);
}

FVector4 operator*(const FVector4& InVector, const FBivector4D& InBivector)
{
	return -(InBivector * InVector);
}

double FBivector4D::Size() const
{
	return FMath::Sqrt(SizeSquared());
}

double FBivector4D::SizeSquared() const
{
	return XY * XY + XZ * XZ + XW * XW + YZ * YZ + YW * YW + ZW * ZW;
}

FBivector4D FBivector4D::Normalized(double InTolerance) const
{
	double size = Size();
	if (size < InTolerance)
	{
		return FBivector4D();
	}
	return *this / size;
}

void FBivector4D::Normalize(double InTolerance)
{
	double size = Size();
	if (size < InTolerance)
	{
		*this = FBivector4D();
		return;
	}
	*this /= size;
}

// Operators.

bool FBivector4D::operator==(const FBivector4D& InBivector) const
{
	return XY == InBivector.XY && XZ == InBivector.XZ && XW == InBivector.XW && YZ == InBivector.YZ && YW == InBivector.YW && ZW == InBivector.ZW;
}

FBivector4D FBivector4D::operator*(const double InScale) const
{
	return FBivector4D(XY * InScale, XZ * InScale, XW * InScale, YZ * InScale, YW * InScale, ZW * InScale);
}

FBivector4D FBivector4D::operator*=(const double InScale)
{
	XY *= InScale;
	XZ *= InScale;
	XW *= InScale;
	YZ *= InScale;
	YW *= InScale;
	ZW *= InScale;
	return *this;
}

FBivector4D FBivector4D::operator/(const double InScale) const
{
	return (*this) * (1 / InScale);
}

FBivector4D FBivector4D::operator/=(const double InScale)
{
	*this *= (1 / InScale);
	return *this;
}

FBivector4D FBivector4D::operator+(const FBivector4D& InBivector) const
{
	return FBivector4D(XY + InBivector.XY, XZ + InBivector.XZ, XW + InBivector.XW, YZ + InBivector.YZ, YW + InBivector.YW, ZW + InBivector.ZW);
}

FBivector4D FBivector4D::operator-(const FBivector4D& InBivector) const
{
	return FBivector4D(XY - InBivector.XY, XZ - InBivector.XZ, XW - InBivector.XW, YZ - InBivector.YZ, YW - InBivector.YW, ZW - InBivector.ZW);
}

FBivector4D FBivector4D::operator-() const
{
	return FBivector4D(-XY, -XZ, -XW, -YZ, -YW, -ZW);
}

FVector4 FBivector4D::operator*(const FVector4& InVector) const
{
	return FVector4(
			+XY * InVector.Y + XZ * InVector.Z + XW * InVector.W,
			-XY * InVector.X + YZ * InVector.Z + YW * InVector.W,
			-XZ * InVector.X - YZ * InVector.Y + ZW * InVector.W,
			-XW * InVector.X - YW * InVector.Y - ZW * InVector.Z);
}

FBivector4D FBivector4D::operator*(const FBivector4D& InBivector) const
{
	return FBivector4D(
			(-XZ * InBivector.YZ - XW * InBivector.YW + YZ * InBivector.XZ + YW * InBivector.XW) * 0.5, // XY
			(+XY * InBivector.YZ - XW * InBivector.ZW - YZ * InBivector.XY + ZW * InBivector.XW) * 0.5, // XZ
			(+XY * InBivector.YW + XZ * InBivector.ZW - YW * InBivector.XY - ZW * InBivector.XZ) * 0.5, // XW
			(-XY * InBivector.XZ - YW * InBivector.ZW + XZ * InBivector.XY + ZW * InBivector.YW) * 0.5, // YZ
			(-XY * InBivector.XW + YZ * InBivector.ZW + XW * InBivector.XY - ZW * InBivector.YZ) * 0.5, // YW
			(-XZ * InBivector.XW - YZ * InBivector.YW + XW * InBivector.XZ + YW * InBivector.YZ) * 0.5  // ZW
	);
}

double FBivector4D::operator|(const FBivector4D& InBivector) const
{
	return XY * InBivector.XY + XZ * InBivector.XZ + XW * InBivector.XW + YZ * InBivector.YZ + YW * InBivector.YW + ZW * InBivector.ZW;
}

// Constructors.

// The 4D vector wedge product, which yields a bivector representing the biplanes spaned by the two vectors.
FBivector4D::FBivector4D(const FVector4& InLeftVector, const FVector4& InRightVector) :
	XY(InLeftVector.X * InRightVector.Y - InLeftVector.Y * InRightVector.X),
	XZ(InLeftVector.X * InRightVector.Z - InLeftVector.Z * InRightVector.X),
	XW(InLeftVector.X * InRightVector.W - InLeftVector.W * InRightVector.X),
	YZ(InLeftVector.Y * InRightVector.Z - InLeftVector.Z * InRightVector.Y),
	YW(InLeftVector.Y * InRightVector.W - InLeftVector.W * InRightVector.Y),
	ZW(InLeftVector.Z * InRightVector.W - InLeftVector.W * InRightVector.Z) {
}
