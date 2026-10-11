#include "G4MFMesh4D.h"

#include "../../Mesh/Tetra/ArrayTetraMesh4D.h"
#include "../G4MFState4D.h"

UArrayTetraMesh4D* UG4MFMesh4D::ImportGenerateTetraMeshSurface(const UG4MFState4D* InG4MFState, const TArray<FVector4>& InVertexValues, const int32 InSurfaceIndex, UObject* InOuter, const EObjectFlags InObjectFlags) const
{
	if (InG4MFState == nullptr || !IsValid(InG4MFState) || !Surfaces.IsValidIndex(InSurfaceIndex))
	{
		return nullptr;
	}
	const UG4MFMeshSurface4D* surface = Surfaces[InSurfaceIndex];
	if (!IsValid(surface))
	{
		return nullptr;
	}
	return surface->ImportGenerateTetraMeshSurface(InG4MFState, InVertexValues, InOuter, InObjectFlags);
}

bool UG4MFMesh4D::CanGenerateTetraMeshesForAllSurfaces() const
{
	for (int32 surfaceIndex = 0; surfaceIndex < Surfaces.Num(); surfaceIndex++)
	{
		const UG4MFMeshSurface4D* surface = Surfaces[surfaceIndex];
		if (!IsValid(surface) || surface->SimplexesAccessorIndex < 0)
		{
			return false;
		}
	}
	// For the use cases of this function, also return true if there are zero surfaces,
	// but that is a degenerate case, since the spec requires at least one surface per mesh.
	return true;
}

void UG4MFMesh4D::ConvertCoordinateSystem(UG4MFState4D* InG4MFState, const FMatrix& InOrthonormalBasis, const double InAbsScaleFactor, const bool bInFlipSimplexWindingOrder)
{
	if (InG4MFState == nullptr) { return; }
	// Transform any mesh vertices based on the basis and scale factor.
	if (VerticesAccessorIndex >= 0)
	{
		TArray<FVector4> vertices = LoadVertices(InG4MFState);
		for (int32 vertexIndex = 0; vertexIndex < vertices.Num(); vertexIndex++)
		{
			vertices[vertexIndex] = InOrthonormalBasis.TransformFVector4(vertices[vertexIndex]) * InAbsScaleFactor;
		}
		VerticesAccessorIndex = UG4MFAccessor4D::EncodeNewAccessorFromVector4s(InG4MFState, vertices, true);
	}
	// Swap the winding order of all tetrahedral simplex cell indices in the mesh, and transform the normal values.
	for (TObjectPtr<UG4MFMeshSurface4D>& surface : Surfaces)
	{
		if (ensure(IsValid(surface)))
		{
			surface->ConvertCoordinateSystem(InG4MFState, InOrthonormalBasis, bInFlipSimplexWindingOrder);
		}
	}
}

bool UG4MFMesh4D::IsEqualExact(const UG4MFMesh4D* InOther) const
{
	if (InOther == nullptr || !IsValid(InOther)) { return false; }
	const int32 surfaceCount = Surfaces.Num();
	if (surfaceCount != InOther->Surfaces.Num())
	{
		return false;
	}
	if (VerticesAccessorIndex != InOther->VerticesAccessorIndex)
	{
		return false;
	}
	for (int32 surfaceIndex = 0; surfaceIndex < surfaceCount; surfaceIndex++)
	{
		const UG4MFMeshSurface4D* selfSurface = Surfaces[surfaceIndex];
		const UG4MFMeshSurface4D* otherSurface = InOther->Surfaces[surfaceIndex];
		if (!IsValid(selfSurface) || !IsValid(otherSurface))
		{
			return false;
		}
		if (!selfSurface->IsEqualExact(otherSurface))
		{
			return false;
		}
	}
	return true;
}

TArray<FVector4> UG4MFMesh4D::LoadVertices(const UG4MFState4D* InG4MFState) const
{
	if (InG4MFState == nullptr || !IsValid(InG4MFState)) { return TArray<FVector4>(); }
	if (!InG4MFState->G4MFAccessors.IsValidIndex(VerticesAccessorIndex))
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("UG4MFMesh4D::LoadVertices: Invalid vertices accessor index. Returning empty vertex array."));
		return TArray<FVector4>();
	}
	const UG4MFAccessor4D* accessor = InG4MFState->G4MFAccessors[VerticesAccessorIndex];
	if (!IsValid(accessor))
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("UG4MFMesh4D::LoadVertices: Vertices accessor is not valid. Returning empty vertex array."));
		return TArray<FVector4>();
	}
	return accessor->DecodeVector4sFromBytes(InG4MFState);
}

UMesh4D* UG4MFMesh4D::ImportGenerateMesh(const UG4MFState4D* InG4MFState, UObject *InOuter, const EObjectFlags InObjectFlags) const
{
	const int surfaceCount = Surfaces.Num();
	if (surfaceCount == 0)
	{
		UE_LOG(LogUnreal4DCore, Warning, TEXT("UG4MFMesh4D::GenerateMesh: Mesh has no surfaces. Returning null."));
		return nullptr;
	}
	const TArray<FVector4> vertices = LoadVertices(InG4MFState);
	if (vertices.IsEmpty())
	{
		UE_LOG(LogUnreal4DCore, Warning, TEXT("UG4MFMesh4D::GenerateMesh: Mesh has no vertices. Returning null."));
		return nullptr;
	}
	if (surfaceCount > 1)
	{
		UE_LOG(LogUnreal4DCore, Warning, TEXT("UG4MFMesh4D::GenerateMesh: Mesh has more than one surface. These will be merged into one surface."));
	}
	UArrayTetraMesh4D* tetraMesh = ImportGenerateTetraMeshSurface(InG4MFState, vertices, 0, InOuter, InObjectFlags);
	if (tetraMesh == nullptr)
	{
		UE_LOG(LogUnreal4DCore, Warning, TEXT("UG4MFMesh4D::GenerateMesh: Failed to generate a tetra mesh for the surface. Returning null."));
		return nullptr;
	}
	// TODO: The merge_with function is not ideal for this, since it may result in duplicate vertices.
	for (int32 surfaceIndex = 1; surfaceIndex < surfaceCount; surfaceIndex++)
	{
		UArrayTetraMesh4D* nextTetraMesh = ImportGenerateTetraMeshSurface(InG4MFState, vertices, surfaceIndex, InOuter, InObjectFlags);
		if (nextTetraMesh == nullptr)
		{
			UE_LOG(LogUnreal4DCore, Warning, TEXT("UG4MFMesh4D::GenerateMesh: Failed to generate a tetra mesh for surface index %d. Returning the mesh generated so far."), surfaceIndex);
			return tetraMesh;
		}
		tetraMesh->MergeWith(nextTetraMesh);
	}
	return tetraMesh;
}

int32 UG4MFMesh4D::ExportConvertMeshIntoState(UG4MFState4D* InG4MFState, UMesh4D* InMesh, const bool bInDeduplicate)
{
	if (!IsValid(InG4MFState) || !IsValid(InMesh)) { return -1; }
	USingleSurfaceMesh4D* singleSurfaceMesh = Cast<USingleSurfaceMesh4D>(InMesh);
	if (singleSurfaceMesh == nullptr)
	{
		UE_LOG(LogUnreal4DCore, Warning, TEXT("UG4MFMesh4D: Only a USingleSurfaceMesh4D can be converted to a G4MF mesh: '%s'."), *InMesh->GetPathName());
		return -1;
	}
	const TArray<FVector4> vertices = singleSurfaceMesh->GetVertexValues();
	if (vertices.IsEmpty())
	{
		UE_LOG(LogUnreal4DCore, Warning, TEXT("UG4MFMesh4D: Mesh has no vertices, cannot convert to G4MF mesh."));
		return -1;
	}
	const int32 verticesAccessor = UG4MFAccessor4D::EncodeNewAccessorFromVector4s(InG4MFState, vertices, bInDeduplicate);
	if (verticesAccessor < 0)
	{
		UE_LOG(LogUnreal4DCore, Warning, TEXT("UG4MFMesh4D: Failed to encode vertices into UG4MFState4D."));
		return -1;
	}
	UG4MFMeshSurface4D* surface = UG4MFMeshSurface4D::ExportConvertMeshSurfaceForState(InG4MFState, singleSurfaceMesh, bInDeduplicate);
	if (!IsValid(surface))
	{
		UE_LOG(LogUnreal4DCore, Warning, TEXT("UG4MFMesh4D: Failed to convert the mesh surface."));
		return -1;
	}
	// Prepare a G4MFMesh4D with the surface.
	const FName desiredName = FName(InMesh->GetName());
	UG4MFMesh4D *g4mfMesh = NewObject<UG4MFMesh4D>(InG4MFState, desiredName);
	g4mfMesh->VerticesAccessorIndex = verticesAccessor;
	g4mfMesh->Surfaces.Add(surface);
	// Add the G4MFMesh4D to the G4MFState4D, but check for duplicates first.
	const int32 stateMeshCount = InG4MFState->G4MFMeshes.Num();
	if (bInDeduplicate)
	{
		for (int32 meshIndex = 0; meshIndex < stateMeshCount; meshIndex++)
		{
			const UG4MFMesh4D* existingMesh = InG4MFState->G4MFMeshes[meshIndex];
			if (IsValid(existingMesh) && existingMesh->IsEqualExact(g4mfMesh))
			{
				return meshIndex;
			}
		}
	}
	InG4MFState->G4MFMeshes.Add(g4mfMesh);
	return stateMeshCount;
}

UG4MFMesh4D* UG4MFMesh4D::FromJson(UG4MFState4D* InG4MFState, const FJsonObject* InJson)
{
	// General code for all UG4MFItem4D-derived classes.
	if (InJson == nullptr) { return nullptr; }
	const FName uniqueName = InG4MFState->ReserveUniqueFNameFromJson(InJson);
	UG4MFMesh4D* Mesh = NewObject<UG4MFMesh4D>(InG4MFState, uniqueName);
	Mesh->ReadItemEntriesFromJson(InJson);
	// UG4MFMesh4D-specific code.
	if (InJson->HasField(TEXT("surfaces")))
	{
		const TArray<TSharedPtr<FJsonValue>> surfacesJsonArray = InJson->GetArrayField(TEXT("surfaces"));
		const int32 surfaceCount = surfacesJsonArray.Num();
		Mesh->Surfaces.SetNum(surfaceCount);
		for (int32 i = 0; i < surfaceCount; ++i)
		{
			const FJsonObject* surfaceJsonObjectPtr = surfacesJsonArray[i]->AsObject().Get();
			if (surfaceJsonObjectPtr != nullptr)
			{
				Mesh->Surfaces[i] = UG4MFMeshSurface4D::FromJson(InG4MFState, surfaceJsonObjectPtr);
			}
		}
	}
	if (InJson->HasField(TEXT("vertices")))
	{
		Mesh->VerticesAccessorIndex = static_cast<int32>(InJson->GetNumberField(TEXT("vertices")));
	}
	return Mesh;
}

TSharedPtr<FJsonObject> UG4MFMesh4D::ToJson() const
{
	TSharedPtr<FJsonObject> json = WriteItemEntriesToJson();
	if (Surfaces.Num() > 0)
	{
		TArray<TSharedPtr<FJsonValue>> surfacesJsonArray;
		surfacesJsonArray.SetNum(Surfaces.Num());
		for (int32 i = 0; i < Surfaces.Num(); ++i)
		{
			const TObjectPtr<UG4MFMeshSurface4D> surface = Surfaces[i];
			if (IsValid(surface))
			{
				TSharedPtr<FJsonObject> surfaceJson = surface->ToJson();
				surfacesJsonArray[i] = MakeShared<FJsonValueObject>(surfaceJson);
			}
		}
		json->SetArrayField(TEXT("surfaces"), surfacesJsonArray);
	}
	if (VerticesAccessorIndex >= 0)
	{
		json->SetNumberField(TEXT("vertices"), static_cast<double>(VerticesAccessorIndex));
	}
	return json;
}
