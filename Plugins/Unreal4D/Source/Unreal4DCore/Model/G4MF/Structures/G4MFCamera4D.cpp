#include "G4MFCamera4D.h"

#include "../G4MFState4D.h"
#include "../../../Scene/Components/Camera4DComponent.h"

UCamera4DComponent* UG4MFCamera4D::ImportGenerateCameraComponent(UG4MFState4D* InG4MFState, AActor* InActor) const
{
	if (!IsValid(InG4MFState) || !IsValid(InActor))
	{
		return nullptr;
	}
	UCamera4DComponent* ret = NewObject<UCamera4DComponent>(InActor, FName(GetItemName()));
	ret->FieldOfView = FMath::RadiansToDegrees(FovRadians);
	ret->OrthoWidth = SizeMeters;
	if (ProjectionType == TEXT("orthographic"))
	{
		ret->ProjectionMode = ECameraProjectionMode::Orthographic;
	}
	else
	{
		ret->ProjectionMode = ECameraProjectionMode::Perspective;
	}
	return ret;
}

UG4MFCamera4D* UG4MFCamera4D::ExportConvertCameraComponent(UG4MFState4D* InG4MFState, const UCamera4DComponent* InCamera)
{
	if (!IsValid(InG4MFState) || !IsValid(InCamera))
	{
		return nullptr;
	}
	const FName desiredName = FName(InCamera->GetName());
	UG4MFCamera4D* ret = NewObject<UG4MFCamera4D>(InG4MFState, desiredName);
	if (InCamera->ProjectionMode == ECameraProjectionMode::Orthographic)
	{
		ret->ProjectionType = TEXT("orthographic");
		ret->SizeMeters = InCamera->OrthoWidth;
	}
	else
	{
		ret->ProjectionType = TEXT("perspective");
		ret->FovRadians = FMath::DegreesToRadians(InCamera->FieldOfView);
	}
	return ret;
}

UG4MFCamera4D* UG4MFCamera4D::FromJson(UG4MFState4D* InG4MFState, const FJsonObject* InJson)
{
	// General code for all UG4MFItem4D-derived classes.
	if (InJson == nullptr) { return nullptr; }
	const FName uniqueName = InG4MFState->ReserveUniqueFNameFromJson(InJson);
	UG4MFCamera4D* ret = NewObject<UG4MFCamera4D>(InG4MFState, uniqueName);
	ret->ReadItemEntriesFromJson(InJson);
	// UG4MFCamera4D-specific code.
	if (InJson->HasField(TEXT("fov")))
	{
		ret->FovRadians = InJson->GetNumberField(TEXT("fov"));
	}
	if (InJson->HasField(TEXT("size")))
	{
		ret->SizeMeters = InJson->GetNumberField(TEXT("size"));
	}
	if (InJson->HasField(TEXT("type")))
	{
		ret->ProjectionType = InJson->GetStringField(TEXT("type"));
	}
	return ret;
}

TSharedPtr<FJsonObject> UG4MFCamera4D::ToJson() const
{
	TSharedPtr<FJsonObject> ret = WriteItemEntriesToJson();
	if (ProjectionType == TEXT("perspective"))
	{
		if (!FMath::IsNearlyEqual(FovRadians, UNREAL4D_TAU / 4.0, UNREAL4D_COMPARISON_EPSILON))
		{
			ret->SetNumberField(TEXT("fov"), FovRadians);
		}
	}
	else
	{
		ret->SetStringField(TEXT("type"), ProjectionType);
		if (!FMath::IsNearlyEqual(SizeMeters, 1.0, UNREAL4D_COMPARISON_EPSILON))
		{
			ret->SetNumberField(TEXT("size"), SizeMeters);
		}
	}
	return ret;
}
