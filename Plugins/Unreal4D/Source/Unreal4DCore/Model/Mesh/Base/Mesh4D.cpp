#include "Mesh4D.h"

const TArray<FVector4> UMesh4D::EmptyFVector4Array = TArray<FVector4>();
const TArray<FVector> UMesh4D::EmptyFVectorArray = TArray<FVector>();
const TArray<int32> UMesh4D::EmptyInt32Array = TArray<int32>();

bool UMesh4D::IsMeshDataValid()
{
	if (bIsMeshDataValid)
	{
		return true;
	}
	bIsMeshDataValid = ValidateMeshData();
	if (!bIsMeshDataValid)
	{
		UE_LOG(LogUnreal4DCore, Warning, TEXT("UMesh4D: Mesh data is invalid."));
	}
	return bIsMeshDataValid;
}

void UMesh4D::ResetMeshDataValidation()
{
	bIsMeshDataValid = false;
	MeshClearCaches();
}

void UMesh4D::MeshClearCaches()
{
	LocalOrigin = FVector4::Zero();
	LocalExtents = FVector4::Zero();
	bLocalBoundsDirty = true;

	// Mark the current resource snapshot as stale. Existing scene proxies keep their
	// shared references to the old snapshot until they are removed from the scene.
	bRenderResourceDirty = true;
	OnMesh4DCacheCleared.Broadcast();
}

void UMesh4D::ValidateMaterialForMesh(UMaterial4D* InMaterial)
{
	UE_LOG(LogUnreal4DCore, Error, TEXT("UMesh4D::ValidateMaterialForMesh: This method should be overridden in derived classes."));
}

UMaterial4D* UMesh4D::GetMaterial() const
{
	return Material;
}

void UMesh4D::SetMaterial(UMaterial4D* InMaterial)
{
	ValidateMaterialForMesh(InMaterial);
	Material = InMaterial;
	OnMesh4DCacheCleared.Broadcast();
}

UMaterial4D* UMesh4D::GetFallbackMaterial() const
{
	UE_LOG(LogUnreal4DCore, Error, TEXT("UMesh4D::GetFallbackMaterial: This method should be overridden in derived classes."));
	return nullptr;
}

const TArray<FVector4>& UMesh4D::GetVertexValues() const
{
	UE_LOG(LogUnreal4DCore, Error, TEXT("UMesh4D::GetVertexValues: This method should be overridden in derived classes."));
	return EmptyFVector4Array;
}

const TArray<FVector4>& UMesh4D::GetNormalValues() const
{
	UE_LOG(LogUnreal4DCore, Error, TEXT("UMesh4D::GetNormalValues: This method should be overridden in derived classes."));
	return EmptyFVector4Array;
}

const TArray<FVector>& UMesh4D::GetTextureMapValues() const
{
	UE_LOG(LogUnreal4DCore, Error, TEXT("UMesh4D::GetTextureMapValues: This method should be overridden in derived classes."));
	return EmptyFVectorArray;
}

bool UMesh4D::ValidateMeshData()
{
	UE_LOG(LogUnreal4DCore, Error, TEXT("UMesh4D::ValidateMeshData: This method should be overridden in derived classes."));
	return false;
}


FTetraMesh4DRenderResourcePtr UMesh4D::GetOrCreateRenderResource()
{
	// This function may be called from CreateSceneProxy(), which can run on task
	// threads. Resource creation/rebuild is still expected to be pre-created on the
	// game thread by CreateRenderState_Concurrent(); task-thread calls only take a
	// shared reference to the already-created resource.
	if (!IsInGameThread())
	{
		return RenderResource;
	}

	if (bRenderResourceDirty)
	{
		// Drop the asset reference to the old snapshot. If scene proxies still use it,
		// they keep it alive. The custom deleter releases the GPU buffers later, on
		// the render thread, after the last shared reference is gone.
		RenderResource.Reset();
		bRenderResourceDirty = false;
	}

	if (RenderResource.IsValid())
	{
		return RenderResource;
	}

	RenderResource = CreateRenderResource();
	if (!RenderResource.IsValid())
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("UMesh4D::GetOrCreateRenderResource: Failed to create render resource for '%s'."), *GetPathName());
		return nullptr;
	}

	return RenderResource;
}

void UMesh4D::BeginDestroy()
{
	RenderResource.Reset();
	Super::BeginDestroy();
}

void UMesh4D::PostLoad()
{
	Super::PostLoad();

	UpdateLocalBounds();
}

FTetraMesh4DRenderResourcePtr UMesh4D::CreateRenderResource()
{
	ensureMsgf(false, TEXT("UMesh4D::CreateRenderResource must be overridden in derived classes."));
	UE_LOG(LogUnreal4DCore, Error, TEXT("UMesh4D::CreateRenderResource was called on the base mesh class '%s'."), *GetPathName());
	return nullptr;
}

void UMesh4D::CalculateLocalBounds(FVector4& OutOrigin, FVector4& OutExtents) const
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

void UMesh4D::GetLocalBounds(FVector4& OutOrigin, FVector4& OutExtents) const
{
	if (bLocalBoundsDirty)
	{
		const_cast<UMesh4D*>(this)->UpdateLocalBounds();
	}

	OutOrigin = LocalOrigin;
	OutExtents = LocalExtents;
}

void UMesh4D::UpdateLocalBounds()
{
	CalculateLocalBounds(LocalOrigin, LocalExtents);
	bLocalBoundsDirty = false;
}
