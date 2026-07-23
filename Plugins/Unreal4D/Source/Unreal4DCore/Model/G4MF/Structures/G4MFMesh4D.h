#pragma once

#include "G4MFMeshSurface4D.h"

#include "G4MFMesh4D.generated.h"

// UG4MFMesh4D is a class for 4D meshes in the G4MF format. It is used to represent
// visual geometry found in a G4MF scene. It may be attached to G4MF nodes using a
// UG4MFMeshInstance4D so that it may be rendered in Unreal at the node's 4D transform.
// A mesh is a collection of surfaces, each of which may have different materials.
// Note: Unreal 4D's mesh classes currently only support one surface
// per mesh, but multiple surfaces are allowed in the G4MF format.
UCLASS(Category = "Unreal 4D|Model|G4MF")
class UNREAL4DCORE_API UG4MFMesh4D : public UG4MFItem4D
{
	GENERATED_BODY()

private:
	UArrayTetraMesh4D* ImportGenerateTetraMeshSurface(const UG4MFState4D* InG4MFState, const TArray<FVector4>& InVertexValues, const int32 InSurface, UObject* InOuter, const EObjectFlags InObjectFlags = RF_NoFlags) const;

public:
	UPROPERTY()
	TArray<TObjectPtr<UG4MFMeshSurface4D>> Surfaces;

	UPROPERTY()
	int32 VerticesAccessorIndex = -1;

	bool CanGenerateTetraMeshesForAllSurfaces() const;
	void ConvertCoordinateSystem(UG4MFState4D* InG4MFState, const FMatrix& InOrthonormalBasis, const double InAbsScaleFactor, const bool bInFlipSimplexWindingOrder);
	bool IsEqualExact(const UG4MFMesh4D* InOther) const;

	TArray<FVector4> LoadVertices(const UG4MFState4D* InG4MFState) const;
	UMesh4D* ImportGenerateMesh(const UG4MFState4D* InG4MFState, UObject *InOuter, const EObjectFlags InObjectFlags = RF_NoFlags) const;
	bool ImportOverwriteMesh(const UG4MFState4D* InG4MFState, UMesh4D* InMesh) const;
	static int32 ExportConvertMeshIntoState(UG4MFState4D* InG4MFState, UMesh4D* InMesh, const bool InDeduplicate = true);

	static UG4MFMesh4D* FromJson(UG4MFState4D* InG4MFState, const FJsonObject* InJson);
	TSharedPtr<FJsonObject> ToJson() const;
};
