#include "Euler4D.h"

// FMatrix conversions. These functions follow Unreal's conventions,
// with +Z as up, +X as forward, and +Y as right.

FEuler4D FEuler4D::DegreesFromBasis(const FMatrix& InBasis, bool bInUseSpecialCases)
{
	return RadiansFromBasis(InBasis, bInUseSpecialCases).RadToDeg();
}

FMatrix FEuler4D::DegreesToBasis() const
{
	return DegToRad().RadiansToBasis();
}

FMatrix FEuler4D::RadiansToBasis() const
{
	FMatrix yzBasis = UBasis4D::Basis4D_FromYZ(YZ);
	FMatrix zxBasis = UBasis4D::Basis4D_FromZX(ZX);
	FMatrix xyBasis = UBasis4D::Basis4D_FromXY(XY);
	// It can be tricky to figure out if we should have ex: XW or WX.
	// From looking at the matrix, and trying to stay consistent with the
	// existing 3D rotations, we should use XW, WY, and ZW. The rules are
	// that adjacent axes have the upper-right sine negative (XY, YZ, ZW),
	// axes one apart have the lower-left sine negative (ZX, WY), and axes
	// two apart have the upper-right sine negative again (XW, not WX).
	// If you are curious, a similar pattern could be extrapolated to 5D etc.
	// 5D would be ZX, ZW, VZ, XW, VX, WV, YZ, XY, YV, WY.
	FMatrix xwBasis = UBasis4D::Basis4D_FromXW(XW);
	FMatrix wyBasis = UBasis4D::Basis4D_FromWY(WY);
	FMatrix zwBasis = UBasis4D::Basis4D_FromZW(ZW);

	// Here's what determines the rotation order of Euler angles.
	// For 3D, we have Yaw, Pitch, and Roll, in that order.
	//
	// Yaw acts as horizontal rotation (XY plane, no Z so it's horizontal).
	// For the 4D equivalent of Yaw, we have three angles that rotate
	// horizontally (no Z): XY, XW, and WY. Therefore, these three go first.
	//
	// Next is Pitch. Pitch is vertical rotation that changes the direction
	// an object is pointing by moving its forward vector (+X) up or down (Z),
	// so pitch only exists in the ZX plane. Therefore, for the 4D equivalent
	// of pitch, there is only one angle that does this: ZX.
	//
	// Lastly is Roll. Roll is also vertical rotation, but it brings the
	// sides of an object higher or lower. For the 4D equivalent of roll,
	// we want every angle that involves Z but not X, so that gives us
	// two angles: YZ and ZW. Therefore, these go last.
	//
	// Outside of the broad Yaw-Pitch-Roll ordering, there is a fascinating
	// symmetry that arises. If we order our rotations in the right way, we
	// can make the outer-most rotations (XY and ZW) be perpendicular,
	// the inner-most rotations (WY and ZX) be perpendicular, which makes the
	// remaining rotations (XW and YZ) also perpendicular. Pitch (ZX) must be
	// the 4th rotation so that implies XW is the 3rd rotation to obey the
	// symmetry. The 1st/2nd could be swapped if the 5th/6th are swapped,
	// so this part is arbitrary, but XY first is consistent with 3D.
	return UBasis4D::Basis4D_Compose(UBasis4D::Basis4D_Compose(
		UBasis4D::Basis4D_Compose(xyBasis, xwBasis),
		UBasis4D::Basis4D_Compose(wyBasis, zxBasis)),
		UBasis4D::Basis4D_Compose(yzBasis, zwBasis));
}

FEuler4D FEuler4D::RadiansFromBasis(const FMatrix& InBasis, const bool bInUseSpecialCases)
{
	FMatrix basis = UBasis4D::Basis4D_Orthonormalized(InBasis);

	// Stage 0: Determine if this is a special case. This is optional, for UX.
	// This ensures that single rotations in the inspector don't transform into
	// other rotations. Unreal does the same thing for 3D euler angles.
	if (bInUseSpecialCases)
	{
		if (FMath::IsNearlyZero(basis.M[1][2], UNREAL4D_COMPARISON_EPSILON) &&
		    FMath::IsNearlyZero(basis.M[2][1], UNREAL4D_COMPARISON_EPSILON) &&
		    FMath::IsNearlyZero(basis.M[2][0], UNREAL4D_COMPARISON_EPSILON) &&
		    FMath::IsNearlyZero(basis.M[0][2], UNREAL4D_COMPARISON_EPSILON) &&
		    FMath::IsNearlyZero(basis.M[0][3], UNREAL4D_COMPARISON_EPSILON) &&
		    FMath::IsNearlyZero(basis.M[3][0], UNREAL4D_COMPARISON_EPSILON) &&
		    FMath::IsNearlyZero(basis.M[3][1], UNREAL4D_COMPARISON_EPSILON) &&
		    FMath::IsNearlyZero(basis.M[1][3], UNREAL4D_COMPARISON_EPSILON))
		{
			// Special case: No YZ, ZX, XW, or WY rotations, so the only rotations may be in XY or ZW.
			return FEuler4D(0.0, 0.0, FMath::Atan2(basis.M[0][1], basis.M[0][0]), 0.0, 0.0, FMath::Atan2(basis.M[2][3], basis.M[2][2]));
		}
		if (FMath::IsNearlyZero(basis.M[2][0], UNREAL4D_COMPARISON_EPSILON) &&
		    FMath::IsNearlyZero(basis.M[0][2], UNREAL4D_COMPARISON_EPSILON) &&
		    FMath::IsNearlyZero(basis.M[0][1], UNREAL4D_COMPARISON_EPSILON) &&
		    FMath::IsNearlyZero(basis.M[1][0], UNREAL4D_COMPARISON_EPSILON) &&
		    FMath::IsNearlyZero(basis.M[3][1], UNREAL4D_COMPARISON_EPSILON) &&
		    FMath::IsNearlyZero(basis.M[1][3], UNREAL4D_COMPARISON_EPSILON) &&
		    FMath::IsNearlyZero(basis.M[2][3], UNREAL4D_COMPARISON_EPSILON) &&
		    FMath::IsNearlyZero(basis.M[3][2], UNREAL4D_COMPARISON_EPSILON))
		{
			// Special case: No ZX, XY, WY, or ZW rotations, so the only rotations may be in YZ or XW.
			return FEuler4D(FMath::Atan2(basis.M[1][2], basis.M[1][1]), 0.0, 0.0, FMath::Atan2(basis.M[0][3], basis.M[0][0]), 0.0, 0.0);
		}
		if (FMath::IsNearlyZero(basis.M[1][2], UNREAL4D_COMPARISON_EPSILON) &&
		    FMath::IsNearlyZero(basis.M[2][1], UNREAL4D_COMPARISON_EPSILON) &&
		    FMath::IsNearlyZero(basis.M[0][1], UNREAL4D_COMPARISON_EPSILON) &&
		    FMath::IsNearlyZero(basis.M[1][0], UNREAL4D_COMPARISON_EPSILON) &&
		    FMath::IsNearlyZero(basis.M[0][3], UNREAL4D_COMPARISON_EPSILON) &&
		    FMath::IsNearlyZero(basis.M[3][0], UNREAL4D_COMPARISON_EPSILON) &&
		    FMath::IsNearlyZero(basis.M[2][3], UNREAL4D_COMPARISON_EPSILON) &&
		    FMath::IsNearlyZero(basis.M[3][2], UNREAL4D_COMPARISON_EPSILON))
		{
			// Special case: No YZ, XY, XW, or ZW rotations, so the only rotations may be in ZX or WY.
			return FEuler4D(0.0, FMath::Atan2(basis.M[2][0], basis.M[2][2]), 0.0, 0.0, FMath::Atan2(basis.M[3][1], basis.M[3][3]), 0.0);
		}
	}

	// Stage 1: All the rotations involving Z can be worked out from the Z row of the matrix, which is unaffected by the later rotations.
	const double zx = -FMath::Atan2(basis.M[0][2], FMath::Sqrt(basis.M[1][2]*basis.M[1][2] + basis.M[2][2]*basis.M[2][2] + basis.M[3][2]*basis.M[3][2]));
	const double yz = FMath::Atan2(basis.M[1][2], FMath::Sqrt(basis.M[2][2]*basis.M[2][2] + basis.M[3][2]*basis.M[3][2]));
	const double zw = -FMath::Atan2(basis.M[3][2], basis.M[2][2]);

	// Rotate the Z axis back to the Z axis.
	basis = UBasis4D::Basis4D_Compose(UBasis4D::Basis4D_Compose(UBasis4D::Basis4D_Compose(basis, UBasis4D::Basis4D_FromZW(-zw)), UBasis4D::Basis4D_FromYZ(-yz)), UBasis4D::Basis4D_FromZX(-zx));

	// Stage 2: this is now just a 3D rotation in XYW.
	const double xy = FMath::Atan2(basis.M[0][1], basis.M[0][0]);
	const double xw = FMath::Atan2(basis.M[0][3], FMath::Sqrt(basis.M[0][0]*basis.M[0][0] + basis.M[0][1]*basis.M[0][1]));
	const double wy = -FMath::Atan2(basis.M[1][3], basis.M[3][3]);

	return FEuler4D(yz, zx, xy, xw, wy, zw);
}

FQuaternionPair FEuler4D::RadiansToQuatPair() const
{
	FQuaternionPair xyQuats = FQuaternionPair::FromXY(XY);
	FQuaternionPair xwQuats = FQuaternionPair::FromXW(XW);
	FQuaternionPair wyQuats = FQuaternionPair::FromWY(WY);
	FQuaternionPair zxQuats = FQuaternionPair::FromZX(ZX);
	FQuaternionPair yzQuats = FQuaternionPair::FromYZ(YZ);
	FQuaternionPair zwQuats = FQuaternionPair::FromZW(ZW);

	return xyQuats * xwQuats * wyQuats * zxQuats * yzQuats * zwQuats;
}

FEuler4D FEuler4D::RadiansFromQuatPair(const FQuaternionPair& InQuatPair, const bool bInUseSpecialCases)
{
	FVector4 X = InQuatPair * FVector4(1, 0, 0, 0);
	FVector4 Y = InQuatPair * FVector4(0, 1, 0, 0);
	FVector4 Z = InQuatPair * FVector4(0, 0, 1, 0);
	FVector4 W = InQuatPair * FVector4(0, 0, 0, 1);

	// Stage 0: Determine if this is a special case. This is optional, for UX.
	// This ensures that single rotations in the inspector don't transform into
	// other rotations. Unreal does the same thing for 3D euler angles.
	if (bInUseSpecialCases)
	{
		if ((X * FVector4(0, 0, 1, 1)).IsNearlyZero(UNREAL4D_COMPARISON_EPSILON) &&
		    (Y * FVector4(0, 0, 1, 1)).IsNearlyZero(UNREAL4D_COMPARISON_EPSILON))
		{
			// Special case: No YZ, ZX, XW, or WY rotations, so the only rotations may be in XY or ZW.
			return FEuler4D(0.0, 0.0, FMath::Atan2(X.Y, X.X), 0.0, 0.0, FMath::Atan2(Z.W, Z.Z));
		}
		if ((Y * FVector4(1, 0, 0, 1)).IsNearlyZero(UNREAL4D_COMPARISON_EPSILON) &&
		    (Z * FVector4(1, 0, 0, 1)).IsNearlyZero(UNREAL4D_COMPARISON_EPSILON))
		{
			// Special case: No ZX, XY, WY, or ZW rotations, so the only rotations may be in YZ or XW.
			return FEuler4D(FMath::Atan2(Y.Z, Y.Y), 0.0, 0.0, FMath::Atan2(X.W, X.X), 0.0, 0.0);
		}
		if ((X * FVector4(0, 1, 0, 1)).IsNearlyZero(UNREAL4D_COMPARISON_EPSILON) &&
		    (Z * FVector4(0, 1, 0, 1)).IsNearlyZero(UNREAL4D_COMPARISON_EPSILON))
		{
			// Special case: No YZ, XY, XW, or ZW rotations, so the only rotations may be in ZX or WY.
			return FEuler4D(0.0, FMath::Atan2(Z.X, Z.Z), 0.0, 0.0, FMath::Atan2(-Y.W, Y.Y), 0.0);
		}
	}

	// Stage 1: All the rotations involving Z can be worked out from the Z row of the matrix, which is unaffected by the later rotations.
	const double zx = -FMath::Atan2(X.Z, FMath::Sqrt(Y.Z * Y.Z + Z.Z * Z.Z + W.Z * W.Z));
	const double yz = FMath::Atan2(Y.Z, FMath::Sqrt(Z.Z * Z.Z + W.Z * W.Z));
	const double zw = -FMath::Atan2(W.Z, Z.Z);

	// Rotate the Z axis back to the Z axis.
	FQuaternionPair QuatPair = InQuatPair * FQuaternionPair::FromZW(-zw) * FQuaternionPair::FromYZ(-yz) * FQuaternionPair::FromZX(-zx);
	X = QuatPair * FVector4(1, 0, 0, 0);
	Y = QuatPair * FVector4(0, 1, 0, 0);
	Z = QuatPair * FVector4(0, 0, 1, 0);
	W = QuatPair * FVector4(0, 0, 0, 1);

	// Stage 2: this is now just a 3D rotation in XYW.
	const double xy = FMath::Atan2(X.Y, X.X);
	const double xw = FMath::Atan2(X.W, FMath::Sqrt(X.X * X.X + X.Y * X.Y));
	const double wy = -FMath::Atan2(Y.W, W.W);

	return FEuler4D(yz, zx, xy, xw, wy, zw);
}

// Misc methods.

FEuler4D FEuler4D::Compose(const FEuler4D& InChild) const
{
	return RadiansFromBasis(UBasis4D::Basis4D_Compose(RadiansToBasis(), InChild.RadiansToBasis()));
}

bool FEuler4D::IsEqualApprox(const FEuler4D& InOther) const
{
	return FMath::IsNearlyEqual(YZ, InOther.YZ, UNREAL4D_COMPARISON_EPSILON) &&
	       FMath::IsNearlyEqual(ZX, InOther.ZX, UNREAL4D_COMPARISON_EPSILON) &&
	       FMath::IsNearlyEqual(XY, InOther.XY, UNREAL4D_COMPARISON_EPSILON) &&
	       FMath::IsNearlyEqual(XW, InOther.XW, UNREAL4D_COMPARISON_EPSILON) &&
	       FMath::IsNearlyEqual(WY, InOther.WY, UNREAL4D_COMPARISON_EPSILON) &&
	       FMath::IsNearlyEqual(ZW, InOther.ZW, UNREAL4D_COMPARISON_EPSILON);
}

FEuler4D FEuler4D::Lerp(const FEuler4D& InTo, const double InWeight) const
{
	return FEuler4D(
		FMath::Lerp(YZ, InTo.YZ, InWeight),
		FMath::Lerp(ZX, InTo.ZX, InWeight),
		FMath::Lerp(XY, InTo.XY, InWeight),
		FMath::Lerp(XW, InTo.XW, InWeight),
		FMath::Lerp(WY, InTo.WY, InWeight),
		FMath::Lerp(ZW, InTo.ZW, InWeight));
}

FMatrix FEuler4D::RotateBasis(const FMatrix& InBasis) const
{
	return UBasis4D::Basis4D_Compose(RadiansToBasis(), InBasis);
}

FVector4 FEuler4D::RotatePoint(const FVector4& InPoint) const
{
	return RadiansToBasis().TransformFVector4(InPoint);
}

FEuler4D FEuler4D::RotationTo(const FEuler4D& InTo) const
{
	return RadiansFromBasis(UBasis4D::Basis4D_Compose(UBasis4D::Basis4D_Inverse(RadiansToBasis()), InTo.RadiansToBasis()));
}

FEuler4D FEuler4D::Snapped(const double InStep) const
{
	return FEuler4D(
			FMath::GridSnap(YZ, InStep),
			FMath::GridSnap(ZX, InStep),
			FMath::GridSnap(XY, InStep),
			FMath::GridSnap(XW, InStep),
			FMath::GridSnap(WY, InStep),
			FMath::GridSnap(ZW, InStep));
}

FEuler4D FEuler4D::Wrapped() const
{
	auto WrapAngle = [](double InAngle) -> double {
		while (InAngle > PI) InAngle -= 2.0 * PI;
		while (InAngle < -PI) InAngle += 2.0 * PI;
		return InAngle;
	};

	return FEuler4D(
			WrapAngle(YZ),
			WrapAngle(ZX),
			WrapAngle(XY),
			WrapAngle(XW),
			WrapAngle(WY),
			WrapAngle(ZW));
}

// Radians/degrees.

FEuler4D FEuler4D::DegToRad() const
{
	return *this * (PI / 180.0);
}

FEuler4D FEuler4D::RadToDeg() const
{
	return *this * (180.0 / PI);
}

// Operators.

FEuler4D& FEuler4D::operator+=(const FEuler4D& InOther)
{
	YZ += InOther.YZ;
	ZX += InOther.ZX;
	XY += InOther.XY;
	XW += InOther.XW;
	WY += InOther.WY;
	ZW += InOther.ZW;
	return *this;
}

FEuler4D FEuler4D::operator+(const FEuler4D& InOther) const
{
	return FEuler4D(YZ + InOther.YZ, ZX + InOther.ZX, XY + InOther.XY, XW + InOther.XW, WY + InOther.WY, ZW + InOther.ZW);
}

FEuler4D& FEuler4D::operator-=(const FEuler4D& InOther)
{
	YZ -= InOther.YZ;
	ZX -= InOther.ZX;
	XY -= InOther.XY;
	XW -= InOther.XW;
	WY -= InOther.WY;
	ZW -= InOther.ZW;
	return *this;
}

FEuler4D FEuler4D::operator-(const FEuler4D& InOther) const
{
	return FEuler4D(YZ - InOther.YZ, ZX - InOther.ZX, XY - InOther.XY, XW - InOther.XW, WY - InOther.WY, ZW - InOther.ZW);
}

FEuler4D& FEuler4D::operator*=(const FEuler4D& InChild)
{
	*this = RadiansFromBasis(UBasis4D::Basis4D_Compose(RadiansToBasis(), InChild.RadiansToBasis()));
	return *this;
}

FEuler4D FEuler4D::operator*(const FEuler4D& InChild) const
{
	return RadiansFromBasis(UBasis4D::Basis4D_Compose(RadiansToBasis(), InChild.RadiansToBasis()));
}

FEuler4D& FEuler4D::operator/=(const FEuler4D& InChild)
{
	*this = RadiansFromBasis(UBasis4D::Basis4D_Compose(RadiansToBasis(), UBasis4D::Basis4D_Inverse(InChild.RadiansToBasis())));
	return *this;
}

FEuler4D FEuler4D::operator/(const FEuler4D& InChild) const
{
	return RadiansFromBasis(UBasis4D::Basis4D_Compose(RadiansToBasis(), UBasis4D::Basis4D_Inverse(InChild.RadiansToBasis())));
}

FEuler4D& FEuler4D::operator*=(const double InScalar)
{
	YZ *= InScalar;
	ZX *= InScalar;
	XY *= InScalar;
	XW *= InScalar;
	WY *= InScalar;
	ZW *= InScalar;
	return *this;
}

FEuler4D FEuler4D::operator*(const double InScalar) const
{
	return FEuler4D(YZ * InScalar, ZX * InScalar, XY * InScalar, XW * InScalar, WY * InScalar, ZW * InScalar);
}

FEuler4D& FEuler4D::operator/=(const double InScalar)
{
	YZ /= InScalar;
	ZX /= InScalar;
	XY /= InScalar;
	XW /= InScalar;
	WY /= InScalar;
	ZW /= InScalar;
	return *this;
}

FEuler4D FEuler4D::operator/(const double InScalar) const
{
	return FEuler4D(YZ / InScalar, ZX / InScalar, XY / InScalar, XW / InScalar, WY / InScalar, ZW / InScalar);
}

FEuler4D FEuler4D::operator-() const
{
	return FEuler4D(-YZ, -ZX, -XY, -XW, -WY, -ZW);
}

bool FEuler4D::operator==(const FEuler4D& InOther) const
{
	return (XY == InOther.XY && ZX == InOther.ZX && XW == InOther.XW && YZ == InOther.YZ && WY == InOther.WY && ZW == InOther.ZW);
}

bool FEuler4D::operator!=(const FEuler4D& InOther) const
{
	return (YZ != InOther.YZ || ZX != InOther.ZX || XY != InOther.XY || XW != InOther.XW || WY != InOther.WY || ZW != InOther.ZW);
}

FEuler4D::operator FString() const
{
	return FString::Printf(TEXT("(YZ: %f, ZX: %f, XY: %f, XW: %f, WY: %f, ZW: %f)"),
		YZ, ZX, XY, XW, WY, ZW);
}

// Conversion.

FEuler4D FEuler4D::From3D(const FRotator& InFrom3D)
{
	return FEuler4D(InFrom3D.Roll, InFrom3D.Pitch, InFrom3D.Yaw);
}

FEuler4D FEuler4D::From3D(const FVector& InFrom3D)
{
	return FEuler4D(InFrom3D.X, InFrom3D.Y, InFrom3D.Z);
}

FVector FEuler4D::To3D() const
{
	return FVector(YZ, ZX, XY);
}

FEuler4D FEuler4D::FromArray(const TArray<double>& InFromArray)
{
	FEuler4D euler;
	for (int32 i = 0; i < FMath::Min((int32)InFromArray.Num(), 6); ++i)
	{
		(&euler.YZ)[i] = InFromArray[i];
	}
	return euler;
}

TArray<double> FEuler4D::ToArray() const
{
	TArray<double> arr;
	arr.SetNum(6);
	arr[0] = YZ;
	arr[1] = ZX;
	arr[2] = XY;
	arr[3] = XW;
	arr[4] = WY;
	arr[5] = ZW;
	return arr;
}

static_assert(sizeof(FEuler4D) == 6 * sizeof(double), "FEuler4D must be 6 doubles or 48 bytes in size");
