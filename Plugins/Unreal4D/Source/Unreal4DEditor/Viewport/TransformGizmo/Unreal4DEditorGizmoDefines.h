#pragma once

#include "../../Unreal4DEditorDefines.h"

#include "Unreal4DEditorGizmoDefines.generated.h"

class AEditorGizmoActor4D;
class UEditorTransformGizmo4D;
class UEditorTransformProxy4D;

static constexpr double UNREAL4D_EDITOR_TRANSFORM_GIZMO_EXTENTS = 200.0;
static constexpr double UNREAL4D_EDITOR_TRANSFORM_GIZMO_BIPLANE_DIAMETER_RATIO = 0.25;
static constexpr double UNREAL4D_EDITOR_TRANSFORM_GIZMO_BIPLANE_GAP_RATIO = 0.05;
static constexpr double UNREAL4D_EDITOR_TRANSFORM_GIZMO_BIPLANE_OFFSET_RATIO = 0.25;
static constexpr double UNREAL4D_EDITOR_TRANSFORM_GIZMO_CENTER_TARGET_RADIUS_RATIO = 0.125;
static constexpr double UNREAL4D_EDITOR_TRANSFORM_GIZMO_CENTER_VISUAL_RADIUS_RATIO = UNREAL4D_EDITOR_TRANSFORM_GIZMO_CENTER_TARGET_RADIUS_RATIO * 0.5;

// Conformal means orthogonal and uniform scale, and is required for many things such as physics.
// This is mapped from Unreal's "PreserveNonUniformScale" checkbox (see UEditorTransformGizmo4DFactory::CreateGizmo).
// TODO: Consider exposing some or all of these options as a dropdown beyond just the checkbox.
UENUM()
enum class EEditorTransform4DKeepMode : uint8
{
	Freeform,
	Orthogonal,
	ConformalChangesOnly,
	ConformalChangesAndResults,
	Orthonormal,
};

UENUM()
enum class EEditorTransformGizmo4DPart : uint8
{
	None,
	MoveScreenRelative,
	MoveX,
	MoveY,
	MoveZ,
	MoveW,
	MoveXY,
	MoveXZ,
	MoveXW,
	MoveYZ,
	MoveYW,
	MoveZW,
	RotateXY,
	RotateXZ,
	RotateXW,
	RotateYZ,
	RotateYW,
	RotateZW,
	ScaleUniform,
	ScaleX,
	ScaleY,
	ScaleZ,
	ScaleW,
	ScaleXY,
	ScaleXZ,
	ScaleXW,
	ScaleYZ,
	ScaleYW,
	ScaleZW,
	StretchPosX,
	StretchNegX,
	StretchPosY,
	StretchNegY,
	StretchPosZ,
	StretchNegZ,
	StretchPosW,
	StretchNegW,
};

struct FEditorTransformGizmo4DPartVisibilitySettings
{
	bool bIsMoveScreenRelativeEnabled = true;
	bool bIsMoveLinearEnabled = true;
	bool bIsMovePlanarEnabled = false;
	bool bIsRotatePlanarEnabled = false;
	bool bIsScaleUniformEnabled = false;
	bool bIsScaleLinearEnabled = false;
	bool bIsScalePlanarEnabled = false;
	bool bIsStretchEnabled = false;

	bool AreArrowsVisible() const
	{
		return bIsMoveLinearEnabled || bIsScaleLinearEnabled;
	}

	bool ArePlanesVisible() const
	{
		return bIsMovePlanarEnabled || bIsScalePlanarEnabled;
	}

	bool IsCenterVisible() const
	{
		return bIsMoveScreenRelativeEnabled || bIsScaleUniformEnabled;
	}
};
