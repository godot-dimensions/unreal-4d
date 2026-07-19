#pragma once

#include "RaycastResult4D.h"
#include "VectorFuncs4D.h"

#include "Rect4D.generated.h"

USTRUCT(BlueprintType, Category = "Unreal 4D|Math|Rectangle")
struct UNREAL4DCORE_API FRect4D
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal 4D|Math|Rectangle")
	FVector4 Position = FVector4(0.0, 0.0, 0.0, 0.0);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal 4D|Math|Rectangle")
	FVector4 Size = FVector4(0.0, 0.0, 0.0, 0.0);

	FVector4 GetCenter() const
	{
		return Position + (Size * 0.5);
	}

	void SetCenter(const FVector4& InCenter)
	{
		Position = InCenter - (Size * 0.5);
	}

	FVector4 GetEnd() const
	{
		return Position + Size;
	}

	void SetEnd(const FVector4& InEnd)
	{
		Size = InEnd - Position;
	}

	// Basic math functions.
	double GetHypervolume() const
	{
		return Size.X * Size.Y * Size.Z * Size.W;
	}

	bool HasHypervolume() const
	{
		return Size.X > 0.0 && Size.Y > 0.0 && Size.Z > 0.0 && Size.W > 0.0;
	}

	double GetSurfaceVolume() const
	{
		return 2.0 * (Size.X * Size.Y * Size.Z + Size.X * Size.Y * Size.W + Size.X * Size.Z * Size.W + Size.Y * Size.Z * Size.W);
	}

	bool HasSurfaceVolume() const
	{
		return Size.X > 0.0 || Size.Y > 0.0 || Size.Z > 0.0 || Size.W > 0.0;
	}

	bool HasAnySize() const
	{
		return Size.X != 0.0 || Size.Y != 0.0 || Size.Z != 0.0 || Size.W != 0.0;
	}

	FRect4D Abs() const
	{
		const FVector4 sizeAbs = FVector4(FMath::Abs(Size.X), FMath::Abs(Size.Y), FMath::Abs(Size.Z), FMath::Abs(Size.W));
		const FVector4 sizeMinZero = FVector4(FMath::Min(Size.X, 0.0), FMath::Min(Size.Y, 0.0), FMath::Min(Size.Z, 0.0), FMath::Min(Size.W, 0.0));
		return FRect4D(Position + sizeMinZero, sizeAbs);
	}

	FVector4 GetLongestAxis() const;
	FVector4 GetLongestAxisDirection() const;
	EAxis4D GetLongestAxisIndex() const;
	double GetLongestAxisSize() const;

	FVector4 GetShortestAxis() const;
	FVector4 GetShortestAxisDirection() const;
	EAxis4D GetShortestAxisIndex() const;
	double GetShortestAxisSize() const;

	// Point functions.
	FVector4 GetNearestPoint(const FVector4& InPoint) const;
	FRect4D ExpandToPoint(const FVector4& InPoint) const;
	void ExpandSelfToPoint(const FVector4& InPoint);
	bool HasPoint(const FVector4& InPoint) const;
	FVector4 GetSupportPoint(const FVector4& InDirection) const;

	// Rect math functions.
	FRect4D Grow(const double InBy) const;
	FRect4D Intersection(const FRect4D& InOther) const;
	FRect4D Merge(const FRect4D& InOther) const;

	// Rect collision functions.
	double ContinuousCollisionDepth(const FVector4& InRelativeVelocity, const FRect4D& InObstacle, FVector4* OutNormal = nullptr) const;
	bool ContinuousCollisionOverlaps(const FVector4& InRelativeVelocity, const FRect4D& InObstacle) const;

	bool RaycastIntersectsInfiniteFast(const FVector4& InRayOrigin, const FVector4& InRayDirection, const bool bInInsideIsZero, double *OutDistance, FVector4* OutNormal) const;
	FRaycastResult4D RaycastIntersectsFull(const FVector4& InRayOrigin, const FVector4& InRayDirection, const double InMaxDistance = 1e300, const bool bInInsideIsZero = false) const;

	// Rect comparison functions.
	bool EnclosesExclusive(const FRect4D& InOther) const;
	bool EnclosesInclusive(const FRect4D& InOther) const;
	bool IntersectsExclusive(const FRect4D& InOther) const;
	bool IntersectsInclusive(const FRect4D& InOther) const;
	bool IsEqualApprox(const FRect4D& InOther) const;
	bool IsFinite() const;

	// Operators.
	bool operator==(const FRect4D& InOther) const;
	bool operator!=(const FRect4D& InOther) const;
	operator FString() const;

	// Constructors.
	FRect4D() {}
	FRect4D(const FVector4& InPos, const FVector4& InSize) :
			Position(InPos),
			Size(InSize) {}

	FRect4D(const double InPosX, const double InPosY, const double InPosZ, const double InPosW, const double InSizeX, const double InSizeY, const double InSizeZ, const double InSizeW) :
			Position(InPosX, InPosY, InPosZ, InPosW),
			Size(InSizeX, InSizeY, InSizeZ, InSizeW) {}
};
