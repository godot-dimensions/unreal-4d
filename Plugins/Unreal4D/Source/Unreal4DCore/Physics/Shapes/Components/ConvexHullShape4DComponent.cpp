#include "ConvexHullShape4DComponent.h"

#include "../../../Model/Mesh/Base/Mesh4D.h"

void UConvexHullShape4DComponent::SetMesh(UMesh4D* InMesh, UConvexHullShape4DData* InShapeData)
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

void UConvexHullShape4DComponent::UpdateComponentFromShape()
{
	UConvexHullShape4DData* convexHullShapeData = Cast<UConvexHullShape4DData>(Shape4D);
	if (!ensure(convexHullShapeData != nullptr))
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("UConvexHullShape4DComponent::UpdateComponentFromShape: Shape4D is not a UConvexHullShape4DData."));
		return;
	}
	// Don't attempt to reconstruct the mesh from the shape data. Instead, callers can use `SetMesh()`.
}

void UConvexHullShape4DComponent::UpdateShapeFromComponent()
{
	UConvexHullShape4DData* convexHullShapeData = Cast<UConvexHullShape4DData>(Shape4D);
	if (!ensure(convexHullShapeData != nullptr))
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("UConvexHullShape4DComponent::UpdateShapeFromComponent: Shape4D is not a UConvexHullShape4DData."));
		return;
	}

	if (Mesh != nullptr)
	{
		UConvexHullShape4DData* NewShapeData = UConvexHullShape4DData::CreateFromMesh(Mesh);
		if (NewShapeData != nullptr)
		{
			convexHullShapeData->SetPoints(NewShapeData->GetPoints());
		}
	}
}

void UConvexHullShape4DComponent::PostInitProperties()
{
	EnsureShapeDataIsOwned();
	if (Shape4D == nullptr || !IsValid(Shape4D) || !Shape4D->IsValidLowLevel())
	{
		Shape4D = NewObject<UConvexHullShape4DData>(this);
	}
	UpdateShapeFromComponent();
	Super::PostInitProperties();
}

void UConvexHullShape4DComponent::PostLoad()
{
	EnsureShapeDataIsOwned();
	if (Shape4D == nullptr || !IsValid(Shape4D) || !Shape4D->IsValidLowLevel())
	{
		Shape4D = NewObject<UConvexHullShape4DData>(this);
	}
	UpdateShapeFromComponent();
	Super::PostLoad();
}

#if WITH_EDITOR
void UConvexHullShape4DComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	UConvexHullShape4DData* ConvexHullShapeData = Cast<UConvexHullShape4DData>(Shape4D);
	if (!ensure(ConvexHullShapeData != nullptr))
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("UConvexHullShape4DComponent::PostEditChangeProperty: Shape4D is not a UConvexHullShape4DData."));
		return;
	}
	UpdateShapeFromComponent();
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif // WITH_EDITOR
