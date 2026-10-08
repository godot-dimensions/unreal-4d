#pragma once

#include "../../Unreal4DEditorDefines.h"

#include "Widgets/SLeafWidget.h"

class FBlueprintEditor;
class FEditorViewportClient;
class ILevelEditor;
class SEditorViewport;
class SLevelViewport;
class SSCSEditorViewport;
class SWidget;
class FUnreal4DEditorMode;

enum class EEditorViewportRotationHitType4D : uint8
{
	None,
	Background,
	AxisCirclePositive,
	AxisCircleNegative,
	AxisLine,
	Plane,
};

struct FUnrealViewportRotationHitTarget4D
{
	FVector2f ScreenPoint = FVector2f::ZeroVector;
	float ZIndex = -1000.0f; // This is rendering terminology, not Unreal's Z-up.
	float Angle = 0.0f; // Only used by the PLANE hit type.
	EEditorViewportRotationHitType4D HitType = EEditorViewportRotationHitType4D::None;
	int8 PrimaryAxisNumber = -1; // 0 to 3 for XYZW, or -1 for none.
	int8 SecondaryAxisNumber = -1; // 0 to 3 for XYZW. Only used by the PLANE hit type.
};

class SEditorViewportRotationWidget4D final : public SLeafWidget
{
	FUnrealViewportRotationHitTarget4D _FocusedTarget;
	FVector2f _DragStartPosition;
	bool _IsMouseButtonDown = false;
	bool _IsMouseOrbiting = false;

	// These need to be raw pointers because they are not UObject-derived.
	FEditorViewportClient* _EditorViewportClient = nullptr;
	FUnreal4DEditorMode* _EditorMode4D = nullptr;
	SEditorViewport* _SlateViewport = nullptr;

	// Main logic functions.
	static TArray<FVector2f> BuildArcPoints(const FVector2f& InCenter, const float InRadius, const float InStartAngle, const float InEndAngle, const int32 InPointCount);
	static void DrawFilledArc(const FVector2f &InCenter, const float InRadius, const float InStartAngle, const float InEndAngle, const FLinearColor &InColor, const FGeometry& InMyGeometry, FSlateWindowElementList& OutDrawElements, int32 InLayerId);
	static void DrawCircle(const FVector2f& InCenter, float InRadius, const FLinearColor& InColor, const FGeometry& InMyGeometry, FSlateWindowElementList& OutDrawElements, int32 InLayerId);
	void DrawAxisCircle(const FUnrealViewportRotationHitTarget4D& InTarget, const FGeometry& InMyGeometry, FSlateWindowElementList& OutDrawElements, int32 InLayerId) const;
	void DrawAxisLine(const FUnrealViewportRotationHitTarget4D& InTarget, const FVector2f &InCenter, const FGeometry& InMyGeometry, FSlateWindowElementList& OutDrawElements, int32 InLayerId) const;
	void DrawPlaneSemicircles(const FUnrealViewportRotationHitTarget4D& InTarget, const FGeometry& InMyGeometry, FSlateWindowElementList& OutDrawElements, int32 InLayerId) const;
	static FUnrealViewportRotationHitTarget4D MakePlaneTarget(const FMatrix& InBasis, const int InRight, const int InUp, const FVector2f& InCenter, const float InRadius);
	void GetSortedAxisScreenAligned(const FMatrix& InBasis, const FVector2f& InCenter, const float InRadius, const int8 InRightIndex, const int8 InUpIndex, const int8 InPerpRightIndex, const int8 InPerpUpIndex, TArray<FUnrealViewportRotationHitTarget4D>& OutTargets) const;
	void GetSortedAxisTargets(const FVector2f& InCenter, TArray<FUnrealViewportRotationHitTarget4D>& OutTargets) const;

	// How far apart the axis circles are from the center of the gizmo.
	// Unreal's 3D axes are approximately 80, Godot's 3D uses 80, Godot 4D uses 100.
	// 4D has more axes, so we need to space them out more compared to 3D.
	static constexpr float _BASE_4D_ROTATION_WIDGET_SIZE = 100.0f;

	// Slate integration helpers.
	static float GetSlateScale();
	static FPaintGeometry MakePaintGeometry(const FGeometry& Geometry);
	static FPaintGeometry MakePaintGeometry(const FGeometry& Geometry, const FVector2f& LocalPosition, const FVector2f& LocalSize);

public:
	// Slate constructor parameters, required for construction via the SNew macro.
	SLATE_BEGIN_ARGS(SEditorViewportRotationWidget4D) {}
		SLATE_ARGUMENT(SEditorViewport*, SlateViewport)
	SLATE_END_ARGS()
	void Construct(const FArguments& InArgs);

	// Configure the widget after construction.
	void Setup(FEditorViewportClient* InEditorViewportClient, FUnreal4DEditorMode* InEditorMode4D);

	// Slate integration virtual function overrides.
	virtual FVector2D ComputeDesiredSize(const float InLayoutScaleMultiplier) const override;
	virtual int32 OnPaint(const FPaintArgs& InArgs, const FGeometry& InAllottedGeometry, const FSlateRect& InMyCullingRect,
		FSlateWindowElementList& InOutDrawElements, int32 InLayerId, const FWidgetStyle& InWidgetStyle, bool bInParentEnabled) const override;
	// NOTE: The original Godot 4D code handled touch input as well, this Unreal version only handles mouse input.
	virtual FReply OnMouseButtonDown(const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply OnMouseButtonUp(const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply OnMouseMove(const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void OnMouseLeave(const FPointerEvent& InMouseEvent) override;
	virtual FCursorReply OnCursorQuery(const FGeometry& InMyGeometry, const FPointerEvent& InCursorEvent) const override;
};
