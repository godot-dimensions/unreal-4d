#pragma once

#include "G4MFItem4D.h"

#include "G4MFCamera4D.generated.h"

class UCamera4DComponent;

// UG4MFCamera4D is a class for cameras in the G4MF format. It may be set directly on a
// G4MF node to make that node a camera, and its properties define how it views the scene.
UCLASS(Category = "Unreal 4D|Model|G4MF")
class UNREAL4DCORE_API UG4MFCamera4D : public UG4MFItem4D
{
	GENERATED_BODY()

public:
	// Only include the properties relevant to Unreal's camera system.
	// This class is intentionally ignoring and discarding G4MF's "clipFar", "clipNear", and "keepAspect" properties,
	// because Unreal's camera system uses global near and far planes, and the field of view only has one definition.
	UPROPERTY()
	FString ProjectionType = TEXT("perspective");

	UPROPERTY()
	double FovRadians = UNREAL4D_TAU / 4.0;

	UPROPERTY()
	double SizeMeters = 1.0;

	UCamera4DComponent* ImportGenerateCameraComponent(UG4MFState4D* InG4MFState, AActor* InActor) const;
	static UG4MFCamera4D* ExportConvertCameraComponent(UG4MFState4D* InG4MFState, const UCamera4DComponent* InCamera);

	static UG4MFCamera4D* FromJson(UG4MFState4D* InG4MFState, const FJsonObject* InJson);
	TSharedPtr<FJsonObject> ToJson() const;
};
