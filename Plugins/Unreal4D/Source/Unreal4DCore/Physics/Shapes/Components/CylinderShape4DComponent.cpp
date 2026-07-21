#include "CylinderShape4DComponent.h"

#include "../Data/CylinderShape4DData.h"

void UCylinderShape4DComponent::SetHeight(const double InNewHeight)
{
	Height = InNewHeight;
	UpdateShapeFromComponent();
}

void UCylinderShape4DComponent::SetRadius(const double InNewRadius)
{
	Radius = InNewRadius;
	UpdateShapeFromComponent();
}

void UCylinderShape4DComponent::UpdateComponentFromShape()
{
	UCylinderShape4DData* cylinderShapeData = Cast<UCylinderShape4DData>(Shape4D);
	if (!ensure(cylinderShapeData != nullptr))
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("UCylinderShape4DComponent::UpdateComponentFromShape: Shape4D is not a UCylinderShape4DData."));
		return;
	}
	Height = cylinderShapeData->Height;
	Radius = cylinderShapeData->Radius;
}

void UCylinderShape4DComponent::UpdateShapeFromComponent()
{
	UCylinderShape4DData* cylinderShapeData = Cast<UCylinderShape4DData>(Shape4D);
	if (!ensure(cylinderShapeData != nullptr))
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("UCylinderShape4DComponent::UpdateShapeFromComponent: Shape4D is not a UCylinderShape4DData."));
		return;
	}
	cylinderShapeData->Height = Height;
	cylinderShapeData->Radius = Radius;
}

void UCylinderShape4DComponent::PostInitProperties()
{
	EnsureShapeDataIsOwned();
	if (Shape4D == nullptr || !IsValid(Shape4D) || !Shape4D->IsValidLowLevel())
	{
		Shape4D = NewObject<UCylinderShape4DData>(this);
	}
	UpdateShapeFromComponent();
	Super::PostInitProperties();
}

void UCylinderShape4DComponent::PostLoad()
{
	EnsureShapeDataIsOwned();
	if (Shape4D == nullptr || !IsValid(Shape4D) || !Shape4D->IsValidLowLevel())
	{
		Shape4D = NewObject<UCylinderShape4DData>(this);
	}
	UpdateShapeFromComponent();
	Super::PostLoad();
}

#if WITH_EDITOR
void UCylinderShape4DComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	UCylinderShape4DData* CylinderShapeData = Cast<UCylinderShape4DData>(Shape4D);
	if (!ensure(CylinderShapeData != nullptr))
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("UCylinderShape4DComponent::PostEditChangeProperty: Shape4D is not a UCylinderShape4DData."));
		return;
	}
	UpdateShapeFromComponent();
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif // WITH_EDITOR
