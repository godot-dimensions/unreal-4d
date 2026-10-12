#pragma once

#include "Unreal4DEditorDefines.h"

#include "EdMode.h"
#include "Elements/Framework/TypedElementHandle.h"

#include "../Unreal4DCore/Math/Transform4D.h"
#include "../Unreal4DCore/Math/VectorFuncs4D.h"

struct FEditorViewportCameraTransform4D
{
	FTransform4D ParentOfGroundTransform3Dtomakeit4D = FTransform4D::Identity();
	FTransform4D GroundTransform4D = FTransform4D::Identity();
	FTransform GroundTransform3D = FTransform::Identity;
	double PitchDegrees = 0.0;
};

// Unreal Editor Mode which provides 4D camera controls in the 3D editor viewport.
// Note that this is using the "older" FEdMode class, not the "newer" UEdMode class,
// because UEdMode has some limitations like not being able to receive and block
// input events from the editor viewport, which is necessary for camera controls.
class FUnreal4DEditorMode : public FEdMode
{
	static constexpr double WAxisMoveSpeed = 500.0;

	// Purposefully typed with `FEditorViewportClient*` for `.Contains()` checks.
	TSet<FEditorViewportClient*> _levelViewportClientsWithRegisteredSignals;

	TMap<FEditorViewportClient*, FEditorViewportCameraTransform4D> _viewportCameraTransforms;
	FEditorViewportCameraTransform4D& GetOrAddViewportCameraTransform(FEditorViewportClient* InViewportClient);

	// See the comment above its usage in `FUnreal4DEditorMode::Tick`.
	int32 _pendingRefocus3DFrames = 0;

	// NOTE: Unreal's built-in viewport camera is hard-coded to use right-click, it's not configurable.
	bool bRightMouseDown = false;
	bool bRotate4DModifierDown = false;
	bool bWAxisNegativeDown = false;
	bool bWAxisPositiveDown = false;

	FORCEINLINE bool ShouldMouseMovementRotateIn4D() const
	{
		return bRightMouseDown && bRotate4DModifierDown;
	}

	FORCEINLINE double GetLocalWAxisMovement() const
	{
		if (bRightMouseDown)
		{
			return (bWAxisPositiveDown ? 1.0 : 0.0) - (bWAxisNegativeDown ? 1.0 : 0.0);
		}
		return 0.0;
	}

	static bool DoesKeyMatchCommand(const FKey& InKey, FViewport* InViewport, const TSharedPtr<FUICommandInfo>& InCommand);
	static bool IsCommandCurrentlyDown(FViewport* InViewport, const TSharedPtr<FUICommandInfo>& InCommand);

	void UpdateRenderingServerWithEditorCameraTransform(FEditorViewportClient* InViewportClient);

public:
	static const FEditorModeID ModeID;

	static FLinearColor GetAxisColor(const EAxis4D InAxis);

	// FEdMode interface.
	virtual void Exit() override;
	virtual void Tick(FEditorViewportClient* InViewportClient, float InDeltaTime) override;
	virtual bool InputKey(FEditorViewportClient* ViewportClient, FViewport* InViewport, FKey InKey, EInputEvent InEvent) override;
	virtual bool InputAxis(FEditorViewportClient* InViewportClient, FViewport* InViewport, int32 InControllerId, FKey InKey, float InDelta, float InDeltaTime) override;
	virtual bool IsCompatibleWith(FEditorModeID InOtherModeID) const override { return true; }
	virtual bool GetOverrideCursorVisibility(bool& bOutWantsOverride, bool& bOutHardwareCursorVisible, bool bInSoftwareCursorVisible) const override;
	virtual bool ReceivedFocus(FEditorViewportClient* InViewportClient, FViewport* InViewport) override; // There is also LostFocus if needed.

	// Editor camera transforms.
	FTransform4D GetFullEditorCameraTransform4D(FEditorViewportClient* InViewportClient);
	FTransform4D GetEditorCameraGroundTransform4D(FEditorViewportClient* InViewportClient);
	FTransform GetEditorCameraGroundTransform3D(FEditorViewportClient* InViewportClient);
	FVector4 GetEditorCameraPosition4D(FEditorViewportClient* InViewportClient);

	void RotateEditorCameraLocal4D(FEditorViewportClient* InViewportClient, const FMatrix& InLocalRotationBasis4D);
	void SetEditorCameraPosition4D(FEditorViewportClient* InViewportClient, const FVector4& InNewPosition);
	void SetEditorCameraGroundBasis4D(FEditorViewportClient* InViewportClient, const FMatrix& InNewGroundBasis);
	void SetEditorCameraRotation4Dand3D(FEditorViewportClient* InViewportClient, const FMatrix& InNewGroundBasis, const double InYawDegrees3D, const double InPitchDegrees3D);

	// Handler called when drop elements have their transforms updated during drag or drop operations.
	void OnDropElementTransformUpdated(AActor* InActor, const FTypedElementHandle& InElement, const FTransform& InTransform3D);
	void SyncBlueprint4DTransformsWithArchetypes();
};
