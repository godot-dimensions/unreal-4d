#include "EditorTransformGizmoBiplaneMesh4D.h"

#include "BaseGizmos/GizmoRenderingUtil.h"

void UEditorTransformGizmoBiplaneMesh4D::SetSecondaryColors(const FLinearColor& InSecondaryNormalColor, const FLinearColor& InSecondaryHighlightColor)
{
	SecondaryNormalColor = InSecondaryNormalColor;
	SecondaryHighlightColor = InSecondaryHighlightColor;
	MarkRenderStateDirty();
}

void UEditorTransformGizmoBiplaneMesh4D::SetSecondaryTargetPosition(const FVector& InSecondaryPosition)
{

	if (SecondaryTargetPosition != InSecondaryPosition)
	{
		SecondaryTargetPosition = InSecondaryPosition;
		MarkRenderStateDirty();
	}
}

// This proxy class is required for integration with Unreal's threaded rendering system.
class FEditorTransformGizmoBiplaneMeshSceneProxy4D : public FEditorTransformGizmoBaseMeshSceneProxy4D
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
		// Don't draw if either target position is too small.
		if (PrimaryTargetPosition.IsNearlyZero(1.0) || SecondaryTargetPosition.IsNearlyZero(1.0))
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
				FPrimitiveDrawInterface* pdi = Collector.GetPDI(ViewIndex);
				const double scale = UE::GizmoRenderingUtil::CalculateLocalPixelToWorldScale(View, origin);
				// Primary triangle.
				{
					const FVector a = origin + scale * (SecondaryTargetPosition * UNREAL4D_EDITOR_TRANSFORM_GIZMO_BIPLANE_OFFSET_RATIO + PrimaryTargetPosition * (UNREAL4D_EDITOR_TRANSFORM_GIZMO_BIPLANE_OFFSET_RATIO + UNREAL4D_EDITOR_TRANSFORM_GIZMO_BIPLANE_GAP_RATIO));
					const FVector b = origin + scale * (SecondaryTargetPosition * UNREAL4D_EDITOR_TRANSFORM_GIZMO_BIPLANE_OFFSET_RATIO + PrimaryTargetPosition * (UNREAL4D_EDITOR_TRANSFORM_GIZMO_BIPLANE_OFFSET_RATIO + UNREAL4D_EDITOR_TRANSFORM_GIZMO_BIPLANE_DIAMETER_RATIO));
					const FVector c = origin + scale * (SecondaryTargetPosition * (UNREAL4D_EDITOR_TRANSFORM_GIZMO_BIPLANE_OFFSET_RATIO + UNREAL4D_EDITOR_TRANSFORM_GIZMO_BIPLANE_DIAMETER_RATIO - UNREAL4D_EDITOR_TRANSFORM_GIZMO_BIPLANE_GAP_RATIO) + PrimaryTargetPosition * (UNREAL4D_EDITOR_TRANSFORM_GIZMO_BIPLANE_OFFSET_RATIO + UNREAL4D_EDITOR_TRANSFORM_GIZMO_BIPLANE_DIAMETER_RATIO));
					pdi->DrawLine(a, b, PrimaryColor, SDPG_Foreground, LINE_THICKNESS, DepthBias, true);
					pdi->DrawLine(b, c, PrimaryColor, SDPG_Foreground, LINE_THICKNESS, DepthBias, true);
					pdi->DrawLine(c, a, PrimaryColor, SDPG_Foreground, LINE_THICKNESS, DepthBias, true);
				}
				// Secondary triangle.
				{
					const FVector a = origin + scale * (PrimaryTargetPosition * UNREAL4D_EDITOR_TRANSFORM_GIZMO_BIPLANE_OFFSET_RATIO + SecondaryTargetPosition * (UNREAL4D_EDITOR_TRANSFORM_GIZMO_BIPLANE_OFFSET_RATIO + UNREAL4D_EDITOR_TRANSFORM_GIZMO_BIPLANE_GAP_RATIO));
					const FVector b = origin + scale * (PrimaryTargetPosition * UNREAL4D_EDITOR_TRANSFORM_GIZMO_BIPLANE_OFFSET_RATIO + SecondaryTargetPosition * (UNREAL4D_EDITOR_TRANSFORM_GIZMO_BIPLANE_OFFSET_RATIO + UNREAL4D_EDITOR_TRANSFORM_GIZMO_BIPLANE_DIAMETER_RATIO));
					const FVector c = origin + scale * (PrimaryTargetPosition * (UNREAL4D_EDITOR_TRANSFORM_GIZMO_BIPLANE_OFFSET_RATIO + UNREAL4D_EDITOR_TRANSFORM_GIZMO_BIPLANE_DIAMETER_RATIO - UNREAL4D_EDITOR_TRANSFORM_GIZMO_BIPLANE_GAP_RATIO) + SecondaryTargetPosition * (UNREAL4D_EDITOR_TRANSFORM_GIZMO_BIPLANE_OFFSET_RATIO + UNREAL4D_EDITOR_TRANSFORM_GIZMO_BIPLANE_DIAMETER_RATIO));
					pdi->DrawLine(a, b, SecondaryColor, SDPG_Foreground, LINE_THICKNESS, DepthBias, true);
					pdi->DrawLine(b, c, SecondaryColor, SDPG_Foreground, LINE_THICKNESS, DepthBias, true);
					pdi->DrawLine(c, a, SecondaryColor, SDPG_Foreground, LINE_THICKNESS, DepthBias, true);
				}
			}
		}
	}

	virtual uint32 GetMemoryFootprint() const override
	{
		return IntCastChecked<uint32>(sizeof(*this) + FPrimitiveSceneProxy::GetAllocatedSize());
	}

	FEditorTransformGizmoBiplaneMeshSceneProxy4D(const UEditorTransformGizmoBiplaneMesh4D* InComponent) : FEditorTransformGizmoBaseMeshSceneProxy4D(InComponent)
	{
		PrimaryColor = InComponent->GetCurrentPrimaryColor();
		PrimaryTargetPosition = InComponent->GetPrimaryTargetPosition();
		SecondaryColor = InComponent->GetCurrentSecondaryColor();
		SecondaryTargetPosition = InComponent->GetSecondaryTargetPosition();
		DepthBias = InComponent->GetDepthBias();
	}
};

FBoxSphereBounds UEditorTransformGizmoBiplaneMesh4D::CalcBounds(const FTransform& InLocalToWorld) const
{
	// Take the maximum of the target positions.
	// Note: This assumes that both target positions are positive (for our use cases they will be).
	const FVector extent = PrimaryTargetPosition.ComponentMax(SecondaryTargetPosition);
	return FBoxSphereBounds(FVector::ZeroVector, extent, 1.0).TransformBy(InLocalToWorld);
}

FPrimitiveSceneProxy* UEditorTransformGizmoBiplaneMesh4D::CreateSceneProxy()
{
	return new FEditorTransformGizmoBiplaneMeshSceneProxy4D(this);
}
