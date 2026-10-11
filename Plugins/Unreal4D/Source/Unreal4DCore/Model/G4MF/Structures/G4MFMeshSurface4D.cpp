#include "G4MFMeshSurface4D.h"

#include "../../Mesh/Tetra/ArrayTetraMesh4D.h"
#include "../../Mesh/Tetra/BoxTetraMesh4D.h"
#include "../G4MFState4D.h"

void UG4MFMeshSurface4D::ConvertCoordinateSystem(UG4MFState4D* InG4MFState, const FMatrix& InOrthonormalBasis, const bool bInFlipSimplexWindingOrder)
{
	if (InG4MFState == nullptr) { return; }
	// The normal map values need to be transformed by the orthonormal basis, but the texture map values do not.
	if (IsValid(NormalsBinding))
	{
		TArray<FVector4> normalValues = NormalsBinding->LoadValuesAsVector4s(InG4MFState);
		for (int32 normalIndex = 0; normalIndex < normalValues.Num(); normalIndex++)
		{
			normalValues[normalIndex] = InOrthonormalBasis.TransformFVector4(normalValues[normalIndex]);
		}
		NormalsBinding->ValuesAccessorIndex = UG4MFAccessor4D::EncodeNewAccessorFromVector4s(InG4MFState, normalValues, true);
	}
	// Swap the winding order when the determinant is negative to maintain a consistent winding order convention.
	if (!bInFlipSimplexWindingOrder)
	{
		return;
	}
	if (SimplexesAccessorIndex >= 0)
	{
		TArray<int32> simplexes = LoadSimplexIndices(InG4MFState);
		for (int32 simplexIndex = 0; simplexIndex < simplexes.Num(); simplexIndex += 4)
		{
			// This is an arbitrary choice, but swap indices 2 and 3 to keep the first edge the same (0 to 1).
			// NOTE: Make sure this follows the same convention as `UG4MFDocument4D::ConvertCoordinateSystem`!
			Swap(simplexes[simplexIndex + 2], simplexes[simplexIndex + 3]);
		}
		SimplexesAccessorIndex = UG4MFAccessor4D::EncodeNewAccessorFromInts<int32>(InG4MFState, simplexes, true);
	}
	if (IsValid(NormalsBinding))
	{
		NormalsBinding->FlipSimplexWindingOrder(InG4MFState);
	}
	if (IsValid(TextureMapBinding))
	{
		TextureMapBinding->FlipSimplexWindingOrder(InG4MFState);
	}
}

bool UG4MFMeshSurface4D::IsEqualExact(const UG4MFMeshSurface4D* InOther) const
{
	if (InOther == nullptr || !IsValid(InOther)) return false;
	if (EdgesAccessorIndex != InOther->EdgesAccessorIndex
		|| MaterialIndex != InOther->MaterialIndex
		|| SeamsAccessorIndex != InOther->SeamsAccessorIndex
		|| SimplexesAccessorIndex != InOther->SimplexesAccessorIndex
		|| PolytopeSimplexes != InOther->PolytopeSimplexes
		|| IsValid(NormalsBinding) != IsValid(InOther->NormalsBinding)
		|| IsValid(TextureMapBinding) != IsValid(InOther->TextureMapBinding))
	{
		return false;
	}
	if (IsValid(NormalsBinding) && !NormalsBinding->IsEqualExact(InOther->NormalsBinding))
	{
		return false;
	}
	if (IsValid(TextureMapBinding) && !TextureMapBinding->IsEqualExact(InOther->TextureMapBinding))
	{
		return false;
	}
	return true;
}

TArray<int32> UG4MFMeshSurface4D::LoadSimplexIndices(const UG4MFState4D* InG4MFState) const
{
	if (InG4MFState == nullptr) { return TArray<int32>(); }
	if (!InG4MFState->G4MFAccessors.IsValidIndex(SimplexesAccessorIndex))
	{
		return TArray<int32>();
	}
	const UG4MFAccessor4D* accessor = InG4MFState->G4MFAccessors[SimplexesAccessorIndex];
	if (!IsValid(accessor))
	{
		return TArray<int32>();
	}
	TArray<int32> ret = accessor->DecodeNumbersFromBytes<int32>(InG4MFState);
	if (ret.Num() % 4 != 0)
	{
		return TArray<int32>();
	}
	return ret;
}

TArray<int32> UG4MFMeshSurface4D::LoadEdgeIndices(const UG4MFState4D* InG4MFState) const
{
	if (InG4MFState == nullptr) { return TArray<int32>(); }
	if (!InG4MFState->G4MFAccessors.IsValidIndex(EdgesAccessorIndex))
	{
		return TArray<int32>();
	}
	const TObjectPtr<UG4MFAccessor4D> accessor = InG4MFState->G4MFAccessors[EdgesAccessorIndex];
	if (!IsValid(accessor))
	{
		return TArray<int32>();
	}
	TArray<int32> ret = accessor->DecodeNumbersFromBytes<int32>(InG4MFState);
	if (ret.Num() % 2 != 0)
	{
		return TArray<int32>();
	}
	return ret;
}

UArrayTetraMesh4D* UG4MFMeshSurface4D::ImportGenerateTetraMeshSurface(const UG4MFState4D* InG4MFState, const TArray<FVector4>& InVertexValues, UObject* InOuter, const EObjectFlags InObjectFlags) const
{
	if (InG4MFState == nullptr) { return nullptr; }
	const FName desiredName = FName(GetItemName());
	UArrayTetraMesh4D* ret = NewObject<UArrayTetraMesh4D>(InOuter, desiredName, InObjectFlags);
	ret->SetVertexValues(InVertexValues);
	if (SimplexesAccessorIndex >= 0)
	{
		ret->SetSimplexCellVertexIndices(LoadSimplexIndices(InG4MFState));
		ret->CalculateBoundaryNormals();
	}
	if (IsValid(NormalsBinding))
	{
		const TArray<FVector4> normalValues = NormalsBinding->LoadValuesAsVector4s(InG4MFState);
		const TArray<int32> normalIndices = NormalsBinding->LoadSimplexIndices(InG4MFState);
		ret->SetNormalValues(normalValues);
		ret->SetSimplexCellNormalIndices(normalIndices);
	}
	if (IsValid(TextureMapBinding))
	{
		const TArray<FVector> textureMapValues = TextureMapBinding->LoadValuesAsVector3s(InG4MFState);
		const TArray<int32> textureMapIndices = TextureMapBinding->LoadSimplexIndices(InG4MFState);
		ret->SetTextureMapValues(textureMapValues);
		ret->SetSimplexCellTextureMapIndices(textureMapIndices);
	}
	bool bIsValid = ret->IsMeshDataValid();
	if (!bIsValid)
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("UG4MFMeshSurface4D: The mesh data is not valid. Returning null instead."));
		return nullptr;
	}
	ret->UpdateLocalBounds();
	ret->PopulateInverseMetricCache();
	if (MaterialIndex >= 0)
	{
		if (InG4MFState->G4MFMaterials.IsValidIndex(MaterialIndex))
		{
			const UG4MFMaterial4D* g4mfMaterial = InG4MFState->G4MFMaterials[MaterialIndex];
			if (IsValid(g4mfMaterial))
			{
				UTetraMaterial4D* material = g4mfMaterial->ImportGenerateTetraMaterial(InG4MFState, ret);
				ret->SetMaterial(material);
			}
		}
	}
	return ret;
}

UG4MFMeshSurface4D* UG4MFMeshSurface4D::ExportConvertMeshSurfaceForState(UG4MFState4D* InG4MFState, USingleSurfaceMesh4D* InMesh, const bool bInDeduplicate)
{
	if (InG4MFState == nullptr || InMesh == nullptr) { return nullptr; }
	const FName desiredName = FName(InMesh->GetName());
	UG4MFMeshSurface4D* surface = NewObject<UG4MFMeshSurface4D>(InG4MFState, desiredName);
	const UMaterial4D* material = InMesh->GetMaterial();
	// Convert the material.
	if (IsValid(material) && !material->IsDefaultMaterial())
	{
		const int32 materialIndex = UG4MFMaterial4D::ExportConvertMaterialIntoState(InG4MFState, material, bInDeduplicate);
		surface->MaterialIndex = materialIndex;
		if (materialIndex < 0)
		{
			UE_LOG(LogUnreal4DCore, Warning, TEXT("UG4MFMeshSurface4D: Failed to encode material into UG4MFState4D."));
		}
	}
	// Convert tetrahedral simplex cells into an accessor.
	UTetraMesh4D* tetraMesh = Cast<UTetraMesh4D>(InMesh);
	if (tetraMesh != nullptr)
	{
		const TArray<int32> simplexVertexIndices = tetraMesh->GetSimplexCellVertexIndices();
		if (!simplexVertexIndices.IsEmpty())
		{
			surface->SimplexesAccessorIndex = UG4MFAccessor4D::EncodeNewAccessorFromInts<int32>(InG4MFState, simplexVertexIndices, bInDeduplicate);
			if (surface->SimplexesAccessorIndex < 0)
			{
				UE_LOG(LogUnreal4DCore, Error, TEXT("UG4MFMeshSurface4D: Failed to encode simplex cells into UG4MFState4D."));
			}
		}
		const TArray<int32> simplexCellNormalIndices = tetraMesh->GetSimplexCellNormalIndices();
		if (!simplexCellNormalIndices.IsEmpty())
		{
			surface->NormalsBinding = NewObject<UG4MFMeshSurfaceBinding4D>(surface);
			const TArray<FVector4> normalValues = tetraMesh->GetNormalValues();
			surface->NormalsBinding->ValuesAccessorIndex = UG4MFAccessor4D::EncodeNewAccessorFromVector4s(InG4MFState, normalValues, true);
			surface->NormalsBinding->SimplexesAccessorIndex = UG4MFAccessor4D::EncodeNewAccessorFromInts<int32>(InG4MFState, simplexCellNormalIndices, false);
		}
		const TArray<int32> simplexCellTextureMapIndices = tetraMesh->GetSimplexCellTextureMapIndices();
		if (!simplexCellTextureMapIndices.IsEmpty())
		{
			surface->TextureMapBinding = NewObject<UG4MFMeshSurfaceBinding4D>(surface);
			const TArray<FVector> textureMapValues = tetraMesh->GetTextureMapValues();
			surface->TextureMapBinding->ValuesAccessorIndex = UG4MFAccessor4D::EncodeNewAccessorFromVector3s(InG4MFState, textureMapValues, true);
			surface->TextureMapBinding->SimplexesAccessorIndex = UG4MFAccessor4D::EncodeNewAccessorFromInts<int32>(InG4MFState, simplexCellTextureMapIndices, false);
		}
		// For UArrayTetraMesh4D, the only edges are those implicitly calculated from simplex cells.
		// However, for other tetra meshes like BoxTetraMesh4D, use its explicitly defined edges.
		const UBoxTetraMesh4D* boxTetraMesh = Cast<UBoxTetraMesh4D>(tetraMesh);
		if (boxTetraMesh != nullptr && boxTetraMesh->GetTetraDecomp() == EBoxTetraDecomp4D::Decomp48CellPolytope)
		{
			surface->PolytopeSimplexes = true;
			// Don't return here, so that we keep the edge indices code below.
		}
		else
		{
			return surface;
		}
	}
	return surface;
}

UG4MFMeshSurface4D* UG4MFMeshSurface4D::FromJson(UG4MFState4D* InG4MFState, const FJsonObject* InJson)
{
	// General code for all UG4MFItem4D-derived classes.
	if (InJson == nullptr) { return nullptr; }
	const FName uniqueName = InG4MFState->ReserveUniqueFNameFromJson(InJson);
	UG4MFMeshSurface4D* surface = NewObject<UG4MFMeshSurface4D>(InG4MFState, uniqueName);
	surface->ReadItemEntriesFromJson(InJson);
	// UG4MFMeshSurface4D-specific code.
	if (InJson->HasField(TEXT("edges")))
	{
		surface->EdgesAccessorIndex = InJson->GetIntegerField(TEXT("edges"));
	}
	if (InJson->HasField(TEXT("material")))
	{
		surface->MaterialIndex = InJson->GetIntegerField(TEXT("material"));
	}
	if (InJson->HasField(TEXT("normals")))
	{
		surface->NormalsBinding = UG4MFMeshSurfaceBinding4D::FromJson(InG4MFState, surface, InJson->GetObjectField(TEXT("normals")).Get());
	}
	if (InJson->HasField(TEXT("polytopeSimplexes")))
	{
		surface->PolytopeSimplexes = InJson->GetBoolField(TEXT("polytopeSimplexes"));
	}
	if (InJson->HasField(TEXT("seams")))
	{
		surface->SeamsAccessorIndex = InJson->GetIntegerField(TEXT("seams"));
	}
	if (InJson->HasField(TEXT("simplexes")))
	{
		surface->SimplexesAccessorIndex = InJson->GetIntegerField(TEXT("simplexes"));
	}
	if (InJson->HasField(TEXT("textureMap")))
	{
		surface->TextureMapBinding = UG4MFMeshSurfaceBinding4D::FromJson(InG4MFState, surface, InJson->GetObjectField(TEXT("textureMap")).Get());
	}
	return surface;
}

TSharedPtr<FJsonObject> UG4MFMeshSurface4D::ToJson() const
{
	TSharedPtr<FJsonObject> json = UG4MFItem4D::WriteItemEntriesToJson();
	if (EdgesAccessorIndex >= 0)
	{
		json->SetNumberField(TEXT("edges"), EdgesAccessorIndex);
	}
	if (MaterialIndex >= 0)
	{
		json->SetNumberField(TEXT("material"), MaterialIndex);
	}
	if (IsValid(NormalsBinding))
	{
		json->SetObjectField(TEXT("normals"), NormalsBinding->ToJson());
	}
	if (PolytopeSimplexes)
	{
		json->SetBoolField(TEXT("polytopeSimplexes"), PolytopeSimplexes);
	}
	if (SeamsAccessorIndex >= 0)
	{
		json->SetNumberField(TEXT("seams"), SeamsAccessorIndex);
	}
	if (SimplexesAccessorIndex >= 0)
	{
		json->SetNumberField(TEXT("simplexes"), SimplexesAccessorIndex);
	}
	if (IsValid(TextureMapBinding))
	{
		json->SetObjectField(TEXT("textureMap"), TextureMapBinding->ToJson());
	}
	return json;
}
