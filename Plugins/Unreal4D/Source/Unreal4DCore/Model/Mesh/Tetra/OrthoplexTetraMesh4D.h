#pragma once

#include "TetraMesh4D.h"

#include "OrthoplexTetraMesh4D.generated.h"

UCLASS(BlueprintType, Category = "Unreal 4D|Mesh|Tetra")
class UNREAL4DCORE_API UOrthoplexTetraMesh4D : public UTetraMesh4D
{
	GENERATED_BODY()

private:
	TArray<FVector4> VertexValuesCache;
	TArray<FVector4> NormalValuesCache;

	static const TArray<FVector4> NormalValuesStatic;
	static const TArray<FVector> TextureMapValuesStatic;
	static const TArray<int32> SimplexCellVertexIndices;
	static const TArray<int32> SimplexCellNormalIndices;
	static const TArray<int32> SimplexCellTextureMapIndices;

	UPROPERTY(EditAnywhere, Category="Unreal 4D|Mesh|Tetra|Orthoplex")
	FVector4 Size = FVector4(100.0, 100.0, 100.0, 100.0);

protected:
	virtual bool ValidateMeshData() override { return true; }

public:
	FVector4 GetHalfExtents() const;
	void SetHalfExtents(const FVector4& InHalfExtents);
	FVector4 GetSize() const { return Size; }
	void SetSize(const FVector4& InSize);

	virtual const TArray<FVector4>& GetSimplexCellBoundaryNormals() const override;
	virtual const TArray<FVector4>& GetVertexValues() const override;
	virtual const TArray<FVector4>& GetNormalValues() const override;
	virtual const TArray<FVector>& GetTextureMapValues() const override;
	virtual const TArray<int32>& GetSimplexCellVertexIndices() const override;
	virtual const TArray<int32>& GetSimplexCellNormalIndices() const override;
	virtual const TArray<int32>& GetSimplexCellTextureMapIndices() const override;

	UOrthoplexTetraMesh4D() { SetHalfExtents(FVector4(0.5, 0.5, 0.5, 0.5)); }

	virtual void PostLoad() override;
};
