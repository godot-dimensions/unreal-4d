#include "DuocylinderShape4DComponent.h"

#include "../Data/DuocylinderShape4DData.h"

void UDuocylinderShape4DComponent::SetRadiusXY(const double InNewRadiusXY)
{
	RadiusXY = InNewRadiusXY;
	UpdateShapeFromComponent();
}

void UDuocylinderShape4DComponent::SetRadiusZW(const double InNewRadiusZW)
{
	RadiusZW = InNewRadiusZW;
	UpdateShapeFromComponent();
}

void UDuocylinderShape4DComponent::UpdateComponentFromShape()
{
	UDuocylinderShape4DData* duocylinderShapeData = Cast<UDuocylinderShape4DData>(Shape4D);
	if (!ensure(duocylinderShapeData != nullptr))
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("UDuocylinderShape4DComponent::UpdateComponentFromShape: Shape4D is not a UDuocylinderShape4DData."));
		return;
	}
	RadiusXY = duocylinderShapeData->RadiusXY;
	RadiusZW = duocylinderShapeData->RadiusZW;
}

void UDuocylinderShape4DComponent::UpdateShapeFromComponent()
{
	UDuocylinderShape4DData* duocylinderShapeData = Cast<UDuocylinderShape4DData>(Shape4D);
	if (!ensure(duocylinderShapeData != nullptr))
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("UDuocylinderShape4DComponent::UpdateShapeFromComponent: Shape4D is not a UDuocylinderShape4DData."));
		return;
	}
	duocylinderShapeData->RadiusXY = RadiusXY;
	duocylinderShapeData->RadiusZW = RadiusZW;
}

void UDuocylinderShape4DComponent::PostInitProperties()
{
	EnsureShapeDataIsOwned();
	if (Shape4D == nullptr || !IsValid(Shape4D) || !Shape4D->IsValidLowLevel())
	{
		Shape4D = NewObject<UDuocylinderShape4DData>(this);
	}
	UpdateShapeFromComponent();
	Super::PostInitProperties();
}

void UDuocylinderShape4DComponent::PostLoad()
{
	EnsureShapeDataIsOwned();
	if (Shape4D == nullptr || !IsValid(Shape4D) || !Shape4D->IsValidLowLevel())
	{
		Shape4D = NewObject<UDuocylinderShape4DData>(this);
	}
	UpdateShapeFromComponent();
	Super::PostLoad();
}

#if WITH_EDITOR
void UDuocylinderShape4DComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	UDuocylinderShape4DData* DuocylinderShapeData = Cast<UDuocylinderShape4DData>(Shape4D);
	if (!ensure(DuocylinderShapeData != nullptr))
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("UDuocylinderShape4DComponent::PostEditChangeProperty: Shape4D is not a UDuocylinderShape4DData."));
		return;
	}
	UpdateShapeFromComponent();
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif // WITH_EDITOR
