#include "GeneralShapeCurve4D.h"

int32 UGeneralShapeCurve4D::GetRadiiDimension() const
{
	int32 dimension = 0;
	for (int32 i = 0; i < 4; i++)
	{
		if (Radii[i] > 0.0)
		{
			dimension++;
		}
	}
	return dimension;
}

double UGeneralShapeCurve4D::GetRadiiSum() const
{
	return Radii.X + Radii.Y + Radii.Z + Radii.W;
}

TArray<int32> UGeneralShapeCurve4D::GetUsedAxes() const
{
	TArray<int32> usedAxes;
	for (int32 i = 0; i < 4; i++)
	{
		if (Radii[i] > 0.0)
		{
			usedAxes.Add(i);
		}
	}
	return usedAxes;
}

TArray<int32> UGeneralShapeCurve4D::GetZeroAxes() const
{
	TArray<int32> zeroAxes;
	for (int32 i = 0; i < 4; i++)
	{
		if (Radii[i] == 0.0)
		{
			zeroAxes.Add(i);
		}
	}
	return zeroAxes;
}

bool FGeneralShapeCurveTaperPoint4D::IsEqualExact(const FGeneralShapeCurveTaperPoint4D& InOther) const
{
	if (Position != InOther.Position)
	{
		return false;
	}
	if (Radii != InOther.Radii)
	{
		return false;
	}
	if (Exponent != InOther.Exponent)
	{
		return false;
	}
	return true;
}

bool UGeneralShapeCurve4D::IsEqualExact(const UGeneralShapeCurve4D* InOther) const
{
	if (InOther == nullptr)
	{
		return false;
	}
	if (Radii != InOther->Radii)
	{
		return false;
	}
	if (Exponent != InOther->Exponent)
	{
		return false;
	}
	for (int32 i = 0; i < Taper.Num(); i++)
	{
		if (!Taper[i].IsEqualExact(InOther->Taper[i]))
		{
			return false;
		}
	}
	return true;
}
