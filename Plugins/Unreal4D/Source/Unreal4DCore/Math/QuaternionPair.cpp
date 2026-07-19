#include "QuaternionPair.h"

#include "Bivector4D.h"
#include "Basis4D.h"

#include "GenericPlatform/GenericPlatformMath.h"
#include "Math/UnrealMath.h"

const FQuaternionPair FQuaternionPair::Identity(FQuat::Identity, FQuat::Identity);

// This is just part of the algorithm for turning matrices to QuatPairs.
FQuaternionPair FQuaternionPair::GivensRotate(FMatrix& OutMatrix, const int InAxis1, const int InAxis2, const FBivector4D InBivector)
{
	double a = OutMatrix.M[InAxis1][InAxis1];
	double b = OutMatrix.M[InAxis1][InAxis2];
	double angle = -FMath::Atan2(b, a);
	double cosAngle = FMath::Cos(angle);
	double sinAngle = FMath::Sin(angle);
	for (int i = 0; i < 4; i++)
	{
		double row1 = OutMatrix.M[i][InAxis1];
		double row2 = OutMatrix.M[i][InAxis2];
		OutMatrix.M[i][InAxis1] = cosAngle * row1 - sinAngle * row2;
		OutMatrix.M[i][InAxis2] = cosAngle * row2 + sinAngle * row1;
	}
	return FQuaternionPair(InBivector, angle);
}

FQuaternionPair::FQuaternionPair(const FMatrix& InMatrix)
{
	// Perform a QR decomposition using Givens rotations. This is a somewhat expensive algorithm, but at least it's quite numerically stable.
	*this = Identity;
	FMatrix matrix = InMatrix;

	// invariant: this->ToMatrix() * matrix = InMatrix (where the product is in normal matrix product order, not Unreal matrix product order).
	*this *= GivensRotate(matrix, 0, 1, FBivector4D(1.0, 0.0, 0.0, 0.0, 0.0, 0.0));
	*this *= GivensRotate(matrix, 0, 2, FBivector4D(0.0, 1.0, 0.0, 0.0, 0.0, 0.0));
	*this *= GivensRotate(matrix, 0, 3, FBivector4D(0.0, 0.0, 1.0, 0.0, 0.0, 0.0));
	*this *= GivensRotate(matrix, 1, 2, FBivector4D(0.0, 0.0, 0.0, 1.0, 0.0, 0.0));
	*this *= GivensRotate(matrix, 1, 3, FBivector4D(0.0, 0.0, 0.0, 0.0, 1.0, 0.0));
	*this *= GivensRotate(matrix, 2, 3, FBivector4D(0.0, 0.0, 0.0, 0.0, 0.0, 1.0));

	// The remaining data in matrix is now an upper-triangular matrix representing all the scaling and shearing of InMatrix.
	// If InMatrix was orthonormal, then the remainder will be an identity matrix (apart from rounding errors).
	// Since we're actually looking for the rotational part, this is discarded.
}

double FQuaternionPair::Size() const
{
	return FMath::Sqrt(Dot(*this));
}

double FQuaternionPair::Distance(const FQuaternionPair& InQuaternionPair) const
{
	return FQuaternionPair(Left - InQuaternionPair.Left, Right - InQuaternionPair.Right).Size();
}

// The dot product of a normalized QuatPair with itself is 2, not 1 as might be expected.
double FQuaternionPair::Dot(const FQuaternionPair& InQuaternionPair) const
{
	return (Left | InQuaternionPair.Left) + (Right | InQuaternionPair.Right);
}

double FQuaternionPair::ScaleFactor() const
{
	return Left.Size() * Right.Size();
}

void FQuaternionPair::Normalize(const double InTolerance)
{
	Left.Normalize(InTolerance);
	Right.Normalize(InTolerance);
}

FQuaternionPair FQuaternionPair::GetNormalized(const double InTolerance) const
{
	return FQuaternionPair(Left.GetNormalized(InTolerance), Right.GetNormalized(InTolerance));
}

FMatrix FQuaternionPair::ToMatrix() const
{
	return UBasis4D::Basis4D_FromVectors(
			*this * FVector4(1.0, 0.0, 0.0, 0.0),
			*this * FVector4(0.0, 1.0, 0.0, 0.0),
			*this * FVector4(0.0, 0.0, 1.0, 0.0),
			*this * FVector4(0.0, 0.0, 0.0, 1.0));
}

FQuaternionPair FQuaternionPair::Inverse() const
{
	return FQuaternionPair(Left.Inverse(), Right.Inverse());
}

double FQuaternionPair::GetLargerAngle() const
{
	double largerAngle;
	double smallerAngle;
	GetAngles(largerAngle, smallerAngle);
	return largerAngle;
}

void FQuaternionPair::GetAngles(double& OutLargerAngle, double& OutSmallerAngle) const
{
	Log().Eigenvalues(OutLargerAngle, OutSmallerAngle);
}

FBivector4D FQuaternionPair::MainBiplane() const
{
	return Log().MainBiplane();
}

void FQuaternionPair::ToPlanesAndAngles(FBivector4D& OutMainBiplane, double& OutMainAngle, FBivector4D& OutMinorBiplane, double& OutMinorAngle) const
{
	const FBivector4D log = Log();
	OutMainBiplane = log.MainBiplane();
	OutMinorBiplane = OutMainBiplane.Dual();
	OutMainAngle = log | OutMainBiplane;
	OutMinorAngle = log | OutMinorBiplane;
	if (OutMinorAngle < 0) {
		OutMinorBiplane *= -1;
		OutMinorAngle *= -1;
	}
}

FBivector4D FQuaternionPair::Log() const
{
	// Since B and -B represent the same rotation, -B may be used instead if its logarithm is smaller.
	const double flipSign = (Left.W + Right.W < 0.0) ? -1.0 : 1.0;
	// Ideally a more stable algorithm for the log of a single quaternion would be used,
	// but if that's changed the change may as well be made to TQuat::Log rather than
	// having a different version exclusively for FQuaternionPair::Log.
	const FQuat leftLog = (Left * flipSign).Log();
	const FQuat rightLog = (Right * flipSign).Log();
	return FBivector4D(
			-leftLog.Z + rightLog.Z, // XY
			+leftLog.Y - rightLog.Y, // XZ
			+leftLog.X + rightLog.X, // XW
			-leftLog.X + rightLog.X, // YZ
			+leftLog.Y + rightLog.Y, // YW
			+leftLog.Z + rightLog.Z  // ZW
	);
}

// This implementation is numerically unstable in the case where the difference
// between the QuatPairs is very close to a double 180° rotation. I'm not sure
// it's worth the extra complexity to deal with that rare case carefully though.
FQuaternionPair FQuaternionPair::Slerp(const FQuaternionPair& InQuatPairA, const FQuaternionPair& InQuatPairB, const double InFactor)
{
	const double flipSign = ((InQuatPairA | InQuatPairB) < 0.0) ? -1.0 : 1.0;
	const FQuat left = FQuat::SlerpFullPath(InQuatPairA.Left, InQuatPairB.Left * flipSign, InFactor);
	const FQuat right = FQuat::SlerpFullPath(InQuatPairA.Right, InQuatPairB.Right * flipSign, InFactor);
	return FQuaternionPair(left, right);
}

// Operators.

FQuaternionPair FQuaternionPair::operator*(const FQuaternionPair& InQuaternionPair) const
{
	return FQuaternionPair(Left * InQuaternionPair.Left, InQuaternionPair.Right * Right);
}

FQuaternionPair FQuaternionPair::operator*=(const FQuaternionPair& InQuaternionPair)
{
	Left *= InQuaternionPair.Left;
	Right = InQuaternionPair.Right * Right;
	return *this;
}

FVector4 FQuaternionPair::operator*(const FVector4& V) const
{
	const FQuat result = Left * FQuat(V.X, V.Y, V.Z, V.W) * Right;
	return FVector4(result.X, result.Y, result.Z, result.W);
}

// The dot product of a normalized QuatPair with itself is 2, not 1 as might be expected.
double FQuaternionPair::operator|(const FQuaternionPair& InQuaternionPair) const
{
	return (Left | InQuaternionPair.Left) + (Right | InQuaternionPair.Right);
}

// Constructors.

// It would be more efficient to implement these directly rather than using the bivector constructor,
// but this makes the code much simpler. We can do the optimization later if necessary.
FQuaternionPair FQuaternionPair::FromXY(const double InAngle)
{
	return FQuaternionPair(FBivector4D(-1.0, 0.0, 0.0, 0.0, 0.0, 0.0), InAngle);
}

FQuaternionPair FQuaternionPair::FromXZ(const double InAngle)
{
	return FQuaternionPair(FBivector4D(0.0, -1.0, 0.0, 0.0, 0.0, 0.0), InAngle);
}

FQuaternionPair FQuaternionPair::FromXW(const double InAngle)
{
	return FQuaternionPair(FBivector4D(0.0, 0.0, -1.0, 0.0, 0.0, 0.0), InAngle);
}

FQuaternionPair FQuaternionPair::FromYZ(const double InAngle)
{
	return FQuaternionPair(FBivector4D(0.0, 0.0, 0.0, -1.0, 0.0, 0.0), InAngle);
}

FQuaternionPair FQuaternionPair::FromYW(const double InAngle)
{
	return FQuaternionPair(FBivector4D(0.0, 0.0, 0.0, 0.0, -1.0, 0.0), InAngle);
}

FQuaternionPair FQuaternionPair::FromZW(const double InAngle)
{
	return FQuaternionPair(FBivector4D(0.0, 0.0, 0.0, 0.0, 0.0, -1.0), InAngle);
}

FQuaternionPair FQuaternionPair::FromYX(const double InAngle)
{
	return FromXY(-InAngle);
}

FQuaternionPair FQuaternionPair::FromZX(const double InAngle)
{
	return FromXZ(-InAngle);
}

FQuaternionPair FQuaternionPair::FromZY(const double InAngle)
{
	return FromYZ(-InAngle);
}

FQuaternionPair FQuaternionPair::FromWX(const double InAngle)
{
	return FromXW(-InAngle);
}

FQuaternionPair FQuaternionPair::FromWY(const double InAngle)
{
	return FromYW(-InAngle);
}

FQuaternionPair FQuaternionPair::FromWZ(const double InAngle)
{
	return FromZW(-InAngle);
}

FQuaternionPair::FQuaternionPair(const FQuat& InQuaternion)
	: Left(InQuaternion), Right(InQuaternion.Inverse()) {}

FQuaternionPair::FQuaternionPair(const FBivector4D& InPlane, const double InAngleRad)
{
	*this = (InPlane.Normalized() * InAngleRad).Exp();
}
