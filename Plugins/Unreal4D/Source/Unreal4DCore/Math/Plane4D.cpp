#include "Plane4D.h"

#include "Basis4D.h"
#include "VectorFuncs4D.h"

double FHyperplane4D::DistanceTo(const FVector4& InPoint) const
{
	return Dot4(Normal, InPoint) - Distance;
}

bool FHyperplane4D::IsPointOver(const FVector4& InPoint) const
{
	return Dot4(Normal, InPoint) > Distance;
}

bool FHyperplane4D::HasPoint(const FVector4& InPoint) const
{
	return FMath::IsNearlyZero(DistanceTo(InPoint));
}

FVector4 FHyperplane4D::Project(const FVector4& InPoint) const
{
	return InPoint - Normal * DistanceTo(InPoint);
}

double FHyperplane4D::IntersectRayFactor(const FVector4& InRayOrigin, const FVector4& InRayDirection) const
{
	const double denominator = Dot4(Normal, InRayDirection);
	if (FMath::IsNearlyZero(denominator))
	{
		UE_LOG(LogUnreal4DCore, Warning, TEXT("FHyperplane4D::IntersectRayFactor: Ray is parallel to plane. Denominator: %f"), denominator);
		return -std::numeric_limits<double>::infinity();
	}
	return (Distance - Dot4(Normal, InRayOrigin)) / denominator;
}

FVector4 FHyperplane4D::IntersectRayPoint(const FVector4& InRayOrigin, const FVector4& InRayDirection) const
{
	const double factor = IntersectRayFactor(InRayOrigin, InRayDirection);
	if (factor < 0.0)
	{
		UE_LOG(LogUnreal4DCore, Warning, TEXT("FHyperplane4D::IntersectRayPoint: Intersection is behind the ray origin. Factor: %f"), factor);
		return FVector4(NAN, NAN, NAN, NAN);
	}
	return InRayOrigin + InRayDirection * factor;
}

FVector4 FHyperplane4D::IntersectLineSegment(const FVector4& InBegin, const FVector4& InEnd) const
{
	const FVector4 direction = InEnd - InBegin;
	const double denominator = Dot4(Normal, direction);
	if (FMath::IsNearlyZero(denominator))
	{
		return FVector4(NAN, NAN, NAN, NAN);
	}
	const double factor = (Distance - Dot4(Normal, InBegin)) / denominator;
	if (factor < 0.0 || factor > 1.0)
	{
		return FVector4(NAN, NAN, NAN, NAN);
	}
	return InBegin + direction * factor;
}

FHyperplane4D FHyperplane4D::Normalized() const
{
	const double length = Normal.Size();
	if (length == 0.0)
	{
		return FHyperplane4D();
	}
	return FHyperplane4D(Normal / length, Distance / length);
}

// Plane comparison functions.

bool FHyperplane4D::IsEqualApproxAnySide(const FHyperplane4D& InPlane) const
{
	return (Normal.Equals(InPlane.Normal, UNREAL4D_COMPARISON_EPSILON) && FMath::IsNearlyEqual(Distance, InPlane.Distance, UNREAL4D_COMPARISON_EPSILON)) || (Normal.Equals(-InPlane.Normal, UNREAL4D_COMPARISON_EPSILON) && FMath::IsNearlyEqual(Distance, -InPlane.Distance, UNREAL4D_COMPARISON_EPSILON));
}

bool FHyperplane4D::IsEqualApprox(const FHyperplane4D& InPlane) const
{
	return Normal.Equals(InPlane.Normal, UNREAL4D_COMPARISON_EPSILON) && FMath::IsNearlyEqual(Distance, InPlane.Distance, UNREAL4D_COMPARISON_EPSILON);
}

// Operators.

bool FHyperplane4D::operator==(const FHyperplane4D& InOther) const
{
	return Normal == InOther.Normal && Distance == InOther.Distance;
}

bool FHyperplane4D::operator!=(const FHyperplane4D& InOther) const
{
	return Normal != InOther.Normal || Distance != InOther.Distance;
}

FHyperplane4D::operator FString() const
{
	return "[N: " + Normal.ToString() + ", D: " + FString::SanitizeFloat(Distance) + "]";
}

FHyperplane4D::FHyperplane4D(double InX, double InY, double InZ, double InW, double InDistance)
{
	Normal = FVector4(InX, InY, InZ, InW);
	Distance = InDistance;
}

FHyperplane4D::FHyperplane4D(const FVector4& InNormal, double InDistance)
{
	Normal = InNormal;
	Distance = InDistance;
}

FHyperplane4D::FHyperplane4D(const FVector4& InNormal, const FVector4& InPoint)
{
	Normal = InNormal;
	Distance = Dot4(InNormal, InPoint);
}

FHyperplane4D FHyperplane4D::FromPointsFaceAwayFromOrigin(const FVector4& InPoint1, const FVector4& InPoint2, const FVector4& InPoint3, const FVector4& InPoint4)
{
	const FVector4 added = InPoint1 + InPoint2 + InPoint3 + InPoint4;
	const FPlane4d x = InPoint2 - InPoint1;
	const FPlane4d y = InPoint3 - InPoint1;
	const FPlane4d z = InPoint4 - InPoint1;
	const FPlane4d w = added;
	FMatrix basis = FMatrix(x, y, z, w);
	basis = UBasis4D::Basis4D_Orthonormalized(basis);
	const FVector4 normal = UBasis4D::Basis4D_GetRow(basis, 3);
	const double distance = Dot4(normal, InPoint1);
	return FHyperplane4D(normal, distance);
}

FHyperplane4D FHyperplane4D::FromCoplanarDirections(const FVector4& InDir1, const FVector4& InDir2, const FVector4& InDir3, const FVector4& InPoint)
{
	FVector4 normal = UVectorFuncs4D::Vector4_Perpendicular(InDir1, InDir2, InDir3);
	normal = UVectorFuncs4D::Vector4_Normalized(normal);
	return FHyperplane4D(normal, InPoint);
}
