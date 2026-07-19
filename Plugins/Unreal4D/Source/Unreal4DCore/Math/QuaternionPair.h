#pragma once

#include "Math/UnrealMath.h"

#include "Bivector4D.h"

#include "QuaternionPair.generated.h"

/**
 * A QuaternionPair represents a 4D rotation like a Quat represents
 * a 3D one. Just like a quaternion, you shouldn't access the
 * individual components unless you're sure you know what you're
 * doing.
 */
USTRUCT(BlueprintType, Category = "Unreal 4D|Math|Quaternion Pair")
struct UNREAL4DCORE_API FQuaternionPair
{
	GENERATED_USTRUCT_BODY()

	static FQuaternionPair GivensRotate(FMatrix& OutMatrix, const int InAxis1, const int InAxis2, const FBivector4D InBivector);

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal 4D|Math|Quaternion Pair", SaveGame)
	FQuat Left; // Self-dual part.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal 4D|Math|Quaternion Pair", SaveGame)
	FQuat Right; // Anti-self-dual part.

	static const FQuaternionPair Identity;

	// This can be used for things like measuring similarity, but does not match the scaling this QuatPair applies to vectors.
	double Size() const;

	double Distance(const FQuaternionPair& InQuaternionPair) const;

	double Dot(const FQuaternionPair& InQuaternionPair) const;

	// QuaternionPairs, like Quats, can represent uniform scaling not just rotation. Usually they're normalized so this is 1 though.
	double ScaleFactor() const;

	void Normalize(const double InTolerance = UE_SMALL_NUMBER);

	FQuaternionPair GetNormalized(const double InTolerance = UE_SMALL_NUMBER) const;

	FMatrix ToMatrix() const;

	FQuaternionPair Inverse() const;

	double GetLargerAngle() const;

	void GetAngles(double& OutLargerAngle, double& OutSmallerAngle) const;

	FBivector4D MainBiplane() const;

	void ToPlanesAndAngles(FBivector4D& OutMainBiplane, double& OutMainAngle, FBivector4D& OutMinorBiplane, double& OutMinorAngle) const;

	FBivector4D Log() const;

	static FQuaternionPair Slerp(const FQuaternionPair& InQuatPairA, const FQuaternionPair& InQuatPairB, const double InFactor);

	// Operators.
	FQuaternionPair operator*(const FQuaternionPair& InQuaternionPair) const;
	FQuaternionPair operator*=(const FQuaternionPair& InQuaternionPair);
	FVector4 operator*(const FVector4& V) const;
	double operator|(const FQuaternionPair& InQuaternionPair) const;

	// Constructors.
	static FQuaternionPair FromXY(const double InAngle);
	static FQuaternionPair FromXZ(const double InAngle);
	static FQuaternionPair FromXW(const double InAngle);
	static FQuaternionPair FromYX(const double InAngle);
	static FQuaternionPair FromYZ(const double InAngle);
	static FQuaternionPair FromYW(const double InAngle);
	static FQuaternionPair FromZX(const double InAngle);
	static FQuaternionPair FromZY(const double InAngle);
	static FQuaternionPair FromZW(const double InAngle);
	static FQuaternionPair FromWX(const double InAngle);
	static FQuaternionPair FromWY(const double InAngle);
	static FQuaternionPair FromWZ(const double InAngle);

	FQuaternionPair() : Left(FQuat::Identity), Right(FQuat::Identity) {}
	FQuaternionPair(const FQuat& InLeft, const FQuat& InRight) : Left(InLeft), Right(InRight) {}

	/**
	 * The resulting QuatPair rotates the XYZ hyperplane according to the given rotation, and leaves the W axis fixed.
	 */
	FQuaternionPair(const FQuat& InQuaternion);

	/**
	 * Creates a QuatPair representing a rotation in the given plane.
	 */
	FQuaternionPair(const FBivector4D& InPlane, const double InAngleRad);

	FQuaternionPair(const FMatrix& InMatrix);
};
