#include "Material4D.h"

bool UMaterial4D::IsDefaultMaterial() const
{
	return AlbedoSourceFlags == EColorSourceFlags4D::SingleColor && AlbedoColor.Equals(FLinearColor(1.0, 1.0, 1.0, 1.0));
}

void UMaterial4D::MergeWith(UMaterial4D* InMaterial, int32 FirstItemCount, int32 SecondItemCount)
{
	if (!InMaterial || FirstItemCount < 0 || SecondItemCount < 0)
	{
		return;
	}

	const FLinearColor startAlbedoColor = AlbedoColor;
	const EColorSourceFlags4D startAlbedoSourceFlags = AlbedoSourceFlags;
	AlbedoSourceFlags = static_cast<EColorSourceFlags4D>(static_cast<uint32>(AlbedoSourceFlags) | static_cast<uint32>(InMaterial->AlbedoSourceFlags));

	// If single colors differ, fall back to array usage.
	if (EnumHasAnyFlags(AlbedoSourceFlags, EColorSourceFlags4D::SingleColor) && !AlbedoColor.Equals(InMaterial->AlbedoColor))
	{
		AlbedoSourceFlags = static_cast<EColorSourceFlags4D>(static_cast<uint32>(AlbedoSourceFlags) & ~static_cast<uint32>(EColorSourceFlags4D::SingleColor));
		AlbedoSourceFlags = static_cast<EColorSourceFlags4D>(static_cast<uint32>(AlbedoSourceFlags) | static_cast<uint32>(EColorSourceFlags4D::UsesColorArray));
	}

	if (EnumHasAnyFlags(AlbedoSourceFlags, EColorSourceFlags4D::UsesColorArray))
	{
		const int32 endSize = FirstItemCount + SecondItemCount;
		const int32 existingSize = AlbedoColorArray.Num();
		AlbedoColorArray.SetNum(endSize);

		if (!EnumHasAnyFlags(startAlbedoSourceFlags, EColorSourceFlags4D::UsesColorArray))
		{
			for (int32 i = 0; i < FirstItemCount; ++i)
			{
				AlbedoColorArray[i] = FLinearColor::White;
			}
		}

		if (!EnumHasAnyFlags(AlbedoSourceFlags, EColorSourceFlags4D::SingleColor) && EnumHasAnyFlags(startAlbedoSourceFlags, EColorSourceFlags4D::SingleColor))
		{
			for (int32 i = 0; i < FirstItemCount; ++i)
			{
				AlbedoColorArray[i] *= startAlbedoColor;
			}
		}

		if (EnumHasAnyFlags(InMaterial->AlbedoSourceFlags, EColorSourceFlags4D::UsesColorArray))
		{
			if (InMaterial->AlbedoColorArray.Num() < SecondItemCount)
			{
				return;
			}
			for (int32 i = 0; i < SecondItemCount; ++i)
			{
				AlbedoColorArray[FirstItemCount + i] = InMaterial->AlbedoColorArray[i];
			}
		}
		else
		{
			for (int32 i = FirstItemCount; i < endSize; ++i)
			{
				AlbedoColorArray[i] = FLinearColor::White;
			}
		}

		if (!EnumHasAnyFlags(AlbedoSourceFlags, EColorSourceFlags4D::SingleColor) && EnumHasAnyFlags(InMaterial->AlbedoSourceFlags, EColorSourceFlags4D::SingleColor))
		{
			for (int32 i = FirstItemCount; i < endSize; ++i)
			{
				AlbedoColorArray[i] *= InMaterial->AlbedoColor;
			}
		}

		if (existingSize < endSize)
		{
			for (int32 i = existingSize; i < endSize; ++i)
			{
				if (AlbedoColorArray[i] == FLinearColor())
				{
					AlbedoColorArray[i] = FLinearColor::White;
				}
			}
		}
	}
}

EColorSourceFlags4D UMaterial4D::GetAlbedoSourceFlags() const
{
	return AlbedoSourceFlags;
}

void UMaterial4D::SetAlbedoSourceFlags(EColorSourceFlags4D InAlbedoSourceFlags)
{
	AlbedoSourceFlags = InAlbedoSourceFlags;
}

void UMaterial4D::AppendAlbedoColor(const FLinearColor& InAlbedoColor)
{
	AlbedoColorArray.Add(InAlbedoColor);
}

void UMaterial4D::ResizeAlbedoColorArray(int32 InSize, const FLinearColor& InFillColor)
{
	const int32 existingSize = AlbedoColorArray.Num();
	AlbedoColorArray.SetNum(InSize);
	for (int32 i = existingSize; i < InSize; ++i)
	{
		AlbedoColorArray[i] = InFillColor;
	}
}
