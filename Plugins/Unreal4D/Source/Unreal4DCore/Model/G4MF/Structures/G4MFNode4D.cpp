#include "G4MFNode4D.h"

#include "../../../Scene/Components/Scene4DComponent.h"
#include "../../../Scene/Components/Camera4DComponent.h"
#include "../../../Scene/Components/Mesh4DComponent.h"
#include "../G4MFState4D.h"
#include "G4MFCamera4D.h"
#include "G4MFMesh4D.h"
#include "G4MFModel4D.h"

// Returns the global transform of this node relative to the root node of the scene.
FTransform4D UG4MFNode4D::GetSceneGlobalTransform(const UG4MFState4D* InG4MFState) const
{
	FTransform4D globalTransform = Transform;
	if (ParentIndex <= 0 || InG4MFState == nullptr)
	{
		return globalTransform;
	}
	int32 currentAncestor = ParentIndex;
	// Avoid cycles. This is pedantic because G4MF forbids cycles, but we want to be robust against malformed G4MF files.
	TSet<int32> visitedAncestors;
	// This is not >= 0 because we are calculating the global transform relative to the root node, which is at index 0.
	while (currentAncestor > 0)
	{
		if (!InG4MFState->G4MFNodes.IsValidIndex(currentAncestor) || visitedAncestors.Contains(currentAncestor))
		{
			return globalTransform;
		}
		visitedAncestors.Add(currentAncestor);
		const TObjectPtr<UG4MFNode4D> currentG4MFNode = InG4MFState->G4MFNodes[currentAncestor];
		if (!IsValid(currentG4MFNode))
		{
			return globalTransform;
		}
		globalTransform = currentG4MFNode->Transform.Compose(globalTransform);
		currentAncestor = currentG4MFNode->ParentIndex;
	}
	return globalTransform;
}

FVector4 UG4MFNode4D::GetScale() const
{
	return UBasis4D::Basis4D_GetScale(Transform.Basis);
}

void UG4MFNode4D::SetScale(const FVector4& InScale)
{
	Transform.Basis = UBasis4D::Basis4D_WithScale(Transform.Basis, InScale);
}

UG4MFNode4D* UG4MFNode4D::FromSceneComponentBasic(UG4MFState4D* InG4MFState, const UScene4DComponent* InComponent)
{
	if (InG4MFState == nullptr || InComponent == nullptr) { return nullptr; }
	// Note: The name will be reserved as a unique name later in `G4MFState4D::AppendG4MFNode()`
	const FName desiredName = FName(InComponent->GetName());
	UG4MFNode4D* g4mfNode = NewObject<UG4MFNode4D>(InG4MFState, desiredName);
	g4mfNode->bIsVisible = InComponent->IsVisible();
	const UScene4DComponent* component4D = Cast<UScene4DComponent>(InComponent);
	if (IsValid(component4D))
	{
		g4mfNode->Transform = component4D->GetLocalTransform4D();
	}
	return g4mfNode;
}

void UG4MFNode4D::ExportConvertFunctionalComponents(UG4MFState4D* InG4MFState, const UScene4DComponent* InComponent)
{
	if (InG4MFState == nullptr || InComponent == nullptr) { return; }
	// Convert functional parts from this component into the node's properties.
	const UMesh4DComponent* meshComponent = Cast<UMesh4DComponent>(InComponent);
	if (IsValid(meshComponent))
	{
		MeshInstance = UG4MFMeshInstance4D::ExportConvertMeshComponent(InG4MFState, meshComponent);
		// G4MF only allows one functional component of each type per node, so we can stop after the first one.
		return;
	}
	// Convert functional parts from any component children into the node's properties.
	for (USceneComponent* child : InComponent->GetAttachChildren())
	{
		const UCamera4DComponent* cameraComponent = Cast<UCamera4DComponent>(child);
		if (IsValid(cameraComponent))
		{
			Camera = UG4MFCamera4D::ExportConvertCameraComponent(InG4MFState, cameraComponent);
			return;
		}
	}
}

UG4MFNode4D* UG4MFNode4D::FromSceneComponent(UG4MFState4D* InG4MFState, const UScene4DComponent* InComponent)
{
	if (InG4MFState == nullptr || InComponent == nullptr) { return nullptr; }
	TObjectPtr<UG4MFNode4D> g4mfNode = FromSceneComponentBasic(InG4MFState, InComponent);
	g4mfNode->ExportConvertFunctionalComponents(InG4MFState, InComponent);
	return g4mfNode;
}

UScene4DComponent* UG4MFNode4D::ImportGenerateSceneComponent(UG4MFState4D* InG4MFState, AActor* InActor) const
{
	UScene4DComponent* ret = nullptr;
	// These must be the root.
	if (IsValid(ModelInstance))
	{
		// TODO: Apply ModelInstance node overrides and additional children after
		// the referenced hierarchy has been instantiated.
		ret = ModelInstance->ImportInstantiateModelComponent(InG4MFState, InActor);
	}
	else if (IsValid(Physics))
	{
		UE_LOG(LogUnreal4DCore, Warning, TEXT("UG4MFNode4D::ImportGenerateSceneComponent: Physics import not yet implemented."));
		//ret = Physics->ImportGeneratePhysicsComponent(InG4MFState, this, InActor);
	}
	// These may be the root.
	if (ret == nullptr)
	{
		const FName desiredName = FName(GetItemName());
		if (IsValid(MeshInstance))
		{
			ret = MeshInstance->ImportGenerateMeshComponent(InG4MFState, InActor);
		}
		else
		{
			ret = NewObject<UScene4DComponent>(InActor, desiredName);
		}
	}
	// These must be children of the root.
	if (IsValid(Light))
	{
		// TODO: Generate an Unreal 4D light component once a runtime light component/API exists.
		UE_LOG(LogUnreal4DCore, Warning, TEXT("UG4MFNode4D::ImportGenerateSceneComponent: Light import not yet implemented."));
	}
	if (IsValid(Camera))
	{
		UCamera4DComponent* cameraComponent = Camera->ImportGenerateCameraComponent(InG4MFState, InActor);
		if (!IsValid(cameraComponent))
		{
			UE_LOG(LogUnreal4DCore, Error, TEXT("UG4MFNode4D::ImportGenerateSceneComponent: Failed to generate camera component."));
			return nullptr;
		}
		cameraComponent->SetupAttachment(ret);
	}
	ApplyToSceneComponent(ret);
	return ret;
}

void UG4MFNode4D::ApplyToSceneComponent(USceneComponent* InComponent) const
{
	const FString itemName = GetItemName();
	// This name is guaranteed to be a valid unique non-empty name by the logic in
	// `G4MFState4D::ReserveUniqueNamesForNodeAndSubItems()`, but assert just in case.
	ensure(!itemName.IsEmpty());
	// Check if the name matches the current name or if the outer already has that name.
	if (InComponent->GetName() != itemName && FindObject<UObject>(InComponent->GetOuter(), *itemName) == nullptr)
	{
		InComponent->Rename(*itemName);
	}
	UScene4DComponent* component4D = Cast<UScene4DComponent>(InComponent);
	if (IsValid(component4D))
	{
		component4D->SetLocalTransform4D(Transform);
		component4D->SetVisible4D(bIsVisible);
	}
	else
	{
		InComponent->SetVisibility(bIsVisible);
	}
	// TODO: Handle extras.
}

UG4MFNode4D* UG4MFNode4D::FromJsonNameOnly(UG4MFState4D* InG4MFState, const FJsonObject* InJson)
{
	// General code for all UG4MFItem4D-derived classes.
	if (InG4MFState == nullptr || InJson == nullptr) { return nullptr; }
	const FName uniqueName = InG4MFState->ReserveUniqueFNameFromJson(InJson);
	UG4MFNode4D* ret = NewObject<UG4MFNode4D>(InG4MFState, uniqueName);
	ret->ReadItemEntriesFromJson(InJson);
	return ret;
}

void UG4MFNode4D::ReadNodeEntriesFromJson(UG4MFState4D* InG4MFState, const FJsonObject* InJson)
{
	if (InG4MFState == nullptr || InJson == nullptr) { return; }
	// UG4MFNode4D-specific code.
	if (InJson->HasField(TEXT("basis")))
	{
		Transform.Basis = UBasis4D::Basis4D_FromJsonArray(InJson->GetArrayField(TEXT("basis")));
	}
	else
	{
		if (InJson->HasField(TEXT("rotor")))
		{
			UE_LOG(LogUnreal4DCore, Warning, TEXT("UG4MFNode4D::FromJson: Rotor import not yet implemented. Ignoring rotor data."));
		}
		if (InJson->HasField(TEXT("scale")))
		{
			const TArray<TSharedPtr<FJsonValue>> scaleArray = InJson->GetArrayField(TEXT("scale"));
			if (scaleArray.Num() == 1)
			{
				double uniformScale = 1.0;
				if (scaleArray[0]->TryGetNumber(uniformScale))
				{
					Transform.Basis = UBasis4D::Basis4D_ScaledUniform(Transform.Basis, uniformScale);
				}
			}
			else
			{
				FVector4 scale = UVectorFuncs4D::Vector4_FromJsonArray(scaleArray);
				if (!scale.Equals(FVector4(0.0, 0.0, 0.0, 0.0)) && !scale.Equals(FVector4(1.0, 1.0, 1.0, 1.0)))
				{
					Transform.Basis = UBasis4D::Basis4D_ScaledLocal(Transform.Basis, scale);
				}
			}
		}
	}
	if (InJson->HasField(TEXT("children")))
	{
		ChildrenIndices = JsonArrayToInt32Array(InJson->GetArrayField(TEXT("children")));
	}
	if (InJson->HasField(TEXT("parent")))
	{
		// Not a part of the G4MF spec but useful if an extension wants to
		// compose external G4MF nodes on top of a G4MF scene.
		ParentIndex = InJson->GetIntegerField(TEXT("parent"));
	}
	if (InJson->HasField(TEXT("position")))
	{
		Transform.Origin = UVectorFuncs4D::Vector4_FromJsonArray(InJson->GetArrayField(TEXT("position")));
	}
	if (InJson->HasField(TEXT("visible")))
	{
		bIsVisible = InJson->GetBoolField(TEXT("visible"));
	}
	// Functional properties (in alphabetical order).
	if (InJson->HasField(TEXT("camera")))
	{
		const TSharedPtr<FJsonObject>& cameraJson = InJson->GetObjectField(TEXT("camera"));
		Camera = UG4MFCamera4D::FromJson(InG4MFState, cameraJson.Get());
	}
	if (InJson->HasField(TEXT("light")))
	{
		const TSharedPtr<FJsonObject>& lightJson = InJson->GetObjectField(TEXT("light"));
		Light = UG4MFLight4D::FromJson(InG4MFState, lightJson.Get());
	}
	if (InJson->HasField(TEXT("meshInstance")))
	{
		const TSharedPtr<FJsonObject>& meshInstanceJson = InJson->GetObjectField(TEXT("meshInstance"));
		MeshInstance = UG4MFMeshInstance4D::FromJson(InG4MFState, meshInstanceJson.Get());
	}
	if (InJson->HasField(TEXT("modelInstance")))
	{
		const TSharedPtr<FJsonObject>& modelInstanceJson = InJson->GetObjectField(TEXT("modelInstance"));
		ModelInstance = UG4MFModelInstance4D::FromJson(InG4MFState, modelInstanceJson.Get());
	}
	if (InJson->HasField(TEXT("physics")))
	{
		const TSharedPtr<FJsonObject>& nodePhysicsJson = InJson->GetObjectField(TEXT("physics"));
		Physics = UG4MFNodePhysics4D::FromJson(InG4MFState, nodePhysicsJson.Get());
	}
}

TSharedPtr<FJsonObject> UG4MFNode4D::ToJson(const bool bInPreferBasis) const
{
	TSharedPtr<FJsonObject> ret = UG4MFItem4D::WriteItemEntriesToJson();
	if (!Transform.Basis.Equals(FMatrix::Identity, UNREAL4D_COMPARISON_EPSILON))
	{
		if (!bInPreferBasis && UBasis4D::Basis4D_IsDiagonal(Transform.Basis) && Transform.Basis.M[0][0] > 0.0 && Transform.Basis.M[1][1] > 0.0 && Transform.Basis.M[2][2] > 0.0 && Transform.Basis.M[3][3] > 0.0)
		{
			const FVector4 scaleAbs = UBasis4D::Basis4D_GetScaleAbs(Transform.Basis);
			TArray<TSharedPtr<FJsonValue>> scaleJsonArray;
			scaleJsonArray.Add(MakeShared<FJsonValueNumber>(scaleAbs.X));
			if (FMath::IsNearlyEqual(scaleAbs.X, scaleAbs.Y, UNREAL4D_COMPARISON_EPSILON) && FMath::IsNearlyEqual(scaleAbs.X, scaleAbs.Z, UNREAL4D_COMPARISON_EPSILON) && FMath::IsNearlyEqual(scaleAbs.X, scaleAbs.W, UNREAL4D_COMPARISON_EPSILON))
			{
				// Uniform scale (don't need to add Y, Z, and W).
			}
			else
			{
				// Non-uniform scale (add Y, Z, and W).
				scaleJsonArray.Add(MakeShared<FJsonValueNumber>(scaleAbs.Y));
				scaleJsonArray.Add(MakeShared<FJsonValueNumber>(scaleAbs.Z));
				scaleJsonArray.Add(MakeShared<FJsonValueNumber>(scaleAbs.W));
			}
			ret->SetArrayField(TEXT("scale"), scaleJsonArray);
		}
		else
		{
			ret->SetArrayField(TEXT("basis"), UBasis4D::Basis4D_ToJsonArray(Transform.Basis));
		}
	}
	if (ChildrenIndices.Num() > 0)
	{
		ret->SetArrayField(TEXT("children"), Int32ArrayToJsonArray(ChildrenIndices));
	}
	if (!Transform.Origin.Equals(FVector4(0.0, 0.0, 0.0, 0.0), UNREAL4D_COMPARISON_EPSILON))
	{
		ret->SetArrayField(TEXT("position"), UVectorFuncs4D::Vector4_ToJsonArray(Transform.Origin));
	}
	if (!bIsVisible)
	{
		ret->SetBoolField(TEXT("visible"), bIsVisible);
	}
	// Functional properties (in alphabetical order).
	if (IsValid(Camera))
	{
		ret->SetObjectField(TEXT("camera"), Camera->ToJson());
	}
	if (IsValid(Light))
	{
		ret->SetObjectField(TEXT("light"), Light->ToJson());
	}
	if (IsValid(MeshInstance))
	{
		ret->SetObjectField(TEXT("meshInstance"), MeshInstance->ToJson());
	}
	if (IsValid(ModelInstance))
	{
		ret->SetObjectField(TEXT("modelInstance"), ModelInstance->ToJson());
	}
	if (IsValid(Physics))
	{
		ret->SetObjectField(TEXT("physics"), Physics->ToJson());
	}
	return ret;
}
