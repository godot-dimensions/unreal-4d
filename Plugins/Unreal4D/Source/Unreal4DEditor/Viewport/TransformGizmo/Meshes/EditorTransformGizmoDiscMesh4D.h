#pragma once

#include "EditorTransformGizmoBaseMesh4D.h"

#include "EditorTransformGizmoDiscMesh4D.generated.h"

// Represents a view-aligned disc (filled circle) mesh for the 4D transform gizmo's center,
// used for the move screen-relative and scale uniform parts.
UCLASS(MinimalAPI)
class UEditorTransformGizmoDiscMesh4D : public UEditorTransformGizmoBaseMesh4D
{
	GENERATED_BODY()

public:
	virtual FBoxSphereBounds CalcBounds(const FTransform& InLocalToWorld) const override;
	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;

	UEditorTransformGizmoDiscMesh4D() : Super() {}
};
