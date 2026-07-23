#pragma once

#include "Physics/G4MFNodePhysics4D.h"
#include "G4MFCamera4D.h"
#include "G4MFLight4D.h"
#include "G4MFMeshInstance4D.h"
#include "G4MFModelInstance4D.h"

#include "G4MFNode4D.generated.h"

class UScene4DComponent;

// UG4MFNode4D represents all nodes in a G4MF file. All G4MF nodes have a transform,
// have an array of child nodes, and may have other data set on them.
UCLASS(Category = "Unreal 4D|Model|G4MF")
class UNREAL4DCORE_API UG4MFNode4D : public UG4MFItem4D
{
	GENERATED_BODY()

public:
	// Node hierarchy.
	UPROPERTY()
	TArray<int32> ChildrenIndices;

	UPROPERTY()
	int32 ParentIndex = -1;

	UPROPERTY()
	bool bIsVisible = true;

	// Transform properties.
	UPROPERTY()
	FTransform4D Transform = FTransform4D();

	// Functional properties.
	UPROPERTY()
	TObjectPtr<UG4MFCamera4D> Camera;

	UPROPERTY()
	TObjectPtr<UG4MFLight4D> Light;

	UPROPERTY()
	TObjectPtr<UG4MFMeshInstance4D> MeshInstance;

	UPROPERTY()
	TObjectPtr<UG4MFModelInstance4D> ModelInstance;

	UPROPERTY()
	TObjectPtr<UG4MFNodePhysics4D> Physics;

	// Convenience accessors for transform components
	FVector4 GetPosition() const { return Transform.Origin; }
	void SetPosition(const FVector4& InPosition) { Transform.Origin = InPosition; }

	FVector4 GetScale() const;
	void SetScale(const FVector4& InScale);

	FTransform4D GetSceneGlobalTransform(const UG4MFState4D* InG4MFState) const;

	// This split is needed:
	// 1) To allow for the component to be given a unique node name and index before any child component parts.
	// 2) By UG4MFDocument4D for models, so we can skip the "functional components" for the model instance node.
	static UG4MFNode4D* FromSceneComponentBasic(UG4MFState4D* InG4MFState, const UScene4DComponent* InComponent);
	void ExportConvertFunctionalComponents(UG4MFState4D* InG4MFState, const UScene4DComponent* InComponent);
	// However, for the main version, we can just make a single function that wraps both of those.
	static UG4MFNode4D* FromSceneComponent(UG4MFState4D* InG4MFState, const UScene4DComponent* InComponent);

	UScene4DComponent* ImportGenerateSceneComponent(UG4MFState4D* InG4MFState, AActor* InActor) const;
	void ApplyToSceneComponent(USceneComponent* InComponent) const;

	static UG4MFNode4D* FromJsonNameOnly(UG4MFState4D* InG4MFState, const FJsonObject* InJson);
	void ReadNodeEntriesFromJson(UG4MFState4D* InG4MFState, const FJsonObject* InJson);
	TSharedPtr<FJsonObject> ToJson(const bool InPreferBasis = true) const;
};
