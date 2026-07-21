#include "OrthoplexShape4DComponent.h"

#include "../Data/OrthoplexShape4DData.h"

void UOrthoplexShape4DComponent::SetSize(const FVector4& InNewSize)
{
	Size = InNewSize;
	UpdateShapeFromComponent();
}

FVector4 UOrthoplexShape4DComponent::GetHalfExtents() const
{
	return Size * 0.5;
}

void UOrthoplexShape4DComponent::SetHalfExtents(const FVector4& InHalfExtents)
{
	Size = InHalfExtents * 2.0;
	UpdateShapeFromComponent();
}

void UOrthoplexShape4DComponent::UpdateComponentFromShape()
{
	UOrthoplexShape4DData* orthoplexShapeData = Cast<UOrthoplexShape4DData>(Shape4D);
	if (!ensure(orthoplexShapeData != nullptr))
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("UOrthoplexShape4DComponent::UpdateComponentFromShape: Shape4D is not a UOrthoplexShape4DData."));
		return;
	}
	Size = orthoplexShapeData->Size;
}

void UOrthoplexShape4DComponent::UpdateShapeFromComponent()
{
	UOrthoplexShape4DData* orthoplexShapeData = Cast<UOrthoplexShape4DData>(Shape4D);
	if (!ensure(orthoplexShapeData != nullptr))
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("UOrthoplexShape4DComponent::UpdateShapeFromComponent: Shape4D is not a UOrthoplexShape4DData."));
		return;
	}
	orthoplexShapeData->Size = Size;
}

void UOrthoplexShape4DComponent::PostInitProperties()
{
	EnsureShapeDataIsOwned();
	if (Shape4D == nullptr || !IsValid(Shape4D) || !Shape4D->IsValidLowLevel())
	{
		Shape4D = NewObject<UOrthoplexShape4DData>(this);
	}
	UpdateShapeFromComponent();
	Super::PostInitProperties();
}

void UOrthoplexShape4DComponent::PostLoad()
{
	EnsureShapeDataIsOwned();
	if (Shape4D == nullptr || !IsValid(Shape4D) || !Shape4D->IsValidLowLevel())
	{
		Shape4D = NewObject<UOrthoplexShape4DData>(this);
	}
	UpdateShapeFromComponent();
	Super::PostLoad();
}

#if WITH_EDITOR
void UOrthoplexShape4DComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	UOrthoplexShape4DData* OrthoplexShapeData = Cast<UOrthoplexShape4DData>(Shape4D);
	if (!ensure(OrthoplexShapeData != nullptr))
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("UOrthoplexShape4DComponent::PostEditChangeProperty: Shape4D is not a UOrthoplexShape4DData."));
		return;
	}
	UpdateShapeFromComponent();
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif // WITH_EDITOR
