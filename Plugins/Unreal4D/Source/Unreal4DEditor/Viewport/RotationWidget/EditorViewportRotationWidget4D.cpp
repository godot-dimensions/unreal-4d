#include "EditorViewportRotationWidget4D.h"

#include "../../Unreal4DEditorMode.h"

#include "../../../Unreal4DCore/Math/Basis4D.h"
#include "../../../Unreal4DCore/Math/Math4D.h"
#include "../../../Unreal4DCore/Math/VectorFuncs4D.h"

#include "BlueprintEditor.h"
#include "Fonts/FontMeasure.h"
#include "SLevelViewport.h"
#include "SSCSEditorViewport.h"

TArray<FVector2f> SEditorViewportRotationWidget4D::BuildArcPoints(const FVector2f& InCenter, const float InRadius, const float InStartAngle, const float InEndAngle, const int32 InPointCount)
{
	ensure(InPointCount >= 2);
	TArray<FVector2f> points;
	points.Reserve(InPointCount);
	const float angleStep = (InEndAngle - InStartAngle) / float(InPointCount - 1);
	for (int32 index = 0; index < InPointCount; ++index)
	{
		const float angle = InStartAngle + float(index) * angleStep;
		points.Add(InCenter + FVector2f(FMath::Cos(angle), FMath::Sin(angle)) * InRadius);
	}
	return points;
}

void SEditorViewportRotationWidget4D::DrawFilledArc(const FVector2f&InCenter, const float InRadius, const float InStartAngle, const float InEndAngle, const FLinearColor &InColor, const FGeometry& InMyGeometry, FSlateWindowElementList& OutDrawElements, int32 InLayerId)
{
	const TArray<FVector2f> arcPoints = BuildArcPoints(InCenter, InRadius, InStartAngle, InEndAngle, 48);
	// Draw a filled arc using global-screen-space triangles.
	TArray<FSlateVertex> verts;
	verts.SetNum(arcPoints.Num());
	const FColor color = InColor.ToFColor(true);
	const FVector2f geometryScale = InMyGeometry.GetAbsoluteSize() / InMyGeometry.GetLocalSize();
	for (int32 i = 0; i < arcPoints.Num(); i++)
	{
		FSlateVertex vert = FSlateVertex();
		vert.Position = FVector2f(arcPoints[i]) * geometryScale + InMyGeometry.GetAbsolutePosition();
		vert.Color = color;
		vert.SecondaryColor = color;
		vert.TexCoords[0] = 0.5f;
		vert.TexCoords[1] = 0.5f;
		vert.TexCoords[2] = 0.5f;
		vert.TexCoords[3] = 0.5f;
		verts[i] = vert;
	}
	TArray<SlateIndex> indices;
	indices.SetNum((arcPoints.Num() - 2) * 3);
	for (int32 i = 0; i < arcPoints.Num() - 2; i++)
	{
		indices[i * 3] = 0;
		indices[i * 3 + 1] = i + 1;
		indices[i * 3 + 2] = i + 2;
	}
	const FSlateBrush* whiteBrush = FAppStyle::GetBrush("WhiteSquare");
	FSlateDrawElement::MakeCustomVerts(OutDrawElements, InLayerId, whiteBrush->GetRenderingResource(), verts, indices, nullptr, 0, 0);
}

void SEditorViewportRotationWidget4D::DrawCircle(const FVector2f& InCenter, float InRadius, const FLinearColor& InColor, const FGeometry& InMyGeometry, FSlateWindowElementList& OutDrawElements, int32 InLayerId)
{
	DrawFilledArc(InCenter, InRadius, 0.0f, UNREAL4D_TAU, InColor, InMyGeometry, OutDrawElements, InLayerId);
	// A hollow circle would be: FSlateDrawElement::MakeLines(OutDrawElements, InLayerId, MakePaintGeometry(InMyGeometry), circlePoints, ESlateDrawEffect::None, InColor, true, 1.5f * GetSlateScale());
}

void SEditorViewportRotationWidget4D::DrawAxisCircle(const FUnrealViewportRotationHitTarget4D& InTarget, const FGeometry& InMyGeometry, FSlateWindowElementList& OutDrawElements, int32 InLayerId) const
{
	const bool bIsFocused = _FocusedTarget.HitType == InTarget.HitType && _FocusedTarget.PrimaryAxisNumber == InTarget.PrimaryAxisNumber && _FocusedTarget.SecondaryAxisNumber == InTarget.SecondaryAxisNumber;
	const FLinearColor axisColor = FUnreal4DEditorMode::GetAxisColor((EAxis4D)InTarget.PrimaryAxisNumber);
	const float unfocusedAlpha = FMath::Min(2.0f, InTarget.ZIndex + 2.0f);
	FLinearColor color = bIsFocused ? UMath4D::ColorLightened(axisColor, 0.25f) : axisColor;
	color.A = bIsFocused ? 1.0f : unfocusedAlpha;
	const float axisCircleRadius = (8.0f + InTarget.ZIndex) * GetSlateScale();
	DrawCircle(InTarget.ScreenPoint, axisCircleRadius, color, InMyGeometry, OutDrawElements, InLayerId);
	if (InTarget.HitType == EEditorViewportRotationHitType4D::AxisCirclePositive)
	{
		// Draw the axis letter for the positive axes.
		const FString axisLetter = UVectorFuncs4D::Vector4_AxisLetter((EAxis4D)InTarget.PrimaryAxisNumber);
		const FSlateFontInfo font = FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), 10);
		const TSharedRef<FSlateFontMeasure> fontMeasure = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
		const FVector2f textSize = fontMeasure->Measure(FText::FromString(axisLetter), font);
		const FVector2f textPos = InTarget.ScreenPoint - (textSize * 0.5f);
		FSlateDrawElement::MakeText(OutDrawElements, InLayerId + 1, MakePaintGeometry(InMyGeometry, textPos, textSize), FText::FromString(axisLetter), font, ESlateDrawEffect::None, FLinearColor(0.0f, 0.0f, 0.0f, unfocusedAlpha * 0.6f));
	}
	else
	{
		ensure(InTarget.HitType == EEditorViewportRotationHitType4D::AxisCircleNegative);
		// Draw an outline around the negative axes.
		DrawCircle(InTarget.ScreenPoint, axisCircleRadius * 0.8f, color * 0.6f, InMyGeometry, OutDrawElements, InLayerId + 1);
	}
}

void SEditorViewportRotationWidget4D::DrawAxisLine(const FUnrealViewportRotationHitTarget4D& InTarget, const FVector2f&InCenter, const FGeometry& InMyGeometry, FSlateWindowElementList& OutDrawElements, int32 InLayerId) const
{
	const bool bIsFocused = InTarget.PrimaryAxisNumber == _FocusedTarget.PrimaryAxisNumber && _FocusedTarget.HitType == EEditorViewportRotationHitType4D::AxisCirclePositive;
	const FLinearColor axisColor = FUnreal4DEditorMode::GetAxisColor((EAxis4D)InTarget.PrimaryAxisNumber);
	const float unfocusedAlpha = FMath::Min(2.0f, InTarget.ZIndex + 2.0f);
	FLinearColor color = bIsFocused ? UMath4D::ColorLightened(axisColor, 0.25f) : axisColor;
	color.A = bIsFocused ? 1.0f : unfocusedAlpha;
	const TArray<FVector2f> points = { InCenter, InTarget.ScreenPoint };
	FSlateDrawElement::MakeLines(OutDrawElements, InLayerId, MakePaintGeometry(InMyGeometry), points, ESlateDrawEffect::None, color, true, 1.5f * GetSlateScale());
}

void SEditorViewportRotationWidget4D::DrawPlaneSemicircles(const FUnrealViewportRotationHitTarget4D& InTarget, const FGeometry& InMyGeometry, FSlateWindowElementList& OutDrawElements, int32 InLayerId) const
{
	const bool bIsFocused = _FocusedTarget.HitType == InTarget.HitType && _FocusedTarget.PrimaryAxisNumber == InTarget.PrimaryAxisNumber && _FocusedTarget.SecondaryAxisNumber == InTarget.SecondaryAxisNumber;
	FLinearColor primaryColor = FUnreal4DEditorMode::GetAxisColor((EAxis4D)InTarget.PrimaryAxisNumber);
	FLinearColor secondaryColor = FUnreal4DEditorMode::GetAxisColor((EAxis4D)InTarget.SecondaryAxisNumber);
	if (bIsFocused)
	{
		// NOTE: Unreal's color space is a bit different from Godot, so use 0.25 instead of 0.5.
		primaryColor = UMath4D::ColorLightened(primaryColor, 0.25f);
		secondaryColor = UMath4D::ColorLightened(secondaryColor, 0.25f);
	}
	else
	{
		const float alpha = FMath::Min(2.0f, InTarget.ZIndex + 2.0f);
		primaryColor.A = alpha;
		secondaryColor.A = alpha;
	}
	constexpr float QUARTER_TURN = UNREAL4D_TAU / 4.0f;
	const float slateScale = GetSlateScale();
	const float outerRadius = (4.0f + InTarget.ZIndex) * slateScale;
	DrawFilledArc(InTarget.ScreenPoint, outerRadius, InTarget.Angle + QUARTER_TURN, InTarget.Angle + QUARTER_TURN * 3.0f, primaryColor, InMyGeometry, OutDrawElements, InLayerId);
	DrawFilledArc(InTarget.ScreenPoint, outerRadius, InTarget.Angle - QUARTER_TURN, InTarget.Angle + QUARTER_TURN, secondaryColor, InMyGeometry, OutDrawElements, InLayerId);
	if (InTarget.ZIndex < 4.0f)
	{
		const float innerRadius = (3.0f + InTarget.ZIndex) * slateScale;
		DrawFilledArc(InTarget.ScreenPoint, innerRadius, InTarget.Angle + QUARTER_TURN, InTarget.Angle + QUARTER_TURN * 3.0f, primaryColor, InMyGeometry, OutDrawElements, InLayerId + 1);
		DrawFilledArc(InTarget.ScreenPoint, innerRadius, InTarget.Angle - QUARTER_TURN, InTarget.Angle + QUARTER_TURN, secondaryColor, InMyGeometry, OutDrawElements, InLayerId + 1);
	}
	else
	{
		// If the circle is big enough, draw letters.
		const FString primaryLetter = UVectorFuncs4D::Vector4_AxisLetter((EAxis4D)InTarget.PrimaryAxisNumber);
		const FString secondaryLetter = UVectorFuncs4D::Vector4_AxisLetter((EAxis4D)InTarget.SecondaryAxisNumber);
		const FSlateFontInfo font = FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), 10);
		const TSharedRef<FSlateFontMeasure> fontMeasure = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
		const FVector2f primaryTextSize = fontMeasure->Measure(FText::FromString(primaryLetter), font);
		const FVector2f secondaryTextSize = fontMeasure->Measure(FText::FromString(secondaryLetter), font);
		const FVector2f primaryTextOffset = FVector2f(-5.5f * slateScale - 0.5f * primaryTextSize.X, -primaryTextSize.Y * 0.5f);
		const FVector2f secondaryTextOffset = FVector2f(5.5f * slateScale - 0.5f * secondaryTextSize.X, -secondaryTextSize.Y * 0.5f);
		FSlateDrawElement::MakeText(OutDrawElements, InLayerId + 1, MakePaintGeometry(InMyGeometry, InTarget.ScreenPoint + primaryTextOffset, primaryTextSize), FText::FromString(primaryLetter), font, ESlateDrawEffect::None, FLinearColor(0.0f, 0.0f, 0.0f, 0.6f));
		FSlateDrawElement::MakeText(OutDrawElements, InLayerId + 1, MakePaintGeometry(InMyGeometry, InTarget.ScreenPoint + secondaryTextOffset, secondaryTextSize), FText::FromString(secondaryLetter), font, ESlateDrawEffect::None, FLinearColor(0.0f, 0.0f, 0.0f, 0.6f));
	}
}

FUnrealViewportRotationHitTarget4D SEditorViewportRotationWidget4D::MakePlaneTarget(const FMatrix &InBasis, const int InRight, const int InUp, const FVector2f& InCenter, const float InRadius)
{
	FUnrealViewportRotationHitTarget4D ret;
	ret.HitType = EEditorViewportRotationHitType4D::Plane;
	ret.PrimaryAxisNumber = InRight;
	ret.SecondaryAxisNumber = InUp;
	const FVector4 rightVec4 = UBasis4D::Basis4D_GetRow(InBasis, InRight);
	const FVector4 upVec4 = UBasis4D::Basis4D_GetRow(InBasis, InUp);
	const FVector4 axis4D = (rightVec4 + upVec4) * 0.70710678118654752440084436210485;
	ret.ScreenPoint = InCenter + FVector2f((float)axis4D.Y, -(float)axis4D.Z) * InRadius;
	// NOTE: The vectors are swapped compared to Godot 4D. I'm not quite sure why, but it has to be this way in Unreal.
	ret.Angle = UMath4D::Vector2_AngleToPoint(FVector2f((float)upVec4.Y, -(float)upVec4.Z), FVector2f((float)rightVec4.Y, -(float)rightVec4.Z));
	ret.ZIndex = -(axis4D.X + UNREAL4D_COMPARISON_EPSILON);
	return ret;
}

void SEditorViewportRotationWidget4D::GetSortedAxisScreenAligned(const FMatrix& InBasis, const FVector2f& InCenter, const float InRadius, const int8 InRightIndex, const int8 InUpIndex, const int8 InPerpRightIndex, const int8 InPerpUpIndex, TArray<FUnrealViewportRotationHitTarget4D>& OutTargets) const
{
	// TODO: Check Unreal's screen-space coordinates.
	const FVector2f axisRightOffset = InRadius * FVector2f((float)InBasis.M[InRightIndex][1], -(float)InBasis.M[InRightIndex][2]);
	const FVector2f axisUpOffset = InRadius * FVector2f((float)InBasis.M[InUpIndex][1], -(float)InBasis.M[InUpIndex][2]);
	// Axis lines.
	FUnrealViewportRotationHitTarget4D axisRight;
	axisRight.PrimaryAxisNumber = InRightIndex;
	axisRight.ZIndex = -1.0f;
	axisRight.HitType = EEditorViewportRotationHitType4D::AxisLine;
	axisRight.ScreenPoint = InCenter + axisRightOffset;
	OutTargets.Add(axisRight);
	FUnrealViewportRotationHitTarget4D axisUp;
	axisUp.PrimaryAxisNumber = InUpIndex;
	axisUp.ZIndex = -1.0f;
	axisUp.HitType = EEditorViewportRotationHitType4D::AxisLine;
	axisUp.ScreenPoint = InCenter + axisUpOffset;
	OutTargets.Add(axisUp);
	// Axis circles.
	axisRight.HitType = EEditorViewportRotationHitType4D::AxisCirclePositive;
	axisRight.ZIndex = 0.0f;
	OutTargets.Add(axisRight);
	axisRight.HitType = EEditorViewportRotationHitType4D::AxisCircleNegative;
	axisRight.ScreenPoint = InCenter - axisRightOffset;
	OutTargets.Add(axisRight);
	axisUp.HitType = EEditorViewportRotationHitType4D::AxisCirclePositive;
	axisUp.ZIndex = 0.0f;
	OutTargets.Add(axisUp);
	axisUp.HitType = EEditorViewportRotationHitType4D::AxisCircleNegative;
	axisUp.ScreenPoint = InCenter - axisUpOffset;
	OutTargets.Add(axisUp);
	// Parallel direction.
	FUnrealViewportRotationHitTarget4D axisParallel;
	axisParallel.HitType = EEditorViewportRotationHitType4D::Plane;
	axisParallel.PrimaryAxisNumber = InRightIndex;
	axisParallel.SecondaryAxisNumber = InUpIndex;
	axisParallel.ZIndex = 1.0f;
	axisParallel.Angle = UMath4D::Vector2_AngleToPoint(axisUpOffset, axisRightOffset);
	const FVector4 axisParallel4D = (UBasis4D::Basis4D_GetRow(InBasis, InRightIndex) + UBasis4D::Basis4D_GetRow(InBasis, InUpIndex)).GetSafeNormal();
	axisParallel.ScreenPoint = InCenter + InRadius * FVector2f((float)axisParallel4D.Y, -(float)axisParallel4D.Z);
	OutTargets.Add(axisParallel);
	// Perpendicular direction.
	FUnrealViewportRotationHitTarget4D axisPerp;
	axisPerp.HitType = EEditorViewportRotationHitType4D::Plane;
	axisPerp.PrimaryAxisNumber = InPerpRightIndex;
	axisPerp.SecondaryAxisNumber = InPerpUpIndex;
	axisPerp.ScreenPoint = InCenter;
	axisPerp.ZIndex = 8.0f;
	OutTargets.Add(axisPerp);
}

void SEditorViewportRotationWidget4D::GetSortedAxisTargets(const FVector2f& InCenter, TArray<FUnrealViewportRotationHitTarget4D>& OutTargets) const
{
	// Explicit early returning here is necessary to avoid crashes when
	// viewports are closed/destroyed, an ensure alone is not enough.
	if (_EditorMode4D == nullptr || _EditorViewportClient == nullptr)
	{
		ensure(false);
		return;
	}
	const FMatrix cameraBasisTransposed = _EditorMode4D->GetFullEditorCameraTransform4D(_EditorViewportClient).Basis.GetTransposed();
	const float radius = InCenter.X - 10.0f * GetSlateScale();
	// Special cases: Axes are aligned with the screen. NOTE: UNREAL4D_COMPARISON_EPSILON is too small for this use case.
	constexpr double SPECIAL_CASE_THRESHOLD = UE_KINDA_SMALL_NUMBER;
	// NOTE: Unreal's FMatrix uses row-major order, but also stores the basis vectors as row vectors.
	// These differences cancel out here, making the access pattern the same as in Godot 4D, but with numbers, and Y/Z instead of X/Y.
	if (FMath::Abs(cameraBasisTransposed.M[0][1]) < SPECIAL_CASE_THRESHOLD && FMath::Abs(cameraBasisTransposed.M[0][2]) < SPECIAL_CASE_THRESHOLD) {
		if (FMath::Abs(cameraBasisTransposed.M[1][1]) < SPECIAL_CASE_THRESHOLD && FMath::Abs(cameraBasisTransposed.M[1][2]) < SPECIAL_CASE_THRESHOLD) {
			// NOTE: The order of each pair of numbers here needs to match the calls to MakePlaneTarget below.
			GetSortedAxisScreenAligned(cameraBasisTransposed, InCenter, radius, 3, 2, 1, 0, OutTargets);
			return;
		} else if (FMath::Abs(cameraBasisTransposed.M[2][1]) < SPECIAL_CASE_THRESHOLD && FMath::Abs(cameraBasisTransposed.M[2][2]) < SPECIAL_CASE_THRESHOLD) {
			GetSortedAxisScreenAligned(cameraBasisTransposed, InCenter, radius, 1, 3, 0, 2, OutTargets);
			return;
		} else if (FMath::Abs(cameraBasisTransposed.M[3][1]) < SPECIAL_CASE_THRESHOLD && FMath::Abs(cameraBasisTransposed.M[3][2]) < SPECIAL_CASE_THRESHOLD) {
			GetSortedAxisScreenAligned(cameraBasisTransposed, InCenter, radius, 1, 2, 0, 3, OutTargets);
			return;
		}
	} else if (FMath::Abs(cameraBasisTransposed.M[1][1]) < SPECIAL_CASE_THRESHOLD && FMath::Abs(cameraBasisTransposed.M[1][2]) < SPECIAL_CASE_THRESHOLD) {
		if (FMath::Abs(cameraBasisTransposed.M[2][1]) < SPECIAL_CASE_THRESHOLD && FMath::Abs(cameraBasisTransposed.M[2][2]) < SPECIAL_CASE_THRESHOLD) {
			GetSortedAxisScreenAligned(cameraBasisTransposed, InCenter, radius, 0, 3, 1, 2, OutTargets);
			return;
		} else if (FMath::Abs(cameraBasisTransposed.M[3][1]) < SPECIAL_CASE_THRESHOLD && FMath::Abs(cameraBasisTransposed.M[3][2]) < SPECIAL_CASE_THRESHOLD) {
			GetSortedAxisScreenAligned(cameraBasisTransposed, InCenter, radius, 0, 2, 1, 3, OutTargets);
			return;
		}
	} else if (FMath::Abs(cameraBasisTransposed.M[2][1]) < SPECIAL_CASE_THRESHOLD && FMath::Abs(cameraBasisTransposed.M[2][2]) < SPECIAL_CASE_THRESHOLD) {
		if (FMath::Abs(cameraBasisTransposed.M[3][1]) < SPECIAL_CASE_THRESHOLD && FMath::Abs(cameraBasisTransposed.M[3][2]) < SPECIAL_CASE_THRESHOLD) {
			GetSortedAxisScreenAligned(cameraBasisTransposed, InCenter, radius, 1, 0, 3, 2, OutTargets);
			return;
		}
	}
	// Add axes in each direction.
	for (int i = 0; i < 4; i++) {
		const FVector4 axis4D = UBasis4D::Basis4D_GetRow(cameraBasisTransposed, i);
		const FVector2f axisScreenPosition = FVector2f((float)axis4D.Y, -(float)axis4D.Z) * radius;
		const double axisDepth = axis4D.X; // Unreal uses X as forward / into the screen depth.

		if (axisScreenPosition.IsNearlyZero(UNREAL4D_COMPARISON_EPSILON)) {
			// Special case when the axis is aligned with the camera.
			FUnrealViewportRotationHitTarget4D axis;
			axis.HitType = EEditorViewportRotationHitType4D::AxisCirclePositive;
			axis.PrimaryAxisNumber = i;
			axis.ScreenPoint = InCenter;
			axis.ZIndex = -axisDepth;
			OutTargets.Add(axis);
		} else {
			FUnrealViewportRotationHitTarget4D posAxis;
			posAxis.HitType = EEditorViewportRotationHitType4D::AxisCirclePositive;
			posAxis.PrimaryAxisNumber = i;
			posAxis.ScreenPoint = InCenter + axisScreenPosition;
			posAxis.ZIndex = -axisDepth; // Negative depth to indicate a closer positive side.
			OutTargets.Add(posAxis);

			FUnrealViewportRotationHitTarget4D lineAxis;
			lineAxis.HitType = EEditorViewportRotationHitType4D::AxisLine;
			lineAxis.PrimaryAxisNumber = i;
			lineAxis.ScreenPoint = InCenter + axisScreenPosition;
			// Ensure the lines draw behind their connected circles.
			lineAxis.ZIndex = FMath::Min(-axisDepth, 0.0) - (float)UNREAL4D_COMPARISON_EPSILON;
			OutTargets.Add(lineAxis);

			FUnrealViewportRotationHitTarget4D negAxis;
			negAxis.HitType = EEditorViewportRotationHitType4D::AxisCircleNegative;
			negAxis.PrimaryAxisNumber = i;
			negAxis.ScreenPoint = InCenter - axisScreenPosition;
			negAxis.ZIndex = axisDepth; // Positive depth to indicate a closer negative side.
			OutTargets.Add(negAxis);
		}
	}
	// Add orthogonal planes. The order of axes follows Unreal's conventions.
	OutTargets.Add(MakePlaneTarget(cameraBasisTransposed, 1, 0, InCenter, radius));
	OutTargets.Add(MakePlaneTarget(cameraBasisTransposed, 1, 2, InCenter, radius));
	OutTargets.Add(MakePlaneTarget(cameraBasisTransposed, 0, 2, InCenter, radius));
	OutTargets.Add(MakePlaneTarget(cameraBasisTransposed, 0, 3, InCenter, radius));
	OutTargets.Add(MakePlaneTarget(cameraBasisTransposed, 1, 3, InCenter, radius));
	OutTargets.Add(MakePlaneTarget(cameraBasisTransposed, 3, 2, InCenter, radius));
	// Sort the axes by ZIndex.
	Algo::SortBy(OutTargets, &FUnrealViewportRotationHitTarget4D::ZIndex);
}

// Slate integration helpers.

float SEditorViewportRotationWidget4D::GetSlateScale()
{
	return FSlateApplication::IsInitialized() ? FSlateApplication::Get().GetApplicationScale() : 1.0f;
}

FPaintGeometry SEditorViewportRotationWidget4D::MakePaintGeometry(const FGeometry& Geometry)
{
	return Geometry.ToPaintGeometry(FVector2f(Geometry.GetLocalSize()), FSlateLayoutTransform());
}

FPaintGeometry SEditorViewportRotationWidget4D::MakePaintGeometry(const FGeometry& Geometry, const FVector2f& LocalPosition, const FVector2f& LocalSize)
{
	return Geometry.ToPaintGeometry(FVector2f(LocalSize), FSlateLayoutTransform(FVector2f(LocalPosition)));
}

// Construction.

void SEditorViewportRotationWidget4D::Construct(const FArguments& InArgs)
{
	_SlateViewport = InArgs._SlateViewport;
	SetCursor(EMouseCursor::Default);
}

void SEditorViewportRotationWidget4D::Setup(FEditorViewportClient* InEditorViewportClient, FUnreal4DEditorMode* InEditorMode4D)
{
	_EditorViewportClient = InEditorViewportClient;
	_EditorMode4D = InEditorMode4D;
}

// Slate integration virtual function overrides.

FVector2D SEditorViewportRotationWidget4D::ComputeDesiredSize(const float InLayoutScaleMultiplier) const
{
	const float size = _BASE_4D_ROTATION_WIDGET_SIZE * GetSlateScale();
	return FVector2D(size, size);
}

int32 SEditorViewportRotationWidget4D::OnPaint(const FPaintArgs& InArgs, const FGeometry& InAllottedGeometry, const FSlateRect& InMyCullingRect, FSlateWindowElementList& InOutDrawElements, int32 InLayerId, const FWidgetStyle& InWidgetStyle, bool bInParentEnabled) const
{
	const FVector2f center = InAllottedGeometry.GetLocalSize() / 2.0f;
	if (_IsMouseButtonDown || _FocusedTarget.HitType != EEditorViewportRotationHitType4D::None) {
		DrawCircle(center, center.X, FLinearColor(0.5f, 0.5f, 0.5f, 0.25f), InAllottedGeometry, InOutDrawElements, InLayerId);
	}
	TArray<FUnrealViewportRotationHitTarget4D> axisTargetsToDraw;
	GetSortedAxisTargets(center, axisTargetsToDraw);
	int32 currentLayer = InLayerId + 1;
	for (int i = 0; i < axisTargetsToDraw.Num(); ++i) {
		FUnrealViewportRotationHitTarget4D axisTarget = axisTargetsToDraw[i];
		if (axisTarget.HitType == EEditorViewportRotationHitType4D::AxisLine) {
			DrawAxisLine(axisTarget, center, InAllottedGeometry, InOutDrawElements, currentLayer);
		} else if (axisTarget.HitType == EEditorViewportRotationHitType4D::Plane) {
			DrawPlaneSemicircles(axisTarget, InAllottedGeometry, InOutDrawElements, currentLayer);
		} else {
			ensure(axisTarget.HitType == EEditorViewportRotationHitType4D::AxisCirclePositive || axisTarget.HitType == EEditorViewportRotationHitType4D::AxisCircleNegative);
			DrawAxisCircle(axisTarget, InAllottedGeometry, InOutDrawElements, currentLayer);
		}
		currentLayer += 2;
	}
	return currentLayer;
}

FReply SEditorViewportRotationWidget4D::OnMouseButtonDown(const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() != EKeys::LeftMouseButton)
	{
		return FReply::Unhandled();
	}
	_IsMouseButtonDown = true;
	_IsMouseOrbiting = false;
	_DragStartPosition = InMouseEvent.GetScreenSpacePosition();
	// Don't actually hide yet, see OnMouseMove.
	return FReply::Handled().CaptureMouse(SharedThis(this));
}

FReply SEditorViewportRotationWidget4D::OnMouseButtonUp(const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() != EKeys::LeftMouseButton)
	{
		return FReply::Unhandled();
	}
	// If the mouse being down did not result in an orbit,
	// then it could be a click on a part of the widget.
	if (_IsMouseButtonDown && !_IsMouseOrbiting)
	{
		switch (_FocusedTarget.HitType)
		{
			case EEditorViewportRotationHitType4D::None:
			case EEditorViewportRotationHitType4D::Background:
			{
				// Do nothing for None and Background.
			} break;
			case EEditorViewportRotationHitType4D::AxisCirclePositive:
			case EEditorViewportRotationHitType4D::AxisCircleNegative:
			case EEditorViewportRotationHitType4D::AxisLine:
			{
				// When clicking on any kind of axis, view a slice perpendicular to that axis.
				FMatrix rotationBasis4D = UBasis4D::Basis4D_FromXY(-2.0);
				if (_FocusedTarget.PrimaryAxisNumber == 0)
				{
					rotationBasis4D = UBasis4D::Basis4D_Compose(UBasis4D::Basis4D_FromXW(UNREAL4D_TAU / 4.0), rotationBasis4D);
				}
				else if (_FocusedTarget.PrimaryAxisNumber == 1)
				{
					rotationBasis4D = UBasis4D::Basis4D_Compose(UBasis4D::Basis4D_FromWY(UNREAL4D_TAU / -4.0), rotationBasis4D);
				}
				else if (_FocusedTarget.PrimaryAxisNumber == 2)
				{
					rotationBasis4D = UBasis4D::Basis4D_Compose(UBasis4D::Basis4D_FromZW(UNREAL4D_TAU / 4.0), rotationBasis4D);
				}
				// No else block for 3 or -1, the starting value from Basis4D_FromXY is already good.
				_EditorMode4D->SetEditorCameraRotation4Dand3D(_EditorViewportClient, rotationBasis4D, FMath::RadiansToDegrees(-2.0), FMath::RadiansToDegrees(-0.5));
			} break;
			case EEditorViewportRotationHitType4D::Plane:
			{
				// When clicking on a plane, view parallel to that plane.
				FMatrix rotationBasis4D = FMatrix::Identity;
				double yawDegrees3D = 0.0;
				double pitchDegrees3D = 0.0;
				// Here, only bother handling the cases that this widget generates (see calls to MakePlaneTarget and GetSortedAxisScreenAligned).
				if (_FocusedTarget.PrimaryAxisNumber == 0)
				{
					// Bring X to the right for all of these cases.
					rotationBasis4D = UBasis4D::Basis4D_FromXY(-UNREAL4D_TAU / 4.0);
					yawDegrees3D = -90.0;
					if (_FocusedTarget.SecondaryAxisNumber == 3)
					{
						// Bring W to the up direction.
						rotationBasis4D = UBasis4D::Basis4D_Compose(UBasis4D::Basis4D_FromZW(UNREAL4D_TAU / 4.0), rotationBasis4D);
					}
				}
				else if (_FocusedTarget.PrimaryAxisNumber == 1) // Most cases, because Unreal Y points to the right.
				{
					if (_FocusedTarget.SecondaryAxisNumber == 0)
					{
						pitchDegrees3D = -90.0; // Tilt the camera down to bring X to the up direction.
					}
					else if (_FocusedTarget.SecondaryAxisNumber == 2)
					{
						// Identity case: Do nothing, use the identity values.
					}
					else
					{
						ensure(_FocusedTarget.SecondaryAxisNumber == 3);
						rotationBasis4D = UBasis4D::Basis4D_FromZW(UNREAL4D_TAU / 4.0); // Bring W to the up direction.
					}
				}
				else if (_FocusedTarget.PrimaryAxisNumber == 3)
				{
					ensure(_FocusedTarget.SecondaryAxisNumber == 2); // Only case.
					rotationBasis4D = UBasis4D::Basis4D_FromWY(-UNREAL4D_TAU / 4.0); // Bring W to the right direction.
				}
				_EditorMode4D->SetEditorCameraRotation4Dand3D(_EditorViewportClient, rotationBasis4D, yawDegrees3D, pitchDegrees3D);
			} break;
		}
	}
	// Regardless of the existing state, reset all of this on mouse up.
	_IsMouseButtonDown = false;
	_IsMouseOrbiting = false;
	_FocusedTarget = FUnrealViewportRotationHitTarget4D();
	FSlateApplication::Get().GetPlatformApplication()->Cursor->Show(true);
	return FReply::Handled().ReleaseMouseCapture();
}

FReply SEditorViewportRotationWidget4D::OnMouseMove(const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent)
{
	if (_IsMouseButtonDown)
	{
		// Only orbit the mouse after at least some movement occurs after the button is pressed.
		if (!_IsMouseOrbiting)
		{
			_IsMouseOrbiting = true;
			// The mouse cursor jitters when shown, so we need to hide it.
			FSlateApplication::Get().GetPlatformApplication()->Cursor->Show(false);
		}
		// Due to the warping, `InMouseEvent.GetCursorDelta()` does not work, but this manual calculation works.
		FVector2f currentPos = InMouseEvent.GetScreenSpacePosition();
		FVector2f movement = currentPos - _DragStartPosition;
		FMatrix rotationDelta = UBasis4D::Basis4D_Compose(
			UBasis4D::Basis4D_FromWY(movement.X * 0.008),
			UBasis4D::Basis4D_FromXW(movement.Y * -0.008));
		_EditorMode4D->RotateEditorCameraLocal4D(_EditorViewportClient, rotationDelta);
		// Invalidate the viewport client to trigger a redraw with the updated camera transform.
		if (_EditorViewportClient)
		{
			_EditorViewportClient->Invalidate();
		}
		FSlateApplication::Get().GetPlatformApplication()->Cursor->SetPosition(_DragStartPosition.X, _DragStartPosition.Y);
		return FReply::Handled();
	}
	// Mouse is not down, so we can update the focus target.
	const FVector2f center = InMyGeometry.GetLocalSize() / 2.0f;
	const FVector2f localMousePosition = InMyGeometry.AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition());
	const float hitDistance = 8.0f * GetSlateScale();
	const EEditorViewportRotationHitType4D originalHitType = _FocusedTarget.HitType;
	const int8 originalPrimaryFocus = _FocusedTarget.PrimaryAxisNumber;
	const int8 originalSecondaryFocus = _FocusedTarget.SecondaryAxisNumber;
	_FocusedTarget = FUnrealViewportRotationHitTarget4D();
	_FocusedTarget.ZIndex = -10.0f;
	if (FVector2f::Distance(localMousePosition, center) < center.X)
	{
		_FocusedTarget.HitType = EEditorViewportRotationHitType4D::Background;
	}
	TArray<FUnrealViewportRotationHitTarget4D> axisTargets;
	GetSortedAxisTargets(center, axisTargets);
	for (int i = 0; i < axisTargets.Num(); ++i)
	{
		const FUnrealViewportRotationHitTarget4D& target = axisTargets[i];
		if (target.ZIndex > _FocusedTarget.ZIndex && FVector2f::Distance(localMousePosition, target.ScreenPoint) < hitDistance)
		{
			_FocusedTarget = target;
		}
	}
	if (_FocusedTarget.HitType != originalHitType || _FocusedTarget.PrimaryAxisNumber != originalPrimaryFocus || _FocusedTarget.SecondaryAxisNumber != originalSecondaryFocus)
	{
		Invalidate(EInvalidateWidgetReason::Paint);
	}
	return FReply::Unhandled();
}

void SEditorViewportRotationWidget4D::OnMouseLeave(const FPointerEvent& InMouseEvent)
{
	_IsMouseButtonDown = false;
	_IsMouseOrbiting = false;
	_FocusedTarget = FUnrealViewportRotationHitTarget4D();
	FSlateApplication::Get().GetPlatformApplication()->Cursor->Show(true);
	Invalidate(EInvalidateWidgetReason::Paint);
}

FCursorReply SEditorViewportRotationWidget4D::OnCursorQuery(const FGeometry& InMyGeometry, const FPointerEvent& InCursorEvent) const
{
	if (_IsMouseOrbiting)
	{
		return FCursorReply::Cursor(EMouseCursor::GrabHandClosed);
	}
	return FCursorReply::Unhandled();
}
