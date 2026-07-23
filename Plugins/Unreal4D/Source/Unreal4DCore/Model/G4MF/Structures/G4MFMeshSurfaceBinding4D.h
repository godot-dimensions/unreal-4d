#pragma once

#include "G4MFItem4D.h"

#include "G4MFMeshSurfaceBinding4D.generated.h"

class UG4MFMeshSurfaceBinding4D;

// UG4MFMeshSurfaceBindingGeometry4D defines how binding values are associated
// with specific geometry's decomposed polytope elements of the mesh surface.
UCLASS(Category = "Unreal 4D|Model|G4MF")
class UNREAL4DCORE_API UG4MFMeshSurfaceBindingGeometry4D : public UG4MFItem4D
{
	GENERATED_BODY()

public:
	UPROPERTY()
	int32 IndicesAccessorIndex = -1;

	UPROPERTY()
	int32 GeometryDimension = -1;

	UPROPERTY()
	int32 DecomposeDimension = 0;

	bool IsEqualExact(const UG4MFMeshSurfaceBindingGeometry4D* InOther) const;
	TArray<int32> LoadIndices(const UG4MFState4D* InG4MFState) const;

	static UG4MFMeshSurfaceBindingGeometry4D* FromJson(UG4MFState4D* InG4MFState, UG4MFMeshSurfaceBinding4D* InOwningBinding, const FJsonObject* InJson);
	TSharedPtr<FJsonObject> ToJson() const;
};

// Mesh surface bindings define how data is associated with mesh elements such as vertices, edges, and simplexes. For example, the tetrahedra of a 4D mesh surface may have texture map coordinates, normals, colors, or other data associated with each corner of the tetrahedra. Bindings define an array of data, and indices which sample that data for specific mesh elements.
UCLASS()
class UG4MFMeshSurfaceBinding4D : public UG4MFItem4D
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TArray<TObjectPtr<UG4MFMeshSurfaceBindingGeometry4D>> GeometryDecompositions;

	UPROPERTY()
	int32 PerSimplexAccessorIndex = -1;

	UPROPERTY()
	int32 SimplexesAccessorIndex = -1;

	UPROPERTY()
	int32 ValuesAccessorIndex = -1;

	void FlipSimplexWindingOrder(UG4MFState4D* InG4MFState);
	bool IsEqualExact(const UG4MFMeshSurfaceBinding4D* InOther) const;

	TArray<int32> LoadPerSimplexIndices(const UG4MFState4D* InG4MFState) const;
	TArray<int32> LoadSimplexIndices(const UG4MFState4D* InG4MFState) const;

	TArray<FLinearColor> LoadValuesAsColors(const UG4MFState4D* InG4MFState) const;
	TArray<FVector> LoadValuesAsVector3s(const UG4MFState4D* InG4MFState) const;
	TArray<FVector4> LoadValuesAsVector4s(const UG4MFState4D* InG4MFState) const;

	static UG4MFMeshSurfaceBinding4D* FromJson(UG4MFState4D* InG4MFState, UObject* InOwner, const FJsonObject* InJson);
	TSharedPtr<FJsonObject> ToJson() const;
};
