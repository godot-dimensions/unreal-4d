#pragma once

#include "G4MFMeshSurfaceBinding4D.h"

#include "G4MFMeshSurface4D.generated.h"

class UArrayTetraMesh4D;
class UMaterial4D;
class UMesh4D;

// UG4MFMeshSurface4D is a class for 4D mesh surfaces in the G4MF format.
// A mesh surface is a collection of vertices, edges, and cells that define
// a part of a mesh. Each surface may have a material.
// Note: Unreal 4D's mesh classes currently only support one surface
// per mesh, but multiple surfaces are allowed in the G4MF format.
UCLASS(Category = "Unreal 4D|Model|G4MF")
class UNREAL4DCORE_API UG4MFMeshSurface4D : public UG4MFItem4D
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TObjectPtr<UG4MFMeshSurfaceBinding4D> NormalsBinding;

	UPROPERTY()
	TObjectPtr<UG4MFMeshSurfaceBinding4D> TextureMapBinding;

	UPROPERTY()
	int32 EdgesAccessorIndex = -1;

	UPROPERTY()
	int32 MaterialIndex = -1;

	UPROPERTY()
	int32 SeamsAccessorIndex = -1;

	UPROPERTY()
	int32 SimplexesAccessorIndex = -1;

	UPROPERTY()
	bool PolytopeSimplexes = false;

	void ConvertCoordinateSystem(UG4MFState4D* InG4MFState, const FMatrix& InOrthonormalBasis, const bool bInFlipSimplexWindingOrder);
	bool IsEqualExact(const UG4MFMeshSurface4D* InOther) const;

	TArray<int32> LoadSimplexIndices(const UG4MFState4D* InG4MFState) const;
	TArray<int32> LoadEdgeIndices(const UG4MFState4D* InG4MFState) const;

	UArrayTetraMesh4D* ImportGenerateTetraMeshSurface(const UG4MFState4D* InG4MFState, const TArray<FVector4>& InVertexValues, UObject* InOuter, const EObjectFlags InObjectFlags = RF_NoFlags) const;
	static UG4MFMeshSurface4D* ExportConvertMeshSurfaceForState(UG4MFState4D* InG4MFState, UMesh4D* InMesh, const bool InDeduplicate = true);

	static UG4MFMeshSurface4D* FromJson(UG4MFState4D* InG4MFState, const FJsonObject* InJson);
	TSharedPtr<FJsonObject> ToJson() const;
};
