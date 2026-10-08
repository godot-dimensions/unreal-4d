#include "EditorTransformGizmoActor4D.h"

// Unreal4D includes.
#include "../../../../Unreal4DCore/Math/Math4D.h"
#include "../../../../Unreal4DCore/Math/VectorFuncs4D.h"
#include "../../../../Unreal4DCore/Render/RenderingServer4D.h"
#include "../../../../Unreal4DCore/Scene/Components/Scene4DComponent.h"
#include "../../../Unreal4DEditorMode.h"
#include "EditorTransformGizmoArrowMesh4D.h"
#include "EditorTransformGizmoBiplaneMesh4D.h"
#include "EditorTransformGizmoBivectorRingMesh4D.h"
#include "EditorTransformGizmoDiscMesh4D.h"

// Unreal includes.
#include "BaseGizmos/GizmoViewContext.h"
#include "EditorModeManager.h"
#include "Settings/EditorStyleSettings.h"

template<typename T>
T* AEditorGizmoActor4D::CreatePart()
{
	T* NewPart = NewObject<T>(this);
	AddInstanceComponent(NewPart);
	NewPart->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	NewPart->RegisterComponent();
	return NewPart;
}

void AEditorGizmoActor4D::SetupAndGenerateParts(const FEditorTransformGizmo4DPartVisibilitySettings InVisibilitySettings, UWorld* InWorld, UGizmoViewContext* InGizmoViewContext, FEditorModeTools* InModeTools, FEditorViewportClient* InViewportClient)
{
	_modeTools = InModeTools;
	_viewportClient = InViewportClient;

	const FLinearColor xRegularColor = FUnreal4DEditorMode::GetAxisColor(EAxis4D::X);
	const FLinearColor xHighlightColor = UMath4D::ColorLightened(xRegularColor, 0.25f);
	const FLinearColor yRegularColor = FUnreal4DEditorMode::GetAxisColor(EAxis4D::Y);
	const FLinearColor yHighlightColor = UMath4D::ColorLightened(yRegularColor, 0.25f);
	const FLinearColor zRegularColor = FUnreal4DEditorMode::GetAxisColor(EAxis4D::Z);
	const FLinearColor zHighlightColor = UMath4D::ColorLightened(zRegularColor, 0.25f);
	const FLinearColor wRegularColor = FUnreal4DEditorMode::GetAxisColor(EAxis4D::W);
	const FLinearColor wHighlightColor = UMath4D::ColorLightened(wRegularColor, 0.25f);
	const FLinearColor centerRegularColor = FLinearColor::Gray;
	const FLinearColor centerHighlightColor = UMath4D::ColorLightened(centerRegularColor, 0.25f);
	if (InVisibilitySettings.bIsMoveScreenRelativeEnabled)
	{
		UEditorTransformGizmoDiscMesh4D* disc = CreatePart<UEditorTransformGizmoDiscMesh4D>();
		disc->SetPrimaryColors(centerRegularColor, centerHighlightColor);
		_gizmoParts4D.Emplace(EEditorTransformGizmo4DPart::MoveScreenRelative, disc);
	}
	if (InVisibilitySettings.bIsScaleUniformEnabled)
	{
		UEditorTransformGizmoDiscMesh4D* disc = CreatePart<UEditorTransformGizmoDiscMesh4D>();
		disc->SetPrimaryColors(centerRegularColor, centerHighlightColor);
		_gizmoParts4D.Emplace(EEditorTransformGizmo4DPart::ScaleUniform, disc);
	}
	if (InVisibilitySettings.bIsMoveLinearEnabled)
	{
		// X axis.
		UEditorTransformGizmoArrowMesh4D* arrowX = CreatePart<UEditorTransformGizmoArrowMesh4D>();
		arrowX->TipType = EEditorTransformGizmoArrowMeshTipType4D::Cone;
		arrowX->SetPrimaryColors(xRegularColor, xHighlightColor);
		_gizmoParts4D.Emplace(EEditorTransformGizmo4DPart::MoveX, arrowX);
		// Y axis.
		UEditorTransformGizmoArrowMesh4D* arrowY = CreatePart<UEditorTransformGizmoArrowMesh4D>();
		arrowY->TipType = EEditorTransformGizmoArrowMeshTipType4D::Cone;
		arrowY->SetPrimaryColors(yRegularColor, yHighlightColor);
		_gizmoParts4D.Emplace(EEditorTransformGizmo4DPart::MoveY, arrowY);
		// Z axis.
		UEditorTransformGizmoArrowMesh4D* arrowZ = CreatePart<UEditorTransformGizmoArrowMesh4D>();
		arrowZ->TipType = EEditorTransformGizmoArrowMeshTipType4D::Cone;
		arrowZ->SetPrimaryColors(zRegularColor, zHighlightColor);
		_gizmoParts4D.Emplace(EEditorTransformGizmo4DPart::MoveZ, arrowZ);
		// W axis.
		UEditorTransformGizmoArrowMesh4D* arrowW = CreatePart<UEditorTransformGizmoArrowMesh4D>();
		arrowW->TipType = EEditorTransformGizmoArrowMeshTipType4D::Cone;
		arrowW->SetPrimaryColors(wRegularColor, wHighlightColor);
		_gizmoParts4D.Emplace(EEditorTransformGizmo4DPart::MoveW, arrowW);
	}
	if (InVisibilitySettings.bIsScaleLinearEnabled)
	{
		// X axis.
		UEditorTransformGizmoArrowMesh4D* arrowX = CreatePart<UEditorTransformGizmoArrowMesh4D>();
		arrowX->TipType = EEditorTransformGizmoArrowMeshTipType4D::Cylinder;
		arrowX->SetPrimaryColors(xRegularColor, xHighlightColor);
		_gizmoParts4D.Emplace(EEditorTransformGizmo4DPart::ScaleX, arrowX);
		// Y axis.
		UEditorTransformGizmoArrowMesh4D* arrowY = CreatePart<UEditorTransformGizmoArrowMesh4D>();
		arrowY->TipType = EEditorTransformGizmoArrowMeshTipType4D::Cylinder;
		arrowY->SetPrimaryColors(yRegularColor, yHighlightColor);
		_gizmoParts4D.Emplace(EEditorTransformGizmo4DPart::ScaleY, arrowY);
		// Z axis.
		UEditorTransformGizmoArrowMesh4D* arrowZ = CreatePart<UEditorTransformGizmoArrowMesh4D>();
		arrowZ->TipType = EEditorTransformGizmoArrowMeshTipType4D::Cylinder;
		arrowZ->SetPrimaryColors(zRegularColor, zHighlightColor);
		_gizmoParts4D.Emplace(EEditorTransformGizmo4DPart::ScaleZ, arrowZ);
		// W axis.
		UEditorTransformGizmoArrowMesh4D* arrowW = CreatePart<UEditorTransformGizmoArrowMesh4D>();
		arrowW->TipType = EEditorTransformGizmoArrowMeshTipType4D::Cylinder;
		arrowW->SetPrimaryColors(wRegularColor, wHighlightColor);
		_gizmoParts4D.Emplace(EEditorTransformGizmo4DPart::ScaleW, arrowW);
	}
	if (InVisibilitySettings.bIsMovePlanarEnabled)
	{
		// XY plane.
		UEditorTransformGizmoBiplaneMesh4D* biplaneXY = CreatePart<UEditorTransformGizmoBiplaneMesh4D>();
		biplaneXY->SetPrimaryColors(xRegularColor, xHighlightColor);
		biplaneXY->SetSecondaryColors(yRegularColor, yHighlightColor);
		_gizmoParts4D.Emplace(EEditorTransformGizmo4DPart::MoveXY, biplaneXY);
		// XZ plane.
		UEditorTransformGizmoBiplaneMesh4D* biplaneXZ = CreatePart<UEditorTransformGizmoBiplaneMesh4D>();
		biplaneXZ->SetPrimaryColors(xRegularColor, xHighlightColor);
		biplaneXZ->SetSecondaryColors(zRegularColor, zHighlightColor);
		_gizmoParts4D.Emplace(EEditorTransformGizmo4DPart::MoveXZ, biplaneXZ);
		// XW plane.
		UEditorTransformGizmoBiplaneMesh4D* biplaneXW = CreatePart<UEditorTransformGizmoBiplaneMesh4D>();
		biplaneXW->SetPrimaryColors(xRegularColor, xHighlightColor);
		biplaneXW->SetSecondaryColors(wRegularColor, wHighlightColor);
		_gizmoParts4D.Emplace(EEditorTransformGizmo4DPart::MoveXW, biplaneXW);
		// YZ plane.
		UEditorTransformGizmoBiplaneMesh4D* biplaneYZ = CreatePart<UEditorTransformGizmoBiplaneMesh4D>();
		biplaneYZ->SetPrimaryColors(yRegularColor, yHighlightColor);
		biplaneYZ->SetSecondaryColors(zRegularColor, zHighlightColor);
		_gizmoParts4D.Emplace(EEditorTransformGizmo4DPart::MoveYZ, biplaneYZ);
		// YW plane.
		UEditorTransformGizmoBiplaneMesh4D* biplaneYW = CreatePart<UEditorTransformGizmoBiplaneMesh4D>();
		biplaneYW->SetPrimaryColors(yRegularColor, yHighlightColor);
		biplaneYW->SetSecondaryColors(wRegularColor, wHighlightColor);
		_gizmoParts4D.Emplace(EEditorTransformGizmo4DPart::MoveYW, biplaneYW);
		// ZW plane.
		UEditorTransformGizmoBiplaneMesh4D* biplaneZW = CreatePart<UEditorTransformGizmoBiplaneMesh4D>();
		biplaneZW->SetPrimaryColors(zRegularColor, zHighlightColor);
		biplaneZW->SetSecondaryColors(wRegularColor, wHighlightColor);
		_gizmoParts4D.Emplace(EEditorTransformGizmo4DPart::MoveZW, biplaneZW);
	}
	if (InVisibilitySettings.bIsScalePlanarEnabled)
	{
		// XY plane.
		UEditorTransformGizmoBiplaneMesh4D* biplaneXY = CreatePart<UEditorTransformGizmoBiplaneMesh4D>();
		biplaneXY->SetPrimaryColors(xRegularColor, xHighlightColor);
		biplaneXY->SetSecondaryColors(yRegularColor, yHighlightColor);
		_gizmoParts4D.Emplace(EEditorTransformGizmo4DPart::ScaleXY, biplaneXY);
		// XZ plane.
		UEditorTransformGizmoBiplaneMesh4D* biplaneXZ = CreatePart<UEditorTransformGizmoBiplaneMesh4D>();
		biplaneXZ->SetPrimaryColors(xRegularColor, xHighlightColor);
		biplaneXZ->SetSecondaryColors(zRegularColor, zHighlightColor);
		_gizmoParts4D.Emplace(EEditorTransformGizmo4DPart::ScaleXZ, biplaneXZ);
		// XW plane.
		UEditorTransformGizmoBiplaneMesh4D* biplaneXW = CreatePart<UEditorTransformGizmoBiplaneMesh4D>();
		biplaneXW->SetPrimaryColors(xRegularColor, xHighlightColor);
		biplaneXW->SetSecondaryColors(wRegularColor, wHighlightColor);
		_gizmoParts4D.Emplace(EEditorTransformGizmo4DPart::ScaleXW, biplaneXW);
		// YZ plane.
		UEditorTransformGizmoBiplaneMesh4D* biplaneYZ = CreatePart<UEditorTransformGizmoBiplaneMesh4D>();
		biplaneYZ->SetPrimaryColors(yRegularColor, yHighlightColor);
		biplaneYZ->SetSecondaryColors(zRegularColor, zHighlightColor);
		_gizmoParts4D.Emplace(EEditorTransformGizmo4DPart::ScaleYZ, biplaneYZ);
		// YW plane.
		UEditorTransformGizmoBiplaneMesh4D* biplaneYW = CreatePart<UEditorTransformGizmoBiplaneMesh4D>();
		biplaneYW->SetPrimaryColors(yRegularColor, yHighlightColor);
		biplaneYW->SetSecondaryColors(wRegularColor, wHighlightColor);
		_gizmoParts4D.Emplace(EEditorTransformGizmo4DPart::ScaleYW, biplaneYW);
		// ZW plane.
		UEditorTransformGizmoBiplaneMesh4D* biplaneZW = CreatePart<UEditorTransformGizmoBiplaneMesh4D>();
		biplaneZW->SetPrimaryColors(zRegularColor, zHighlightColor);
		biplaneZW->SetSecondaryColors(wRegularColor, wHighlightColor);
		_gizmoParts4D.Emplace(EEditorTransformGizmo4DPart::ScaleZW, biplaneZW);
	}
	if (InVisibilitySettings.bIsRotatePlanarEnabled)
	{
		// XY ring.
		UEditorTransformGizmoBivectorRingMesh4D* ringXY = CreatePart<UEditorTransformGizmoBivectorRingMesh4D>();
		ringXY->SetPrimaryColors(xRegularColor, xHighlightColor);
		ringXY->SetSecondaryColors(yRegularColor, yHighlightColor);
		_gizmoParts4D.Emplace(EEditorTransformGizmo4DPart::RotateXY, ringXY);
		// XZ ring.
		UEditorTransformGizmoBivectorRingMesh4D* ringXZ = CreatePart<UEditorTransformGizmoBivectorRingMesh4D>();
		ringXZ->SetPrimaryColors(xRegularColor, xHighlightColor);
		ringXZ->SetSecondaryColors(zRegularColor, zHighlightColor);
		_gizmoParts4D.Emplace(EEditorTransformGizmo4DPart::RotateXZ, ringXZ);
		// XW ring.
		UEditorTransformGizmoBivectorRingMesh4D* ringXW = CreatePart<UEditorTransformGizmoBivectorRingMesh4D>();
		ringXW->SetPrimaryColors(xRegularColor, xHighlightColor);
		ringXW->SetSecondaryColors(wRegularColor, wHighlightColor);
		_gizmoParts4D.Emplace(EEditorTransformGizmo4DPart::RotateXW, ringXW);
		// YZ ring.
		UEditorTransformGizmoBivectorRingMesh4D* ringYZ = CreatePart<UEditorTransformGizmoBivectorRingMesh4D>();
		ringYZ->SetPrimaryColors(yRegularColor, yHighlightColor);
		ringYZ->SetSecondaryColors(zRegularColor, zHighlightColor);
		_gizmoParts4D.Emplace(EEditorTransformGizmo4DPart::RotateYZ, ringYZ);
		// YW ring.
		UEditorTransformGizmoBivectorRingMesh4D* ringYW = CreatePart<UEditorTransformGizmoBivectorRingMesh4D>();
		ringYW->SetPrimaryColors(yRegularColor, yHighlightColor);
		ringYW->SetSecondaryColors(wRegularColor, wHighlightColor);
		_gizmoParts4D.Emplace(EEditorTransformGizmo4DPart::RotateYW, ringYW);
		// ZW ring.
		UEditorTransformGizmoBivectorRingMesh4D* ringZW = CreatePart<UEditorTransformGizmoBivectorRingMesh4D>();
		ringZW->SetPrimaryColors(zRegularColor, zHighlightColor);
		ringZW->SetSecondaryColors(wRegularColor, wHighlightColor);
		_gizmoParts4D.Emplace(EEditorTransformGizmo4DPart::RotateZW, ringZW);
	}
	TryUpdateCameraTransform4D();
	UpdatePartMeshes();
}

bool AEditorGizmoActor4D::TryUpdateCameraTransform4D()
{
	if (_modeTools != nullptr && _viewportClient != nullptr)
	{
		if (FUnreal4DEditorMode* editorMode = static_cast<FUnreal4DEditorMode*>(_modeTools->GetActiveMode(FUnreal4DEditorMode::ModeID)); editorMode != nullptr)
		{
			const FTransform4D currentCameraTransform4D = editorMode->GetEditorCameraGroundTransform4D(_viewportClient);
			const FTransform currentCameraTransform3D = editorMode->GetEditorCameraGroundTransform3D(_viewportClient);
			if (_cameraTransform4D == currentCameraTransform4D && _cameraTransform3D.Equals(currentCameraTransform3D))
			{
				return false;
			}
			_cameraTransform4D = currentCameraTransform4D;
			_cameraTransform3D = currentCameraTransform3D;
			_root4DComponent->UpdatedTransform4D();
			return true;
		}
	}

	UWorld* world = GetWorld();
	if (world == nullptr)
	{
		return false;
	}
	// Fallback path for cases where an editor mode/view client isn't available.
	// RenderingServer4D may be null in unusual viewports. In this case, treat the
	// 4D camera as having a 4D identity transform, otherwise get the actual 4D camera transform.
	FTransform4D currentCameraTransform4D = FTransform4D::Identity();
	FTransform currentCameraTransform3D = FTransform::Identity;
	URenderingServer4D* renderingServer4D = world->GetSubsystem<URenderingServer4D>();
	if (renderingServer4D != nullptr)
	{
		currentCameraTransform4D = renderingServer4D->GetCurrentCameraTransform4D();
		currentCameraTransform3D = renderingServer4D->GetCurrentCameraTransform3D();
	}
	// No need to update if the camera transform hasn't changed since the last update.
	if (_cameraTransform4D == currentCameraTransform4D && _cameraTransform3D.Equals(currentCameraTransform3D))
	{
		return false;
	}
	_cameraTransform4D = currentCameraTransform4D;
	_cameraTransform3D = currentCameraTransform3D;
	_root4DComponent->UpdatedTransform4D();
	return true;
}

void AEditorGizmoActor4D::Tick(float InDeltaTime)
{
	if (TryUpdateCameraTransform4D())
	{
		UpdatePartMeshes();
	}
	Super::Tick(InDeltaTime);
}

void AEditorGizmoActor4D::UpdatePartMeshes()
{
	// Transform the gizmo's 4D world space transform to camera-relative 4D space,
	// then convert the 4D transform to 3D, then apply the camera's 3D transform to
	// get world space. This is required to handle camera rotations into the W axis.
	// See the similar note in UScene4DComponent::UpdatedTransform4D for details.
	const FTransform4D relToCamera4D = _cameraTransform4D.Inverse().Compose(_root4DComponent->GetLocalTransform4D());
	const FTransform& cameraTransform3D = _cameraTransform3D;
	// Cast the camera-relative basis vectors to 3D and scale them to find the camera-relative target positions for each axis.
	const FVector xCamera3D = UVectorFuncs4D::Vector4_To3D(relToCamera4D[0]) * UNREAL4D_EDITOR_TRANSFORM_GIZMO_EXTENTS;
	const FVector yCamera3D = UVectorFuncs4D::Vector4_To3D(relToCamera4D[1]) * UNREAL4D_EDITOR_TRANSFORM_GIZMO_EXTENTS;
	const FVector zCamera3D = UVectorFuncs4D::Vector4_To3D(relToCamera4D[2]) * UNREAL4D_EDITOR_TRANSFORM_GIZMO_EXTENTS;
	const FVector wCamera3D = UVectorFuncs4D::Vector4_To3D(relToCamera4D[3]) * UNREAL4D_EDITOR_TRANSFORM_GIZMO_EXTENTS;
	// Convert the camera-relative target positions to world space.
	const FVector x3D = cameraTransform3D.TransformVector(xCamera3D);
	const FVector y3D = cameraTransform3D.TransformVector(yCamera3D);
	const FVector z3D = cameraTransform3D.TransformVector(zCamera3D);
	const FVector w3D = cameraTransform3D.TransformVector(wCamera3D);
	for (const auto& pair : _gizmoParts4D)
	{
		const EEditorTransformGizmo4DPart partEnum = pair.Key;
		UEditorTransformGizmoBaseMesh4D* partComponent = pair.Value;
		switch (partEnum)
		{
			case EEditorTransformGizmo4DPart::None:
			{
				ensure(false); // This case should not be possible.
			} break;
			case EEditorTransformGizmo4DPart::MoveScreenRelative:
			case EEditorTransformGizmo4DPart::ScaleUniform:
			{
				// The axis choice doesn't matter here, the circle just uses the length of the vector.
				partComponent->SetPrimaryTargetPosition((UNREAL4D_EDITOR_TRANSFORM_GIZMO_CENTER_VISUAL_RADIUS_RATIO) * x3D);
				partComponent->SetDepthBias(0.0f);
			} break;
			case EEditorTransformGizmo4DPart::MoveX:
			case EEditorTransformGizmo4DPart::ScaleX:
			{
				partComponent->SetPrimaryTargetPosition(x3D);
				partComponent->SetDepthBias(xCamera3D.X * -1000.0f);
			} break;
			case EEditorTransformGizmo4DPart::MoveY:
			case EEditorTransformGizmo4DPart::ScaleY:
			{
				partComponent->SetPrimaryTargetPosition(y3D);
				partComponent->SetDepthBias(yCamera3D.X * -1000.0f);
			} break;
			case EEditorTransformGizmo4DPart::MoveZ:
			case EEditorTransformGizmo4DPart::ScaleZ:
			{
				partComponent->SetPrimaryTargetPosition(z3D);
				partComponent->SetDepthBias(zCamera3D.X * -1000.0f);
			} break;
			case EEditorTransformGizmo4DPart::MoveW:
			case EEditorTransformGizmo4DPart::ScaleW:
			{
				partComponent->SetPrimaryTargetPosition(w3D);
				partComponent->SetDepthBias(wCamera3D.X * -1000.0f);
			} break;
			case EEditorTransformGizmo4DPart::MoveXY:
			case EEditorTransformGizmo4DPart::ScaleXY:
			{
				UEditorTransformGizmoBiplaneMesh4D* biplaneMesh = Cast<UEditorTransformGizmoBiplaneMesh4D>(partComponent);
				biplaneMesh->SetPrimaryTargetPosition(x3D);
				biplaneMesh->SetSecondaryTargetPosition(y3D);
				biplaneMesh->SetDepthBias((xCamera3D.X + yCamera3D.X) * -375.0f);
			} break;
			case EEditorTransformGizmo4DPart::MoveXZ:
			case EEditorTransformGizmo4DPart::ScaleXZ:
			{
				UEditorTransformGizmoBiplaneMesh4D* biplaneMesh = Cast<UEditorTransformGizmoBiplaneMesh4D>(partComponent);
				biplaneMesh->SetPrimaryTargetPosition(x3D);
				biplaneMesh->SetSecondaryTargetPosition(z3D);
				biplaneMesh->SetDepthBias((xCamera3D.X + zCamera3D.X) * -375.0f);
			} break;
			case EEditorTransformGizmo4DPart::MoveXW:
			case EEditorTransformGizmo4DPart::ScaleXW:
			{
				UEditorTransformGizmoBiplaneMesh4D* biplaneMesh = Cast<UEditorTransformGizmoBiplaneMesh4D>(partComponent);
				biplaneMesh->SetPrimaryTargetPosition(x3D);
				biplaneMesh->SetSecondaryTargetPosition(w3D);
				biplaneMesh->SetDepthBias((xCamera3D.X + wCamera3D.X) * -375.0f);
			} break;
			case EEditorTransformGizmo4DPart::MoveYZ:
			case EEditorTransformGizmo4DPart::ScaleYZ:
			{
				UEditorTransformGizmoBiplaneMesh4D* biplaneMesh = Cast<UEditorTransformGizmoBiplaneMesh4D>(partComponent);
				biplaneMesh->SetPrimaryTargetPosition(y3D);
				biplaneMesh->SetSecondaryTargetPosition(z3D);
				biplaneMesh->SetDepthBias((yCamera3D.X + zCamera3D.X) * -375.0f);
			} break;
			case EEditorTransformGizmo4DPart::MoveYW:
			case EEditorTransformGizmo4DPart::ScaleYW:
			{
				UEditorTransformGizmoBiplaneMesh4D* biplaneMesh = Cast<UEditorTransformGizmoBiplaneMesh4D>(partComponent);
				biplaneMesh->SetPrimaryTargetPosition(y3D);
				biplaneMesh->SetSecondaryTargetPosition(w3D);
				biplaneMesh->SetDepthBias((yCamera3D.X + wCamera3D.X) * -375.0f);
			} break;
			case EEditorTransformGizmo4DPart::MoveZW:
			case EEditorTransformGizmo4DPart::ScaleZW:
			{
				UEditorTransformGizmoBiplaneMesh4D* biplaneMesh = Cast<UEditorTransformGizmoBiplaneMesh4D>(partComponent);
				biplaneMesh->SetPrimaryTargetPosition(z3D);
				biplaneMesh->SetSecondaryTargetPosition(w3D);
				biplaneMesh->SetDepthBias((zCamera3D.X + wCamera3D.X) * -375.0f);
			} break;
			case EEditorTransformGizmo4DPart::RotateXY:
			{
				UEditorTransformGizmoBivectorRingMesh4D* bivectorRingMesh = Cast<UEditorTransformGizmoBivectorRingMesh4D>(partComponent);
				bivectorRingMesh->SetPrimaryTargetPosition(x3D);
				bivectorRingMesh->SetSecondaryTargetPosition(y3D);
				bivectorRingMesh->SetDepthBias((xCamera3D.X + yCamera3D.X) * -10.0f);
			} break;
			case EEditorTransformGizmo4DPart::RotateXZ:
			{
				UEditorTransformGizmoBivectorRingMesh4D* bivectorRingMesh = Cast<UEditorTransformGizmoBivectorRingMesh4D>(partComponent);
				bivectorRingMesh->SetPrimaryTargetPosition(x3D);
				bivectorRingMesh->SetSecondaryTargetPosition(z3D);
				bivectorRingMesh->SetDepthBias((xCamera3D.X + zCamera3D.X) * -10.0f);
			} break;
			case EEditorTransformGizmo4DPart::RotateXW:
			{
				UEditorTransformGizmoBivectorRingMesh4D* bivectorRingMesh = Cast<UEditorTransformGizmoBivectorRingMesh4D>(partComponent);
				bivectorRingMesh->SetPrimaryTargetPosition(x3D);
				bivectorRingMesh->SetSecondaryTargetPosition(w3D);
				bivectorRingMesh->SetDepthBias((xCamera3D.X + wCamera3D.X) * -10.0f);
			} break;
			case EEditorTransformGizmo4DPart::RotateYZ:
			{
				UEditorTransformGizmoBivectorRingMesh4D* bivectorRingMesh = Cast<UEditorTransformGizmoBivectorRingMesh4D>(partComponent);
				bivectorRingMesh->SetPrimaryTargetPosition(y3D);
				bivectorRingMesh->SetSecondaryTargetPosition(z3D);
				bivectorRingMesh->SetDepthBias((yCamera3D.X + zCamera3D.X) * -10.0f);
			} break;
			case EEditorTransformGizmo4DPart::RotateYW:
			{
				UEditorTransformGizmoBivectorRingMesh4D* bivectorRingMesh = Cast<UEditorTransformGizmoBivectorRingMesh4D>(partComponent);
				bivectorRingMesh->SetPrimaryTargetPosition(y3D);
				bivectorRingMesh->SetSecondaryTargetPosition(w3D);
				bivectorRingMesh->SetDepthBias((yCamera3D.X + wCamera3D.X) * -10.0f);
			} break;
			case EEditorTransformGizmo4DPart::RotateZW:
			{
				UEditorTransformGizmoBivectorRingMesh4D* bivectorRingMesh = Cast<UEditorTransformGizmoBivectorRingMesh4D>(partComponent);
				bivectorRingMesh->SetPrimaryTargetPosition(z3D);
				bivectorRingMesh->SetSecondaryTargetPosition(w3D);
				bivectorRingMesh->SetDepthBias((zCamera3D.X + wCamera3D.X) * -10.0f);
			} break;
			case EEditorTransformGizmo4DPart::StretchPosX:
			{
			} break;
			case EEditorTransformGizmo4DPart::StretchNegX:
			{
			} break;
			case EEditorTransformGizmo4DPart::StretchPosY:
			{
			} break;
			case EEditorTransformGizmo4DPart::StretchNegY:
			{
			} break;
			case EEditorTransformGizmo4DPart::StretchPosZ:
			{
			} break;
			case EEditorTransformGizmo4DPart::StretchNegZ:
			{
			} break;
			case EEditorTransformGizmo4DPart::StretchPosW:
			{
			} break;
			case EEditorTransformGizmo4DPart::StretchNegW:
			{
			} break;
		}
	}
}

void AEditorGizmoActor4D::SetGizmoTransform4D(const FTransform4D& InNewTransform)
{
	if (!IsValid(_root4DComponent))
	{
		return;
	}
	_root4DComponent->SetLocalTransform4D(InNewTransform.Orthonormalized());
	TryUpdateCameraTransform4D();
	UpdatePartMeshes();
}

void AEditorGizmoActor4D::SetPartHighlighted(EEditorTransformGizmo4DPart InPart, bool bInHighlight)
{
	if (!_gizmoParts4D.Contains(InPart)) return;
	UEditorTransformGizmoBaseMesh4D* partComponent = _gizmoParts4D[InPart];
	partComponent->SetHighlighted(bInHighlight);
}

AEditorGizmoActor4D::AEditorGizmoActor4D() : Super()
{
	_root4DComponent = CreateDefaultSubobject<UScene4DComponent>(TEXT("_root4DComponent"));
	SetRootComponent(_root4DComponent);
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
}
