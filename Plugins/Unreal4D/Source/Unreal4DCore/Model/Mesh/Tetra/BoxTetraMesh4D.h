#pragma once

#include "TetraMesh4D.h"

#include "BoxTetraMesh4D.generated.h"

UENUM(BlueprintType, Category = "Unreal 4D|Mesh|Tetra")
enum class EBoxTetraDecomp4D : uint8
{
	Decomp40Cell = 0,
	Decomp48Cell = 1,
	Decomp48CellPolytope = 2,
};

UENUM(BlueprintType, Category = "Unreal 4D|Mesh|Tetra")
enum class EBoxCellTextureMap4D : uint8
{
	CrossIsland = 0,
	FillEachSide = 1,
	Compact2x2x2Grid = 2,
	LongCross = 3,
};

UCLASS(BlueprintType, Category = "Unreal 4D|Mesh|Tetra")
class UNREAL4DCORE_API UBoxTetraMesh4D : public UTetraMesh4D
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category="Unreal 4D|Mesh|Tetra|Box")
	TArray<FVector4> VertexValuesCache;
	UPROPERTY(EditAnywhere, Category="Unreal 4D|Mesh|Tetra|Box")
	EBoxTetraDecomp4D TetraDecomp = EBoxTetraDecomp4D::Decomp40Cell;
	UPROPERTY(EditAnywhere, Category="Unreal 4D|Mesh|Tetra|Box")
	EBoxCellTextureMap4D CellTextureMap = EBoxCellTextureMap4D::CrossIsland;

	static const TArray<FVector4> NormalValues;
	static const TArray<FVector> TextureMapValuesCrossIsland;
	static const TArray<FVector> TextureMapValuesFillEachSide;
	static const TArray<FVector> TextureMapValuesCompact2x2x2Grid;
	static const TArray<FVector> TextureMapValuesLongCross;

	static const TArray<FVector4> SimplexCellBoundaryNormals40Cell;
	static const TArray<int32> SimplexCellVertexIndices40Cell;
	static const TArray<int32> SimplexCellNormalIndices40Cell;
	static const TArray<int32> SimplexCellTextureMapIndices40CellCrossIsland;
	static const TArray<int32> SimplexCellTextureMapIndices40CellFillEachSide;
	static const TArray<int32> SimplexCellTextureMapIndices40CellCompact2x2x2Grid;
	static const TArray<int32> SimplexCellTextureMapIndices40CellLongCross;

	static const TArray<FVector4> SimplexCellBoundaryNormals48Cell;
	static const TArray<int32> SimplexCellVertexIndices48Cell;
	static const TArray<int32> SimplexCellNormalIndices48Cell;
	static const TArray<int32> SimplexCellTextureMapIndices48CellCrossIsland;
	static const TArray<int32> SimplexCellTextureMapIndices48CellFillEachSide;
	static const TArray<int32> SimplexCellTextureMapIndices48CellCompact2x2x2Grid;
	static const TArray<int32> SimplexCellTextureMapIndices48CellLongCross;

	UPROPERTY(EditAnywhere, Category="Unreal 4D|Mesh|Tetra|Box")
	FVector4 Size = FVector4(100.0, 100.0, 100.0, 100.0);

protected:
	virtual bool ValidateMeshData() override { return true; }

public:
	FVector4 GetHalfExtents() const;
	void SetHalfExtents(const FVector4& InHalfExtents);
	FVector4 GetSize() const { return Size; }
	void SetSize(const FVector4& InSize);
	EBoxTetraDecomp4D GetTetraDecomp() const { return TetraDecomp; }
	void SetTetraDecomp(EBoxTetraDecomp4D InDecomp);
	EBoxCellTextureMap4D GetCellTextureMap() const { return CellTextureMap; }
	void SetCellTextureMap(EBoxCellTextureMap4D InMap);

	virtual const TArray<FVector4>& GetSimplexCellBoundaryNormals() const override;
	virtual const TArray<FVector4>& GetVertexValues() const override;
	virtual const TArray<FVector4>& GetNormalValues() const override;
	virtual const TArray<FVector>& GetTextureMapValues() const override;
	virtual const TArray<int32>& GetSimplexCellVertexIndices() const override;
	virtual const TArray<int32>& GetSimplexCellNormalIndices() const override;
	virtual const TArray<int32>& GetSimplexCellTextureMapIndices() const override;

	UBoxTetraMesh4D() { SetHalfExtents(FVector4(0.5, 0.5, 0.5, 0.5)); }

	virtual void PostLoad() override;
};
