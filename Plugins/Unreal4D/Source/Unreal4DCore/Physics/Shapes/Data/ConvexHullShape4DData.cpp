#include "ConvexHullShape4DData.h"

#include "../../../Model/Mesh/Tetra/ArrayTetraMesh4D.h"

void UConvexHullShape4DData::SetPoints(const TArray<FVector4>& InPoints)
{
	_points = InPoints;
}

bool UConvexHullShape4DData::IsEqualExact(const UShape4DData* InShape) const
{
	const UConvexHullShape4DData* otherShape = Cast<UConvexHullShape4DData>(InShape);
	if (otherShape == nullptr)
	{
		return false;
	}
	const TArray<FVector4> otherPoints = otherShape->GetPoints();
	return _points == otherPoints;
}

UTetraMesh4D* UConvexHullShape4DData::ToTetraMesh() const
{
	UArrayTetraMesh4D* mesh = NewObject<UArrayTetraMesh4D>();
	mesh->SetVertexValues(_points);
	UE_LOG(LogUnreal4DCore, Warning, TEXT("UConvexHullShape4DData::ToTetraMesh: Calculating the tetrahedral mesh cells is not implemented yet."));
	return mesh;
}

UConvexHullShape4DData* UConvexHullShape4DData::CreateFromMesh(UMesh4D* InMesh)
{
	UConvexHullShape4DData* shape = NewObject<UConvexHullShape4DData>();
	shape->SetPoints(InMesh->GetVertexValues());
	UE_LOG(LogUnreal4DCore, Warning, TEXT("UConvexHullShape4DData::CreateFromMesh: Calculating the convex hull from mesh vertices is not implemented yet."));
	return shape;
}
