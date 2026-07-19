#pragma once

#include "Basis4D.h"
#include "QuaternionPair.h"

#include "Euler4D.generated.h"

USTRUCT(BlueprintType, Category = "Unreal 4D|Math|Euler")
struct UNREAL4DCORE_API FEuler4D
{
	GENERATED_USTRUCT_BODY()

	// YZ, ZX, and XY are analogous to 3D (X, Y, and Z resp), but
	// XW, WY, and ZW are not (they are perpendicular to YZ, ZX, XY).
	// We'll put the 3D-like ones first, on top, in YZ-ZX-XY order.
	// For UX reasons, we want the 3D-like ones to be first.
	// Yes I'm aware of the irony of trying to have good UX
	// for 4D, which is inherently complex regardless.

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal 4D|Math|Euler")
	double YZ = 0.0; // 5th rotation, Roll 1, Yaw2-perpendicular

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal 4D|Math|Euler")
	double ZX = 0.0; // 4th rotation, Pitch, Yaw3-perpendicular

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal 4D|Math|Euler")
	double XY = 0.0; // 1st rotation, Yaw 1, Roll2-perpendicular

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal 4D|Math|Euler")
	double XW = 0.0; // 2nd rotation, Yaw 2, Roll1-perpendicular

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal 4D|Math|Euler")
	double WY = 0.0; // 3rd rotation, Yaw 3, Pitch-perpendicular

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal 4D|Math|Euler")
	double ZW = 0.0; // 6th rotation, Roll 2, Yaw1-perpendicular

	// Misc methods.
	FEuler4D Compose(const FEuler4D& InChild) const;
	bool IsEqualApprox(const FEuler4D& InOther) const;
	FEuler4D Lerp(const FEuler4D& InTo, const double InWeight) const;
	FMatrix RotateBasis(const FMatrix& InBasis) const;
	FVector4 RotatePoint(const FVector4& InPoint) const;
	FEuler4D RotationTo(const FEuler4D& InTo) const;
	FEuler4D Snapped(const double InStep) const;
	FEuler4D Wrapped() const;
	static FEuler4D Zero() { return FEuler4D(); }

	// Radians/degrees.
	FEuler4D DegToRad() const;
	FEuler4D RadToDeg() const;

	// Component with-style setters.
	FEuler4D WithYZ(const double InYZ) const {
		FEuler4D e = *this;
		e.YZ = InYZ;
		return e;
	}
	FEuler4D WithZX(const double InZX) const {
		FEuler4D e = *this;
		e.ZX = InZX;
		return e;
	}
	FEuler4D WithXY(const double InXY) const {
		FEuler4D e = *this;
		e.XY = InXY;
		return e;
	}
	FEuler4D WithXW(const double InXW) const {
		FEuler4D e = *this;
		e.XW = InXW;
		return e;
	}
	FEuler4D WithWY(const double InWY) const {
		FEuler4D e = *this;
		e.WY = InWY;
		return e;
	}
	FEuler4D WithZW(const double InZW) const {
		FEuler4D e = *this;
		e.ZW = InZW;
		return e;
	}

	// Operators.
	double& operator[](int32 InAxis) {
		check(static_cast<uint32>(InAxis) < 6);
		switch (InAxis)
		{
			case 0: return YZ;
			case 1: return ZX;
			case 2: return XY;
			case 3: return XW;
			case 4: return WY;
			default: return ZW;
		}
	}

	const double& operator[](int32 InAxis) const {
		check(static_cast<uint32>(InAxis) < 6);
		switch (InAxis)
		{
			case 0: return YZ;
			case 1: return ZX;
			case 2: return XY;
			case 3: return XW;
			case 4: return WY;
			default: return ZW;
		}
	}

	bool operator==(const FEuler4D& InOther) const;
	bool operator!=(const FEuler4D& InOther) const;
	FEuler4D operator-() const;

	FEuler4D& operator+=(const FEuler4D& InOther);
	FEuler4D operator+(const FEuler4D& InOther) const;
	FEuler4D& operator-=(const FEuler4D& InOther);
	FEuler4D operator-(const FEuler4D& InOther) const;

	FEuler4D& operator*=(const FEuler4D& InChild);
	FEuler4D operator*(const FEuler4D& InChild) const;
	FEuler4D& operator/=(const FEuler4D& InChild);
	FEuler4D operator/(const FEuler4D& InChild) const;

	FEuler4D& operator*=(const double InScalar);
	FEuler4D operator*(const double InScalar) const;
	FEuler4D& operator/=(const double InScalar);
	FEuler4D operator/(const double InScalar) const;

	operator FString() const;

	// Conversion.
	static FEuler4D From3D(const FRotator& InFrom3D);
	static FEuler4D From3D(const FVector& InFrom3D);
	FVector To3D() const;

	static FEuler4D FromArray(const TArray<double>& InFromArray);
	TArray<double> ToArray() const;
	static FEuler4D DegreesFromBasis(const FMatrix& InBasis, bool bInUseSpecialCases = true);
	FMatrix DegreesToBasis() const;
	static FEuler4D RadiansFromBasis(const FMatrix& InBasis, bool bInUseSpecialCases = true);
	FMatrix RadiansToBasis() const;
	static FEuler4D RadiansFromQuatPair(const FQuaternionPair& InQuatPair, bool bInUseSpecialCases = true);
	FQuaternionPair RadiansToQuatPair() const;

	// Constructors.
	FEuler4D() {}

	FEuler4D(double InYZ, double InZX, double InXY, double InXW = 0.0, double InWY = 0.0, double InZW = 0.0)
		: YZ(InYZ), ZX(InZX), XY(InXY), XW(InXW), WY(InWY), ZW(InZW)
	{
	}
};
