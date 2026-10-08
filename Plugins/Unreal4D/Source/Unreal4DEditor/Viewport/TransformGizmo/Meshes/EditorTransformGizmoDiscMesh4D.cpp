#include "EditorTransformGizmoDiscMesh4D.h"

#include "BaseGizmos/GizmoRenderingUtil.h"

// This proxy class is required for integration with Unreal's threaded rendering system.
class FEditorTransformGizmoDiscMeshSceneProxy4D : public FEditorTransformGizmoBaseMeshSceneProxy4D
{
	static constexpr int32 DISC_SEGMENTS = 64;
	static void BuildViewAlignedXY(const FVector& InViewDirection, FVector& OutPlaneX, FVector& OutPlaneY)
	{
		const FVector upTemp = FMath::Abs(InViewDirection.Dot(FVector::UpVector)) < 0.99f ? FVector::UpVector : FVector::ForwardVector;
		OutPlaneX = InViewDirection.Cross(upTemp).GetSafeNormal();
		OutPlaneY = InViewDirection.Cross(OutPlaneX);
	}

public:
	SIZE_T GetTypeHash() const override
	{
		static size_t uniquePointer;
		return reinterpret_cast<size_t>(&uniquePointer);
	}

	virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& InViews, const FSceneViewFamily& InViewFamily, uint32 InVisibilityMap, FMeshElementCollector& InCollector) const override
	{
		// Don't draw if the target position is too small (the size is used as the disc radius).
		if (PrimaryTargetPosition.IsNearlyZero(1.0))
		{
			return;
		}
		const FVector origin = GetLocalToWorld().GetOrigin();
		for (int32 ViewIndex = 0; ViewIndex < InViews.Num(); ViewIndex++)
		{
			if (InVisibilityMap & (1 << ViewIndex))
			{
				const FSceneView* View = InViews[ViewIndex];
				const FVector viewDirection = View->GetViewDirection();
				const double scale = UE::GizmoRenderingUtil::CalculateLocalPixelToWorldScale(View, origin);
				const double useRadius = scale * PrimaryTargetPosition.Size();
				if (useRadius < KINDA_SMALL_NUMBER)
				{
					continue;
				}
				FVector planeX;
				FVector planeY;
				BuildViewAlignedXY(viewDirection, planeX, planeY);
				FPrimitiveDrawInterface* pdi = InCollector.GetPDI(ViewIndex);
				const double angleDelta = UNREAL4D_TAU / DISC_SEGMENTS;
				// Draw radial lines from center to edge to fill the disc.
				for (int32 segmentIndex = 0; segmentIndex < DISC_SEGMENTS; segmentIndex++)
				{
					const double angle = angleDelta * segmentIndex;
					const double deltaX = FMath::Cos(angle);
					const double deltaY = FMath::Sin(angle);
					const FVector edgePoint = origin + useRadius * (planeX * deltaX + planeY * deltaY);
					pdi->DrawLine(origin, edgePoint, PrimaryColor, SDPG_Foreground, LINE_THICKNESS, DepthBias, true);
				}
			}
		}
	}

	virtual uint32 GetMemoryFootprint() const override
	{
		return IntCastChecked<uint32>(sizeof(*this) + FPrimitiveSceneProxy::GetAllocatedSize());
	}

	FEditorTransformGizmoDiscMeshSceneProxy4D(const UEditorTransformGizmoDiscMesh4D* InComponent) : FEditorTransformGizmoBaseMeshSceneProxy4D(InComponent)
	{
		PrimaryColor = InComponent->GetCurrentPrimaryColor();
		PrimaryTargetPosition = InComponent->GetPrimaryTargetPosition();
		DepthBias = InComponent->GetDepthBias();
	}
};

FBoxSphereBounds UEditorTransformGizmoDiscMesh4D::CalcBounds(const FTransform& InLocalToWorld) const
{
	const float localRadius = static_cast<float>(PrimaryTargetPosition.Size());
	const FVector localExtent(localRadius);
	return FBoxSphereBounds(FVector::ZeroVector, localExtent, localRadius).TransformBy(InLocalToWorld);
}

FPrimitiveSceneProxy* UEditorTransformGizmoDiscMesh4D::CreateSceneProxy()
{
	return new FEditorTransformGizmoDiscMeshSceneProxy4D(this);
}
