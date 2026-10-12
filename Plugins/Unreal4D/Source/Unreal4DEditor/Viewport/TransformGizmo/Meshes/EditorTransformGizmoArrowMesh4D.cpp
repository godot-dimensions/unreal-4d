#include "EditorTransformGizmoArrowMesh4D.h"

#include "BaseGizmos/GizmoRenderingUtil.h"
#include "MeshElementCollector.h"
#include "PrimitiveDrawInterface.h"

// This proxy class is required for integration with Unreal's threaded rendering system.
class FEditorTransformGizmoArrowMeshSceneProxy4D : public FEditorTransformGizmoBaseMeshSceneProxy4D
{
	static constexpr double CIRCLE_RADIUS_RATIO = 0.06;
	static constexpr int CIRCLE_SEGMENTS = 64;
public:
	EEditorTransformGizmoArrowMeshTipType4D TipType = EEditorTransformGizmoArrowMeshTipType4D::None;

	SIZE_T GetTypeHash() const override
	{
		static size_t uniquePointer;
		return reinterpret_cast<size_t>(&uniquePointer);
	}

	virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const override
	{
		// Don't draw if the target position is too small.
		if (PrimaryTargetPosition.IsNearlyZero(1.0))
		{
			return;
		}
		// The calculation of TargetPosition is performed in 4D space and is relative to the gizmo's origin.
		// When drawing the 3D visuals of this gizmo, we don't apply the full 3D transformation, just the origin.
		const FVector origin = GetLocalToWorld().GetOrigin();
		for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
		{
			if (VisibilityMap & (1 << ViewIndex))
			{
				const FSceneView* View = Views[ViewIndex];
				const double scale = UE::GizmoRenderingUtil::CalculateLocalPixelToWorldScale(View, origin);
				const FVector start = origin + (scale * (0.035 + UNREAL4D_EDITOR_TRANSFORM_GIZMO_CENTER_VISUAL_RADIUS_RATIO)) * PrimaryTargetPosition;
				const FVector end = origin + scale * PrimaryTargetPosition;
				FPrimitiveDrawInterface* pdi = Collector.GetPDI(ViewIndex);
				pdi->DrawLine(start, end, PrimaryColor, SDPG_Foreground, LINE_THICKNESS, DepthBias, true);
				if (TipType != EEditorTransformGizmoArrowMeshTipType4D::None)
				{
					const FVector base = origin + (scale * 0.9) * PrimaryTargetPosition;
					const FVector tip = origin + (scale * 1.1) * PrimaryTargetPosition;
					const FVector direction = (end - start).GetUnsafeNormal();
					const FVector upTemp = FMath::Abs(direction.Dot(FVector::UpVector)) < 0.99f ? FVector::UpVector : FVector::ForwardVector;
					const double extent = scale * CIRCLE_RADIUS_RATIO * PrimaryTargetPosition.Size();
					// The directions don't matter for this algorithm, they just need to be
					// mutually perpendicular with each other and the main direction.
					FVector ortho1 = direction.Cross(upTemp).GetUnsafeNormal();
					FVector ortho2 = direction.Cross(ortho1); // Will be normalized already.
					ortho1 *= extent;
					ortho2 *= extent;
					FVector lastBasePoint = base + ortho1;
					for (int i = 1; i <= CIRCLE_SEGMENTS; i++) {
						const double angle = i * (UNREAL4D_TAU / CIRCLE_SEGMENTS);
						const double a = FMath::Cos(angle);
						const double b = FMath::Sin(angle);
						const FVector basePoint = base + ortho1 * a + ortho2 * b;
						pdi->DrawLine(basePoint, lastBasePoint, PrimaryColor, SDPG_Foreground, LINE_THICKNESS * 0.25f, DepthBias, true);
						if (TipType == EEditorTransformGizmoArrowMeshTipType4D::Cone)
						{
							pdi->DrawLine(basePoint, tip, PrimaryColor, SDPG_Foreground, LINE_THICKNESS * 0.25f, DepthBias, true);
						}
						else if (TipType == EEditorTransformGizmoArrowMeshTipType4D::Cylinder)
						{
							const FVector baseToTip = tip - base;
							pdi->DrawLine(basePoint + baseToTip, lastBasePoint + baseToTip, PrimaryColor, SDPG_Foreground, LINE_THICKNESS * 0.25f, DepthBias, true);
							pdi->DrawLine(basePoint, basePoint + baseToTip, PrimaryColor, SDPG_Foreground, LINE_THICKNESS * 0.25f, DepthBias, true);
						}
						else
						{
							ensure(false); // This case should not be possible.
						}
						lastBasePoint = basePoint;
					}
				}
			}
		}
	}

	virtual uint32 GetMemoryFootprint() const override
	{
		return IntCastChecked<uint32>(sizeof(*this) + FPrimitiveSceneProxy::GetAllocatedSize());
	}

	FEditorTransformGizmoArrowMeshSceneProxy4D(const UEditorTransformGizmoArrowMesh4D* InComponent) : FEditorTransformGizmoBaseMeshSceneProxy4D(InComponent)
	{
		PrimaryColor = InComponent->GetCurrentPrimaryColor();
		PrimaryTargetPosition = InComponent->GetPrimaryTargetPosition();
		DepthBias = InComponent->GetDepthBias();
		TipType = InComponent->TipType;
	}
};

FBoxSphereBounds UEditorTransformGizmoArrowMesh4D::CalcBounds(const FTransform& InLocalToWorld) const
{
	// Note: This assumes the target position is positive (for our use cases it will be).
	const FBox localBox = FBox(FVector::ZeroVector, PrimaryTargetPosition);
	return FBoxSphereBounds(localBox).TransformBy(InLocalToWorld);
}

FPrimitiveSceneProxy* UEditorTransformGizmoArrowMesh4D::CreateSceneProxy()
{
	return new FEditorTransformGizmoArrowMeshSceneProxy4D(this);
}
