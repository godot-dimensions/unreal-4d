#include "BoxShape4DComponent.h"

#include "../Data/BoxShape4DData.h"

void UBoxShape4DComponent::SetSize(const FVector4& InNewSize)
{
	Size = InNewSize;
	UpdateShapeFromComponent();
}

void UBoxShape4DComponent::UpdateComponentFromShape()
{
	UBoxShape4DData* boxShapeData = Cast<UBoxShape4DData>(Shape4D);
	if (!ensure(boxShapeData != nullptr))
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("UBoxShape4DComponent::UpdateComponentFromShape: Shape4D is not a UBoxShape4DData."));
		return;
	}
	Size = boxShapeData->Size;
}

void UBoxShape4DComponent::UpdateShapeFromComponent()
{
	UBoxShape4DData* boxShapeData = Cast<UBoxShape4DData>(Shape4D);
	if (!ensure(boxShapeData != nullptr))
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("UBoxShape4DComponent::UpdateShapeFromComponent: Shape4D is not a UBoxShape4DData."));
		return;
	}
	boxShapeData->Size = Size;
}

void UBoxShape4DComponent::PostInitProperties()
{
	EnsureShapeDataIsOwned();
	if (Shape4D == nullptr || !IsValid(Shape4D) || !Shape4D->IsValidLowLevel())
	{
		Shape4D = NewObject<UBoxShape4DData>(this);
	}
	UpdateShapeFromComponent();
	Super::PostInitProperties();
}

void UBoxShape4DComponent::PostLoad()
{
	EnsureShapeDataIsOwned();
	// Handle missing shapes in loaded components (corruption or old version).
	if (Shape4D == nullptr || !IsValid(Shape4D) || !Shape4D->IsValidLowLevel())
	{
		Shape4D = NewObject<UBoxShape4DData>(this);
	}
	UpdateShapeFromComponent();
	Super::PostLoad();
}

#if WITH_EDITOR
void UBoxShape4DComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	UBoxShape4DData* boxShapeData = Cast<UBoxShape4DData>(Shape4D);
	if (!ensure(boxShapeData != nullptr))
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("UBoxShape4DComponent::PostEditChangeProperty: Shape4 member is not a UBoxShape4DData."));
		return;
	}
	UpdateShapeFromComponent();
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif // WITH_EDITOR
