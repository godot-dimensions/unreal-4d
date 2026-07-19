// Based on Voxel Plugin Free Legacy, which is copyright Voxel Plugin SAS.


#pragma once

#include "../Unreal4DDefines.h"

#include "IntRect4D.generated.h"

// A box with int32 coordinates.
// This follows the (min,max) convention of Voxel Plugin for ease of porting code from there, not the (min,size) convension of FRect4D.
USTRUCT(BlueprintType, Category = "Unreal 4D|Math|Integer Rectangle")
struct UNREAL4DCORE_API FIntRect4D
{
	GENERATED_USTRUCT_BODY()

	// Min of the box. Inclusive
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal 4D|Math|Integer Rectangle")
	FIntVector4 Min;

	// Max of the box. Exclusive
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal 4D|Math|Integer Rectangle")
	FIntVector4 Max;

	FIntRect4D()
		: Min(FIntVector4::ZeroValue) , Max(FIntVector4::ZeroValue) {}

	FIntRect4D(const FIntVector4& InMin, const FIntVector4& InMax);

	explicit FIntRect4D(const int32 InMin, const FIntVector4& InMax)
		: FIntRect4D(FIntVector4(InMin), InMax) {}

	explicit FIntRect4D(const FIntVector4& InMin, const int32 InMax)
		: FIntRect4D(InMin, FIntVector4(InMax)) {}

	explicit FIntRect4D(const int32 InMin, const int32 InMax)
		: FIntRect4D(FIntVector4(InMin), FIntVector4(InMax)) {}

	explicit FIntRect4D(const FIntVector4& InPosition)
		: FIntRect4D(InPosition, InPosition + FIntVector4(1)) {}

	explicit FIntRect4D(const int32 InX, const int32 InY, const int32 InZ, const int32 InW)
		: FIntRect4D(FIntVector4(InX, InY, InZ, InW), FIntVector4(InX + 1, InY + 1, InZ + 1, InW + 1)) {}

	FORCEINLINE FIntVector4 Size() const
	{
		return Max - Min;
	}

	FORCEINLINE FVector4 GetCenter() const
	{
		return FVector4(Min + Max) / 2.0;
	}

	FORCEINLINE uint64 Count() const
	{
		return
			uint64(Max.X - Min.X) *
			uint64(Max.Y - Min.Y) *
			uint64(Max.Z - Min.Z) *
			uint64(Max.W - Min.W);
	}

	// Get the corners that are inside the box (max - 1).
	TStaticArray<FIntVector4, 16> GetCorners(const int32 InMaxBorderSize) const;

	FString ToString() const;

	FORCEINLINE bool IsValid() const
	{
		return Min.X < Max.X && Min.Y < Max.Y && Min.Z < Max.Z && Min.W < Max.W;
	}

	template<typename T>
	bool ContainsTemplate(const T InX, const T InY, const T InZ, const T InW) const
	{
		return ((InX >= Min.X) && (InX < Max.X) && (InY >= Min.Y) && (InY < Max.Y) && (InZ >= Min.Z) && (InZ < Max.Z) && (InW >= Min.W) && (InW < Max.W));
	}

	template<typename T>
	typename TEnableIf<std::is_same_v<T, FVector4> || std::is_same_v<T, FIntVector4>, bool>::Type ContainsTemplate(const T& InV) const
	{
		return ContainsTemplate(InV.X, InV.Y, InV.Z, InV.W);
	}

	FORCEINLINE bool Contains(const int32 InX, const int32 InY, const int32 InZ, const int32 InW) const
	{
		return ContainsTemplate(InX, InY, InZ, InW);
	}

	FORCEINLINE bool Contains(const FIntVector4& V) const
	{
		return ContainsTemplate(V);
	}

	FORCEINLINE bool Contains(const FIntRect4D& InOther) const
	{
		return Min.X <= InOther.Min.X && Min.Y <= InOther.Min.Y && Min.Z <= InOther.Min.Z && Min.W <= InOther.Min.W &&
				 Max.X >= InOther.Max.X && Max.Y >= InOther.Max.Y && Max.Z >= InOther.Max.Z && Max.W >= InOther.Max.W;
	}

	// Not an overload as the float behavior can be a bit tricky. Use ContainsTemplate if the input type is unknown
	FORCEINLINE bool ContainsFloat(const float InX, const float InY, const float InZ, const float InW) const
	{
		return ContainsTemplate(InX, InY, InZ, InW);
	}

	FORCEINLINE bool ContainsFloat(const FVector4& V) const
	{
		return ContainsTemplate(V);
	}

	template<typename T>
	bool Contains(T X, T Y, T Z, T W) const = delete;

	template<typename T>
	FIntVector4 Clamp(T InP) const
	{
		Clamp(InP.X, InP.Y, InP.Z, InP.W);
		return InP;
	}

	FORCEINLINE void Clamp(int32& OutX, int32& OutY, int32& OutZ, int32& OutW) const
	{
		OutX = FMath::Clamp(OutX, Min.X, Max.X - 1);
		OutY = FMath::Clamp(OutY, Min.Y, Max.Y - 1);
		OutZ = FMath::Clamp(OutZ, Min.Z, Max.Z - 1);
		OutW = FMath::Clamp(OutW, Min.W, Max.W - 1);
		ensure(Contains(OutX, OutY, OutZ, OutW));
	}

	template<typename T>
	void Clamp(T& OutX, T& OutY, T& OutZ, T& OutW) const
	{
		// Note: use - 1 even if that's not the closest value for which Contains would return true
		// because it's really hard to figure out that value (largest float f such that f < i)
		OutX = FMath::Clamp<T>(OutX, Min.X, Max.X - 1);
		OutY = FMath::Clamp<T>(OutY, Min.Y, Max.Y - 1);
		OutZ = FMath::Clamp<T>(OutZ, Min.Z, Max.Z - 1);
		OutW = FMath::Clamp<T>(OutW, Min.W, Max.W - 1);
		ensure(ContainsTemplate(OutX, OutY, OutZ, OutW));
	}

	FORCEINLINE FIntRect4D Clamp(const FIntRect4D& InOther) const
	{
		// It's not valid to call Clamp if we're not intersecting InOther
		ensure(Intersect(InOther));

		FIntRect4D result;

		result.Min.X = FMath::Clamp(InOther.Min.X, Min.X, Max.X - 1);
		result.Min.Y = FMath::Clamp(InOther.Min.Y, Min.Y, Max.Y - 1);
		result.Min.Z = FMath::Clamp(InOther.Min.Z, Min.Z, Max.Z - 1);
		result.Min.W = FMath::Clamp(InOther.Min.W, Min.W, Max.W - 1);

		result.Max.X = FMath::Clamp(InOther.Max.X, Min.X + 1, Max.X);
		result.Max.Y = FMath::Clamp(InOther.Max.Y, Min.Y + 1, Max.Y);
		result.Max.Z = FMath::Clamp(InOther.Max.Z, Min.Z + 1, Max.Z);
		result.Max.W = FMath::Clamp(InOther.Max.W, Min.W + 1, Max.W);

		ensure(InOther.Contains(result));
		return result;
	}

	/**
	 * Checks whether the given bounding box intersects this bounding box.
	 *
	 * @param InOther The bounding box to intersect with.
	 * @return true if the boxes intersect, false otherwise.
	 */
	template<typename TBox>
	bool Intersect(const TBox& InOther) const
	{
		if ((Min.X >= InOther.Max.X) || (InOther.Min.X >= Max.X))
		{
			return false;
		}

		if ((Min.Y >= InOther.Max.Y) || (InOther.Min.Y >= Max.Y))
		{
			return false;
		}

		if ((Min.Z >= InOther.Max.Z) || (InOther.Min.Z >= Max.Z))
		{
			return false;
		}

		if ((Min.W >= InOther.Max.W) || (InOther.Min.W >= Max.W))
		{
			return false;
		}

		return true;
	}

	/**
	 * Useful for templates taking a box or coordinates
	 */
	template<typename TNumeric>
	bool Intersect(const TNumeric InX, const TNumeric InY, const TNumeric InZ, const TNumeric InW) const
	{
		return ContainsTemplate(InX, InY, InZ, InW);
	}

	FIntRect4D Overlap(const FIntRect4D& InOther) const;

	FIntRect4D Union(const FIntRect4D& InOther) const;

	FORCEINLINE FIntRect4D Extend(const FIntVector4& InAmount) const
	{
		return { Min - InAmount, Max + InAmount };
	}
	FORCEINLINE FIntRect4D Extend(const int32 InAmount) const
	{
		return Extend(FIntVector4(InAmount));
	}
	FORCEINLINE FIntRect4D Translate(const FIntVector4& InPosition) const
	{
		return FIntRect4D(Min + InPosition, Max + InPosition);
	}

	FORCEINLINE FIntRect4D RemoveTranslation() const
	{
		return FIntRect4D(0, Max - Min);
	}

	FORCEINLINE FIntRect4D operator*(const int32 InScale) const
	{
		return FIntRect4D(Min * InScale, Max * InScale);
	}

	FORCEINLINE FIntRect4D& operator*=(const int32 InScale)
	{
		Min *= InScale;
		Max *= InScale;
		return *this;
	}

	FORCEINLINE bool operator==(const FIntRect4D& InOther) const
	{
		return Min == InOther.Min && Max == InOther.Max;
	}
	FORCEINLINE bool operator!=(const FIntRect4D& InOther) const
	{
		return Min != InOther.Min || Max != InOther.Max;
	}

	template<typename T>
	FORCEINLINE void Iterate(T InLambda) const
	{
		for (int32 x = Min.X; x < Max.X; x++)
		{
			for (int32 y = Min.Y; y < Max.Y; y++)
			{
				for (int32 z = Min.Z; z < Max.Z; z++)
				{
					for (int32 w = Min.W; w < Max.W; w++)
					{
						InLambda(FIntVector4(x, y, z, w));
					}
				}
			}
		}
	}

	template<typename T>
	void Iterate(const int32 InStep, T InLambda) const
	{
		check(InStep > 0);
		for (int32 x = Min.X; x < Max.X; x += InStep)
		{
			for (int32 y = Min.Y; y < Max.Y; y += InStep)
			{
				for (int32 z = Min.Z; z < Max.Z; z += InStep)
				{
					for (int32 w = Min.W; w < Max.W; w += InStep)
					{
						InLambda(FIntVector4(x, y, z, w));
					}
				}
			}
		}
	}
};

FORCEINLINE FIntRect4D operator*(int32 InScale, const FIntRect4D& InBox)
{
	return InBox * InScale;
}
