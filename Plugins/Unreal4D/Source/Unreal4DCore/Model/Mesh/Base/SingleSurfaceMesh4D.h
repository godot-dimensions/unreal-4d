#pragma once

#include "Mesh4D.h"

#include "SingleSurfaceMesh4D.generated.h"

UCLASS(Abstract, BlueprintType, Category = "Unreal 4D|Mesh")
class UNREAL4DCORE_API USingleSurfaceMesh4D : public UMesh4D
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal 4D|Mesh")
	UMaterial4D* Material = nullptr;

	virtual void CalculateLocalBounds(FVector4& OutOrigin, FVector4& OutExtents) const override;

public:
	UMaterial4D* GetMaterial() const;
	void SetMaterial(UMaterial4D* InMaterial);
	virtual UMaterial4D* GetFallbackMaterial() const;

	// Data which is sampled by indices.
	virtual const TArray<FVector4>& GetVertexValues() const;
	virtual const TArray<FVector4>& GetNormalValues() const;
	virtual const TArray<FVector>& GetTextureMapValues() const;
};
