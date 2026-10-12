#include "EditorTransformGizmo4D.h"

#include "../../../Unreal4DCore/Math/Geometry4D.h"
#include "../../../Unreal4DCore/Math/Plane4D.h"
#include "../../../Unreal4DCore/Math/VectorFuncs4D.h"
#include "../../Unreal4DEditorMode.h"
#include "Meshes/EditorTransformGizmoActor4D.h"

#include "BaseGizmos/GizmoViewContext.h"
#include "BaseGizmos/GizmoRenderingUtil.h"
#include "BaseBehaviors/ClickDragBehavior.h"
#include "BaseBehaviors/MouseHoverBehavior.h"
#include "Components/StaticMeshComponent.h"
#include "ContextObjectStore.h"
#include "Editor/UnrealEdEngine.h"
#include "EditorModeManager.h"
#include "EditorViewportClient.h"
#include "Framework/Application/SlateApplication.h"
#include "InteractiveGizmoManager.h"
#include "InputState.h"
#include "Settings/LevelEditorViewportSettings.h"
#include "UnrealEdGlobals.h"

TWeakObjectPtr<UEditorTransformGizmo4D> UEditorTransformGizmo4D::_activeGizmo = nullptr;

void UEditorTransformGizmo4D::HandleEditorViewportSettingsChanged(UObject* InObject, FPropertyChangedEvent& InPropertyChangedEvent)
{
	if (InObject == GetMutableDefault<ULevelEditorViewportSettings>())
	{
		ReadFromEditorViewportSettings();
	}
}

void UEditorTransformGizmo4D::HandleCoordSystemChanged(ECoordSystem InCoordSystem)
{
	if (_transformProxy4D != nullptr)
	{
		_transformProxy4D->SetCoordSystem(InCoordSystem);
	}
}

void UEditorTransformGizmo4D::ReadFromEditorViewportSettings()
{
	if (_transformProxy4D == nullptr)
	{
		return;
	}
	const ULevelEditorViewportSettings* viewportSettings = GetDefault<ULevelEditorViewportSettings>();
	if (!viewportSettings)
	{
		return;
	}
	const bool bPreserveNonUniformScale = viewportSettings->PreserveNonUniformScale;
	const EEditorTransform4DKeepMode keepMode = bPreserveNonUniformScale ? EEditorTransform4DKeepMode::ConformalChangesOnly : EEditorTransform4DKeepMode::Freeform;
	_keepMode = keepMode;
	_transformProxy4D->SetKeepMode(keepMode);
}

FVector4 UEditorTransformGizmo4D::OriginAxisAlignedBivectorRingRaycast(const FVector4& InRayOrigin, const FVector4& InRayDirection, const FVector4& InAxis1, const FVector4& InAxis2, const FVector4& InPerp)
{
	FVector4 planeNormal = UVectorFuncs4D::Vector4_Perpendicular(InAxis1, InAxis2, InPerp);
	if (planeNormal.SizeSquared() < UNREAL4D_COMPARISON_EPSILON) {
		return FVector4::Zero();
	}
	planeNormal = UVectorFuncs4D::Vector4_Normalized(planeNormal);
	const double denominator = Dot4(planeNormal, InRayDirection);
	if (FMath::IsNearlyZero(denominator)) {
		return FVector4::Zero();
	}
	const double factor = -Dot4(planeNormal, InRayOrigin) / denominator;
	if (factor < 0.0) {
		return FVector4::Zero(); // Gizmo is behind the camera.
	}
	const FVector4 pointOnPlane = InRayOrigin + InRayDirection * factor;
	// The visible ring is an ellipse due to foreshortening along InPerp.
	// To unsquash: solve the 2x2 Gram matrix system for the slid (projected) axes.
	// This handles cross-coupling when InPerp has components along both ring axes.
	const double axis1DotPerp = Dot4(InAxis1, InPerp);
	const double axis2DotPerp = Dot4(InAxis2, InPerp);
	const double det = 1.0 - axis1DotPerp * axis1DotPerp - axis2DotPerp * axis2DotPerp;
	if (FMath::IsNearlyZero(det))
	{
		return FVector4::Zero(); // Ring is nearly edge-on to the camera.
	}
	// Project the hit point onto each slid axis to get its apparent (foreshortened) coordinates.
	const double pointOnPlaneDotPerp = Dot4(pointOnPlane, InPerp);
	const double apparentAxis1 = Dot4(pointOnPlane, InAxis1) - pointOnPlaneDotPerp * axis1DotPerp;
	const double apparentAxis2 = Dot4(pointOnPlane, InAxis2) - pointOnPlaneDotPerp * axis2DotPerp;
	// Solve the 2x2 system via Cramer's rule.
	const double projAxis1 = (apparentAxis1 * (1.0 - axis2DotPerp * axis2DotPerp) + apparentAxis2 * axis1DotPerp * axis2DotPerp) / det;
	const double projAxis2 = (apparentAxis2 * (1.0 - axis1DotPerp * axis1DotPerp) + apparentAxis1 * axis1DotPerp * axis2DotPerp) / det;
	return InAxis1 * projAxis1 + InAxis2 * projAxis2;
}

// The ellipse is parameterized by Cos(Angle) * InAxis1 + Sin(Angle) * InAxis2. The axes are not necessarily perpendicular or non-degenerate, and the ellipse's centre is always the origin.
// I'm not sure this algorithm is entirely correct if the closest point on the ray is its end, but that case is not very important for this application.
static double DistanceBetweenRayAndEllipse(const FVector4& InRayOrigin, const FVector4& InRayDirection, const FVector4& InAxis1, const FVector4& InAxis2)
{
	auto PointAtAngle = [&](const double Angle)
	{
		return InAxis1 * FMath::Cos(Angle) + InAxis2 * FMath::Sin(Angle);
	};

	// d/dAngle (distance to ray)^2 /2
	auto Derivative = [&] (const double Angle)
	{
		const FVector4 EllipsePoint = PointAtAngle(Angle);
		const FVector4 RayPoint = UGeometry4D::ClosestPointOnRay(InRayOrigin, InRayDirection, EllipsePoint);
		const FVector4 DPoint = -InAxis1 * FMath::Sin(Angle) + InAxis2 * FMath::Cos(Angle);
		return Dot4(DPoint, (EllipsePoint - RayPoint));
		// The derivative of RayPoint with angle is always either 0 (if it's at the ray's end) or perpendicular to (EllipsePoint - RayPoint), so in either case it does not affect the derivative of the distance.
	};

	const FVector4 FlatAxis1 = UVectorFuncs4D::Vector4_Slide(InAxis1, InRayDirection);
	const FVector4 FlatAxis2 = UVectorFuncs4D::Vector4_Slide(InAxis2, InRayDirection);
	double MajorAxisAngle = 0.5 * FMath::Atan2(2 * Dot4(FlatAxis2, FlatAxis1), Dot4(FlatAxis1, FlatAxis1) - Dot4(FlatAxis2, FlatAxis2));
	if (!FMath::IsFinite(MajorAxisAngle))
	{
		// This indicates the ellipse's projection perpendicular to the ray is a circle.
		MajorAxisAngle = 0.0;
	}

	// Iteratively shrink the angle interval which contains the minimum.
	double End1;
	double End2;
	// Check what side of the ellipse the ray is on, and start the iteration at that half of the ellipse. It's possible for each side to contain a local minimum, so this check is needed to ensure we find the global minimum.
	if (Derivative(MajorAxisAngle) < 0)
	{
		End1 = MajorAxisAngle;
		End2 = MajorAxisAngle + UE_DOUBLE_PI;
	}
	else
	{
		End1 = MajorAxisAngle - UE_DOUBLE_PI;
		End2 = MajorAxisAngle;
	}
	double End1Derivative = Derivative(End1);
	double End2Derivative = Derivative(End2);
	// loop invariant: End1 < End2, End1Derivative <= 0, End2Derivative >= 0, and the global minimum is inside the interval (except some constraints may fail to hold if the nearest point is the ray's end).
	while (FMath::Abs(End1 - End2) > UE_KINDA_SMALL_NUMBER) // This limit could be much larger without causing problems, but the iteration should converge quickly, so a small limit shouldn't be a problem either.
	{
		double Mid;
		if ((End1Derivative > 0) == (End2Derivative > 0))
		{
			Mid = (End1 + End2)/2.0;
		}
		else
		{
			Mid = FMath::Lerp(End1, End2, FMath::Clamp(End1Derivative / (End1Derivative - End2Derivative), 0.1, 0.9));
		}

		const double MidDerivative = Derivative(Mid);
		if (MidDerivative < 0)
		{
			End1 = Mid;
			End1Derivative = MidDerivative;
		}
		else
		{
			End2 = Mid;
			End2Derivative = MidDerivative;
		}
	}

	const FVector4 EllipsePoint = PointAtAngle(End1);
	const FVector4 RayPoint = UGeometry4D::ClosestPointOnRay(InRayOrigin, InRayDirection, EllipsePoint);
	return (EllipsePoint - RayPoint).Size();
}

FString UEditorTransformGizmo4D::GizmoPartEnumToName(const EEditorTransformGizmo4DPart InPart)
{
	switch (InPart)
	{
		case EEditorTransformGizmo4DPart::None: return "None";
		case EEditorTransformGizmo4DPart::MoveScreenRelative: return "MoveScreenRelative";
		case EEditorTransformGizmo4DPart::MoveX: return "MoveX";
		case EEditorTransformGizmo4DPart::MoveY: return "MoveY";
		case EEditorTransformGizmo4DPart::MoveZ: return "MoveZ";
		case EEditorTransformGizmo4DPart::MoveW: return "MoveW";
		case EEditorTransformGizmo4DPart::MoveXY: return "MoveXY";
		case EEditorTransformGizmo4DPart::MoveXZ: return "MoveXZ";
		case EEditorTransformGizmo4DPart::MoveXW: return "MoveXW";
		case EEditorTransformGizmo4DPart::MoveYZ: return "MoveYZ";
		case EEditorTransformGizmo4DPart::MoveYW: return "MoveYW";
		case EEditorTransformGizmo4DPart::MoveZW: return "MoveZW";
		case EEditorTransformGizmo4DPart::RotateXY: return "RotateXY";
		case EEditorTransformGizmo4DPart::RotateXZ: return "RotateXZ";
		case EEditorTransformGizmo4DPart::RotateXW: return "RotateXW";
		case EEditorTransformGizmo4DPart::RotateYZ: return "RotateYZ";
		case EEditorTransformGizmo4DPart::RotateYW: return "RotateYW";
		case EEditorTransformGizmo4DPart::RotateZW: return "RotateZW";
		case EEditorTransformGizmo4DPart::ScaleUniform: return "ScaleUniform";
		case EEditorTransformGizmo4DPart::ScaleX: return "ScaleX";
		case EEditorTransformGizmo4DPart::ScaleY: return "ScaleY";
		case EEditorTransformGizmo4DPart::ScaleZ: return "ScaleZ";
		case EEditorTransformGizmo4DPart::ScaleW: return "ScaleW";
		case EEditorTransformGizmo4DPart::ScaleXY: return "ScaleXY";
		case EEditorTransformGizmo4DPart::ScaleXZ: return "ScaleXZ";
		case EEditorTransformGizmo4DPart::ScaleXW: return "ScaleXW";
		case EEditorTransformGizmo4DPart::ScaleYZ: return "ScaleYZ";
		case EEditorTransformGizmo4DPart::ScaleYW: return "ScaleYW";
		case EEditorTransformGizmo4DPart::ScaleZW: return "ScaleZW";
		case EEditorTransformGizmo4DPart::StretchPosX: return "StretchPosX";
		case EEditorTransformGizmo4DPart::StretchNegX: return "StretchNegX";
		case EEditorTransformGizmo4DPart::StretchPosY: return "StretchPosY";
		case EEditorTransformGizmo4DPart::StretchNegY: return "StretchNegY";
		case EEditorTransformGizmo4DPart::StretchPosZ: return "StretchPosZ";
		case EEditorTransformGizmo4DPart::StretchNegZ: return "StretchNegZ";
		case EEditorTransformGizmo4DPart::StretchPosW: return "StretchPosW";
		case EEditorTransformGizmo4DPart::StretchNegW: return "StretchNegW";
		default: return "Unknown";
	}
}

bool UEditorTransformGizmo4D::IsGizmoPartForMove(const EEditorTransformGizmo4DPart InPart)
{
	switch (InPart)
	{
		case EEditorTransformGizmo4DPart::MoveScreenRelative:
		case EEditorTransformGizmo4DPart::MoveX:
		case EEditorTransformGizmo4DPart::MoveY:
		case EEditorTransformGizmo4DPart::MoveZ:
		case EEditorTransformGizmo4DPart::MoveW:
		case EEditorTransformGizmo4DPart::MoveXY:
		case EEditorTransformGizmo4DPart::MoveXZ:
		case EEditorTransformGizmo4DPart::MoveXW:
		case EEditorTransformGizmo4DPart::MoveYZ:
		case EEditorTransformGizmo4DPart::MoveYW:
		case EEditorTransformGizmo4DPart::MoveZW:
			return true;
		default:
			return false;
	}
}

FVector4 UEditorTransformGizmo4D::GetTransformRaycastValue(const FVector4 &InLocalRayOrigin, const FVector4 &InLocalRayDirection, const FMatrix& InLocalCameraMatrix) const
{
	const FVector4 CameraDirection = UBasis4D::Basis4D_GetRow(InLocalCameraMatrix, 0);
	const FVector4 PerpDirection = UBasis4D::Basis4D_GetRow(InLocalCameraMatrix, 3);
	switch (_currentTransformation)
	{
		case EEditorTransformGizmo4DPart::None:
			return FVector4::Zero();
		case EEditorTransformGizmo4DPart::MoveScreenRelative:
		case EEditorTransformGizmo4DPart::ScaleUniform:
		case EEditorTransformGizmo4DPart::RotateXY:
		case EEditorTransformGizmo4DPart::RotateXZ:
		case EEditorTransformGizmo4DPart::RotateXW:
		case EEditorTransformGizmo4DPart::RotateYZ:
		case EEditorTransformGizmo4DPart::RotateYW:
		case EEditorTransformGizmo4DPart::RotateZW:
			return FHyperplane4D(CameraDirection, 0.0).IntersectRayPoint(InLocalRayOrigin, InLocalRayDirection);
		case EEditorTransformGizmo4DPart::MoveX:
		case EEditorTransformGizmo4DPart::ScaleX:
		case EEditorTransformGizmo4DPart::StretchPosX:
		case EEditorTransformGizmo4DPart::StretchNegX:
			return UGeometry4D::ClosestPointBetweenLines(FVector4::Zero(), FVector4(1, 0, 0, 0), InLocalRayOrigin, InLocalRayDirection);
		case EEditorTransformGizmo4DPart::MoveY:
		case EEditorTransformGizmo4DPart::ScaleY:
		case EEditorTransformGizmo4DPart::StretchPosY:
		case EEditorTransformGizmo4DPart::StretchNegY:
			return UGeometry4D::ClosestPointBetweenLines(FVector4::Zero(), FVector4(0, 1, 0, 0), InLocalRayOrigin, InLocalRayDirection);
		case EEditorTransformGizmo4DPart::MoveZ:
		case EEditorTransformGizmo4DPart::ScaleZ:
		case EEditorTransformGizmo4DPart::StretchPosZ:
		case EEditorTransformGizmo4DPart::StretchNegZ:
			return UGeometry4D::ClosestPointBetweenLines(FVector4::Zero(), FVector4(0, 0, 1, 0), InLocalRayOrigin, InLocalRayDirection);
		case EEditorTransformGizmo4DPart::MoveW:
		case EEditorTransformGizmo4DPart::ScaleW:
		case EEditorTransformGizmo4DPart::StretchPosW:
		case EEditorTransformGizmo4DPart::StretchNegW:
			return UGeometry4D::ClosestPointBetweenLines(FVector4::Zero(), FVector4(0, 0, 0, 1), InLocalRayOrigin, InLocalRayDirection);
		case EEditorTransformGizmo4DPart::MoveXY:
		case EEditorTransformGizmo4DPart::ScaleXY:
			return OriginAxisAlignedBivectorRingRaycast(InLocalRayOrigin, InLocalRayDirection, FVector4(1, 0, 0, 0), FVector4(0, 1, 0, 0), PerpDirection);
		case EEditorTransformGizmo4DPart::MoveXZ:
		case EEditorTransformGizmo4DPart::ScaleXZ:
			return OriginAxisAlignedBivectorRingRaycast(InLocalRayOrigin, InLocalRayDirection, FVector4(1, 0, 0, 0), FVector4(0, 0, 1, 0), PerpDirection);
		case EEditorTransformGizmo4DPart::MoveXW:
		case EEditorTransformGizmo4DPart::ScaleXW:
			return OriginAxisAlignedBivectorRingRaycast(InLocalRayOrigin, InLocalRayDirection, FVector4(1, 0, 0, 0), FVector4(0, 0, 0, 1), PerpDirection);
		case EEditorTransformGizmo4DPart::MoveYZ:
		case EEditorTransformGizmo4DPart::ScaleYZ:
			return OriginAxisAlignedBivectorRingRaycast(InLocalRayOrigin, InLocalRayDirection, FVector4(0, 1, 0, 0), FVector4(0, 0, 1, 0), PerpDirection);
		case EEditorTransformGizmo4DPart::MoveYW:
		case EEditorTransformGizmo4DPart::ScaleYW:
			return OriginAxisAlignedBivectorRingRaycast(InLocalRayOrigin, InLocalRayDirection, FVector4(0, 1, 0, 0), FVector4(0, 0, 0, 1), PerpDirection);
		case EEditorTransformGizmo4DPart::MoveZW:
		case EEditorTransformGizmo4DPart::ScaleZW:
			return OriginAxisAlignedBivectorRingRaycast(InLocalRayOrigin, InLocalRayDirection, FVector4(0, 0, 1, 0), FVector4(0, 0, 0, 1), PerpDirection);
	}
	return FVector4::Zero();
}

double UEditorTransformGizmo4D::ValidateScale(const double InScale)
{
	// The threshold of this check needs to be bigger/looser than other areas,
	// so use 1e-4 (UE_KINDA_SMALL_NUMBER instead of 1e-6 (UNREAL4D_COMPARISON_EPSILON).
	if (FMath::IsNearlyZero(InScale, UE_KINDA_SMALL_NUMBER) || !FMath::IsFinite(InScale))
	{
		return UE_KINDA_SMALL_NUMBER;
	}
	return InScale;
}

FTransform4D UEditorTransformGizmo4D::GetLocalTransformChange(const FVector4& InLocalRayOrigin, const FVector4& InLocalRayDirection, const FMatrix& InLocalCameraMatrix) const
{
	FTransform4D transformChange = FTransform4D::Identity();
	const FVector4 casted = GetTransformRaycastValue(InLocalRayOrigin, InLocalRayDirection, InLocalCameraMatrix);
	if (casted.ContainsNaN())
	{
		return transformChange;
	}
	auto GetAngle = [&](FVector4 Axis1, FVector4 Axis2) {
		const FVector4 cameraUp = UBasis4D::Basis4D_GetRow(InLocalCameraMatrix, 2);
		const FVector4 cameraSideways = UBasis4D::Basis4D_GetRow(InLocalCameraMatrix, 1);
		const double angle = FMath::Atan2(Dot4(cameraUp, casted), Dot4(cameraSideways, casted)) - FMath::Atan2(Dot4(cameraUp, _transformReferenceValue), Dot4(cameraSideways, _transformReferenceValue));
		if (Dot4(Axis1, cameraUp) * Dot4(Axis2, cameraSideways) < Dot4(Axis2, cameraUp) * Dot4(Axis1, cameraSideways)) {
			return angle;
		} else {
			return -angle;
		}
	};
	switch (_currentTransformation)
	{
		case EEditorTransformGizmo4DPart::None:
			break;
		case EEditorTransformGizmo4DPart::MoveScreenRelative:
		case EEditorTransformGizmo4DPart::MoveX:
		case EEditorTransformGizmo4DPart::MoveY:
		case EEditorTransformGizmo4DPart::MoveZ:
		case EEditorTransformGizmo4DPart::MoveW:
		case EEditorTransformGizmo4DPart::MoveXY:
		case EEditorTransformGizmo4DPart::MoveXZ:
		case EEditorTransformGizmo4DPart::MoveXW:
		case EEditorTransformGizmo4DPart::MoveYZ:
		case EEditorTransformGizmo4DPart::MoveYW:
		case EEditorTransformGizmo4DPart::MoveZW:
		{
			transformChange.Origin = (casted - _transformReferenceValue) * (_gizmoScale * UNREAL4D_EDITOR_TRANSFORM_GIZMO_EXTENTS);
		} break;
		case EEditorTransformGizmo4DPart::RotateXY:
		{
			transformChange.Basis = UBasis4D::Basis4D_FromXY(GetAngle(FVector4(1, 0, 0, 0), FVector4(0, 1, 0, 0)));
		} break;
		case EEditorTransformGizmo4DPart::RotateXZ:
		{
			transformChange.Basis = UBasis4D::Basis4D_FromZX(GetAngle(FVector4(0, 0, 1, 0), FVector4(1, 0, 0, 0)));
		} break;
		case EEditorTransformGizmo4DPart::RotateXW:
		{
			transformChange.Basis = UBasis4D::Basis4D_FromXW(GetAngle(FVector4(1, 0, 0, 0), FVector4(0, 0, 0, 1)));
		} break;
		case EEditorTransformGizmo4DPart::RotateYZ:
		{
			transformChange.Basis = UBasis4D::Basis4D_FromYZ(GetAngle(FVector4(0, 1, 0, 0), FVector4(0, 0, 1, 0)));
		} break;
		case EEditorTransformGizmo4DPart::RotateYW:
		{
			transformChange.Basis = UBasis4D::Basis4D_FromWY(GetAngle(FVector4(0, 0, 0, 1), FVector4(0, 1, 0, 0)));
		} break;
		case EEditorTransformGizmo4DPart::RotateZW:
		{
			transformChange.Basis = UBasis4D::Basis4D_FromZW(GetAngle(FVector4(0, 0, 1, 0), FVector4(0, 0, 0, 1)));
		} break;
		case EEditorTransformGizmo4DPart::ScaleUniform:
		{
			const double along = Dot4(casted, _transformReferenceValue) / _transformReferenceValue.SizeSquared();
			// The 0.05 factor is an arbitrary value that can be changed to adjust the sensitivity of the uniform scaling.
			const double uniformScale = FMath::Exp((along - 1.0) * 0.05);
			transformChange.Basis.M[0][0] = uniformScale;
			transformChange.Basis.M[1][1] = uniformScale;
			transformChange.Basis.M[2][2] = uniformScale;
			transformChange.Basis.M[3][3] = uniformScale;
		} break;
		case EEditorTransformGizmo4DPart::ScaleX:
			transformChange.Basis.M[0][0] = ValidateScale(casted.X / _transformReferenceValue.X);
			break;
		case EEditorTransformGizmo4DPart::ScaleY:
			transformChange.Basis.M[1][1] = ValidateScale(casted.Y / _transformReferenceValue.Y);
			break;
		case EEditorTransformGizmo4DPart::ScaleZ:
			transformChange.Basis.M[2][2] = ValidateScale(casted.Z / _transformReferenceValue.Z);
			break;
		case EEditorTransformGizmo4DPart::ScaleW:
			transformChange.Basis.M[3][3] = ValidateScale(casted.W / _transformReferenceValue.W);
			break;
		case EEditorTransformGizmo4DPart::ScaleXY:
			transformChange.Basis.M[0][0] = ValidateScale(casted.X / _transformReferenceValue.X);
			transformChange.Basis.M[1][1] = ValidateScale(casted.Y / _transformReferenceValue.Y);
			break;
		case EEditorTransformGizmo4DPart::ScaleXZ:
			transformChange.Basis.M[0][0] = ValidateScale(casted.X / _transformReferenceValue.X);
			transformChange.Basis.M[2][2] = ValidateScale(casted.Z / _transformReferenceValue.Z);
			break;
		case EEditorTransformGizmo4DPart::ScaleXW:
			transformChange.Basis.M[0][0] = ValidateScale(casted.X / _transformReferenceValue.X);
			transformChange.Basis.M[3][3] = ValidateScale(casted.W / _transformReferenceValue.W);
			break;
		case EEditorTransformGizmo4DPart::ScaleYZ:
			transformChange.Basis.M[1][1] = ValidateScale(casted.Y / _transformReferenceValue.Y);
			transformChange.Basis.M[2][2] = ValidateScale(casted.Z / _transformReferenceValue.Z);
			break;
		case EEditorTransformGizmo4DPart::ScaleYW:
			transformChange.Basis.M[1][1] = ValidateScale(casted.Y / _transformReferenceValue.Y);
			transformChange.Basis.M[3][3] = ValidateScale(casted.W / _transformReferenceValue.W);
			break;
		case EEditorTransformGizmo4DPart::ScaleZW:
			transformChange.Basis.M[2][2] = ValidateScale(casted.Z / _transformReferenceValue.Z);
			transformChange.Basis.M[3][3] = ValidateScale(casted.W / _transformReferenceValue.W);
			break;
		case EEditorTransformGizmo4DPart::StretchPosX:
		{
			const double stretch = casted.X * 0.5;
			transformChange.Basis.M[0][0] = ValidateScale(stretch + 0.5);
			transformChange.Origin.X = stretch - 0.5;
		} break;
		case EEditorTransformGizmo4DPart::StretchNegX:
		{
			const double stretch = casted.X * 0.5;
			transformChange.Basis.M[0][0] = ValidateScale(-stretch + 0.5);
			transformChange.Origin.X = stretch + 0.5;
		} break;
		case EEditorTransformGizmo4DPart::StretchPosY:
		{
			const double stretch = casted.Y * 0.5;
			transformChange.Basis.M[1][1] = ValidateScale(stretch + 0.5);
			transformChange.Origin.Y = stretch - 0.5;
		} break;
		case EEditorTransformGizmo4DPart::StretchNegY:
		{
			const double stretch = casted.Y * 0.5;
			transformChange.Basis.M[1][1] = ValidateScale(-stretch + 0.5);
			transformChange.Origin.Y = stretch + 0.5;
		} break;
		case EEditorTransformGizmo4DPart::StretchPosZ:
		{
			const double stretch = casted.Z * 0.5;
			transformChange.Basis.M[2][2] = ValidateScale(stretch + 0.5);
			transformChange.Origin.Z = stretch - 0.5;
		} break;
		case EEditorTransformGizmo4DPart::StretchNegZ:
		{
			const double stretch = casted.Z * 0.5;
			transformChange.Basis.M[2][2] = ValidateScale(-stretch + 0.5);
			transformChange.Origin.Z = stretch + 0.5;
		} break;
		case EEditorTransformGizmo4DPart::StretchPosW:
		{
			const double stretch = casted.W * 0.5;
			transformChange.Basis.M[3][3] = ValidateScale(stretch + 0.5);
			transformChange.Origin.W = stretch - 0.5;
		} break;
		case EEditorTransformGizmo4DPart::StretchNegW:
		{
			const double stretch = casted.W * 0.5;
			transformChange.Basis.M[3][3] = ValidateScale(-stretch + 0.5);
			transformChange.Origin.W = stretch + 0.5;
		} break;
	}
	if (_keepMode == EEditorTransform4DKeepMode::ConformalChangesOnly || _keepMode == EEditorTransform4DKeepMode::ConformalChangesAndResults)
	{
		switch (_currentTransformation)
		{
			case EEditorTransformGizmo4DPart::ScaleX:
			case EEditorTransformGizmo4DPart::StretchPosX:
			case EEditorTransformGizmo4DPart::StretchNegX:
				transformChange.Basis.M[1][1] = transformChange.Basis.M[0][0];
				transformChange.Basis.M[2][2] = transformChange.Basis.M[0][0];
				transformChange.Basis.M[3][3] = transformChange.Basis.M[0][0];
				break;
			case EEditorTransformGizmo4DPart::ScaleY:
			case EEditorTransformGizmo4DPart::StretchPosY:
			case EEditorTransformGizmo4DPart::StretchNegY:
				transformChange.Basis.M[0][0] = transformChange.Basis.M[1][1];
				transformChange.Basis.M[2][2] = transformChange.Basis.M[1][1];
				transformChange.Basis.M[3][3] = transformChange.Basis.M[1][1];
				break;
			case EEditorTransformGizmo4DPart::ScaleZ:
			case EEditorTransformGizmo4DPart::StretchPosZ:
			case EEditorTransformGizmo4DPart::StretchNegZ:
				transformChange.Basis.M[0][0] = transformChange.Basis.M[2][2];
				transformChange.Basis.M[1][1] = transformChange.Basis.M[2][2];
				transformChange.Basis.M[3][3] = transformChange.Basis.M[2][2];
				break;
			case EEditorTransformGizmo4DPart::ScaleW:
			case EEditorTransformGizmo4DPart::StretchPosW:
			case EEditorTransformGizmo4DPart::StretchNegW:
				transformChange.Basis.M[0][0] = transformChange.Basis.M[3][3];
				transformChange.Basis.M[1][1] = transformChange.Basis.M[3][3];
				transformChange.Basis.M[2][2] = transformChange.Basis.M[3][3];
				break;
			default:
				// Do nothing. Move and rotate operations preserve conformality,
				// and planar scales should not be displayed as an option when in conformal mode.
				// Only linear scales need to have their affect applied to all axes to preserve conformality.
				break;
		}
	}
	return transformChange;
}

EEditorTransformGizmo4DPart UEditorTransformGizmo4D::CheckForBestHit(const FVector4& InLocalRayOrigin, const FVector4& InLocalRayDirection, const FVector4& InLocalPerpDirection) const
{
	// If all distances are farther than this, no hit is registered.
	double closestDistance = UNREAL4D_EDITOR_TRANSFORM_GIZMO_CENTER_TARGET_RADIUS_RATIO;
	EEditorTransformGizmo4DPart closestPart = EEditorTransformGizmo4DPart::None;
	const FVector4 slidRayOrigin = UVectorFuncs4D::Vector4_Slide(InLocalRayOrigin, InLocalPerpDirection);
	const FVector4 slidRayDirection = UVectorFuncs4D::Vector4_Slide(InLocalRayDirection, InLocalPerpDirection);
	// Check center (move screen-relative and scale uniformly).
	if (_partVisibilitySettings.IsCenterVisible())
	{
		const double currentDistance = UGeometry4D::ClosestPointOnRay(slidRayOrigin, slidRayDirection, FVector4::Zero()).Size();
		if (currentDistance < closestDistance)
		{
			closestDistance = currentDistance;
			closestPart = _partVisibilitySettings.bIsMoveScreenRelativeEnabled ? EEditorTransformGizmo4DPart::MoveScreenRelative : EEditorTransformGizmo4DPart::ScaleUniform;
		}
	}
	// Check arrows (linear moves and scales).
	if (_partVisibilitySettings.AreArrowsVisible())
	{
		auto CheckArrow = [this, &closestDistance, &closestPart, &slidRayOrigin, &slidRayDirection, &InLocalPerpDirection]
		(const FVector4& InAxis, EEditorTransformGizmo4DPart InMovePart, EEditorTransformGizmo4DPart InScalePart)
		{
			const FVector4 aligned = UVectorFuncs4D::Vector4_Slide(InAxis, InLocalPerpDirection);
			const FVector4 axisStart = aligned * UNREAL4D_EDITOR_TRANSFORM_GIZMO_CENTER_TARGET_RADIUS_RATIO;
			const double currentDistance = UGeometry4D::DistanceBetweenLineAndSegment(slidRayOrigin, slidRayDirection, axisStart, aligned);
			if (currentDistance < closestDistance)
			{
				closestDistance = currentDistance;
				closestPart = _partVisibilitySettings.bIsMoveLinearEnabled ? InMovePart : InScalePart;
			}
		};
		CheckArrow(FVector4(1.0, 0.0, 0.0, 0.0), EEditorTransformGizmo4DPart::MoveX, EEditorTransformGizmo4DPart::ScaleX);
		CheckArrow(FVector4(0.0, 1.0, 0.0, 0.0), EEditorTransformGizmo4DPart::MoveY, EEditorTransformGizmo4DPart::ScaleY);
		CheckArrow(FVector4(0.0, 0.0, 1.0, 0.0), EEditorTransformGizmo4DPart::MoveZ, EEditorTransformGizmo4DPart::ScaleZ);
		CheckArrow(FVector4(0.0, 0.0, 0.0, 1.0), EEditorTransformGizmo4DPart::MoveW, EEditorTransformGizmo4DPart::ScaleW);
	}
	// Check planes (planar moves and scales).
	if (_partVisibilitySettings.ArePlanesVisible())
	{
		auto CheckPlane = [this, &closestDistance, &closestPart, &slidRayOrigin, &slidRayDirection, &InLocalPerpDirection]
		(const FVector4& InPoint, EEditorTransformGizmo4DPart InMovePart, EEditorTransformGizmo4DPart InScalePart)
		{
			const FVector4 aligned = UVectorFuncs4D::Vector4_Slide(InPoint, InLocalPerpDirection);
			const FVector4 closestPointOnLine = UGeometry4D::ClosestPointOnLine(slidRayOrigin, slidRayDirection, aligned);
			const double currentDistance = (closestPointOnLine - aligned).Size();
			if (currentDistance < closestDistance)
			{
				closestDistance = currentDistance;
				closestPart = _partVisibilitySettings.bIsMovePlanarEnabled ? InMovePart : InScalePart;
			}
		};
		static constexpr double planeOffset = UNREAL4D_EDITOR_TRANSFORM_GIZMO_BIPLANE_OFFSET_RATIO + 0.5 * UNREAL4D_EDITOR_TRANSFORM_GIZMO_BIPLANE_DIAMETER_RATIO;
		CheckPlane(FVector4(planeOffset, planeOffset, 0.0, 0.0), EEditorTransformGizmo4DPart::MoveXY, EEditorTransformGizmo4DPart::ScaleXY);
		CheckPlane(FVector4(planeOffset, 0.0, planeOffset, 0.0), EEditorTransformGizmo4DPart::MoveXZ, EEditorTransformGizmo4DPart::ScaleXZ);
		CheckPlane(FVector4(planeOffset, 0.0, 0.0, planeOffset), EEditorTransformGizmo4DPart::MoveXW, EEditorTransformGizmo4DPart::ScaleXW);
		CheckPlane(FVector4(0.0, planeOffset, planeOffset, 0.0), EEditorTransformGizmo4DPart::MoveYZ, EEditorTransformGizmo4DPart::ScaleYZ);
		CheckPlane(FVector4(0.0, planeOffset, 0.0, planeOffset), EEditorTransformGizmo4DPart::MoveYW, EEditorTransformGizmo4DPart::ScaleYW);
		CheckPlane(FVector4(0.0, 0.0, planeOffset, planeOffset), EEditorTransformGizmo4DPart::MoveZW, EEditorTransformGizmo4DPart::ScaleZW);
	}
	// Check stretch indicators.
	if (_partVisibilitySettings.bIsStretchEnabled)
	{
		auto CheckStretch = [this, &closestDistance, &closestPart, &slidRayOrigin, &slidRayDirection, &InLocalPerpDirection]
		(const FVector4& InPoint, EEditorTransformGizmo4DPart InPart)
		{
			const FVector4 aligned = UVectorFuncs4D::Vector4_Slide(InPoint, InLocalPerpDirection);
			const FVector4 closestPointOnLine = UGeometry4D::ClosestPointOnLine(slidRayOrigin, slidRayDirection, aligned);
			const double currentDistance = (closestPointOnLine - aligned).Size();
			if (currentDistance < closestDistance)
			{
				closestDistance = currentDistance;
				closestPart = InPart;
			}
		};
		CheckStretch(FVector4(1.0, 0.0, 0.0, 0.0), EEditorTransformGizmo4DPart::StretchPosX);
		CheckStretch(FVector4(-1.0, 0.0, 0.0, 0.0), EEditorTransformGizmo4DPart::StretchNegX);
		CheckStretch(FVector4(0.0, 1.0, 0.0, 0.0), EEditorTransformGizmo4DPart::StretchPosY);
		CheckStretch(FVector4(0.0, -1.0, 0.0, 0.0), EEditorTransformGizmo4DPart::StretchNegY);
		CheckStretch(FVector4(0.0, 0.0, 1.0, 0.0), EEditorTransformGizmo4DPart::StretchPosZ);
		CheckStretch(FVector4(0.0, 0.0, -1.0, 0.0), EEditorTransformGizmo4DPart::StretchNegZ);
		CheckStretch(FVector4(0.0, 0.0, 0.0, 1.0), EEditorTransformGizmo4DPart::StretchPosW);
		CheckStretch(FVector4(0.0, 0.0, 0.0, -1.0), EEditorTransformGizmo4DPart::StretchNegW);
	}
	// Check rotation rings.
	if (_partVisibilitySettings.bIsRotatePlanarEnabled)
	{
		auto CheckRing = [this, &closestDistance, &closestPart, &slidRayOrigin, &slidRayDirection, &InLocalPerpDirection]
		(const FVector4& InAxis1, const FVector4& InAxis2, EEditorTransformGizmo4DPart InPart)
		{
			const FVector4 slidAxis1 = UVectorFuncs4D::Vector4_Slide(InAxis1, InLocalPerpDirection);
			const FVector4 slidAxis2 = UVectorFuncs4D::Vector4_Slide(InAxis2, InLocalPerpDirection);
			const double currentDistance = DistanceBetweenRayAndEllipse(slidRayOrigin, slidRayDirection, slidAxis1, slidAxis2);
			if (currentDistance < closestDistance)
			{
				closestDistance = currentDistance;
				closestPart = InPart;
			}
		};
		CheckRing(FVector4(1.0, 0.0, 0.0, 0.0), FVector4(0.0, 1.0, 0.0, 0.0), EEditorTransformGizmo4DPart::RotateXY);
		CheckRing(FVector4(1.0, 0.0, 0.0, 0.0), FVector4(0.0, 0.0, 1.0, 0.0), EEditorTransformGizmo4DPart::RotateXZ);
		CheckRing(FVector4(1.0, 0.0, 0.0, 0.0), FVector4(0.0, 0.0, 0.0, 1.0), EEditorTransformGizmo4DPart::RotateXW);
		CheckRing(FVector4(0.0, 1.0, 0.0, 0.0), FVector4(0.0, 0.0, 1.0, 0.0), EEditorTransformGizmo4DPart::RotateYZ);
		CheckRing(FVector4(0.0, 1.0, 0.0, 0.0), FVector4(0.0, 0.0, 0.0, 1.0), EEditorTransformGizmo4DPart::RotateYW);
		CheckRing(FVector4(0.0, 0.0, 1.0, 0.0), FVector4(0.0, 0.0, 0.0, 1.0), EEditorTransformGizmo4DPart::RotateZW);
	}
	return closestPart;
}

void UEditorTransformGizmo4D::SetHighlightedPart(const EEditorTransformGizmo4DPart InNewHighlightedPart)
{
	if (InNewHighlightedPart == _highlightedTransformation)
	{
		return; // No need to update if the highlighted part hasn't changed.
	}
	// Update which parts of the visual actor are highlighted when the hovered part changes.
	{
		// NOTE: Unreal uses a threaded rendering system for editor gizmos, so
		// we should be careful to check the validity of the visual actor.
		if (_visualActor != nullptr && IsValid(_visualActor) && _visualActor->IsValidLowLevel())
		{
			_visualActor->SetPartHighlighted(_highlightedTransformation, false);
			_visualActor->SetPartHighlighted(InNewHighlightedPart, true);
		}
	}
	_highlightedTransformation = InNewHighlightedPart;
}

void UEditorTransformGizmo4D::BeginTransformation(const FVector4& InLocalRayOrigin, const FVector4& InLocalRayDirection, const FMatrix& InLocalCameraMatrix)
{
	_transformProxy4D->BeginTransformations();
	_transformReferenceValue = GetTransformRaycastValue(InLocalRayOrigin, InLocalRayDirection, InLocalCameraMatrix);
}

void UEditorTransformGizmo4D::ProcessTransformation(const FVector4& InLocalRayOrigin, const FVector4& InLocalRayDirection, const FMatrix& InLocalCameraMatrix)
{
	const FTransform4D localTransformChange = GetLocalTransformChange(InLocalRayOrigin, InLocalRayDirection, InLocalCameraMatrix);
	const double determinant = localTransformChange.Basis.Determinant();
	if (FMath::IsNearlyZero(determinant, 1e-20) || !FMath::IsFinite(determinant))
	{
		UE_LOG(LogUnreal4DEditor, Warning, TEXT("ProcessTransformation: Aborting transform change with zero or near-zero determinant (%f)."), determinant);
		ensure(false);
		return;
	}
	// The above transform changes happen local to the visual gizmo mesh,
	// but we want them parent-relative to the gizmo itself.
	const FTransform4D oldProxyTransform = _transformProxy4D->GetOldTransform4D();
	FTransform4D newProxyTransform = _snapSettings->SnapTransformChange(oldProxyTransform, localTransformChange);
	// Special case: Only in move mode, ignore any snapping that happened to the basis.
	if (_currentTransformation >= EEditorTransformGizmo4DPart::MoveScreenRelative && _currentTransformation <= EEditorTransformGizmo4DPart::MoveZW) {
		newProxyTransform.Basis = oldProxyTransform.Basis;
	}
	// We want the global diff so we can apply it from the left on the global transform of all selected nodes.
	// Without this, the transforms would be relative to each node (ex: moving on X moves on each node's X axis).
	const FTransform4D transformChange = oldProxyTransform.TransformTo(newProxyTransform);
	const bool bAffectsBasisAndThereforeShouldUseKeepMode = !IsGizmoPartForMove(_currentTransformation);
	_transformProxy4D->ApplyTransformChange(transformChange, bAffectsBasisAndThereforeShouldUseKeepMode);
}

// Builder implementation.

UInteractiveGizmo* UEditorTransformGizmo4DBuilder::BuildGizmo(const FToolBuilderState& InSceneState) const
{
	UEditorTransformGizmo4D* newGizmo = NewObject<UEditorTransformGizmo4D>(InSceneState.GizmoManager);
	newGizmo->SetWorld(InSceneState.World);
	return newGizmo;
}

// Unreal UInteractiveGizmo's lifecycle methods.

void UEditorTransformGizmo4D::Setup()
{
	Super::Setup();
	// Get view context for camera/viewport info.
	_gizmoViewContext = GetGizmoManager()->GetContextObjectStore()->FindContext<UGizmoViewContext>();
	// Setup click-drag behavior for handling transformation drags.
	_clickDragBehavior = NewObject<UClickDragInputBehavior>(this);
	_clickDragBehavior->Initialize(this);
	_clickDragBehavior->SetDefaultPriority(FInputCapturePriority(FInputCapturePriority::DEFAULT_GIZMO_PRIORITY));
	AddInputBehavior(_clickDragBehavior);
	// Setup hover behavior for highlighting gizmo parts.
	_hoverBehavior = NewObject<UMouseHoverBehavior>(this);
	_hoverBehavior->Initialize(this);
	_hoverBehavior->SetDefaultPriority(FInputCapturePriority(FInputCapturePriority::DEFAULT_GIZMO_PRIORITY));
	AddInputBehavior(_hoverBehavior);
}

void UEditorTransformGizmo4D::Shutdown()
{
	if (_editorViewportSettingsChangedHandle.IsValid())
	{
		FCoreUObjectDelegates::OnObjectPropertyChanged.Remove(_editorViewportSettingsChangedHandle);
		_editorViewportSettingsChangedHandle.Reset();
	}
	if (_modeTools != nullptr && _coordSystemChangedHandle.IsValid())
	{
		_modeTools->OnCoordSystemChanged().Remove(_coordSystemChangedHandle);
		_coordSystemChangedHandle.Reset();
	}
	if (_transformProxy4D != nullptr && _transformProxyChangedHandle.IsValid() && _transformProxy4D->On4DTransformProxyChanged.IsBound())
	{
		_transformProxy4D->On4DTransformProxyChanged.Remove(_transformProxyChangedHandle);
		_transformProxyChangedHandle.Reset();
	}
	if (_activeGizmo.Get() == this)
	{
		_activeGizmo = nullptr;
	}
	// Stop pointing to the passed in objects.
	_transformProxy4D = nullptr;
	_snapSettings = nullptr;
	_modeTools = nullptr;
	_viewportClient = nullptr;
	_world = nullptr;
	// Clean up and delete visual actor.
	{
		if (_visualActor != nullptr && IsValid(_visualActor) && _visualActor->IsValidLowLevel())
		{
			_visualActor->Destroy();
			_visualActor = nullptr;
		}
	}
	Super::Shutdown();
}

void UEditorTransformGizmo4D::ConfigureGridSnapping(bool bInPositionGridEnabled, bool bInRotationGridEnabled)
{
	if (_snapSettings != nullptr)
	{
		_snapSettings->ConfigureGridSnapping(bInPositionGridEnabled, bInRotationGridEnabled);
	}
}

double UEditorTransformGizmo4D::CalculateGizmoScale() const
{
	if (_gizmoViewContext == nullptr)
	{
		return 1.0;
	}
	// Use the rendered position: project relative to the 4D camera, then return to Unreal world space.
	const FVector4 cameraRelativeOrigin4D = GetEditorCameraTransform4DForViewport().Inverse().TransformPosition(_transformProxy4D->GetOldTransform4D().Origin);
	const FMatrix cameraBasis3D = FRotationMatrix::MakeFromXZ(_gizmoViewContext->GetViewDirection(), _gizmoViewContext->GetViewUp());
	const FVector gizmoOrigin3D = _gizmoViewContext->GetViewLocation() + cameraBasis3D.TransformVector(UVectorFuncs4D::Vector4_To3D(cameraRelativeOrigin4D));
	const double scale = UE::GizmoRenderingUtil::CalculateLocalPixelToWorldScale(_gizmoViewContext, gizmoOrigin3D);
	return FMath::IsFinite(scale) ? FMath::Max(scale, UE_DOUBLE_SMALL_NUMBER) : 1.0;
}

FTransform4D UEditorTransformGizmo4D::GetEditorCameraTransform4DForViewport() const
{
	if (_viewportClient == nullptr || _modeTools == nullptr)
	{
		return FTransform4D::Identity();
	}
	FUnreal4DEditorMode* editorMode = static_cast<FUnreal4DEditorMode*>(_modeTools->GetActiveMode(FUnreal4DEditorMode::ModeID));
	if (editorMode == nullptr)
	{
		return FTransform4D::Identity();
	}
	return editorMode->GetFullEditorCameraTransform4D(_viewportClient);
}

// Called by UEditorTransformGizmo4DFactory at creation time after the UInteractiveGizmo::Setup lifecycle method.
void UEditorTransformGizmo4D::Setup4D(const TArray<UScene4DComponent*>& InObjects4D, const FEditorTransformGizmo4DPartVisibilitySettings InVisibilitySettings, const ECoordSystem InCoordSystem, FEditorModeTools* InModeTools, FEditorViewportClient* InViewportClient)
{
	// Ensure that nothing is connected to the handle yet and that nothing is setup.
	// We want to call this function exactly once per gizmo object.
	ensure(_transformProxyChangedHandle.IsValid() == false);
	ensure(_coordSystemChangedHandle.IsValid() == false);
	ensure(_transformProxy4D == nullptr);
	ensure(_world != nullptr);
	_modeTools = InModeTools;
	_viewportClient = InViewportClient;
	_partVisibilitySettings = InVisibilitySettings;
	ReadFromEditorViewportSettings();
	// Create the transform proxy.
	_transformProxy4D = NewObject<UEditorTransformProxy4D>(this);
	// Fallback only: If InViewportClient is null, use the global active viewport. Some call paths only provide ModeTools,
	// which is not enough to identify a concrete viewport in every editor context, so use the active viewport.
	if (_viewportClient == nullptr && GEditor != nullptr && GEditor->GetActiveViewport() != nullptr)
	{
		_viewportClient = static_cast<FEditorViewportClient*>(GEditor->GetActiveViewport()->GetClient());
	}
	_transformProxy4D->SetEditorContext(_modeTools, _viewportClient);
	_activeGizmo = this;
	const ECoordSystem initialCoordSystem = (_modeTools != nullptr) ? _modeTools->GetCoordSystem() : InCoordSystem;
	_transformProxy4D->SetCoordSystem(initialCoordSystem);
	_transformProxy4D->SetKeepMode(_keepMode);
	if (!_editorViewportSettingsChangedHandle.IsValid())
	{
		_editorViewportSettingsChangedHandle = FCoreUObjectDelegates::OnObjectPropertyChanged.AddUObject(
			this,
			&UEditorTransformGizmo4D::HandleEditorViewportSettingsChanged
		);
	}
	if (_modeTools != nullptr && !_coordSystemChangedHandle.IsValid())
	{
		_coordSystemChangedHandle = _modeTools->OnCoordSystemChanged().AddUObject(
			this,
			&UEditorTransformGizmo4D::HandleCoordSystemChanged
		);
	}
	_transformProxy4D->Select4DObjects(InObjects4D);
	// Create the snap settings and share it with the proxy.
	_snapSettings = NewObject<UEditorTransformSnapSettings4D>(this);
	_transformProxy4D->SnapSettings = _snapSettings;
	// Spawn the visual actor.
	FActorSpawnParameters SpawnParams;
	SpawnParams.ObjectFlags = RF_Transient;
	{
		_visualActor = _world->SpawnActor<AEditorGizmoActor4D>(AEditorGizmoActor4D::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		_visualActor->SetGizmoTransform4D(_transformProxy4D->GetCurrentTransform4D());
		UGizmoViewContext* context = GetGizmoManager()->GetContextObjectStore()->FindContext<UGizmoViewContext>();
		_visualActor->SetupAndGenerateParts(InVisibilitySettings, _world, context, _modeTools, _viewportClient);
	}
	// Update the visual gizmo actor's transform whenever the proxy transform changes.
	TWeakObjectPtr<UEditorTransformGizmo4D> weakSelfPointer(this);
	_transformProxyChangedHandle = _transformProxy4D->On4DTransformProxyChanged.AddLambda([weakSelfPointer](const FTransform4D& InNewTransform)
	{
		UEditorTransformGizmo4D* rawSelfPointer = weakSelfPointer.Get();
		if (rawSelfPointer == nullptr || !IsValid(rawSelfPointer) || !rawSelfPointer->IsValidLowLevel())
		{
			return;
		}
		// NOTE: Unreal uses a threaded rendering system for editor gizmos, so
		// we should be careful to check the validity of the visual actor.
		if (rawSelfPointer->_visualActor && IsValid(rawSelfPointer->_visualActor) && rawSelfPointer->_visualActor->IsValidLowLevel())
		{
			rawSelfPointer->_visualActor->SetGizmoTransform4D(InNewTransform);
		}
	});
}

bool UEditorTransformGizmo4D::DuplicateSelectedObjects()
{
	if (_transformProxy4D == nullptr)
	{
		return false;
	}
	return _transformProxy4D->DuplicateSelectedObjects();
}

bool UEditorTransformGizmo4D::DuplicateSelectedObjectsStatic()
{
	UEditorTransformGizmo4D* activeGizmo = _activeGizmo.Get();
	if (activeGizmo == nullptr || !IsValid(activeGizmo) || !activeGizmo->IsValidLowLevel())
	{
		return false;
	}
	return activeGizmo->DuplicateSelectedObjects();
}

// Helper functions for mouse input processing.

void UEditorTransformGizmo4D::GetLocalRayDataWithScale(const FInputDeviceRay& InDeviceRay, FVector4& OutLocalRayOrigin, FVector4& OutLocalRayDirection, FMatrix& OutLocalCameraMatrix) const
{
	// Unreal provides the ray in 3D world space, but we want it in 4D world space.
	const FRay& worldRay = InDeviceRay.WorldRay;
	// First, we need to convert it to camera-relative space using Unreal's 3D camera transform.
	FTransform cameraTransform3D;
	if (_gizmoViewContext)
	{
		cameraTransform3D.SetLocation(_gizmoViewContext->GetViewLocation());
		FVector ViewDir = _gizmoViewContext->GetViewDirection();
		FVector ViewUp = _gizmoViewContext->GetViewUp();
		cameraTransform3D.SetRotation(FRotationMatrix::MakeFromXZ(ViewDir, ViewUp).ToQuat());
	}
	const FTransform inverseCameraTransform3D = cameraTransform3D.Inverse();
	const FVector4 cameraRelativeRayOrigin4D = UVectorFuncs4D::Vector4_From3D(inverseCameraTransform3D.TransformPosition(worldRay.Origin));
	const FVector4 cameraRelativeRayDirection4D = UVectorFuncs4D::Vector4_From3D(inverseCameraTransform3D.TransformVector(worldRay.Direction));
	// Next, use the editor camera's 4D transform to convert to full 4D world space.
	const FTransform4D cameraTransform4D = GetEditorCameraTransform4DForViewport();
	const FVector4 worldRayOrigin4D = cameraTransform4D.TransformPosition(cameraRelativeRayOrigin4D) - _transformProxy4D->GetShiftSlideOffset();
	const FVector4 worldRayDirection4D = cameraTransform4D.Basis.TransformFVector4(cameraRelativeRayDirection4D);
	// Remember that in Unreal the convention is that basis vectors are stored in matrix rows.
	const FVector4 worldPerpDirection4D = UBasis4D::Basis4D_GetRow(cameraTransform4D.Basis, 3);
	const FTransform4D gizmoOldTransform = _transformProxy4D->GetOldTransform4D();
	ensure(!FMath::IsNearlyZero(gizmoOldTransform.Determinant(), 1e-18));
	double gizmoScale = _gizmoScale;
	if (_currentTransformation == EEditorTransformGizmo4DPart::None)
	{
		gizmoScale = CalculateGizmoScale();
	}
	const FTransform4D inverseGizmoOldTransform = gizmoOldTransform.Inverse();
	OutLocalRayOrigin = inverseGizmoOldTransform.TransformPosition(worldRayOrigin4D) / (gizmoScale * UNREAL4D_EDITOR_TRANSFORM_GIZMO_EXTENTS);
	OutLocalRayDirection = inverseGizmoOldTransform.Basis.TransformFVector4(worldRayDirection4D);
	OutLocalCameraMatrix = cameraTransform4D.Basis * inverseGizmoOldTransform.Basis;
}

// IClickDragBehaviorTarget implementation.

FInputRayHit UEditorTransformGizmo4D::CanBeginClickDragSequence(const FInputDeviceRay& InPressPos)
{
	// Check if we have valid state.
	if (_transformProxy4D == nullptr || _world == nullptr)
	{
		return FInputRayHit();
	}
	// Get local ray data using internal scale handling.
	FVector4 localRayOrigin, localRayDirection;
	FMatrix localCameraMatrix;
	GetLocalRayDataWithScale(InPressPos, localRayOrigin, localRayDirection, localCameraMatrix);
	// Find which part we're hovering over and return an FInputRayHit for it.
	const EEditorTransformGizmo4DPart hitPart = CheckForBestHit(localRayOrigin, localRayDirection, UBasis4D::Basis4D_GetRow(localCameraMatrix, 3));
	if (hitPart != EEditorTransformGizmo4DPart::None)
	{
		FInputRayHit hit = FInputRayHit(0.0);
		hit.HitIdentifier = static_cast<int32>(hitPart);
		return hit;
	}
	return FInputRayHit();
}

void UEditorTransformGizmo4D::OnClickPress(const FInputDeviceRay& InPressPos)
{
	// Check if we have valid state.
	if (_transformProxy4D == nullptr || _world == nullptr)
	{
		return;
	}
	// Calculate and cache the gizmo scale using pixel-to-world conversion.
	_gizmoScale = CalculateGizmoScale();
	// Get local ray data.
	FVector4 localRayOrigin, localRayDirection;
	FMatrix localCameraMatrix;
	GetLocalRayDataWithScale(InPressPos, localRayOrigin, localRayDirection, localCameraMatrix);
	// Determine which part we're starting to transform.
	_currentTransformation = CheckForBestHit(localRayOrigin, localRayDirection, UBasis4D::Basis4D_GetRow(localCameraMatrix, 3));
	if (_currentTransformation != EEditorTransformGizmo4DPart::None)
	{
		// Begin the transformation.
		BeginTransformation(localRayOrigin, localRayDirection, localCameraMatrix);
	}
}

void UEditorTransformGizmo4D::OnClickDrag(const FInputDeviceRay& InDragPos)
{
	// Check if we have valid state.
	if (_transformProxy4D == nullptr || _world == nullptr)
	{
		return;
	}
	// Dragging only works if we're currently transforming something.
	if (_currentTransformation == EEditorTransformGizmo4DPart::None)
	{
		return;
	}
	// Get local ray data using internal scale handling.
	FVector4 localRayOrigin, localRayDirection;
	FMatrix localCameraMatrix;
	GetLocalRayDataWithScale(InDragPos, localRayOrigin, localRayDirection, localCameraMatrix);
	// Process the ongoing transformation.
	ProcessTransformation(localRayOrigin, localRayDirection, localCameraMatrix);
}

void UEditorTransformGizmo4D::OnClickRelease(const FInputDeviceRay& InReleasePos)
{
	if (_currentTransformation != EEditorTransformGizmo4DPart::None)
	{
		// End the transformation by committing the previewed transforms as the actual transforms.
		_transformProxy4D->CommitTransformations();
		_currentTransformation = EEditorTransformGizmo4DPart::None;
	}
	// Ensure Slate does not keep mouse capture after gizmo drags that move the camera while held.
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().ReleaseAllPointerCapture();
	}
}

void UEditorTransformGizmo4D::OnTerminateDragSequence()
{
	// Transformation was forcibly terminated (e.g., by escape key)
	if (_currentTransformation != EEditorTransformGizmo4DPart::None)
	{
		// Cancel the transformation by reverting everything back to the original transforms before the drag began.
		_transformProxy4D->CancelTransformations();
		_currentTransformation = EEditorTransformGizmo4DPart::None;
	}
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().ReleaseAllPointerCapture();
	}
}

// IHoverBehaviorTarget implementation.

FInputRayHit UEditorTransformGizmo4D::BeginHoverSequenceHitTest(const FInputDeviceRay& InPressPos)
{
	// Check if we have valid state.
	if (_transformProxy4D == nullptr || _world == nullptr)
	{
		return FInputRayHit();
	}
	// Don't change hover highlights while actively transforming,
	// we only change highlights when the user is not clicking.
	if (_currentTransformation != EEditorTransformGizmo4DPart::None)
	{
		return FInputRayHit();
	}
	// Get local ray data using internal scale handling.
	FVector4 localRayOrigin, localRayDirection;
	FMatrix localCameraMatrix;
	GetLocalRayDataWithScale(InPressPos, localRayOrigin, localRayDirection, localCameraMatrix);
	// Find which part we're hovering over and return an FInputRayHit for it.
	const EEditorTransformGizmo4DPart hitPart = CheckForBestHit(localRayOrigin, localRayDirection, UBasis4D::Basis4D_GetRow(localCameraMatrix, 3));
	if (hitPart != EEditorTransformGizmo4DPart::None)
	{
		FInputRayHit hit = FInputRayHit(0.0);
		hit.HitIdentifier = static_cast<int32>(hitPart);
		return hit;
	}
	return FInputRayHit();
}

void UEditorTransformGizmo4D::OnBeginHover(const FInputDeviceRay& InDevicePos)
{
	// Get local ray data with internal scale handling.
	FVector4 localRayOrigin, localRayDirection;
	FMatrix localCameraMatrix;
	GetLocalRayDataWithScale(InDevicePos, localRayOrigin, localRayDirection, localCameraMatrix);
	// Find which part we're hovering over and update the highlights.
	SetHighlightedPart(CheckForBestHit(localRayOrigin, localRayDirection, UBasis4D::Basis4D_GetRow(localCameraMatrix, 3)));
}

bool UEditorTransformGizmo4D::OnUpdateHover(const FInputDeviceRay& InDevicePos)
{
	// Don't update highlights while actively transforming.
	if (_currentTransformation != EEditorTransformGizmo4DPart::None)
	{
		return true;
	}
	// Get local ray data with internal scale handling and check which part we're hovering over.
	FVector4 localRayOrigin, localRayDirection;
	FMatrix localCameraMatrix;
	GetLocalRayDataWithScale(InDevicePos, localRayOrigin, localRayDirection, localCameraMatrix);
	SetHighlightedPart(CheckForBestHit(localRayOrigin, localRayDirection, UBasis4D::Basis4D_GetRow(localCameraMatrix, 3)));
	// Return true to continue receiving hover updates, false to end hover sequence.
	return _highlightedTransformation != EEditorTransformGizmo4DPart::None;
}

void UEditorTransformGizmo4D::OnEndHover()
{
	SetHighlightedPart(EEditorTransformGizmo4DPart::None);
}
