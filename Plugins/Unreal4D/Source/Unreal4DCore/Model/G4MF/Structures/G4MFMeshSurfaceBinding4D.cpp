#include "G4MFMeshSurfaceBinding4D.h"

#include "../G4MFState4D.h"
#include "G4MFAccessor4D.h"

bool UG4MFMeshSurfaceBindingGeometry4D::IsEqualExact(const UG4MFMeshSurfaceBindingGeometry4D* InOther) const
{
	if (InOther == nullptr || !IsValid(InOther)) return false;
	return (IndicesAccessorIndex == InOther->IndicesAccessorIndex &&
			DecomposeDimension == InOther->DecomposeDimension &&
			GeometryDimension == InOther->GeometryDimension);
}

TArray<int32> UG4MFMeshSurfaceBindingGeometry4D::LoadIndices(const UG4MFState4D* InG4MFState) const
{
	if (InG4MFState == nullptr) { return TArray<int32>(); }
	if (!InG4MFState->G4MFAccessors.IsValidIndex(IndicesAccessorIndex))
	{
		return TArray<int32>();
	}
	const TObjectPtr<UG4MFAccessor4D> accessor = InG4MFState->G4MFAccessors[IndicesAccessorIndex];
	if (!IsValid(accessor))
	{
		return TArray<int32>();
	}
	return accessor->DecodeNumbersFromBytes<int32>(InG4MFState);
}

void UG4MFMeshSurfaceBinding4D::FlipSimplexWindingOrder(UG4MFState4D* InG4MFState)
{
	if (InG4MFState == nullptr) { return; }
	if (SimplexesAccessorIndex == -1)
	{
		return; // No simplex data to flip.
	}
	TArray<int32> simplexIndices = LoadSimplexIndices(InG4MFState);
	for (int32 simplexIndex = 0; simplexIndex < simplexIndices.Num(); simplexIndex += 4)
	{
		// This is an arbitrary choice, but swap indices 2 and 3 to keep the first edge the same (0 to 1).
		// NOTE: Make sure this follows the same convention as `UG4MFDocument4D::ConvertCoordinateSystem`!
		Swap(simplexIndices[simplexIndex + 2], simplexIndices[simplexIndex + 3]);
	}
	SimplexesAccessorIndex = UG4MFAccessor4D::EncodeNewAccessorFromInts<int32>(InG4MFState, simplexIndices, true);
}

bool UG4MFMeshSurfaceBinding4D::IsEqualExact(const UG4MFMeshSurfaceBinding4D* InOther) const
{
	if (InOther == nullptr || !IsValid(InOther)) return false;
	if (GeometryDecompositions.Num() != InOther->GeometryDecompositions.Num())
	{
		return false;
	}
	for (int32 geometryDecompIndex = 0; geometryDecompIndex < GeometryDecompositions.Num(); geometryDecompIndex++)
	{
		const TObjectPtr<UG4MFMeshSurfaceBindingGeometry4D>& geom = GeometryDecompositions[geometryDecompIndex];
		const TObjectPtr<UG4MFMeshSurfaceBindingGeometry4D>& other = InOther->GeometryDecompositions[geometryDecompIndex];
		if (!IsValid(geom) || !geom->IsEqualExact(other))
		{
			return false;
		}
	}
	return (PerSimplexAccessorIndex == InOther->PerSimplexAccessorIndex &&
			SimplexesAccessorIndex == InOther->SimplexesAccessorIndex &&
			ValuesAccessorIndex == InOther->ValuesAccessorIndex);
}

TArray<int32> UG4MFMeshSurfaceBinding4D::LoadPerSimplexIndices(const UG4MFState4D* InG4MFState) const
{
	if (InG4MFState == nullptr) { return TArray<int32>(); }
	if (PerSimplexAccessorIndex < 0 || PerSimplexAccessorIndex >= InG4MFState->G4MFAccessors.Num())
	{
		return TArray<int32>();
	}
	const TObjectPtr<UG4MFAccessor4D> accessor = InG4MFState->G4MFAccessors[PerSimplexAccessorIndex];
	if (!IsValid(accessor))
	{
		return TArray<int32>();
	}
	return accessor->DecodeNumbersFromBytes<int32>(InG4MFState);
}

TArray<int32> UG4MFMeshSurfaceBinding4D::LoadSimplexIndices(const UG4MFState4D* InG4MFState) const
{
	if (InG4MFState == nullptr) { return TArray<int32>(); }
	if (SimplexesAccessorIndex < 0 || SimplexesAccessorIndex >= InG4MFState->G4MFAccessors.Num())
	{
		return TArray<int32>();
	}
	const TObjectPtr<UG4MFAccessor4D> accessor = InG4MFState->G4MFAccessors[SimplexesAccessorIndex];
	if (!IsValid(accessor))
	{
		return TArray<int32>();
	}
	return accessor->DecodeNumbersFromBytes<int32>(InG4MFState);
}

TArray<FLinearColor> UG4MFMeshSurfaceBinding4D::LoadValuesAsColors(const UG4MFState4D* InG4MFState) const
{
	if (InG4MFState == nullptr) { return TArray<FLinearColor>(); }
	if (!InG4MFState->G4MFAccessors.IsValidIndex(ValuesAccessorIndex))
	{
		return TArray<FLinearColor>();
	}
	const TObjectPtr<UG4MFAccessor4D> accessor = InG4MFState->G4MFAccessors[ValuesAccessorIndex];
	if (!IsValid(accessor))
	{
		return TArray<FLinearColor>();
	}
	return accessor->DecodeColorsFromBytes(InG4MFState);
}

TArray<FVector> UG4MFMeshSurfaceBinding4D::LoadValuesAsVector3s(const UG4MFState4D* InG4MFState) const
{
	if (InG4MFState == nullptr) { return TArray<FVector>(); }
	if (!InG4MFState->G4MFAccessors.IsValidIndex(ValuesAccessorIndex))
	{
		return TArray<FVector>();
	}
	const TObjectPtr<UG4MFAccessor4D> accessor = InG4MFState->G4MFAccessors[ValuesAccessorIndex];
	if (!IsValid(accessor))
	{
		return TArray<FVector>();
	}
	return accessor->DecodeVector3sFromBytes(InG4MFState);
}

TArray<FVector4> UG4MFMeshSurfaceBinding4D::LoadValuesAsVector4s(const UG4MFState4D* InG4MFState) const
{
	if (InG4MFState == nullptr) { return TArray<FVector4>(); }
	if (!InG4MFState->G4MFAccessors.IsValidIndex(ValuesAccessorIndex))
	{
		return TArray<FVector4>();
	}
	const TObjectPtr<UG4MFAccessor4D> accessor = InG4MFState->G4MFAccessors[ValuesAccessorIndex];
	if (!IsValid(accessor))
	{
		return TArray<FVector4>();
	}
	return accessor->DecodeVector4sFromBytes(InG4MFState);
}

UG4MFMeshSurfaceBindingGeometry4D* UG4MFMeshSurfaceBindingGeometry4D::FromJson(UG4MFState4D* InG4MFState, UG4MFMeshSurfaceBinding4D* InOwningBinding, const FJsonObject* InJson)
{
	// General code for all UG4MFItem4D-derived classes.
	if (InJson == nullptr) { return nullptr; }
	// UG4MFMeshSurfaceBindingGeometry4D is owned by sub-objects of the state, but names MUST still be file-unique as per the spec.
	const FName uniqueName = InG4MFState->ReserveUniqueFNameFromJson(InJson);
	UG4MFMeshSurfaceBindingGeometry4D* ret = NewObject<UG4MFMeshSurfaceBindingGeometry4D>(InOwningBinding, uniqueName);
	ret->ReadItemEntriesFromJson(InJson);
	// UG4MFMeshSurfaceBinding4D-specific code.
	if (InJson->HasField(TEXT("accessor")))
	{
		ret->IndicesAccessorIndex = InJson->GetIntegerField(TEXT("accessor"));
	}
	if (InJson->HasField(TEXT("decomposeDimension")))
	{
		ret->DecomposeDimension = InJson->GetIntegerField(TEXT("decomposeDimension"));
	}
	if (InJson->HasField(TEXT("geometryDimension")))
	{
		ret->GeometryDimension = InJson->GetIntegerField(TEXT("geometryDimension"));
	}
	return ret;
}

TSharedPtr<FJsonObject> UG4MFMeshSurfaceBindingGeometry4D::ToJson() const
{
	TSharedPtr<FJsonObject> json = WriteItemEntriesToJson();
	// Required, always set.
	json->SetNumberField(TEXT("accessor"), IndicesAccessorIndex);
	if (DecomposeDimension != 0)
	{
		json->SetNumberField(TEXT("decomposeDimension"), DecomposeDimension);
	}
	// Required, always set.
	json->SetNumberField(TEXT("geometryDimension"), GeometryDimension);
	return json;
}

UG4MFMeshSurfaceBinding4D* UG4MFMeshSurfaceBinding4D::FromJson(UG4MFState4D* InG4MFState, UObject* InOwner, const FJsonObject* InJson)
{
	// General code for all UG4MFItem4D-derived classes.
	if (InJson == nullptr) { return nullptr; }
	// UG4MFMeshSurfaceBinding4D is owned by sub-objects of the state, but names MUST still be file-unique as per the spec.
	const FName uniqueName = InG4MFState->ReserveUniqueFNameFromJson(InJson);
	UG4MFMeshSurfaceBinding4D* ret = NewObject<UG4MFMeshSurfaceBinding4D>(InOwner, uniqueName);
	ret->ReadItemEntriesFromJson(InJson);
	// UG4MFMeshSurfaceBinding4D-specific code.
	if (InJson->HasField(TEXT("geometry")))
	{
		const TArray<TSharedPtr<FJsonValue>>& geometryArray = InJson->GetArrayField(TEXT("geometry"));
		for (const TSharedPtr<FJsonValue>& geomValue : geometryArray)
		{
			if (geomValue.IsValid() && geomValue->Type == EJson::Object)
			{
				const FJsonObject* geomJsonPtr = geomValue->AsObject().Get();
				ret->GeometryDecompositions.Add(UG4MFMeshSurfaceBindingGeometry4D::FromJson(InG4MFState, ret, geomJsonPtr));
			}
		}
	}
	if (InJson->HasField(TEXT("perSimplex")))
	{
		ret->PerSimplexAccessorIndex = InJson->GetIntegerField(TEXT("perSimplex"));
	}
	if (InJson->HasField(TEXT("simplexes")))
	{
		ret->SimplexesAccessorIndex = InJson->GetIntegerField(TEXT("simplexes"));
	}
	if (InJson->HasField(TEXT("values")))
	{
		ret->ValuesAccessorIndex = InJson->GetIntegerField(TEXT("values"));
	}
	return ret;
}

TSharedPtr<FJsonObject> UG4MFMeshSurfaceBinding4D::ToJson() const
{
	TSharedPtr<FJsonObject> json = WriteItemEntriesToJson();
	if (GeometryDecompositions.Num() > 0)
	{
		TArray<TSharedPtr<FJsonValue>> geometryArray;
		for (const TObjectPtr<UG4MFMeshSurfaceBindingGeometry4D>& geom : GeometryDecompositions)
		{
			if (IsValid(geom))
			{
				TSharedPtr<FJsonObject> geomJson = geom->ToJson();
				geometryArray.Add(MakeShared<FJsonValueObject>(geomJson));
			}
		}
		if (geometryArray.Num() > 0)
		{
			json->SetArrayField(TEXT("geometry"), geometryArray);
		}
	}
	if (PerSimplexAccessorIndex >= 0)
	{
		json->SetNumberField(TEXT("perSimplex"), PerSimplexAccessorIndex);
	}
	if (SimplexesAccessorIndex >= 0)
	{
		json->SetNumberField(TEXT("simplexes"), SimplexesAccessorIndex);
	}
	// Required, always set.
	json->SetNumberField(TEXT("values"), ValuesAccessorIndex);
	return json;
}
