#include "EditorTransformGizmoBaseMesh4D.h"

void UEditorTransformGizmoBaseMesh4D::SetPrimaryColors(const FLinearColor& InPrimaryNormalColor, const FLinearColor& InPrimaryHighlightColor)
{
	PrimaryNormalColor = InPrimaryNormalColor;
	PrimaryHighlightColor = InPrimaryHighlightColor;
	MarkRenderStateDirty();
}

void UEditorTransformGizmoBaseMesh4D::SetHighlighted(const bool bInIsHighlighted)
{
	bIsHighlighted = bInIsHighlighted;
	MarkRenderStateDirty();
}

void UEditorTransformGizmoBaseMesh4D::SetPrimaryTargetPosition(const FVector& InPrimaryPosition)
{
	if (PrimaryTargetPosition != InPrimaryPosition)
	{
		PrimaryTargetPosition = InPrimaryPosition;
		MarkRenderStateDirty();
	}
}

UEditorTransformGizmoBaseMesh4D::UEditorTransformGizmoBaseMesh4D()
{
	// Disable collision by default.
	SetCollisionEnabled(ECollisionEnabled::NoCollision);
	// Always render in the foreground, on top of the scene.
	SetCastShadow(false);
	bUseEditorCompositing = true;
}
