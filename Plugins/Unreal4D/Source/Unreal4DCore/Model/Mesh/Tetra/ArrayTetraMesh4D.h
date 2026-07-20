#pragma once

#include "TetraMesh4D.h"

#include "../../../Math/Transform4D.h"
#include "../Base/Mesh4D.h"

#include "ArrayTetraMesh4D.generated.h"

UCLASS(BlueprintType, Category = "Unreal 4D|Mesh|Tetra")
class UNREAL4DCORE_API UArrayTetraMesh4D : public UTetraMesh4D
{
	GENERATED_BODY()

private:
	// These properties are private to ensure that mesh validation and
	// cache management logic is always executed when modifying them.
	// The rendering code does not check these each time, it assumes they are valid.

	// Standalone data, not sampled by indices.
	UPROPERTY()
	TArray<FVector4> SimplexCellBoundaryNormals;

	// Data which is sampled by indices.
	UPROPERTY()
	TArray<FVector4> VertexValues;
	UPROPERTY()
	TArray<FVector4> NormalValues;
	UPROPERTY()
	TArray<FVector> TextureMapValues;

	// Indices which use that data.
	UPROPERTY()
	TArray<int32> SimplexCellVertexIndices;
	UPROPERTY()
	TArray<int32> SimplexCellNormalIndices;
	UPROPERTY()
	TArray<int32> SimplexCellTextureMapIndices;

	FCriticalSection ArrayMeshModifyOrCacheLock;
	int32 AppendVertex_NoLock(const FVector4& InVertex, bool bInDeduplicateVertices = true);
	void CalculateBoundaryNormals_NoLock(bool bInKeepExisting = false);
	void SetFlatShadingNormals_NoLock(bool bInForceRecalculateBoundaryNormals = false);

protected:
	virtual bool ValidateMeshData() override;

public:
	// Append functions.
	void AppendTetraCellPoints(const FVector4& InA, const FVector4& InB, const FVector4& InC, const FVector4& InD, bool bInDeduplicateVertices = true);
	void AppendTetraCellIndices(int32 InIndexA, int32 InIndexB, int32 InIndexC, int32 InIndexD);
	int32 AppendVertex(const FVector4& InVertex, bool bInDeduplicateVertices = true);
	TArray<int32> AppendVertices(const TArray<FVector4>& InVertices, bool bInDeduplicateVertices = true);

	// Misc functions.
	void MergeWith(UArrayTetraMesh4D* InOther, const FTransform4D& InTransform = FTransform4D());
	bool OverwriteFromOtherMesh(UMesh4D* InFromMesh);
	void CalculateBoundaryNormals(bool bInKeepExisting = false);
	void SetFlatShadingNormals(bool bInForceRecalculateBoundaryNormals = false);

	// Standalone data, not sampled by indices.
	virtual const TArray<FVector4>& GetSimplexCellBoundaryNormals() const override;
	void SetSimplexCellBoundaryNormals(const TArray<FVector4>& InSimplexCellBoundaryNormals);

	// Data which is sampled by indices.
	virtual const TArray<FVector4>& GetVertexValues() const override;
	void SetVertexValues(const TArray<FVector4>& InVertexValues);
	virtual const TArray<FVector4>& GetNormalValues() const override;
	void SetNormalValues(const TArray<FVector4>& InNormalValues);
	virtual const TArray<FVector>& GetTextureMapValues() const override;
	void SetTextureMapValues(const TArray<FVector>& InTextureMapValues);

	// Indices which use that data.
	virtual const TArray<int32>& GetSimplexCellVertexIndices() const override;
	void SetSimplexCellVertexIndices(const TArray<int32>& InVertexIndices);
	virtual const TArray<int32>& GetSimplexCellNormalIndices() const override;
	void SetSimplexCellNormalIndices(const TArray<int32>& InNormalIndices);
	virtual const TArray<int32>& GetSimplexCellTextureMapIndices() const override;
	void SetSimplexCellTextureMapIndices(const TArray<int32>& InTextureMapIndices);

	virtual void PostLoad() override;
};
