#include "IntRect4D.h"

// Get the corners that are inside the box (max - 1).
TStaticArray<FIntVector4, 16> FIntRect4D::GetCorners(const int32 InMaxBorderSize) const
{
	return TStaticArray<FIntVector4, 16>(
			FIntVector4(Min.X, Min.Y, Min.Z, Min.W),
			FIntVector4(Max.X - InMaxBorderSize, Min.Y, Min.Z, Min.W),
			FIntVector4(Min.X, Max.Y - InMaxBorderSize, Min.Z, Min.W),
			FIntVector4(Max.X - InMaxBorderSize, Max.Y - InMaxBorderSize, Min.Z, Min.W),
			FIntVector4(Min.X, Min.Y, Max.Z - InMaxBorderSize, Min.W),
			FIntVector4(Max.X - InMaxBorderSize, Min.Y, Max.Z - InMaxBorderSize, Min.W),
			FIntVector4(Min.X, Max.Y - InMaxBorderSize, Max.Z - InMaxBorderSize, Min.W),
			FIntVector4(Max.X - InMaxBorderSize, Max.Y - InMaxBorderSize, Max.Z - InMaxBorderSize, Min.W),
			FIntVector4(Min.X, Min.Y, Min.Z, Max.W - InMaxBorderSize),
			FIntVector4(Max.X - InMaxBorderSize, Min.Y, Min.Z, Max.W - InMaxBorderSize),
			FIntVector4(Min.X, Max.Y - InMaxBorderSize, Min.Z, Max.W - InMaxBorderSize),
			FIntVector4(Max.X - InMaxBorderSize, Max.Y - InMaxBorderSize, Min.Z, Max.W - InMaxBorderSize),
			FIntVector4(Min.X, Min.Y, Max.Z - InMaxBorderSize, Max.W - InMaxBorderSize),
			FIntVector4(Max.X - InMaxBorderSize, Min.Y, Max.Z - InMaxBorderSize, Max.W - InMaxBorderSize),
			FIntVector4(Min.X, Max.Y - InMaxBorderSize, Max.Z - InMaxBorderSize, Max.W - InMaxBorderSize),
			FIntVector4(Max.X - InMaxBorderSize, Max.Y - InMaxBorderSize, Max.Z - InMaxBorderSize, Max.W - InMaxBorderSize)
	);
}

FIntRect4D FIntRect4D::Overlap(const FIntRect4D& InOther) const
{
	if (!Intersect(InOther))
	{
		return FIntRect4D();
	}
	// otherwise they overlap
	// so find overlapping box
	FIntVector4 minVector, maxVector;
	minVector.X = FMath::Max(Min.X, InOther.Min.X);
	minVector.Y = FMath::Max(Min.Y, InOther.Min.Y);
	minVector.Z = FMath::Max(Min.Z, InOther.Min.Z);
	minVector.W = FMath::Max(Min.W, InOther.Min.W);

	maxVector.X = FMath::Min(Max.X, InOther.Max.X);
	maxVector.Y = FMath::Min(Max.Y, InOther.Max.Y);
	maxVector.Z = FMath::Min(Max.Z, InOther.Max.Z);
	maxVector.W = FMath::Min(Max.W, InOther.Max.W);
	return FIntRect4D(minVector, maxVector);
}

FIntRect4D FIntRect4D::Union(const FIntRect4D& InOther) const
{
	FIntVector4 minVector, maxVector;
	minVector.X = FMath::Min(Min.X, InOther.Min.X);
	minVector.Y = FMath::Min(Min.Y, InOther.Min.Y);
	minVector.Z = FMath::Min(Min.Z, InOther.Min.Z);
	minVector.W = FMath::Min(Min.W, InOther.Min.W);

	maxVector.X = FMath::Max(Max.X, InOther.Max.X);
	maxVector.Y = FMath::Max(Max.Y, InOther.Max.Y);
	maxVector.Z = FMath::Max(Max.Z, InOther.Max.Z);
	maxVector.W = FMath::Max(Max.W, InOther.Max.W);
	return FIntRect4D(minVector, maxVector);
}

FString FIntRect4D::ToString() const
{
	return FString::Printf(TEXT("(%d/%d, %d/%d, %d/%d, %d/%d)"), Min.X, Max.X, Min.Y, Max.Y, Min.Z, Max.Z, Min.W, Max.W);
}

FIntRect4D::FIntRect4D(const FIntVector4& InMin, const FIntVector4& InMax) : Min(InMin), Max(InMax)
{
	if (!ensureMsgf(Min.X <= Max.X, TEXT("%d <= %d"), Min.X, Max.X)) Max.X = Min.X;
	if (!ensureMsgf(Min.Y <= Max.Y, TEXT("%d <= %d"), Min.Y, Max.Y)) Max.Y = Min.Y;
	if (!ensureMsgf(Min.Z <= Max.Z, TEXT("%d <= %d"), Min.Z, Max.Z)) Max.Z = Min.Z;
	if (!ensureMsgf(Min.W <= Max.W, TEXT("%d <= %d"), Min.W, Max.W)) Max.W = Min.W;
}
