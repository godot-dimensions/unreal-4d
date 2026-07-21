#include "CubinderShape4DComponent.h"

#include "../Data/CubinderShape4DData.h"

void UCubinderShape4DComponent::SetHeight(const double InNewHeight)
{
	Height = InNewHeight;
	UpdateShapeFromComponent();
}

void UCubinderShape4DComponent::SetRadius(const double InNewRadius)
{
	Radius = InNewRadius;
	UpdateShapeFromComponent();
}

void UCubinderShape4DComponent::SetThickness(const double InNewThickness)
{
	Thickness = InNewThickness;
	UpdateShapeFromComponent();
}

FVector2D UCubinderShape4DComponent::GetSizeZW() const
{
	return FVector2D(Height, Thickness); // (Z, W)
}

void UCubinderShape4DComponent::SetSizeZW(const FVector2D& InSizeZW)
{
	Height = InSizeZW.X; // Z
	Thickness = InSizeZW.Y; // W
	UpdateShapeFromComponent();
}

void UCubinderShape4DComponent::UpdateComponentFromShape()
{
	UCubinderShape4DData* cubinderShapeData = Cast<UCubinderShape4DData>(Shape4D);
	if (!ensure(cubinderShapeData != nullptr))
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("UCubinderShape4DComponent::UpdateComponentFromShape: Shape4D is not a UCubinderShape4DData."));
		return;
	}
	Height = cubinderShapeData->Height;
	Radius = cubinderShapeData->Radius;
	Thickness = cubinderShapeData->Thickness;
}

void UCubinderShape4DComponent::UpdateShapeFromComponent()
{
	UCubinderShape4DData* cubinderShapeData = Cast<UCubinderShape4DData>(Shape4D);
	if (!ensure(cubinderShapeData != nullptr))
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("UCubinderShape4DComponent::UpdateShapeFromComponent: Shape4D is not a UCubinderShape4DData."));
		return;
	}
	cubinderShapeData->Height = Height;
	cubinderShapeData->Radius = Radius;
	cubinderShapeData->Thickness = Thickness;
}

void UCubinderShape4DComponent::PostInitProperties()
{
	EnsureShapeDataIsOwned();
	if (Shape4D == nullptr || !IsValid(Shape4D) || !Shape4D->IsValidLowLevel())
	{
		Shape4D = NewObject<UCubinderShape4DData>(this);
	}
	UpdateShapeFromComponent();
	Super::PostInitProperties();
}

void UCubinderShape4DComponent::PostLoad()
{
	EnsureShapeDataIsOwned();
	if (Shape4D == nullptr || !IsValid(Shape4D) || !Shape4D->IsValidLowLevel())
	{
		Shape4D = NewObject<UCubinderShape4DData>(this);
	}
	UpdateShapeFromComponent();
	Super::PostLoad();
}

#if WITH_EDITOR
void UCubinderShape4DComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	UCubinderShape4DData* CubinderShapeData = Cast<UCubinderShape4DData>(Shape4D);
	if (!ensure(CubinderShapeData != nullptr))
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("UCubinderShape4DComponent::PostEditChangeProperty: Shape4D is not a UCubinderShape4DData."));
		return;
	}
	UpdateShapeFromComponent();
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif // WITH_EDITOR
