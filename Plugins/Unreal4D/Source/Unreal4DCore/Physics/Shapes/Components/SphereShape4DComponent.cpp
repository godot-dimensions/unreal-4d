#include "SphereShape4DComponent.h"

#include "../Data/SphereShape4DData.h"

void USphereShape4DComponent::SetRadius(const double InNewRadius)
{
	Radius = InNewRadius;
	UpdateShapeFromComponent();
}

double USphereShape4DComponent::GetRadiusWorld() const
{
	return (Radius * GetGlobalUniformScale4D());
}

void USphereShape4DComponent::UpdateComponentFromShape()
{
	USphereShape4DData* SphereShapeData = Cast<USphereShape4DData>(Shape4D);
	if (!ensure(SphereShapeData != nullptr))
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("USphereShape4DComponent::UpdateComponentFromShape: Shape4D is not a USphereShape4DData."));
		return;
	}
	Radius = SphereShapeData->Radius;
}

void USphereShape4DComponent::UpdateShapeFromComponent()
{
	USphereShape4DData* SphereShapeData = Cast<USphereShape4DData>(Shape4D);
	if (!ensure(SphereShapeData != nullptr))
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("USphereShape4DComponent::UpdateShapeFromComponent: Shape4D is not a USphereShape4DData."));
		return;
	}
	SphereShapeData->Radius = Radius;
}

void USphereShape4DComponent::PostInitProperties()
{
	EnsureShapeDataIsOwned();
	if (Shape4D == nullptr || !IsValid(Shape4D) || !Shape4D->IsValidLowLevel())
	{
		Shape4D = NewObject<USphereShape4DData>(this);
	}
	UpdateShapeFromComponent();
	Super::PostInitProperties();
}

void USphereShape4DComponent::PostLoad()
{
	EnsureShapeDataIsOwned();
	if (Shape4D == nullptr || !IsValid(Shape4D) || !Shape4D->IsValidLowLevel())
	{
		Shape4D = NewObject<USphereShape4DData>(this);
	}
	UpdateShapeFromComponent();
	Super::PostLoad();
}

#if WITH_EDITOR
void USphereShape4DComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	USphereShape4DData* SphereShapeData = Cast<USphereShape4DData>(Shape4D);
	if (!ensure(SphereShapeData != nullptr))
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("USphereShape4DComponent::PostEditChangeProperty: Shape4D is not a USphereShape4DData."));
		return;
	}
	UpdateShapeFromComponent();
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif // WITH_EDITOR
