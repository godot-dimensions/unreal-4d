#include "VectorFuncs4D.h"

#include "Math4D.h"

#include <algorithm>

const FVector4 FVector4_Zero	= FVector4(0.0, 0.0, 0.0, 0.0);
const FVector4 FVector4_One		= FVector4(1.0, 1.0, 1.0, 1.0);
const FVector4 FVector4_Forward = FVector4(1.0, 0.0, 0.0, 0.0);
const FVector4 FVector4_Back	= FVector4(-1.0, 0.0, 0.0, 0.0);
const FVector4 FVector4_Right	= FVector4(0.0, 1.0, 0.0, 0.0);
const FVector4 FVector4_Left	= FVector4(0.0, -1.0, 0.0, 0.0);
const FVector4 FVector4_Up		= FVector4(0.0, 0.0, 1.0, 0.0);
const FVector4 FVector4_Down	= FVector4(0.0, 0.0, -1.0, 0.0);
const FVector4 FVector4_Ana		= FVector4(0.0, 0.0, 0.0, 1.0);
const FVector4 FVector4_Kata	= FVector4(0.0, 0.0, 0.0, -1.0);

// NOTE: Editor code should grab the colors from the editor theme instead.
// This function is for non-editor code or when the theme is not available.
FLinearColor UVectorFuncs4D::Vector4_AxisColor(const EAxis4D InAxis)
{
	switch (InAxis)
	{
		case EAxis4D::X:
			return FLinearColor(0.60, 0.02, 0.00, 1.0);
		case EAxis4D::Y:
			return FLinearColor(0.14, 0.40, 0.00, 1.0);
		case EAxis4D::Z:
			return FLinearColor(0.03, 0.21, 0.85, 1.0);
		case EAxis4D::W:
			return FLinearColor(0.83, 0.50, 0.00, 1.0);
	}
	return FLinearColor(0.55, 0.55, 0.55, 1.0);
}

FString UVectorFuncs4D::Vector4_AxisLetter(const EAxis4D InAxis)
{
	switch (InAxis)
	{
		case EAxis4D::X:
			return TEXT("X");
		case EAxis4D::Y:
			return TEXT("Y");
		case EAxis4D::Z:
			return TEXT("Z");
		case EAxis4D::W:
			return TEXT("W");
	}
	return TEXT("?");
}

// Non-bindable helpers.

void UVectorFuncs4D::SortAxesMaxToMin(const FVector4& InVector, int32 (&InOutAxes)[4])
{
	std::sort(InOutAxes, InOutAxes + 4, [&InVector](int32 A, int32 B)
	{
		return InVector[A] > InVector[B];
	});
}

// Vector math.

FVector4 UVectorFuncs4D::Vector4_Abs(const FVector4& InVector)
{
	return FVector4(FMath::Abs(InVector.X), FMath::Abs(InVector.Y), FMath::Abs(InVector.Z), FMath::Abs(InVector.W));
}

double UVectorFuncs4D::Vector4_AngleTo(const FVector4& InFrom, const FVector4& InTo)
{
	const double dotProduct = Dot4(InFrom, InTo);
	const double magnitude = FMath::Sqrt(Dot4(InFrom, InFrom) * Dot4(InTo, InTo));
	if (magnitude > UNREAL4D_COMPARISON_EPSILON)
	{
		return FMath::Acos(FMath::Clamp(dotProduct / magnitude, -1.0, 1.0));
	}
	return 0.0;
}

FVector4 UVectorFuncs4D::Vector4_Average(const TArray<FVector4>& InVectors)
{
	if (InVectors.IsEmpty())
	{
		return FVector4(0.0, 0.0, 0.0, 0.0);
	}
	FVector4 sum = FVector4(0.0, 0.0, 0.0, 0.0);
	for (const FVector4& vec : InVectors)
	{
		sum += vec;
	}
	return sum / static_cast<double>(InVectors.Num());
}

FVector4 UVectorFuncs4D::Vector4_Bounce(const FVector4& InVector, const FVector4& InNormal)
{
	const double dotProduct = Dot4(InVector, InNormal);
	return InVector + (-2.0) * dotProduct * InNormal;
}

FVector4 UVectorFuncs4D::Vector4_BounceRatio(const FVector4& InVector, const FVector4& InNormal, const double InBounceRatio)
{
	const double dotProduct = Dot4(InVector, InNormal);
	return InVector + (-1.0 - InBounceRatio) * dotProduct * InNormal;
}

double UVectorFuncs4D::Vector4_Cross(const FVector4& InA, const FVector4& InB)
{
	const double diagonal = Dot4(InA, InA) * Dot4(InB, InB);
	const double nonDiagonal = Dot4(InA, InB);
	return FMath::Sqrt(diagonal - nonDiagonal * nonDiagonal);
}

double UVectorFuncs4D::Vector4_Distance(const FVector4& InA, const FVector4& InB)
{
	return (InB - InA).Size();
}

double UVectorFuncs4D::Vector4_DistanceSquared(const FVector4& InA, const FVector4& InB)
{
	return (InB - InA).SizeSquared();
}

double UVectorFuncs4D::Vector4_Dot(const FVector4& InA, const FVector4& InB)
{
	return Dot4(InA, InB);
}

bool UVectorFuncs4D::Vector4_IsUniform(const FVector4& InVector)
{
	return FMath::IsNearlyEqual(InVector.X, InVector.Y, UNREAL4D_COMPARISON_EPSILON) &&
		   FMath::IsNearlyEqual(InVector.X, InVector.Z, UNREAL4D_COMPARISON_EPSILON) &&
		   FMath::IsNearlyEqual(InVector.X, InVector.W, UNREAL4D_COMPARISON_EPSILON);
}

FVector4 UVectorFuncs4D::Vector4_Lerp(const FVector4& InFrom, const FVector4& InTo, const double InWeight)
{
	return FMath::Lerp(InFrom, InTo, InWeight);
}

FVector4 UVectorFuncs4D::Vector4_LimitLength(const FVector4& InVector, const double InLen)
{
	const double len = InVector.Size();
	FVector4 result = InVector;
	if (len > 0.0 && InLen < len)
	{
		result *= InLen / len;
	}
	return result;
}

FVector4 UVectorFuncs4D::Vector4_LimitLengthTaxicab(const FVector4& InVector, const double InLen)
{
	const FVector4 absVector = FVector4(FMath::Abs(InVector.X), FMath::Abs(InVector.Y), FMath::Abs(InVector.Z), FMath::Abs(InVector.W));
	const double taxicabLength = absVector.X + absVector.Y + absVector.Z + absVector.W;
	if (taxicabLength <= InLen)
	{
		return InVector;
	}
	// Else, we need to take away length from each axis, as equally as possible.
	// But we need to start with the axis with the greatest value because it will be the last to reach 0.
	int32 sortedAxes[4] = { 0, 1, 2, 3 };
	SortAxesMaxToMin(absVector, sortedAxes);
	double remainingTaxicab = taxicabLength - InLen;
	FVector4 limited = InVector;
	for (int32 i = 4; i > 0; --i)
	{
		const double takeaway = remainingTaxicab / i;
		const int32 axis = sortedAxes[i - 1];
		if (absVector[axis] <= takeaway)
		{
			limited[axis] = 0.0;
			// Since this axis reached zero, we need to take away more from the other axes.
			remainingTaxicab -= absVector[axis];
		}
		else
		{
			remainingTaxicab -= takeaway;
			if (limited[axis] < 0.0)
			{
				limited[axis] += takeaway;
			}
			else
			{
				limited[axis] -= takeaway;
			}
		}
	}
	return limited;
}

FVector4 UVectorFuncs4D::Vector4_Max(const FVector4& InA, const FVector4& InB)
{
	return FVector4(FMath::Max(InA.X, InB.X), FMath::Max(InA.Y, InB.Y), FMath::Max(InA.Z, InB.Z), FMath::Max(InA.W, InB.W));
}

EAxis4D UVectorFuncs4D::Vector4_MaxAxisIndex(const FVector4& InVector)
{
	uint8 maxIndex = 0;
	double maxValue = InVector.X;
	for (uint8 i = 1; i < 4; i++) {
		if (InVector[i] > maxValue) {
			maxValue = InVector[i];
			maxIndex = i;
		}
	}
	return EAxis4D(maxIndex);
}

FVector4 UVectorFuncs4D::Vector4_MoveToward(const FVector4& InFrom, const FVector4& InTo, const double InDelta)
{
	const FVector4 offset = InTo - InFrom;
	const double len = offset.Size();
	return (len <= InDelta || len < UE_SMALL_NUMBER) ? InTo : (InFrom + offset / len * InDelta);
}

FVector4 UVectorFuncs4D::Vector4_Normalized(const FVector4& InVector)
{
	const double lengthSquared = InVector.SizeSquared();
	if (lengthSquared == 0.0)
	{
		return FVector4(0.0, 0.0, 0.0, 0.0);
	}
	const double length = FMath::Sqrt(lengthSquared);
	return InVector / length;
}

FVector4 UVectorFuncs4D::Vector4_OrthogonalFromTwo(const FVector4& InWhat, const FVector4& InA, const FVector4& InB)
{
	// Gram-Schmidt Process, now in 4D.
	// https://en.wikipedia.org/wiki/Gram-Schmidt_process
	FVector4 a = Vector4_Normalized(InA);
	FVector4 b = InB;
	b = b - a * Dot4(a, b);
	b = Vector4_Normalized(b);
	return InWhat - a * Dot4(a, InWhat) - b * Dot4(b, InWhat);
}

FVector4 UVectorFuncs4D::Vector4_Perpendicular(const FVector4& InA, const FVector4& InB, const FVector4& InC)
{
	FVector4 perp;
	/* clang-format off */
	perp.X = - InA.Y * (InB.Z * InC.W - InB.W * InC.Z)
	         + InA.Z * (InB.Y * InC.W - InB.W * InC.Y)
	         - InA.W * (InB.Y * InC.Z - InB.Z * InC.Y);
	perp.Y = + InA.X * (InB.Z * InC.W - InB.W * InC.Z)
	         - InA.Z * (InB.X * InC.W - InB.W * InC.X)
	         + InA.W * (InB.X * InC.Z - InB.Z * InC.X);
	perp.Z = - InA.X * (InB.Y * InC.W - InB.W * InC.Y)
	         + InA.Y * (InB.X * InC.W - InB.W * InC.X)
	         - InA.W * (InB.X * InC.Y - InB.Y * InC.X);
	perp.W = + InA.X * (InB.Y * InC.Z - InB.Z * InC.Y)
	         - InA.Y * (InB.X * InC.Z - InB.Z * InC.X)
	         + InA.Z * (InB.X * InC.Y - InB.Y * InC.X);
	/* clang-format on */
	return perp;
}

FVector4 UVectorFuncs4D::Vector4_Project(const FVector4& InVector, const FVector4& InOnNormal)
{
	const double DotProduct = Dot4(InVector, InOnNormal);
	const double LenSquared = Dot4(InOnNormal, InOnNormal);
	return InOnNormal * (DotProduct / LenSquared);
}

FVector4 UVectorFuncs4D::Vector4_Reflect(const FVector4& InVector, const FVector4& InNormal)
{
	const double DotProduct = Dot4(InVector, InNormal);
	return 2.0 * DotProduct * InNormal - InVector;
}

FVector4 UVectorFuncs4D::Vector4_RotateInPlane(const FVector4& InVector, const FVector4& InPlaneFrom, const FVector4& InPlaneTo, const double InAngle)
{
	FVector4 PlaneCos = InPlaneFrom;
	PlaneCos = Vector4_Normalized(PlaneCos);
	FVector4 PlaneSin = InPlaneTo - PlaneCos * Dot4(PlaneCos, InPlaneTo);
	PlaneSin = Vector4_Normalized(PlaneSin);
	// Determine the destination plane vectors.
	const double AngleCos = FMath::Cos(InAngle);
	const double AngleSin = FMath::Sin(InAngle);
	const FVector4 DestCos = PlaneCos * AngleCos + PlaneSin * AngleSin;
	const FVector4 DestSin = PlaneCos * (-AngleSin) + PlaneSin * AngleCos;
	// Reproject the vector onto the plane.
	const double DotCos = Dot4(InVector, PlaneCos);
	const double DotSin = Dot4(InVector, PlaneSin);
	return InVector - (PlaneCos * DotCos + PlaneSin * DotSin) + (DestCos * DotCos + DestSin * DotSin);
}

FVector4 UVectorFuncs4D::Vector4_SafeReciprocal(const FVector4& InVector)
{
	return FVector4(UMath4D::SafeDivide(1.0, InVector.X),
					UMath4D::SafeDivide(1.0, InVector.Y),
					UMath4D::SafeDivide(1.0, InVector.Z),
					UMath4D::SafeDivide(1.0, InVector.W));
}

FVector4 UVectorFuncs4D::Vector4_Sign(const FVector4& InVector)
{
	return FVector4(FMath::Sign(InVector.X), FMath::Sign(InVector.Y), FMath::Sign(InVector.Z), FMath::Sign(InVector.W));
}

// Slide returns the component of the vector along the given plane, specified by its normal vector.
FVector4 UVectorFuncs4D::Vector4_Slide(const FVector4& InVector, const FVector4& InNormal)
{
	const double DotProduct = Dot4(InVector, InNormal);
	return InVector - InNormal * DotProduct;
}

// Snap snaps each component of the vector to the nearest multiple of the snap interval.
FVector4 UVectorFuncs4D::Vector4_Snap(const FVector4& InVector, const double InSnapInterval)
{
	if (InSnapInterval <= 0.0)
	{
		return InVector;
	}
	return FVector4(
		FMath::RoundToInt(InVector.X / InSnapInterval) * InSnapInterval,
		FMath::RoundToInt(InVector.Y / InSnapInterval) * InSnapInterval,
		FMath::RoundToInt(InVector.Z / InSnapInterval) * InSnapInterval,
		FMath::RoundToInt(InVector.W / InSnapInterval) * InSnapInterval
	);
}

// Generation.

FVector4 UVectorFuncs4D::Vector4_RandomInRadius(const double InRadius)
{
	while (true)
	{
		const FVector4 RandomPoint(
			FMath::RandRange(-InRadius, InRadius),
			FMath::RandRange(-InRadius, InRadius),
			FMath::RandRange(-InRadius, InRadius),
			FMath::RandRange(-InRadius, InRadius)
		);
		if (Dot4(RandomPoint, RandomPoint) <= InRadius * InRadius)
		{
			return RandomPoint;
		}
	}
}

FVector4 UVectorFuncs4D::Vector4_RandomInRange(const FVector4& InFrom, const FVector4& InTo)
{
	return FVector4(
		FMath::RandRange(InFrom.X, InTo.X),
		FMath::RandRange(InFrom.Y, InTo.Y),
		FMath::RandRange(InFrom.Z, InTo.Z),
		FMath::RandRange(InFrom.W, InTo.W)
	);
}

// Conversion.

FVector4 UVectorFuncs4D::Vector4_FromArray(const TArray<double>& InArray)
{
	if (InArray.Num() >= 4)
	{
		return FVector4(InArray[0], InArray[1], InArray[2], InArray[3]);
	}
	else if (InArray.Num() == 3)
	{
		return FVector4(InArray[0], InArray[1], InArray[2], 0.0);
	}
	else if (InArray.Num() == 2)
	{
		return FVector4(InArray[0], InArray[1], 0.0, 0.0);
	}
	else if (InArray.Num() == 1)
	{
		return FVector4(InArray[0], 0.0, 0.0, 0.0);
	}
	return FVector4(0.0, 0.0, 0.0, 0.0);
}

TArray<double> UVectorFuncs4D::Vector4_ToArray(const FVector4& InVector)
{
	TArray<double> ret;
	ret.SetNum(4);
	ret[0] = InVector.X;
	ret[1] = InVector.Y;
	ret[2] = InVector.Z;
	ret[3] = InVector.W;
	return ret;
}

FVector4 UVectorFuncs4D::Vector4_FromJsonArray(const TArray<TSharedPtr<FJsonValue>>& InJsonArray)
{
	FVector4 ret = FVector4(0.0, 0.0, 0.0, 0.0);
	if (InJsonArray.Num() >= 4)
	{
		ret.X = InJsonArray[0]->AsNumber();
		ret.Y = InJsonArray[1]->AsNumber();
		ret.Z = InJsonArray[2]->AsNumber();
		ret.W = InJsonArray[3]->AsNumber();
	}
	else if (InJsonArray.Num() == 3)
	{
		ret.X = InJsonArray[0]->AsNumber();
		ret.Y = InJsonArray[1]->AsNumber();
		ret.Z = InJsonArray[2]->AsNumber();
	}
	else if (InJsonArray.Num() == 2)
	{
		ret.X = InJsonArray[0]->AsNumber();
		ret.Y = InJsonArray[1]->AsNumber();
	}
	else if (InJsonArray.Num() == 1)
	{
		ret.X = InJsonArray[0]->AsNumber();
	}
	return ret;
}

TArray<TSharedPtr<FJsonValue>> UVectorFuncs4D::Vector4_ToJsonArray(const FVector4& InVector)
{
	TArray<TSharedPtr<FJsonValue>> ret;
	ret.SetNum(4);
	ret[0] = MakeShared<FJsonValueNumber>(InVector.X);
	ret[1] = MakeShared<FJsonValueNumber>(InVector.Y);
	ret[2] = MakeShared<FJsonValueNumber>(InVector.Z);
	ret[3] = MakeShared<FJsonValueNumber>(InVector.W);
	return ret;
}
