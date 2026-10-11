#pragma once

#include "Material4D.h"

#include "Unreal4DRendering/Rasterization/TetraMesh4DRenderResource.h"

#include "Mesh4D.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnMesh4DCacheCleared);

UCLASS(Abstract, BlueprintType, Category = "Unreal 4D|Mesh")
class UNREAL4DCORE_API UMesh4D : public UObject
{
	GENERATED_BODY()

protected:
	// Slightly under the 32-bit integer limit to avoid overflows. Must be int64.
	static constexpr int64 MAX_VERTICES = 2147483640;

	static const TArray<FVector4> EmptyFVector4Array;
	static const TArray<FVector> EmptyFVectorArray;
	static const TArray<int32> EmptyInt32Array;

	UPROPERTY(VisibleAnywhere, Category = "Unreal 4D|Mesh")
	FVector4 LocalOrigin = FVector4::Zero();

	UPROPERTY(VisibleAnywhere, Category = "Unreal 4D|Mesh")
	FVector4 LocalExtents = FVector4::Zero();

	bool bIsMeshDataValid = false;

	/*
	 * Current immutable render-resource snapshot for this mesh asset.
	 *
	 * Scene proxies keep their own shared reference to the snapshot they were
	 * created with. When the mesh data changes, the asset drops this reference and
	 * creates a new snapshot; the old GPU buffers are released only after the last
	 * old scene proxy releases its shared reference.
	 */
	FTetraMesh4DRenderResourcePtr RenderResource;

	/*
	 * Set by MeshClearCaches() to signal that the render resource must be rebuilt on
	 * the next call to GetOrCreateRenderResource().
	 */
	bool bRenderResourceDirty = false;
	bool bLocalBoundsDirty = true;

	/*
	 * Creates the shared render resource for this mesh asset.
	 *
	 * Derived classes must override this function to build the GPU buffers required
	 * by their concrete mesh representation.
	 */
	virtual FTetraMesh4DRenderResourcePtr CreateRenderResource();

	virtual void CalculateLocalBounds(FVector4& OutOrigin, FVector4& OutExtents) const PURE_VIRTUAL(UMesh4D::CalculateLocalBounds, OutOrigin = FVector4::Zero(); OutExtents = FVector4::Zero(););

public:
	bool IsMeshDataValid();
	void ResetMeshDataValidation();
	void MeshClearCaches();

	virtual void ValidateMaterialForMesh(UMaterial4D* InMaterial);

	void GetLocalBounds(FVector4& OutOrigin, FVector4& OutExtents) const;
	void UpdateLocalBounds();

	virtual bool ValidateMeshData();

	/*
	 * Returns the shared render resource for this mesh asset, creating it on first use.
	 *
	 * The returned pointer remains owned by the mesh asset and must not be released by
	 * the caller.
	 */
	FTetraMesh4DRenderResourcePtr GetOrCreateRenderResource();

	virtual void BeginDestroy() override;
	virtual void PostLoad() override;

	/*
	 * Fired by MeshClearCaches() to notify components that the mesh data has changed
	 * and their render states must be rebuilt. Old render-resource snapshots remain
	 * alive until all scene proxies release their shared references.
	 */
	FOnMesh4DCacheCleared OnMesh4DCacheCleared;

};
