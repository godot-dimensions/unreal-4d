#pragma once

#include "Material4D.h"

#include "Unreal4DRendering/Rasterization/TetraMesh4DRenderResource.h"

#include "Mesh4D.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnMesh4DCacheCleared);

UCLASS(BlueprintType, Category = "Unreal 4D|Mesh")
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal 4D|Mesh")
	UMaterial4D* Material = nullptr;

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

	virtual void CalculateLocalBounds(FVector4& OutOrigin, FVector4& OutExtents) const;

public:
	bool IsMeshDataValid();
	void ResetMeshDataValidation();
	void MeshClearCaches();

	virtual void ValidateMaterialForMesh(UMaterial4D* InMaterial);
	UMaterial4D* GetMaterial() const;
	void SetMaterial(UMaterial4D* InMaterial);
	virtual UMaterial4D* GetFallbackMaterial() const;

	void GetLocalBounds(FVector4& OutOrigin, FVector4& OutExtents) const;
	void UpdateLocalBounds();

	// Data which is sampled by indices.
	virtual const TArray<FVector4>& GetVertexValues() const;
	virtual const TArray<FVector4>& GetNormalValues() const;
	virtual const TArray<FVector>& GetTextureMapValues() const;

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
