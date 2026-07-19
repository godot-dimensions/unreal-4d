#include "Rect4D.h"

// Basic math functions.

FVector4 FRect4D::GetLongestAxis() const
{
	FVector4 longestAxis = FVector4(Size.X, 0, 0, 0);
	double longestSize = Size.X;
	if (Size.Y > longestSize)
	{
		longestSize = Size.Y;
		longestAxis = FVector4(0, Size.Y, 0, 0);
	}
	if (Size.Z > longestSize)
	{
		longestSize = Size.Z;
		longestAxis = FVector4(0, 0, Size.Z, 0);
	}
	if (Size.W > longestSize)
	{
		longestAxis = FVector4(0, 0, 0, Size.W);
	}
	return longestAxis;
}

FVector4 FRect4D::GetLongestAxisDirection() const
{
	FVector4 longestAxis = FVector4(1, 0, 0, 0);
	double longestSize = Size.X;
	if (Size.Y > longestSize)
	{
		longestSize = Size.Y;
		longestAxis = FVector4(0, 1, 0, 0);
	}
	if (Size.Z > longestSize)
	{
		longestSize = Size.Z;
		longestAxis = FVector4(0, 0, 1, 0);
	}
	if (Size.W > longestSize)
	{
		longestAxis = FVector4(0, 0, 0, 1);
	}
	return longestAxis;
}

EAxis4D FRect4D::GetLongestAxisIndex() const
{
	EAxis4D longestAxis = EAxis4D::X;
	double longestSize = Size.X;
	if (Size.Y > longestSize)
	{
		longestSize = Size.Y;
		longestAxis = EAxis4D::Y;
	}
	if (Size.Z > longestSize)
	{
		longestSize = Size.Z;
		longestAxis = EAxis4D::Z;
	}
	if (Size.W > longestSize)
	{
		longestAxis = EAxis4D::W;
	}
	return longestAxis;
}

double FRect4D::GetLongestAxisSize() const
{
	double longestSize = Size.X;
	if (Size.Y > longestSize)
	{
		longestSize = Size.Y;
	}
	if (Size.Z > longestSize)
	{
		longestSize = Size.Z;
	}
	if (Size.W > longestSize)
	{
		longestSize = Size.W;
	}
	return longestSize;
}

FVector4 FRect4D::GetShortestAxis() const
{
	FVector4 shortestAxis = FVector4(Size.X, 0, 0, 0);
	double shortestSize = Size.X;
	if (Size.Y < shortestSize)
	{
		shortestSize = Size.Y;
		shortestAxis = FVector4(0, Size.Y, 0, 0);
	}
	if (Size.Z < shortestSize)
	{
		shortestSize = Size.Z;
		shortestAxis = FVector4(0, 0, Size.Z, 0);
	}
	if (Size.W < shortestSize)
	{
		shortestAxis = FVector4(0, 0, 0, Size.W);
	}
	return shortestAxis;
}

FVector4 FRect4D::GetShortestAxisDirection() const
{
	FVector4 shortestAxis = FVector4(1, 0, 0, 0);
	double shortestSize = Size.X;
	if (Size.Y < shortestSize)
	{
		shortestSize = Size.Y;
		shortestAxis = FVector4(0, 1, 0, 0);
	}
	if (Size.Z < shortestSize)
	{
		shortestSize = Size.Z;
		shortestAxis = FVector4(0, 0, 1, 0);
	}
	if (Size.W < shortestSize)
	{
		shortestAxis = FVector4(0, 0, 0, 1);
	}
	return shortestAxis;
}

EAxis4D FRect4D::GetShortestAxisIndex() const
{
	EAxis4D shortestAxis = EAxis4D::X;
	double shortestSize = Size.X;
	if (Size.Y < shortestSize)
	{
		shortestSize = Size.Y;
		shortestAxis = EAxis4D::Y;
	}
	if (Size.Z < shortestSize)
	{
		shortestSize = Size.Z;
		shortestAxis = EAxis4D::Z;
	}
	if (Size.W < shortestSize)
	{
		shortestAxis = EAxis4D::W;
	}
	return shortestAxis;
}

double FRect4D::GetShortestAxisSize() const
{
	double shortestSize = Size.X;
	if (Size.Y < shortestSize)
	{
		shortestSize = Size.Y;
	}
	if (Size.Z < shortestSize)
	{
		shortestSize = Size.Z;
	}
	if (Size.W < shortestSize)
	{
		shortestSize = Size.W;
	}
	return shortestSize;
}

// Point functions.
FVector4 FRect4D::GetNearestPoint(const FVector4& InPoint) const
{
	if (unlikely(Size.X < 0.0 || Size.Y < 0.0 || Size.Z < 0.0 || Size.W < 0.0))
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("FRect4D.GetNearestPoint: FRect4D size is negative, this is not supported. Use FRect4D.Abs() to get a FRect4D with a positive size."));
	}
	FVector4 end = GetEnd();
	FVector4 closest = InPoint;
	if (InPoint.X < Position.X)
	{
		closest.X = Position.X;
	}
	else if (InPoint.X > end.X)
	{
		closest.X = end.X;
	}
	if (InPoint.Y < Position.Y)
	{
		closest.Y = Position.Y;
	}
	else if (InPoint.Y > end.Y)
	{
		closest.Y = end.Y;
	}
	if (InPoint.Z < Position.Z)
	{
		closest.Z = Position.Z;
	}
	else if (InPoint.Z > end.Z)
	{
		closest.Z = end.Z;
	}
	if (InPoint.W < Position.W)
	{
		closest.W = Position.W;
	}
	else if (InPoint.W > end.W)
	{
		closest.W = end.W;
	}
	return closest;
}

FRect4D FRect4D::ExpandToPoint(const FVector4& InPoint) const
{
	FVector4 end = GetEnd();
	FRect4D newRect = *this;
	if (InPoint.X < Position.X)
	{
		newRect.Position.X = InPoint.X;
	}
	else if (InPoint.X > end.X)
	{
		end.X = InPoint.X;
	}
	if (InPoint.Y < Position.Y)
	{
		newRect.Position.Y = InPoint.Y;
	}
	else if (InPoint.Y > end.Y)
	{
		end.Y = InPoint.Y;
	}
	if (InPoint.Z < Position.Z)
	{
		newRect.Position.Z = InPoint.Z;
	}
	else if (InPoint.Z > end.Z)
	{
		end.Z = InPoint.Z;
	}
	if (InPoint.W < Position.W)
	{
		newRect.Position.W = InPoint.W;
	}
	else if (InPoint.W > end.W)
	{
		end.W = InPoint.W;
	}
	newRect.SetEnd(end);
	return newRect;
}

void FRect4D::ExpandSelfToPoint(const FVector4& InPoint)
{
	FVector4 end = GetEnd();
	if (InPoint.X < Position.X)
	{
		Position.X = InPoint.X;
	}
	else if (InPoint.X > end.X)
	{
		end.X = InPoint.X;
	}
	if (InPoint.Y < Position.Y)
	{
		Position.Y = InPoint.Y;
	}
	else if (InPoint.Y > end.Y)
	{
		end.Y = InPoint.Y;
	}
	if (InPoint.Z < Position.Z)
	{
		Position.Z = InPoint.Z;
	}
	else if (InPoint.Z > end.Z)
	{
		end.Z = InPoint.Z;
	}
	if (InPoint.W < Position.W)
	{
		Position.W = InPoint.W;
	}
	else if (InPoint.W > end.W)
	{
		end.W = InPoint.W;
	}
	SetEnd(end);
}

bool FRect4D::HasPoint(const FVector4& InPoint) const
{
	return InPoint.X >= Position.X && InPoint.X <= Position.X + Size.X &&
			InPoint.Y >= Position.Y && InPoint.Y <= Position.Y + Size.Y &&
			InPoint.Z >= Position.Z && InPoint.Z <= Position.Z + Size.Z &&
			InPoint.W >= Position.W && InPoint.W <= Position.W + Size.W;
}

FVector4 FRect4D::GetSupportPoint(const FVector4& InDirection) const
{
	FVector4 support = Position;
	if (InDirection.X > 0.0)
	{
		support.X += Size.X;
	}
	if (InDirection.Y > 0.0)
	{
		support.Y += Size.Y;
	}
	if (InDirection.Z > 0.0)
	{
		support.Z += Size.Z;
	}
	if (InDirection.W > 0.0)
	{
		support.W += Size.W;
	}
	return support;
}

// Rect math functions.
FRect4D FRect4D::Grow(const double InBy) const
{
	FRect4D newRect = *this;
	newRect.Position -= FVector4(InBy, InBy, InBy, InBy);
	newRect.Size += FVector4(InBy, InBy, InBy, InBy) * 2.0;
	return newRect;
}

FRect4D FRect4D::Intersection(const FRect4D& InOther) const
{
	FVector4 end = GetEnd();
	FVector4 otherEnd = InOther.GetEnd();
	FRect4D newRect = *this;
	if (Position.X > otherEnd.X || end.X < InOther.Position.X ||
			Position.Y > otherEnd.Y || end.Y < InOther.Position.Y ||
			Position.Z > otherEnd.Z || end.Z < InOther.Position.Z ||
			Position.W > otherEnd.W || end.W < InOther.Position.W)
	{
		// No intersection.
		return FRect4D();
	}
	if (InOther.Position.X > Position.X)
	{
		newRect.Position.X = InOther.Position.X;
	}
	if (InOther.Position.Y > Position.Y)
	{
		newRect.Position.Y = InOther.Position.Y;
	}
	if (InOther.Position.Z > Position.Z)
	{
		newRect.Position.Z = InOther.Position.Z;
	}
	if (InOther.Position.W > Position.W)
	{
		newRect.Position.W = InOther.Position.W;
	}
	if (otherEnd.X < end.X)
	{
		end.X = otherEnd.X;
	}
	if (otherEnd.Y < end.Y)
	{
		end.Y = otherEnd.Y;
	}
	if (otherEnd.Z < end.Z)
	{
		end.Z = otherEnd.Z;
	}
	if (otherEnd.W < end.W)
	{
		end.W = otherEnd.W;
	}
	newRect.SetEnd(end);
	return newRect;
}

FRect4D FRect4D::Merge(const FRect4D& InOther) const
{
	FVector4 end = GetEnd();
	FVector4 otherEnd = InOther.GetEnd();
	FRect4D newRect = *this;
	if (InOther.Position.X < Position.X)
	{
		newRect.Position.X = InOther.Position.X;
	}
	if (InOther.Position.Y < Position.Y)
	{
		newRect.Position.Y = InOther.Position.Y;
	}
	if (InOther.Position.Z < Position.Z)
	{
		newRect.Position.Z = InOther.Position.Z;
	}
	if (InOther.Position.W < Position.W)
	{
		newRect.Position.W = InOther.Position.W;
	}
	if (otherEnd.X > end.X)
	{
		end.X = otherEnd.X;
	}
	if (otherEnd.Y > end.Y)
	{
		end.Y = otherEnd.Y;
	}
	if (otherEnd.Z > end.Z)
	{
		end.Z = otherEnd.Z;
	}
	if (otherEnd.W > end.W)
	{
		end.W = otherEnd.W;
	}
	newRect.SetEnd(end);
	return newRect;
}

// Note: This function can return values outside of the expected range,
// including values above 1.0, below 0.0, and even below -1.0.
// Handling the meaning of such values is up to the caller.
// Suggestion: For the expected use case of loops, start with `double ratio = 1.0;`
// and then use `ratio = FMath::Min(ratio, result);` on each loop iteration.
double FRect4D::ContinuousCollisionDepth(const FVector4& InRelativeMotion, const FRect4D& InObstacle, FVector4* OutNormal) const
{
	const FVector4 end = GetEnd();
	FVector4 lowRatios = (InObstacle.Position - end) / (InRelativeMotion);
	FVector4 highRatios = (InObstacle.GetEnd() - Position) / (InRelativeMotion);
	for (int i = 0; i < 4; i++)
	{
		if (InRelativeMotion[i] < 0.0)
		{
			Swap(lowRatios[i], highRatios[i]);
		}
	}
	const double lowRatio = FMath::Max(FMath::Max(lowRatios.X, lowRatios.Y), FMath::Max(lowRatios.Z, lowRatios.W));
	const double highRatio = FMath::Min(FMath::Min(highRatios.X, highRatios.Y), FMath::Min(highRatios.Z, highRatios.W));
	if (lowRatio < highRatio)
	{
		// These checks handles depenetration, choosing the quickest way out of the obstacle.
		if (highRatio > 0.0 && FMath::Abs(lowRatio) < highRatio)
		{
			// Ok, we know there is a collision and we want to return it.
			// But first we need to set the normal if the caller requested it.
			if (OutNormal)
			{
				FVector4 normal = FVector4(0.0, 0.0, 0.0, 0.0);
				EAxis4D axis = UVectorFuncs4D::Vector4_MaxAxisIndex(lowRatios);
				normal[(int32)axis] = InRelativeMotion[(int32)axis] < 0.0 ? 1.0 : -1.0;
				(*OutNormal) = normal;
			}
			return lowRatio;
		}
	}
	if (OutNormal)
	{
		*OutNormal = FVector4(0.0, 0.0, 0.0, 0.0);
	}
	return 1.0;
}

bool FRect4D::ContinuousCollisionOverlaps(const FVector4& InRelativeMotion, const FRect4D& InObstacle) const
{
	const FVector4 end = GetEnd();
	FVector4 lowRatios = (InObstacle.Position - end) / (InRelativeMotion);
	FVector4 highRatios = (InObstacle.GetEnd() - Position) / (InRelativeMotion);
	for (int i = 0; i < 4; i++)
	{
		if (InRelativeMotion[i] < 0.0)
		{
			Swap(lowRatios[i], highRatios[i]);
		}
	}
	const double lowRatio = FMath::Max(FMath::Max(lowRatios.X, lowRatios.Y), FMath::Max(lowRatios.Z, lowRatios.W));
	const double highRatio = FMath::Min(FMath::Min(highRatios.X, highRatios.Y), FMath::Min(highRatios.Z, highRatios.W));
	if (lowRatio < highRatio && lowRatio < 1.0 && highRatio > 0.0)
	{
		return true;
	}
	return false;
}

bool FRect4D::RaycastIntersectsInfiniteFast(const FVector4& InRayOrigin, const FVector4& InRayDirection, const bool bInInsideIsZero, double *OutDistance, FVector4* OutNormal) const
{
	if (!FMath::IsNearlyEqual(InRayDirection.SizeSquared(), 1.0))
	{
		// Keep the name of the function in the error log generic because this function is called by RaycastIntersectsFull too.
		UE_LOG(LogUnreal4DCore, Error, TEXT("FRect4D::RaycastIntersects: Ray direction must be normalized."));
		return false;
	}
	if (bInInsideIsZero && HasPoint(InRayOrigin))
	{
		// The ray starts inside the box, so we can return a distance of 0.0f.
		if (OutDistance != nullptr)
		{
			*OutDistance = 0.0;
		}
		if (OutNormal != nullptr)
		{
			*OutNormal = FVector4(0.0, 0.0, 0.0, 0.0);
		}
		return true;
	}
	const FVector4 end = GetEnd();
	// NOTE: These are the box overlap distances along the ray, used for intermediate calculations. These are not
	// the same as InMaxDistance in the RaycastIntersectsFull function, which is a limit on how far the ray can go.
	double distanceMin = -std::numeric_limits<double>::infinity();
	double distanceMax = std::numeric_limits<double>::infinity();
	uint8 enterAxis = (uint8)0;
	uint8 exitAxis = (uint8)0;
	for (uint8 i = (uint8)0; i < (uint8)4; i++)
	{
		if (InRayDirection[i] == 0.0)
		{
			// Ray is parallel in this axis, so there is no casting: just check if the from point is inside the box in this axis.
			if (InRayOrigin[i] < Position[i] || InRayOrigin[i] > end[i])
			{
				return false;
			}
		}
		else
		{
			double lowRatio = (Position[i] - InRayOrigin[i]) / InRayDirection[i];
			double highRatio = (end[i] - InRayOrigin[i]) / InRayDirection[i];
			// Ray is not parallel in this axis, so we can calculate the intersection distances.
			if (lowRatio > highRatio)
			{
				// Swap the ratios if they are in the wrong order.
				Swap(lowRatio, highRatio);
			}
			if (lowRatio > distanceMin)
			{
				distanceMin = lowRatio;
				enterAxis = i;
			}
			if (highRatio < distanceMax)
			{
				if (highRatio < 0.0)
				{
					// The ray is pointing away from the box, so it will never hit.
					return false;
				}
				distanceMax = highRatio;
				exitAxis = i;
			}
			if (distanceMin > distanceMax)
			{
				// The ray misses the box.
				return false;
			}
		}
	}
	// If we haven't returned false by now, then the ray hits the box.
	// A negative entry distance means the ray starts inside the box. When the caller
	// does not request an immediate zero-distance hit, report the forward exit face.
	const bool bStartsInside = distanceMin < 0.0;
	const double hitDistance = bStartsInside ? distanceMax : distanceMin;
	const uint8 hitAxis = bStartsInside ? exitAxis : enterAxis;
	if (OutDistance != nullptr)
	{
		*OutDistance = hitDistance;
	}
	if (OutNormal != nullptr)
	{
		FVector4 normal = FVector4(0.0, 0.0, 0.0, 0.0);
		const bool bNegate = bStartsInside == (InRayDirection[hitAxis] < 0.0);
		normal[hitAxis] = bNegate ? -1.0 : 1.0;
		*OutNormal = normal;
	}
	return true;
}

FRaycastResult4D FRect4D::RaycastIntersectsFull(const FVector4& InRayOrigin, const FVector4& InRayDirection, const double InMaxDistance, const bool bInInsideIsZero) const
{
	FRaycastResult4D result;
	if (!ensureMsgf(InMaxDistance >= 0.0, TEXT("FRect4D::RaycastIntersectsFull: Maximum distance must be non-negative.")))
	{
		return result;
	}
	double distance = -std::numeric_limits<double>::infinity();
	FVector4 normal = FVector4(0.0, 0.0, 0.0, 0.0);
	bool hitIntersects = RaycastIntersectsInfiniteFast(InRayOrigin, InRayDirection, bInInsideIsZero, &distance, &normal);
	if (hitIntersects && distance > InMaxDistance)
	{
		hitIntersects = false;
	}
	result.bHit = hitIntersects;
	if (hitIntersects)
	{
		result.Distance = distance;
		result.Normal = normal;
		result.Point = InRayOrigin + InRayDirection * distance;
	}
	return result;
}

// Rect comparison functions.
bool FRect4D::EnclosesExclusive(const FRect4D& InOther) const
{
	FVector4 end = GetEnd();
	FVector4 otherEnd = InOther.GetEnd();
	return Position.X < InOther.Position.X && end.X > otherEnd.X &&
			Position.Y < InOther.Position.Y && end.Y > otherEnd.Y &&
			Position.Z < InOther.Position.Z && end.Z > otherEnd.Z &&
			Position.W < InOther.Position.W && end.W > otherEnd.W;
}

bool FRect4D::EnclosesInclusive(const FRect4D& InOther) const
{
	FVector4 end = GetEnd();
	FVector4 otherEnd = InOther.GetEnd();
	return Position.X <= InOther.Position.X && end.X >= otherEnd.X &&
			Position.Y <= InOther.Position.Y && end.Y >= otherEnd.Y &&
			Position.Z <= InOther.Position.Z && end.Z >= otherEnd.Z &&
			Position.W <= InOther.Position.W && end.W >= otherEnd.W;
}

bool FRect4D::IntersectsExclusive(const FRect4D& InOther) const
{
	FVector4 end = GetEnd();
	FVector4 otherEnd = InOther.GetEnd();
	return Position.X < otherEnd.X && end.X > InOther.Position.X &&
			Position.Y < otherEnd.Y && end.Y > InOther.Position.Y &&
			Position.Z < otherEnd.Z && end.Z > InOther.Position.Z &&
			Position.W < otherEnd.W && end.W > InOther.Position.W;
}

bool FRect4D::IntersectsInclusive(const FRect4D& InOther) const
{
	FVector4 end = GetEnd();
	FVector4 otherEnd = InOther.GetEnd();
	return Position.X <= otherEnd.X && end.X >= InOther.Position.X &&
			Position.Y <= otherEnd.Y && end.Y >= InOther.Position.Y &&
			Position.Z <= otherEnd.Z && end.Z >= InOther.Position.Z &&
			Position.W <= otherEnd.W && end.W >= InOther.Position.W;
}

bool FRect4D::IsEqualApprox(const FRect4D& InOther) const
{
	return Position.Equals(InOther.Position, UNREAL4D_COMPARISON_EPSILON) && Size.Equals(InOther.Size, UNREAL4D_COMPARISON_EPSILON);
}

bool FRect4D::IsFinite() const
{
	// NOTE: Unreal's FVector4.ContainsNaN() is poorly named, it also checks for infinity.
	return !Position.ContainsNaN() && !Size.ContainsNaN();
}

// Operators.
bool FRect4D::operator==(const FRect4D& InOther) const
{
	return Position == InOther.Position && Size == InOther.Size;
}

bool FRect4D::operator!=(const FRect4D& InOther) const
{
	return Position != InOther.Position || Size != InOther.Size;
}

FRect4D::operator FString() const
{
	return TEXT("[P: ") + Position.ToString() + TEXT(", S: ") + Size.ToString() + TEXT("]");
}

static_assert(sizeof(FRect4D) == 8 * sizeof(double), "FRect4D must be 8 doubles or 64 bytes in size");
