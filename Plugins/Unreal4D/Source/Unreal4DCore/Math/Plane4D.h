#pragma once

#include "../Unreal4DDefines.h"

#include "Plane4D.generated.h"

// Represents a 4D plane, also known as a triplane or hyperplane, whose surface is 3-dimensional.
// The name "FPlane4D" conflicts with Unreal's "FPlane4d" (4 numbers, double), so we need to
// disambiguate it with a semantically useless "hyper" to clarify that this is really 4D.
USTRUCT(BlueprintType, Category = "Unreal 4D|Math|Plane")
struct UNREAL4DCORE_API FHyperplane4D
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal 4D|Math|Plane")
	FVector4 Normal = FVector4(0.0, 0.0, 1.0, 0.0);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal 4D|Math|Plane")
	double Distance = 0.0;

	// Point functions.
	double DistanceTo(const FVector4& InPoint) const;
	bool IsPointOver(const FVector4& InPoint) const;
	bool HasPoint(const FVector4& InPoint) const;
	FVector4 Project(const FVector4& InPoint) const;

	// Plane math functions.
	FVector4 GetCenter() const { return Normal * Distance; }
	double IntersectRayFactor(const FVector4& InRayOrigin, const FVector4& InRayDirection) const;
	FVector4 IntersectRayPoint(const FVector4& InRayOrigin, const FVector4& InRayDirection) const;
	FVector4 IntersectLineSegment(const FVector4& InBegin, const FVector4& InEnd) const;
	FHyperplane4D Normalized() const;

	// Plane comparison functions.
	bool IsEqualApprox(const FHyperplane4D& InOther) const;
	bool IsEqualApproxAnySide(const FHyperplane4D& InPlane) const;
	bool IsFinite() const;

	// Operators.
	FHyperplane4D operator-() const { return FHyperplane4D(-Normal, -Distance); }
	bool operator==(const FHyperplane4D& InOther) const;
	bool operator!=(const FHyperplane4D& InOther) const;
	operator FString() const;

	FHyperplane4D() {}
	FHyperplane4D(double InX, double InY, double InZ, double InW, double InDistance = 0.0);
	FHyperplane4D(const FVector4& InNormal, double InDistance = 0.0);
	FHyperplane4D(const FVector4& InNormal, const FVector4& InPoint);
	static FHyperplane4D FromPointsFaceAwayFromOrigin(const FVector4& InPoint1, const FVector4& InPoint2, const FVector4& InPoint3, const FVector4& InPoint4);
	static FHyperplane4D FromCoplanarDirections(const FVector4& InDir1, const FVector4& InDir2, const FVector4& InDir3, const FVector4& InPoint = FVector4(0.0, 0.0, 0.0, 0.0));
};
