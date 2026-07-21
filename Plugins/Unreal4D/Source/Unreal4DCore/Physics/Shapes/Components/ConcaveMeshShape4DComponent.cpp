#include "ConcaveMeshShape4DComponent.h"

#include "../../../Model/Mesh/Tetra/TetraMesh4D.h"

void UConcaveMeshShape4DComponent::SetMesh(UTetraMesh4D* InMesh, UConcaveMeshShape4DData* InShapeData)
{
	Mesh = InMesh;
	if (InShapeData == nullptr)
	{
		UpdateShapeFromComponent();
	}
	else
	{
		Shape4D = InShapeData;
	}
}

void UConcaveMeshShape4DComponent::UpdateComponentFromShape()
{
	UConcaveMeshShape4DData* ConcaveMeshShapeData = Cast<UConcaveMeshShape4DData>(Shape4D);
	if (!ensure(ConcaveMeshShapeData != nullptr))
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("UConcaveMeshShape4DComponent::UpdateComponentFromShape: Shape4D is not a UConcaveMeshShape4DData."));
		return;
	}
	// Don't attempt to reconstruct the mesh from the shape data. Instead, callers can use `SetMesh()`.
}

void UConcaveMeshShape4DComponent::UpdateShapeFromComponent()
{
	UConcaveMeshShape4DData* concaveMeshShapeData = Cast<UConcaveMeshShape4DData>(Shape4D);
	if (!ensure(concaveMeshShapeData != nullptr))
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("UConcaveMeshShape4DComponent::UpdateShapeFromComponent: Shape4D is not a UConcaveMeshShape4DData."));
		return;
	}
	if (Mesh != nullptr)
	{
		concaveMeshShapeData->SetFromMesh(Mesh);
	}
}

void UConcaveMeshShape4DComponent::PostInitProperties()
{
	EnsureShapeDataIsOwned();
	if (Shape4D == nullptr || !IsValid(Shape4D) || !Shape4D->IsValidLowLevel())
	{
		Shape4D = NewObject<UConcaveMeshShape4DData>(this);
	}
	UpdateShapeFromComponent();
	Super::PostInitProperties();
}

void UConcaveMeshShape4DComponent::PostLoad()
{
	EnsureShapeDataIsOwned();
	if (Shape4D == nullptr || !IsValid(Shape4D) || !Shape4D->IsValidLowLevel())
	{
		Shape4D = NewObject<UConcaveMeshShape4DData>(this);
	}
	UpdateShapeFromComponent();
	Super::PostLoad();
}

#if WITH_EDITOR
void UConcaveMeshShape4DComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	UConcaveMeshShape4DData* ConcaveMeshShapeData = Cast<UConcaveMeshShape4DData>(Shape4D);
	if (!ensure(ConcaveMeshShapeData != nullptr))
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("UConcaveMeshShape4DComponent::PostEditChangeProperty: Shape4D is not a UConcaveMeshShape4DData."));
		return;
	}
	UpdateShapeFromComponent();
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif // WITH_EDITOR
