#include "PlaneShape4DComponent.h"

#include "../Data/PlaneShape4DData.h"

void UPlaneShape4DComponent::UpdateComponentFromShape()
{
	UPlaneShape4DData* planeShapeData = Cast<UPlaneShape4DData>(Shape4D);
	if (!ensure(planeShapeData != nullptr))
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("UPlaneShape4DComponent::UpdateComponentFromShape: Shape4D is not a UPlaneShape4DData."));
		return;
	}
	// Plane has no editable parameters, so nothing to sync.
}

void UPlaneShape4DComponent::UpdateShapeFromComponent()
{
	UPlaneShape4DData* planeShapeData = Cast<UPlaneShape4DData>(Shape4D);
	if (!ensure(planeShapeData != nullptr))
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("UPlaneShape4DComponent::UpdateShapeFromComponent: Shape4D is not a UPlaneShape4DData."));
		return;
	}
	// Plane has no editable parameters, so nothing to sync.
}

void UPlaneShape4DComponent::PostInitProperties()
{
	EnsureShapeDataIsOwned();
	if (Shape4D == nullptr || !IsValid(Shape4D) || !Shape4D->IsValidLowLevel())
	{
		Shape4D = NewObject<UPlaneShape4DData>(this);
	}
	UpdateShapeFromComponent();
	Super::PostInitProperties();
}

void UPlaneShape4DComponent::PostLoad()
{
	EnsureShapeDataIsOwned();
	if (Shape4D == nullptr || !IsValid(Shape4D) || !Shape4D->IsValidLowLevel())
	{
		Shape4D = NewObject<UPlaneShape4DData>(this);
	}
	UpdateShapeFromComponent();
	Super::PostLoad();
}

#if WITH_EDITOR
void UPlaneShape4DComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	UPlaneShape4DData* PlaneShapeData = Cast<UPlaneShape4DData>(Shape4D);
	if (!ensure(PlaneShapeData != nullptr))
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("UPlaneShape4DComponent::PostEditChangeProperty: Shape4D is not a UPlaneShape4DData."));
		return;
	}
	UpdateShapeFromComponent();
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif // WITH_EDITOR
