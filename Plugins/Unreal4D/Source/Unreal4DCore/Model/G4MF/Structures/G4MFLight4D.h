#pragma once

#include "G4MFItem4D.h"

#include "G4MFLight4D.generated.h"

// UG4MFLight4D is a class for 4D light source properties in the G4MF format.
// A light may be set directly on a UG4MFNode4D's Light property to make that node a light.
// Each light may have its LightType set to "directional", "point", or "spot", which
// determine how light is emitted. The Range property is only used by point and spot lights,
// and the ConeInnerAngle] and ConeOuterAngle properties are only used by spot lights.
// The Color and Intensity properties are used by all light types, with the Intensity unit changing
// depending on the light type. The light emitted is a product of the Color and Intensity properties.
UCLASS(Category = "Unreal 4D|Model|G4MF")
class UNREAL4DCORE_API UG4MFLight4D : public UG4MFItem4D
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FString LightType = TEXT("point");

	UPROPERTY()
	FLinearColor Color = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);

	UPROPERTY()
	double ConeInnerAngle = 0.0;

	UPROPERTY()
	double ConeOuterAngle = 0.7853981633974483;

	UPROPERTY()
	double Intensity = 1.0;

	UPROPERTY()
	double Range = std::numeric_limits<double>::infinity();

	static UG4MFLight4D* FromJson(UG4MFState4D* InG4MFState, const FJsonObject* InJson);
	TSharedPtr<FJsonObject> ToJson() const;
};
