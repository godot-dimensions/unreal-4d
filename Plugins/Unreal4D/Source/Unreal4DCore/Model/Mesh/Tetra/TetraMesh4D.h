#pragma once

#include "../../../Math/RaycastResult4D.h"
#include "../Base/Mesh4D.h"

#include "TetraMaterial4D.h"
#include "TetraMesh4D.generated.h"

class UArrayTetraMesh4D;

UCLASS(BlueprintType, Category = "Unreal 4D|Mesh|Tetra")
class UNREAL4DCORE_API UTetraMesh4D : public UMesh4D
{
	GENERATED_BODY()

protected:
	UPROPERTY()
	TArray<double> _NearestTetraInverseMetricCache;
	FCriticalSection TetraMeshModifyOrCacheLock;

	virtual FTetraMesh4DRenderResourcePtr CreateRenderResource() override;

public:
	// Nearest point and signed distance.
	virtual void PopulateInverseMetricCache();
	UFUNCTION(BlueprintCallable, Category = "Unreal 4D|Mesh|Tetra")
	virtual double GetSignedDistanceToMesh(const FVector4& InLocalPoint, FVector4& OutNearestPointOnTet, int& OutTetrahedronIndex, const double InMaxDistance = 1e300);

	// Raycast.
	bool RaycastIntersectsFast(const FVector4& InLocalRayFrom, const FVector4& InLocalRayDirection, const double InMaxDistance = 1e300);
	// NOTE: The default max distance is 1e300 instead of infinity because the Unreal Header Tool does not like non-literal default values for parameters.
	FRaycastResult4D RaycastIntersectsLocal4D(const FVector4& InLocalRayFrom, const FVector4& InLocalRayDirection, const double InMaxDistance = 1e300);

	// Cache and validation.
	void TetraMeshClearCache();
	virtual bool ValidateMeshData() override;
	virtual void ValidateMaterialForMesh(UMaterial4D* InMaterial) override;

	// Conversion.
	UArrayTetraMesh4D* ToArrayTetraMesh();
	virtual UTetraMesh4D* ToTetraMesh();

	// Standalone data, not sampled by indices.
	virtual const TArray<FVector4>& GetSimplexCellBoundaryNormals() const;

	// Indices which use that data.
	virtual const TArray<int32>& GetSimplexCellNormalIndices() const;
	virtual const TArray<int32>& GetSimplexCellVertexIndices() const;
	virtual const TArray<int32>& GetSimplexCellTextureMapIndices() const;

	virtual UMaterial4D* GetFallbackMaterial() const override;

	static void InitFallbackMaterial();
	static void CleanupFallbackMaterial();

private:
	static UTetraMaterial4D* FallbackMaterial;
};
