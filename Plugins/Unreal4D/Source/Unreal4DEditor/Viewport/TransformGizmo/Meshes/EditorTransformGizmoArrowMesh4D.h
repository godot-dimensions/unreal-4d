#pragma once

#include "EditorTransformGizmoBaseMesh4D.h"

#include "EditorTransformGizmoArrowMesh4D.generated.h"

enum class EEditorTransformGizmoArrowMeshTipType4D : uint8
{
	None,
	Cone,
	Cylinder,
};

// Inspired by Unreal's UGizmoArrowComponent class.
// Due to limitations in that class, we have to implement our own arrow mesh component.
UCLASS(MinimalAPI)
class UEditorTransformGizmoArrowMesh4D : public UEditorTransformGizmoBaseMesh4D
{
	GENERATED_BODY()

public:
	EEditorTransformGizmoArrowMeshTipType4D TipType = EEditorTransformGizmoArrowMeshTipType4D::None;

	virtual FBoxSphereBounds CalcBounds(const FTransform& InLocalToWorld) const override;
	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;

	UEditorTransformGizmoArrowMesh4D() : Super() {}
};
