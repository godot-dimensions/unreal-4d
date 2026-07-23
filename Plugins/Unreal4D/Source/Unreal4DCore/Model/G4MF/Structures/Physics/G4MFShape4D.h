#pragma once

#include "../G4MFItem4D.h"

#include "../../../../Physics/Shapes/Components/Shape4DComponent.h"
#include "../../../../Physics/Shapes/Data/GeneralShape4DData.h"

#include "G4MFShape4D.generated.h"

// UG4MFShape4D is a pure geometric object defined by an enumerated type
// and a set of size parameters. Shapes are useful for things like physics
// calculations, but are not expected to be directly used for rendering.
UCLASS(Category = "Unreal 4D|Model|G4MF")
class UNREAL4DCORE_API UG4MFShape4D : public UG4MFItem4D
{
	GENERATED_BODY()

	UShape4DComponent* ImportGenerateGeneralShapeComponent(UG4MFState4D* InG4MFState, AActor* InActor) const;
	UShape4DData* ImportGenerateGeneralShapeData(UG4MFState4D* InG4MFState) const;

public:
	UPROPERTY()
	FVector4 BaseSize = FVector4(0.0, 0.0, 0.0, 0.0);

	UPROPERTY()
	TArray<TObjectPtr<UGeneralShapeCurve4D>> Curves;

	UPROPERTY()
	FString ShapeType = TEXT("general");

	UPROPERTY()
	double RayLength = 1.0;

	UPROPERTY()
	int32 MeshIndex = -1;

	// HeightMap properties.
	UPROPERTY()
	FVector GridSpacing = FVector(1.0, 1.0, 1.0);

	UPROPERTY()
	FIntVector GridSize = FIntVector(1, 1, 1);

	UPROPERTY()
	int32 HeightsAccessorIndex = -1;

	bool IsEqualExact(const UG4MFShape4D* InOther) const;

	UShape4DComponent* ImportGenerateShapeComponent(UG4MFState4D* InG4MFState, AActor* InActor) const;
	UShape4DData* ImportGenerateShapeData(UG4MFState4D* InG4MFState) const;

	static UG4MFShape4D* FromJson(UG4MFState4D* InG4MFState, const FJsonObject* InJson);
	TSharedPtr<FJsonObject> ToJson() const;
};
