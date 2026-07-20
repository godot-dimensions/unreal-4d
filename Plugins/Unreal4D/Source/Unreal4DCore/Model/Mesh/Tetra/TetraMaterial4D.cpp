#include "TetraMaterial4D.h"

#include "TetraMesh4D.h"

EColorSourceFlags4D UTetraMaterial4D::TetraSourceToFlags(ETetraColorSource4D InTetraSource)
{
	switch (InTetraSource)
	{
		case ETetraColorSource4D::SingleColor:
			return EColorSourceFlags4D::SingleColor;
		case ETetraColorSource4D::PerCellOnly:
			return EColorSourceFlags4D::PerCell;
		case ETetraColorSource4D::Texture3DCellUVWOnly:
			return EColorSourceFlags4D::Texture3DCellUVW;
		case ETetraColorSource4D::Texture4DDirectOnly:
			return EColorSourceFlags4D::Texture4DDirect;
		case ETetraColorSource4D::PerCellAndSingle:
			return EColorSourceFlags4D::PerCell | EColorSourceFlags4D::SingleColor;
		case ETetraColorSource4D::Texture3DCellUVWAndSingle:
			return EColorSourceFlags4D::Texture3DCellUVW | EColorSourceFlags4D::SingleColor;
		case ETetraColorSource4D::Texture4DDirectAndSingle:
			return EColorSourceFlags4D::Texture4DDirect | EColorSourceFlags4D::SingleColor;
		default:
			return EColorSourceFlags4D::None;
	}
}

void UTetraMaterial4D::MergeWith(UMaterial4D* InMaterial, int32 FirstItemCount, int32 SecondItemCount)
{
	UMaterial4D::MergeWith(InMaterial, FirstItemCount, SecondItemCount);

	// Read AlbedoSourceFlags and set the tetra material's albedo source enum.
	if (EnumHasAnyFlags(AlbedoSourceFlags, EColorSourceFlags4D::SingleColor))
	{
		if (EnumHasAnyFlags(AlbedoSourceFlags, EColorSourceFlags4D::PerCell))
		{
			SetAlbedoSource(ETetraColorSource4D::PerCellAndSingle);
		}
		else if (EnumHasAnyFlags(AlbedoSourceFlags, EColorSourceFlags4D::Texture3DCellUVW))
		{
			SetAlbedoSource(ETetraColorSource4D::Texture3DCellUVWAndSingle);
		}
		else if (EnumHasAnyFlags(AlbedoSourceFlags, EColorSourceFlags4D::Texture4DDirect))
		{
			SetAlbedoSource(ETetraColorSource4D::Texture4DDirectAndSingle);
		}
		else
		{
			SetAlbedoSource(ETetraColorSource4D::SingleColor);
		}
	}
	else if (EnumHasAnyFlags(AlbedoSourceFlags, EColorSourceFlags4D::PerCell))
	{
		SetAlbedoSource(ETetraColorSource4D::PerCellOnly);
	}
	else if (EnumHasAnyFlags(AlbedoSourceFlags, EColorSourceFlags4D::Texture3DCellUVW))
	{
		SetAlbedoSource(ETetraColorSource4D::Texture3DCellUVWOnly);
	}
	else if (EnumHasAnyFlags(AlbedoSourceFlags, EColorSourceFlags4D::Texture4DDirect))
	{
		SetAlbedoSource(ETetraColorSource4D::Texture4DDirectOnly);
	}
}

ETetraColorSource4D UTetraMaterial4D::GetAlbedoSource() const
{
	return AlbedoSource;
}

void UTetraMaterial4D::SetAlbedoSource(ETetraColorSource4D InAlbedoSource)
{
	AlbedoSource = InAlbedoSource;
	AlbedoSourceFlags = TetraSourceToFlags(AlbedoSource);
}

UTetraMaterial4D::UTetraMaterial4D()
{
	SetAlbedoSource(ETetraColorSource4D::SingleColor);
}
