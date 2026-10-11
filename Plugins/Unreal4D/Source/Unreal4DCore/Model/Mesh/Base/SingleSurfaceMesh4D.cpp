#include "SingleSurfaceMesh4D.h"

UMaterial4D* USingleSurfaceMesh4D::GetMaterial() const
{
	return Material;
}

void USingleSurfaceMesh4D::SetMaterial(UMaterial4D* InMaterial)
{
	ValidateMaterialForMesh(InMaterial);
	Material = InMaterial;
	OnMesh4DCacheCleared.Broadcast();
}

UMaterial4D* USingleSurfaceMesh4D::GetFallbackMaterial() const
{
	UE_LOG(LogUnreal4DCore, Error, TEXT("USingleSurfaceMesh4D::GetFallbackMaterial: This method should be overridden in derived classes."));
	return nullptr;
}

const TArray<FVector4>& USingleSurfaceMesh4D::GetVertexValues() const
{
	UE_LOG(LogUnreal4DCore, Error, TEXT("USingleSurfaceMesh4D::GetVertexValues: This method should be overridden in derived classes."));
	return EmptyFVector4Array;
}

const TArray<FVector4>& USingleSurfaceMesh4D::GetNormalValues() const
{
	UE_LOG(LogUnreal4DCore, Error, TEXT("USingleSurfaceMesh4D::GetNormalValues: This method should be overridden in derived classes."));
	return EmptyFVector4Array;
}

const TArray<FVector>& USingleSurfaceMesh4D::GetTextureMapValues() const
{
	UE_LOG(LogUnreal4DCore, Error, TEXT("USingleSurfaceMesh4D::GetTextureMapValues: This method should be overridden in derived classes."));
	return EmptyFVectorArray;
}

void USingleSurfaceMesh4D::CalculateLocalBounds(FVector4& OutOrigin, FVector4& OutExtents) const
{
	const TArray<FVector4>& Vertices = GetVertexValues();
	if (Vertices.IsEmpty())
	{
		OutOrigin = FVector4::Zero();
		OutExtents = FVector4::Zero();
		return;
	}

	FVector4 mins = Vertices[0];
	FVector4 maxs = Vertices[0];

	for (const FVector4& Vertex : Vertices)
	{
		mins = mins.ComponentMin(Vertex);
		maxs = maxs.ComponentMax(Vertex);
	}

	OutOrigin = ((mins + maxs) * 0.5f);
	OutExtents = ((maxs - mins) * 0.5f);
}
