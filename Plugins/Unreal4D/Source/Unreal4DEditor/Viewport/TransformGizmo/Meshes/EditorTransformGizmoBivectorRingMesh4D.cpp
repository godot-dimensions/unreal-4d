#include "EditorTransformGizmoBivectorRingMesh4D.h"

#include "BaseGizmos/GizmoRenderingUtil.h"

void UEditorTransformGizmoBivectorRingMesh4D::SetSecondaryColors(const FLinearColor& InSecondaryNormalColor, const FLinearColor& InSecondaryHighlightColor)
{
	SecondaryNormalColor = InSecondaryNormalColor;
	SecondaryHighlightColor = InSecondaryHighlightColor;
	MarkRenderStateDirty();
}

void UEditorTransformGizmoBivectorRingMesh4D::SetSecondaryTargetPosition(const FVector& InSecondaryPosition)
{

	if (SecondaryTargetPosition != InSecondaryPosition)
	{
		SecondaryTargetPosition = InSecondaryPosition;
		MarkRenderStateDirty();
	}
}

// This proxy class is required for integration with Unreal's threaded rendering system.
class FEditorTransformGizmoBivectorRingMeshSceneProxy4D : public FEditorTransformGizmoBaseMeshSceneProxy4D
{
	static constexpr int RING_SEGMENTS = 64;
public:
	FLinearColor SecondaryColor;
	FVector SecondaryTargetPosition;

	SIZE_T GetTypeHash() const override
	{
		static size_t uniquePointer;
		return reinterpret_cast<size_t>(&uniquePointer);
	}

	virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const override
	{
		// The calculation of TargetPosition is performed in 4D space and is relative to the gizmo's origin.
		// When drawing the 3D visuals of this gizmo, we don't apply the full 3D transformation, just the origin.
		const FVector origin = GetLocalToWorld().GetOrigin();
		for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
		{
			if (VisibilityMap & (1 << ViewIndex))
			{
				const FSceneView* View = Views[ViewIndex];
				const double scale = UE::GizmoRenderingUtil::CalculateLocalPixelToWorldScale(View, origin);
				const FVector worldPrimaryTarget = scale * PrimaryTargetPosition;
				const FVector worldSecondaryTarget = scale * SecondaryTargetPosition;
				FPrimitiveDrawInterface* pdi = Collector.GetPDI(ViewIndex);
				FVector startPoint = origin + worldPrimaryTarget;
				for (int i = 0; i < RING_SEGMENTS; i++) {
					FLinearColor color;
					if (i < RING_SEGMENTS / 8) {
						color = SecondaryColor;
					} else if (i < RING_SEGMENTS * 3 / 8) {
						color = PrimaryColor;
					} else if (i < RING_SEGMENTS * 5 / 8) {
						color = SecondaryColor;
					} else if (i < RING_SEGMENTS * 7 / 8) {
						color = PrimaryColor;
					} else {
						color = SecondaryColor;
					}
					const double angle = (i + 1) * (UNREAL4D_TAU / RING_SEGMENTS);
					const double a = FMath::Cos(angle);
					const double b = FMath::Sin(angle);
					const FVector endPoint = origin + worldPrimaryTarget * a + worldSecondaryTarget * b;
					pdi->DrawLine(startPoint, endPoint, color, SDPG_Foreground, LINE_THICKNESS, DepthBias, true);
					startPoint = endPoint;
				}
			}
		}
	}

	virtual uint32 GetMemoryFootprint() const override
	{
		return IntCastChecked<uint32>(sizeof(*this) + FPrimitiveSceneProxy::GetAllocatedSize());
	}

	FEditorTransformGizmoBivectorRingMeshSceneProxy4D(const UEditorTransformGizmoBivectorRingMesh4D* InComponent) : FEditorTransformGizmoBaseMeshSceneProxy4D(InComponent)
	{
		PrimaryColor = InComponent->GetCurrentPrimaryColor();
		PrimaryTargetPosition = InComponent->GetPrimaryTargetPosition();
		SecondaryColor = InComponent->GetCurrentSecondaryColor();
		SecondaryTargetPosition = InComponent->GetSecondaryTargetPosition();
		DepthBias = InComponent->GetDepthBias();
	}
};

// The bounds size should be adjusted as the gizmo size changes to compensate for the distance to the camera, but that information seems to not be available here.
FBoxSphereBounds UEditorTransformGizmoBivectorRingMesh4D::CalcBounds(const FTransform& InLocalToWorld) const
{
	// Unreal seems to not have a hypot function??
	const FVector extent = FVector(
		FMath::Sqrt(PrimaryTargetPosition.X * PrimaryTargetPosition.X + SecondaryTargetPosition.X * SecondaryTargetPosition.X),
		FMath::Sqrt(PrimaryTargetPosition.Y * PrimaryTargetPosition.Y + SecondaryTargetPosition.Y * SecondaryTargetPosition.Y),
		FMath::Sqrt(PrimaryTargetPosition.Z * PrimaryTargetPosition.Z + SecondaryTargetPosition.Z * SecondaryTargetPosition.Z)
	);
	const double A = PrimaryTargetPosition.SizeSquared();
	const double B = SecondaryTargetPosition.SizeSquared();
	const double C = PrimaryTargetPosition.Dot(SecondaryTargetPosition);
	const double radius = 0.5 * (A + B + FMath::Sqrt((A+B)*(A+B) + 4*(C*C - A*B)));
	return FBoxSphereBounds(FVector::ZeroVector, extent, radius).TransformBy(InLocalToWorld);
}

FPrimitiveSceneProxy* UEditorTransformGizmoBivectorRingMesh4D::CreateSceneProxy()
{
	return new FEditorTransformGizmoBivectorRingMeshSceneProxy4D(this);
}
