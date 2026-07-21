#include "GeneralShape4DData.h"

bool UGeneralShape4DData::_warningsEnabled = true;

#define GENERAL_SHAPE_4D_CURVE_ERR_WARN(m_curve, m_ret) \
	if (m_curve == nullptr) { return m_ret; } \
	if (_warningsEnabled) \
	{ \
		if (m_curve->Exponent != 2.0) \
		{ \
			UE_LOG(LogUnreal4DCore, Warning, TEXT("UGeneralShape4DData: Custom curve exponents are not currently supported by this class, and have no effect.")); \
		} \
		if (m_curve->Taper.Num() > 0) \
		{ \
			UE_LOG(LogUnreal4DCore, Warning, TEXT("UGeneralShape4DData: Curve tapers are not currently supported by this class, and have no effect.")); \
		} \
	}

#define GENERAL_SHAPE_4D_RADII_WARN(m_radii, m_radii_average, m_msg) \
	if (_warningsEnabled && ((m_radii.X != 0.0 && !FMath::IsNearlyEqual(m_radii.X, m_radii_average)) || (m_radii.Y != 0.0 && !FMath::IsNearlyEqual(m_radii.Y, m_radii_average)) || (m_radii.Z != 0.0 && !FMath::IsNearlyEqual(m_radii.Z, m_radii_average)) || (m_radii.W != 0.0 && !FMath::IsNearlyEqual(m_radii.W, m_radii_average)))) \
	{ \
		UE_LOG(LogUnreal4DCore, Warning, TEXT("UGeneralShape4DData." m_msg " This approximation works well for ellipsoids that are close to (hyper)spheres, but diverges for highly elongated ellipsoids. You can disable this warning with `UGeneralShape4DData.SetWarningsEnabled(false)`.")); \
	}

double UGeneralShape4DData::GetHypervolume() const
{
	// Start with the volume of the base box.
	double volume = BaseSize.X * BaseSize.Y * BaseSize.Z * BaseSize.W;
	const int64 curveCount = Curves.Num();
	if (curveCount == 0)
	{
		return volume;
	}
	// Note: This function does not handle Steinmetz solids, or any other case of curves sharing axes.
	const UGeneralShapeCurve4D* curve = Curves[0];
	GENERAL_SHAPE_4D_CURVE_ERR_WARN(curve, volume);
	const FVector4 radii = curve->Radii;
	const int32 curveDimension = curve->GetRadiiDimension();
	// Curve dimension 4 is for 4D hyperspheres, also used in 4D capsules and rounded boxes.
	if (curveDimension == 4)
	{
		// 4D part.
		volume += (0.125 * UNREAL4D_TAU * UNREAL4D_TAU) * (radii.X * radii.Y * radii.Z * radii.W);
		// 3D part.
		volume += (UNREAL4D_TAU * 2.0 / 3.0) * (
			radii.X * radii.Y * radii.Z * BaseSize.W +
			radii.X * radii.Y * BaseSize.Z * radii.W +
			radii.X * BaseSize.Y * radii.Z * radii.W +
			BaseSize.X * radii.Y * radii.Z * radii.W
		);
		// 2D part.
		volume += UE_DOUBLE_PI * (
			radii.X * radii.Y * BaseSize.Z * BaseSize.W +
			radii.X * BaseSize.Y * radii.Z * BaseSize.W +
			radii.X * BaseSize.Y * BaseSize.Z * radii.W +
			BaseSize.X * radii.Y * radii.Z * BaseSize.W +
			BaseSize.X * radii.Y * BaseSize.Z * radii.W +
			BaseSize.X * BaseSize.Y * radii.Z * radii.W
		);
		// 1D part.
		volume += 2.0 * (
			radii.X * BaseSize.Y * BaseSize.Z * BaseSize.W +
			BaseSize.X * radii.Y * BaseSize.Z * BaseSize.W +
			BaseSize.X * BaseSize.Y * radii.Z * BaseSize.W +
			BaseSize.X * BaseSize.Y * BaseSize.Z * radii.W
		);
		if (curveCount > 1)
		{
			UE_LOG(LogUnreal4DCore, Error, TEXT("UGeneralShape4DData.GetHypervolume: Steinmetz solids are not supported."));
		}
		return volume;
	}
	const TArray<int32> zeroAxes = curve->GetZeroAxes();
	const TArray<int32> usedAxes = curve->GetUsedAxes();
	// Curve dimension 3 is for 3D spheres, as used in 4D cylinders/spherinders.
	if (curveDimension == 3)
	{
		check(zeroAxes.Num() == 1 && usedAxes.Num() == 3);
		// 3D part.
		double volume3D = (UNREAL4D_TAU * 2.0 / 3.0) * (radii[usedAxes[0]] * radii[usedAxes[1]] * radii[usedAxes[2]]);
		// 2D part.
		volume3D += UE_DOUBLE_PI * (
			radii[usedAxes[0]] * radii[usedAxes[1]] * BaseSize[usedAxes[2]] +
			radii[usedAxes[0]] * BaseSize[usedAxes[1]] * radii[usedAxes[2]] +
			BaseSize[usedAxes[0]] * radii[usedAxes[1]] * radii[usedAxes[2]]
		);
		// 1D part.
		volume3D += 2.0 * (
			radii[usedAxes[0]] * BaseSize[usedAxes[1]] * BaseSize[usedAxes[2]] +
			BaseSize[usedAxes[0]] * radii[usedAxes[1]] * BaseSize[usedAxes[2]] +
			BaseSize[usedAxes[0]] * BaseSize[usedAxes[1]] * radii[usedAxes[2]]
		);
		volume += volume3D * BaseSize[zeroAxes[0]];
		if (curveCount > 1)
		{
			UE_LOG(LogUnreal4DCore, Error, TEXT("UGeneralShape4DData.GetHypervolume: Steinmetz solids are not supported."));
		}
		return volume;
	}
	// Curve dimension 2 is for 2D circles, as used in 4D cubinders and duocylinders.
	if (curveDimension == 2)
	{
		check(zeroAxes.Num() == 2 && usedAxes.Num() == 2);
		// 2D part.
		double volume0_2D = UE_DOUBLE_PI * (radii[usedAxes[0]] * radii[usedAxes[1]]);
		// 1D part.
		volume0_2D += 2.0 * (
			radii[usedAxes[0]] * BaseSize[usedAxes[1]] +
			BaseSize[usedAxes[0]] * radii[usedAxes[1]]
		);
		volume += volume0_2D * (BaseSize[zeroAxes[0]] * BaseSize[zeroAxes[1]]);
		if (curveCount == 1)
		{
			// If this is the only curve, we can return now.
			return volume;
		}
		// There may be more curves in the case of a duocylinder.
		const UGeneralShapeCurve4D* curve1 = Curves[1];
		GENERAL_SHAPE_4D_CURVE_ERR_WARN(curve1, volume);
		const FVector4 radii1 = curve1->Radii;
		const TArray<int32> zeroAxes1 = curve1->GetZeroAxes();
		const TArray<int32> usedAxes1 = curve1->GetUsedAxes();
		if (zeroAxes1.Num() == 2 && usedAxes1.Num() == 2 && (radii * radii1 == FVector4(0.0, 0.0, 0.0, 0.0)))
		{
			double volume1_2D = UE_DOUBLE_PI * (radii1[usedAxes1[0]] * radii1[usedAxes1[1]]);
			volume1_2D += 2.0 * (
				radii1[usedAxes1[0]] * BaseSize[usedAxes1[1]] +
				BaseSize[usedAxes1[0]] * radii1[usedAxes1[1]]
			);
			volume += volume1_2D * (BaseSize[zeroAxes1[0]] * BaseSize[zeroAxes1[1]]);
			// Square meters times square meters gives quartic meters.
			volume += volume0_2D * volume1_2D;
			return volume;
		}
		UE_LOG(LogUnreal4DCore, Error, TEXT("UGeneralShape4DData.GetHypervolume: Steinmetz solids are not supported."));
		return volume;
	}
	// Curve dimension 1 is not really a valid curve, but we can handle it anyway.
	if (curveDimension == 1)
	{
		check(zeroAxes.Num() == 3 && usedAxes.Num() == 1);
		// 1D part.
		double volume1D = 2.0 * (radii[usedAxes[0]]);
		volume += volume1D * (BaseSize[zeroAxes[0]] * BaseSize[zeroAxes[1]] * BaseSize[zeroAxes[2]]);
		return volume;
	}
	UE_LOG(LogUnreal4DCore, Error, TEXT("UGeneralShape4DData.GetHypervolume: Invalid curve dimension."));
	return volume;
}

double UGeneralShape4DData::GetSurfaceVolume() const
{
	// Start with the surface area of the base box.
	double surface = 2.0 * (
		BaseSize.X * BaseSize.Y * BaseSize.Z +
		BaseSize.X * BaseSize.Y * BaseSize.W +
		BaseSize.X * BaseSize.Z * BaseSize.W +
		BaseSize.Y * BaseSize.Z * BaseSize.W);
	const int64 curveCount = Curves.Num();
	if (curveCount == 0)
	{
		return surface;
	}
	// Note: This function does not handle Steinmetz solids, or any other case of curves sharing axes.
	const UGeneralShapeCurve4D* curve = Curves[0];
	GENERAL_SHAPE_4D_CURVE_ERR_WARN(curve, surface);
	const FVector4 radii = curve->Radii;
	const double radiiSum = curve->GetRadiiSum();
	const int32 curveDimension = curve->GetRadiiDimension();
	const double radiiAverage = radiiSum / curveDimension;
 	GENERAL_SHAPE_4D_RADII_WARN(radii, radiiAverage, "GetSurfaceVolume: There is no closed-form solution for the surface area of ellipsoids. An approximation will be used instead: the surface area of a (hyper)sphere with the average radius of the ellipsoid.");
	// Curve dimension 4 is for 4D hyperspheres, also used in 4D capsules and rounded boxes.
	if (curveDimension == 4)
	{
		// 4D curve part.
		surface += (0.5 * UNREAL4D_TAU * UNREAL4D_TAU) * (radiiAverage * radiiAverage * radiiAverage);
		// 3D curve part.
		surface += (2.0 * UNREAL4D_TAU) * (radiiAverage * radiiAverage) * (BaseSize.X + BaseSize.Y + BaseSize.Z + BaseSize.W);
		// 2D curve part.
		surface += UNREAL4D_TAU * (radiiAverage * radiiAverage) * (
			BaseSize.X * BaseSize.Y +
			BaseSize.X * BaseSize.Z +
			BaseSize.X * BaseSize.W +
			BaseSize.Y * BaseSize.Z +
			BaseSize.Y * BaseSize.W +
			BaseSize.Z * BaseSize.W
		);
		// No 1D curve part, since the 1D curve part is already included in the base box surface area.
		if (curveCount > 1)
		{
			UE_LOG(LogUnreal4DCore, Error, TEXT("UGeneralShape4DData.GetSurfaceVolume: Steinmetz solids are not supported."));
		}
		return surface;
	}
	const TArray<int32> zeroAxes = curve->GetZeroAxes();
	const TArray<int32> usedAxes = curve->GetUsedAxes();
	// Curve dimension 3 is for 3D spheres, as used in 4D cylinders/spherinders.
	if (curveDimension == 3)
	{
		check(zeroAxes.Num() == 1 && usedAxes.Num() == 3);
		// 3D curve part.
		surface += (2.0 * UNREAL4D_TAU) * (radiiAverage * radiiAverage) * BaseSize[zeroAxes[0]];
		// 2D curve part.
		surface += UNREAL4D_TAU * radiiAverage * BaseSize[zeroAxes[0]] * (BaseSize[usedAxes[0]] + BaseSize[usedAxes[1]] + BaseSize[usedAxes[2]]);
		// No 1D curve part, since the 1D curve part is already included in the base box surface area.
		// 3D flat part.
		double flatSurface = (UNREAL4D_TAU * 2.0 / 3.0) * (radiiAverage * radiiAverage * radiiAverage);
		// 2D flat part.
		flatSurface += UE_DOUBLE_PI * (radiiAverage * radiiAverage) * (BaseSize[usedAxes[0]] + BaseSize[usedAxes[1]] + BaseSize[usedAxes[2]]);
		// 1D flat part.
		flatSurface += (2.0 * radiiAverage) * (
			BaseSize[usedAxes[0]] * BaseSize[usedAxes[1]] +
			BaseSize[usedAxes[0]] * BaseSize[usedAxes[2]] +
			BaseSize[usedAxes[1]] * BaseSize[usedAxes[2]]
		);
		// The flat part exists on both sides of the cylinder, so we need to multiply it by 2.
		surface += flatSurface * 2.0;
		if (curveCount > 1)
		{
			UE_LOG(LogUnreal4DCore, Error, TEXT("UGeneralShape4DData.GetSurfaceVolume: Steinmetz solids are not supported."));
		}
		return surface;
	}
	// Curve dimension 2 is for 2D circles, as used in 4D cubinders and duocylinders.
	if (curveDimension == 2)
	{
		check(zeroAxes.Num() == 2 && usedAxes.Num() == 2);
		// 2D curve part.
		surface += UNREAL4D_TAU * radiiAverage * BaseSize[zeroAxes[0]] * BaseSize[zeroAxes[1]];
		// No 1D curve part, since the 1D curve part is already included in the base box surface area.
		// 2D flat part.
		double flatSurface0 = UE_DOUBLE_PI * (radiiAverage * radiiAverage);
		// 1D flat part.
		flatSurface0 += (2.0 * radiiAverage) * (BaseSize[usedAxes[0]] + BaseSize[usedAxes[1]]);
		// The flat part exists on multiple sides of the cubinder, so we need to multiply it by 2.
		surface += 2.0 * flatSurface0 * (BaseSize[zeroAxes[0]] + BaseSize[zeroAxes[1]]);
		if (curveCount == 1)
		{
			// If this is the only curve, we can return now.
			return surface;
		}
		// There may be more curves in the case of a duocylinder.
		const UGeneralShapeCurve4D* curve1 = Curves[1];
		GENERAL_SHAPE_4D_CURVE_ERR_WARN(curve1, surface);
		const FVector4 radii1 = curve1->Radii;
		const TArray<int32> zeroAxes1 = curve1->GetZeroAxes();
		const TArray<int32> usedAxes1 = curve1->GetUsedAxes();
		if (zeroAxes1.Num() == 2 && usedAxes1.Num() == 2 && (radii * radii1 == FVector4(0.0, 0.0, 0.0, 0.0)))
		{
			const double radii1Sum = curve1->GetRadiiSum();
			const int32 curve1Dimension = curve1->GetRadiiDimension();
			const double radii1Average = radii1Sum / curve1Dimension;
 			GENERAL_SHAPE_4D_RADII_WARN(radii1, radii1Average, "GetSurfaceVolume: There is no closed-form solution for the surface area of ellipse. An approximation will be used instead: the surface area of a circle with the average radius of the ellipse.");
			// Surface provided by the second curve of the duocylinder.
			surface += UNREAL4D_TAU * radii1Average * BaseSize[zeroAxes1[0]] * BaseSize[zeroAxes1[1]];
			double flatSurface1 = UE_DOUBLE_PI * (radii1Average * radii1Average);
			flatSurface1 += (2.0 * radii1Average) * (BaseSize[usedAxes1[0]] + BaseSize[usedAxes1[1]]);
			surface += 2.0 * flatSurface1 * (BaseSize[zeroAxes1[0]] + BaseSize[zeroAxes1[1]]);
			// Duocylinder-specific surface volume, see UDuocylinderShape4DData.
			surface += (2.0 * UE_DOUBLE_PI * UE_DOUBLE_PI) * (radiiAverage * radii1Average) * (radiiAverage + radii1Average);
		}
		if (curveCount > 2)
		{
			UE_LOG(LogUnreal4DCore, Error, TEXT("UGeneralShape4DData.GetSurfaceVolume: Steinmetz solids are not supported."));
		}
		return surface;
	}
	// Curve dimension 1 is not really a valid curve, but we can handle it anyway.
	if (curveDimension == 1)
	{
		check(zeroAxes.Num() == 3 && usedAxes.Num() == 1);
		// 1D part.
		surface += 2.0 * (radii[usedAxes[0]]) * (BaseSize[zeroAxes[0]] * BaseSize[zeroAxes[1]] * BaseSize[zeroAxes[2]]);
		return surface;
	}
	UE_LOG(LogUnreal4DCore, Error, TEXT("UGeneralShape4DData.GetSurfaceVolume: Invalid curve dimension."));
	return surface;
}

FRect4D UGeneralShape4DData::GetRectBoundsLocal4D(const FTransform4D& InTransformToTarget) const
{
	const FVector4 halfExtents = GetBaseHalfExtents();
	const int64 curveCount = Curves.Num();
	FRect4D rectBounds = FRect4D(InTransformToTarget.Origin, FVector4(0.0, 0.0, 0.0, 0.0));
	if (curveCount == 0)
	{
		for (int8 x = -1; x < 2; x += 2)
		{
			for (int8 y = -1; y < 2; y += 2)
			{
				for (int8 z = -1; z < 2; z += 2)
				{
					for (int8 w = -1; w < 2; w += 2)
					{
						const FVector4 sign = FVector4(x, y, z, w);
						const FVector4 corner = sign * halfExtents;
						rectBounds.ExpandSelfToPoint(InTransformToTarget * corner);
					}
				}
			}
		}
	}
	// Include the axis-aligned radii of the curves in the bounds.
	// This is not perfect, but it should be good enough for most cases.
	FVector4 farthestExtentRadii;
	for (int32 curveIndex = 0; curveIndex < Curves.Num(); curveIndex++)
	{
		const UGeneralShapeCurve4D* curve = Curves[curveIndex];
		GENERAL_SHAPE_4D_CURVE_ERR_WARN(curve, rectBounds);
		farthestExtentRadii = UVectorFuncs4D::Vector4_Max(farthestExtentRadii, curve->Radii);
	}
	for (int8 x = -1; x < 2; x += 2)
	{
		for (int8 y = -1; y < 2; y += 2)
		{
			for (int8 z = -1; z < 2; z += 2)
			{
				for (int8 w = -1; w < 2; w += 2)
				{
					const FVector4 sign = FVector4(x, y, z, w);
					const FVector4 corner = sign * halfExtents;
					for (int8 i = 0; i < 4; i++)
					{
						FVector4 curveExtentRadius = FVector4(0.0, 0.0, 0.0, 0.0);
						curveExtentRadius[i] = sign[i] * farthestExtentRadii[i];
						const FVector4 localEdge = corner + curveExtentRadius;
						rectBounds.ExpandSelfToPoint(InTransformToTarget * localEdge);
					}
				}
			}
		}
	}
	return rectBounds;
}

FVector4 UGeneralShape4DData::GetNearestPointToLocal(const FVector4& InLocalPoint) const
{
	const FVector4 halfExtents = GetBaseHalfExtents();
	FVector4 localOffsetAbs = UVectorFuncs4D::Vector4_Abs(InLocalPoint);
	FVector4 nearestPointAbs = FVector4(0.0, 0.0, 0.0, 0.0);
	// First, account for the base box.
	for (int8 i = 0; i < 4; i++)
	{
		if (localOffsetAbs[i] > halfExtents[i])
		{
			nearestPointAbs[i] = halfExtents[i];
			localOffsetAbs[i] -= halfExtents[i];
		}
		else
		{
			nearestPointAbs[i] = localOffsetAbs[i];
			localOffsetAbs[i] = 0.0;
		}
	}
	// Next, check the curves, if any. Note: This does not handle Steinmetz solids, or any other case of curves sharing axes.
	for (int64 curveIndex = 0; curveIndex < Curves.Num(); curveIndex++)
	{
		const UGeneralShapeCurve4D* curve = Curves[curveIndex];
		GENERAL_SHAPE_4D_CURVE_ERR_WARN(curve, nearestPointAbs);
		const FVector4 radii = curve->Radii;
		const TArray<int32> usedAxes = curve->GetUsedAxes();
		const double exponent = curve->Exponent;
		double ellipsoidDistancePow = 0.0;
		if (FMath::IsNearlyEqual(exponent, 2.0))
		{
			for (int64 usedIndex = 0; usedIndex < usedAxes.Num(); usedIndex++)
			{
				const int32 axis = usedAxes[usedIndex];
				ellipsoidDistancePow += (localOffsetAbs[axis] * localOffsetAbs[axis]) / (radii[axis] * radii[axis]);
			}
		} else {
			for (int64 usedIndex = 0; usedIndex < usedAxes.Num(); usedIndex++)
			{
				const int32 axis = usedAxes[usedIndex];
				ellipsoidDistancePow += FMath::Pow(localOffsetAbs[axis], exponent) / FMath::Pow(radii[axis], exponent);
			}
		}
		if (ellipsoidDistancePow <= 1.0)
		{
			// The point is inside the ellipsoid on these axes, and we are done with them.
			for (int64 usedIndex = 0; usedIndex < usedAxes.Num(); usedIndex++)
			{
				const int32 axis = usedAxes[usedIndex];
				nearestPointAbs[axis] += localOffsetAbs[axis];
				localOffsetAbs[axis] = 0.0;
			}
			continue;
		}
		// The point is outside the ellipsoid.
		const double radiiSum = curve->GetRadiiSum();
		const int32 curveDimension = curve->GetRadiiDimension();
		const double radiiAverage = radiiSum / curveDimension;
		GENERAL_SHAPE_4D_RADII_WARN(radii, radiiAverage, "GetNearestPoint: There is no closed-form solution for the nearest point on ellipsoids. An approximation will be used instead: the nearest point on a scaled (hyper)sphere with the average radius of the ellipsoid.");
		FVector4 scaledOffsetAbs = FVector4(0.0, 0.0, 0.0, 0.0);
		for (int64 usedIndex = 0; usedIndex < usedAxes.Num(); usedIndex++)
		{
			const int32 axis = usedAxes[usedIndex];
			scaledOffsetAbs[axis] = localOffsetAbs[axis] / radii[axis];
		}
		scaledOffsetAbs = scaledOffsetAbs.GetSafeNormal();
		for (int64 usedIndex = 0; usedIndex < usedAxes.Num(); usedIndex++)
		{
			const int32 axis = usedAxes[usedIndex];
			const double rescaledOffsetValue = scaledOffsetAbs[axis] * radii[axis];
			nearestPointAbs[axis] += rescaledOffsetValue;
			localOffsetAbs[axis] -= rescaledOffsetValue;
		}
	}
	return nearestPointAbs * UVectorFuncs4D::Vector4_Sign(InLocalPoint);
}

FVector4 UGeneralShape4DData::GetSupportPointInLocalDirection(const FVector4& InLocalDirection) const
{
	// First, account for the base box.
	const FVector4 halfExtents = GetBaseHalfExtents();
	FVector4 support = FVector4(
		(InLocalDirection.X > 0.0) ? halfExtents.X : -halfExtents.X,
		(InLocalDirection.Y > 0.0) ? halfExtents.Y : -halfExtents.Y,
		(InLocalDirection.Z > 0.0) ? halfExtents.Z : -halfExtents.Z,
		(InLocalDirection.W > 0.0) ? halfExtents.W : -halfExtents.W);
	// Next, check the curves, if any. Note: This does not handle Steinmetz solids, or any other case of curves sharing axes.
	const int64 curveCount = Curves.Num();
	for (int64 curveIndex = 0; curveIndex < curveCount; curveIndex++)
	{
		const UGeneralShapeCurve4D* curve = Curves[curveIndex];
		GENERAL_SHAPE_4D_CURVE_ERR_WARN(curve, support);
		const FVector4 radii = curve->Radii;
		const TArray<int32> usedAxes = curve->GetUsedAxes();
		const double exponent = curve->Exponent;
		// Based on this: https://math.stackexchange.com/questions/2267688/locating-a-line-perpendicular-to-an-ellipse
		FVector4 ellipsoidSupport = FVector4(0.0, 0.0, 0.0, 0.0);
		if (FMath::IsNearlyEqual(exponent, 2.0))
		{
			double ellipsoidSupportLenSquared = 0.0;
			for (int64 usedIndex = 0; usedIndex < usedAxes.Num(); usedIndex++)
			{
				const int32 axis = usedAxes[usedIndex];
				const double radiusSquared = radii[axis] * radii[axis];
				ellipsoidSupport[axis] = radiusSquared;
				ellipsoidSupportLenSquared += (InLocalDirection[axis] * InLocalDirection[axis]) * radiusSquared;
			}
			ellipsoidSupport /= FMath::Sqrt(ellipsoidSupportLenSquared);
		}
		else
		{
			double ellipsoidSupportLenPow = 0.0;
			for (int64 usedIndex = 0; usedIndex < usedAxes.Num(); usedIndex++)
			{
				const int32 axis = usedAxes[usedIndex];
				const double radiusPow = FMath::Pow(radii[axis], exponent);
				ellipsoidSupport[axis] = radiusPow;
				ellipsoidSupportLenPow += FMath::Pow(InLocalDirection[axis], exponent) * radiusPow;
			}
			ellipsoidSupport /= FMath::Pow(ellipsoidSupportLenPow, 1.0 / exponent);
		}
		support += InLocalDirection * ellipsoidSupport;
	}
	return support;
}

bool UGeneralShape4DData::HasLocalPoint(const FVector4& InLocalPoint) const
{
	const FVector4 halfExtents = GetBaseHalfExtents();
	FVector4 localOffsetAbs = UVectorFuncs4D::Vector4_Abs(InLocalPoint);
	// First, account for the base box.
	for (int32 i = 0; i < 4; i++)
	{
		if (localOffsetAbs[i] > halfExtents[i])
		{
			localOffsetAbs[i] -= halfExtents[i];
		}
		else
		{
			localOffsetAbs[i] = 0.0;
		}
	}
	// Next, check the curves, if any. Note: This does not handle Steinmetz solids, or any other case of curves sharing axes.
	for (int64 curveIndex = 0; curveIndex < Curves.Num(); curveIndex++)
	{
		const UGeneralShapeCurve4D* curve = Curves[curveIndex];
		GENERAL_SHAPE_4D_CURVE_ERR_WARN(curve, false);
		const FVector4 radii = curve->Radii;
		const TArray<int32> usedAxes = curve->GetUsedAxes();
		const double exponent = curve->Exponent;
		double ellipsoidDistancePow = 0.0;
		if (FMath::IsNearlyEqual(exponent, 2.0))
		{
			for (int64 usedIndex = 0; usedIndex < usedAxes.Num(); usedIndex++)
			{
				const int32 axis = usedAxes[usedIndex];
				ellipsoidDistancePow += (localOffsetAbs[axis] * localOffsetAbs[axis]) / (radii[axis] * radii[axis]);
			}
		}
		else
		{
			for (int64 usedIndex = 0; usedIndex < usedAxes.Num(); usedIndex++)
			{
				const int32 axis = usedAxes[usedIndex];
				ellipsoidDistancePow += FMath::Pow(localOffsetAbs[axis], exponent) / FMath::Pow(radii[axis], exponent);
			}
		}
		if (ellipsoidDistancePow > 1.0)
		{
			// The point is outside the ellipsoid.
			return false;
		}
		// Since we know the point is inside the ellipsoid on these axes, we are done with them.
		for (int64 usedIndex = 0; usedIndex < usedAxes.Num(); usedIndex++)
		{
			localOffsetAbs[usedAxes[usedIndex]] = 0.0;
		}
	}
	return localOffsetAbs == FVector4(0.0, 0.0, 0.0, 0.0);
}

bool UGeneralShape4DData::IsEqualExact(const UShape4DData* InShape) const
{
	const UGeneralShape4DData* other = Cast<UGeneralShape4DData>(InShape);
	if (other == nullptr)
	{
		return false;
	}
	if (BaseSize != other->BaseSize)
	{
		return false;
	}
	if (Curves.Num() != other->Curves.Num())
	{
		return false;
	}
	for (int64 i = 0; i < Curves.Num(); i++)
	{
		const UGeneralShapeCurve4D* thisCurve = Curves[i];
		const UGeneralShapeCurve4D* otherCurve = other->Curves[i];
		if (thisCurve == nullptr || otherCurve == nullptr)
		{
			return false;
		}
		if (!thisCurve->IsEqualExact(otherCurve))
		{
			return false;
		}
	}
	return true;
}

#undef GENERAL_SHAPE_4D_CURVE_ERR_WARN
#undef GENERAL_SHAPE_4D_RADII_WARN
