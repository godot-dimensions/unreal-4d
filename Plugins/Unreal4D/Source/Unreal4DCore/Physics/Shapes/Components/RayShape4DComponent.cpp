#include "RayShape4DComponent.h"

#include "../Data/RayShape4DData.h"

void URayShape4DComponent::SetRayLength(const double InLength)
{
	RayTargetZ = -InLength;
	UpdateShapeFromComponent();
}

void URayShape4DComponent::UpdateComponentFromShape()
{
	URayShape4DData* rayShapeData = Cast<URayShape4DData>(Shape4D);
	if (!ensure(rayShapeData != nullptr))
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("URayShape4DComponent::UpdateComponentFromShape: Shape4D is not a URayShape4DData."));
		return;
	}
	RayTargetZ = rayShapeData->RayTargetZ;
}

void URayShape4DComponent::UpdateShapeFromComponent()
{
	URayShape4DData* rayShapeData = Cast<URayShape4DData>(Shape4D);
	if (!ensure(rayShapeData != nullptr))
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("URayShape4DComponent::UpdateShapeFromComponent: Shape4D is not a URayShape4DData."));
		return;
	}
	rayShapeData->RayTargetZ = RayTargetZ;
}

void URayShape4DComponent::PostInitProperties()
{
	EnsureShapeDataIsOwned();
	if (Shape4D == nullptr || !IsValid(Shape4D) || !Shape4D->IsValidLowLevel())
	{
		Shape4D = NewObject<URayShape4DData>(this);
	}
	UpdateShapeFromComponent();
	Super::PostInitProperties();
}

void URayShape4DComponent::PostLoad()
{
	EnsureShapeDataIsOwned();
	if (Shape4D == nullptr || !IsValid(Shape4D) || !Shape4D->IsValidLowLevel())
	{
		Shape4D = NewObject<URayShape4DData>(this);
	}
	UpdateShapeFromComponent();
	Super::PostLoad();
}

#if WITH_EDITOR
void URayShape4DComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	URayShape4DData* RayShapeData = Cast<URayShape4DData>(Shape4D);
	if (!ensure(RayShapeData != nullptr))
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("URayShape4DComponent::PostEditChangeProperty: Shape4D is not a URayShape4DData."));
		return;
	}
	UpdateShapeFromComponent();
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif // WITH_EDITOR
