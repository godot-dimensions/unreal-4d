#include "CapsuleShape4DComponent.h"

#include "../Data/CapsuleShape4DData.h"

double UCapsuleShape4DComponent::GetFullHeight() const
{
	if (bHeightIsFull)
	{
		return Height;
	}
	return Height + Radius * 2.0;
}

void UCapsuleShape4DComponent::SetFullHeight(const double InFullHeight)
{
	if (bHeightIsFull)
	{
		Height = InFullHeight;
	}
	else
	{
		Height = InFullHeight - Radius * 2.0;
	}
	UpdateShapeFromComponent();
}

double UCapsuleShape4DComponent::GetMidHeight() const
{
	if (bHeightIsFull)
	{
		return Height - Radius * 2.0;
	}
	return Height;
}

void UCapsuleShape4DComponent::SetMidHeight(const double InMidHeight)
{
	if (bHeightIsFull)
	{
		Height = InMidHeight + Radius * 2.0;
	}
	else
	{
		Height = InMidHeight;
	}
	UpdateShapeFromComponent();
}

void UCapsuleShape4DComponent::SetHeightIsFull(const bool bInHeightIsFull)
{
	if (bInHeightIsFull == bHeightIsFull) { return; }
	bHeightIsFull = bInHeightIsFull;
	if (bInHeightIsFull)
	{
		// Convert from mid-height to full height.
		Height += Radius * 2.0;
	}
	else
	{
		// Convert from full height to mid-height.
		Height -= Radius * 2.0;
	}
	UpdateShapeFromComponent();
}

void UCapsuleShape4DComponent::SetRadius(const double InNewRadius)
{
	Radius = InNewRadius;
	UpdateShapeFromComponent();
}

void UCapsuleShape4DComponent::UpdateComponentFromShape()
{
	UCapsuleShape4DData* capsuleShapeData = Cast<UCapsuleShape4DData>(Shape4D);
	if (!ensure(capsuleShapeData != nullptr))
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("UCapsuleShape4DComponent::UpdateComponentFromShape: Shape4D is not a UCapsuleShape4DData."));
		return;
	}
	Height = capsuleShapeData->Height;
	Radius = capsuleShapeData->Radius;
	bHeightIsFull = capsuleShapeData->bHeightIsFull;
}

void UCapsuleShape4DComponent::UpdateShapeFromComponent()
{
	UCapsuleShape4DData* capsuleShapeData = Cast<UCapsuleShape4DData>(Shape4D);
	if (!ensure(capsuleShapeData != nullptr))
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("UCapsuleShape4DComponent::UpdateShapeFromComponent: Shape4D is not a UCapsuleShape4DData."));
		return;
	}
	capsuleShapeData->Height = Height;
	capsuleShapeData->Radius = Radius;
	capsuleShapeData->bHeightIsFull = bHeightIsFull;
}

void UCapsuleShape4DComponent::PostInitProperties()
{
	EnsureShapeDataIsOwned();
	if (Shape4D == nullptr || !IsValid(Shape4D) || !Shape4D->IsValidLowLevel())
	{
		Shape4D = NewObject<UCapsuleShape4DData>(this);
	}
	UpdateShapeFromComponent();
	Super::PostInitProperties();
}

void UCapsuleShape4DComponent::PostLoad()
{
	EnsureShapeDataIsOwned();
	if (Shape4D == nullptr || !IsValid(Shape4D) || !Shape4D->IsValidLowLevel())
	{
		Shape4D = NewObject<UCapsuleShape4DData>(this);
	}
	UpdateShapeFromComponent();
	Super::PostLoad();
}

#if WITH_EDITOR
void UCapsuleShape4DComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	UCapsuleShape4DData* CapsuleShapeData = Cast<UCapsuleShape4DData>(Shape4D);
	if (!ensure(CapsuleShapeData != nullptr))
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("UCapsuleShape4DComponent::PostEditChangeProperty: Shape4D is not a UCapsuleShape4DData."));
		return;
	}
	UpdateShapeFromComponent();
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif // WITH_EDITOR
