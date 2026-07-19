#pragma once

#include "Math/UnrealMath.h"

#include "Bivector4D.generated.h"

struct FQuaternionPair;

USTRUCT(BlueprintType, Category = "Unreal 4D|Math|Bivector")
struct UNREAL4DCORE_API FBivector4D
{
	GENERATED_USTRUCT_BODY()

public:
	// The component XY sends Y to X when the bivector is considered as a transform (because (X ^ Y) Y = X).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal 4D|Math|Bivector", SaveGame)
	double XY;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal 4D|Math|Bivector", SaveGame)
	double XZ;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal 4D|Math|Bivector", SaveGame)
	double XW;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal 4D|Math|Bivector", SaveGame)
	double YZ;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal 4D|Math|Bivector", SaveGame)
	double YW;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal 4D|Math|Bivector", SaveGame)
	double ZW;

	FMatrix ToMatrix() const;

	double Dot(const FBivector4D& InBivector) const;

	/**
	 * The dual is a linear function, consistent with rotations, such that
	 * B.Dual().Dual() == B, and if B is planar, B and Dual(B) are perpendicular.
	 * Also, (U^V).Dual()*W is the cross product of U,V and W.
	 */
	FBivector4D Dual() const;

	/**
	 * The main way of converting a bivector to a QuatPair. If something rotates
	 * at a rate of B rad/s for t seconds, the total rotation is (B*t).Exp().
	 */
	FQuaternionPair Exp() const;

	/**
	 * B.BiplanarComponent() is the bivector closest to B which can be
	 * expressed as V^U for some vectors V,U. An almost equivalent condition
	 * is that if V1 and V2 are both perpendicular to both of U1 and U2, and
	 * V1^V2 is larger than U1^U2, then (V1^V2 + U1^U2).BiplanarComponent() = V1^V2.
	 */
	FBivector4D BiplanarComponent() const;

	/**
	 * Equivalent to BiplanarComponent().Normalized(), but more efficient,
	 * the unit simple bivector with the greatest alignment with the input.
	 */
	FBivector4D MainBiplane() const;

	/**
	 * B.LargestAxis() is a unit vector V such that B*V is as large as possible.
	 * If V^U is non-zero, (V^U).LargestAxis() is in the plane containing V and U.
	 */
	FVector4 LargestAxis() const;

	/**
	 * The eigenvalues of the corresponding matrix are these two numbers times ±i.
	 * hypot(larger,smaller) is the size of the bivector.
	 */
	void Eigenvalues(double& OutLargerEigenvalue, double& OutSmallerEigenvalue) const;

	double Size() const;

	double SizeSquared() const;

	FBivector4D Normalized(double InTolerance = UE_SMALL_NUMBER) const;

	void Normalize(double InTolerance = UE_SMALL_NUMBER);

	// Operators.
	bool operator==(const FBivector4D& InBivector) const;
	FBivector4D operator*(const double InScale) const;
	FBivector4D operator*=(const double InScale);
	FBivector4D operator/(const double InScale) const;
	FBivector4D operator/=(const double InScale);
	FBivector4D operator+(const FBivector4D& InBivector) const;
	FBivector4D operator-(const FBivector4D& InBivector) const;
	FBivector4D operator-() const;
	FVector4 operator*(const FVector4& InVector) const;
	// Like a matrix product, but only the antisymmetric part.
	FBivector4D operator*(const FBivector4D& InBivector) const;
	FBivector4D operator*=(const FBivector4D& InBivector);
	// A dot product.
	double operator|(const FBivector4D& InBivector) const;

	// Constructors.
	FBivector4D() : XY(0.0), XZ(0.0), XW(0.0), YZ(0.0), YW(0.0), ZW(0.0) {}

	FBivector4D(double InXY, double InXZ, double InXW, double InYZ, double InYW, double InZW) :
		XY(InXY), XZ(InXZ), XW(InXW), YZ(InYZ), YW(InYW), ZW(InZW) {}

	FBivector4D(const FVector4& InLeftVector, const FVector4& InRightVector);
};

// These aren't member functions because their first arguments are vectors and I'm trying to avoid having to modify Unreal Engine code.

FVector4d UNREAL4DCORE_API operator*(const FVector4& InVector, const FBivector4D& InBivector);
