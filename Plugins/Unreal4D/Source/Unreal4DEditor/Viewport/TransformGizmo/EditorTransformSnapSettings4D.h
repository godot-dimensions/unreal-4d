#pragma once

#include "Unreal4DEditorGizmoDefines.h"

#include "../../../Unreal4DCore/Math/Transform4D.h"

#include "EditorTransformSnapSettings4D.generated.h"

struct FPropertyChangedEvent;

UENUM()
enum class EEditorTransformSnapCameraDistanceMode4D : uint8 {
	Constant,
	DynamicPowerOfTen,
	DynamicPowerOfTwo,
};

// In Godot 4D, see EditorTransformSnapSettings4D.
UCLASS(Transient, MinimalAPI)
class UEditorTransformSnapSettings4D : public UObject
{
	GENERATED_BODY()

private:
	double _positionSnapDistanceCentimeters = 10.0;
	double _rotationSnapAngleRadians = UNREAL4D_TAU * (15.0 / 360.0); // 15 degrees in radians.
	double _scaleSnapRatio = 0.1;
	EEditorTransformSnapCameraDistanceMode4D _positionSnapCameraDistanceMode = EEditorTransformSnapCameraDistanceMode4D::Constant;
	bool _bPositionSnapInvertKeybind = false;
	bool _bRotationSnapInvertKeybind = false;
	bool _bScaleSnapInvertKeybind = false;
	FDelegateHandle _editorViewportSettingsChangedHandle;

	static bool IsSnapModifierPressedInEditor();
	void HandleEditorViewportSettingsChanged(UObject* InObject, FPropertyChangedEvent& InPropertyChangedEvent);
	FVector4 SnapPosition(const FVector4& InPosition, const bool bInIsModifierPressed) const;
	FMatrix SnapRotation(const FMatrix& InBasis, const bool bInIsModifierPressed) const;
	FTransform4D SnapScale(const FTransform4D& InTransform, const bool bInIsModifierPressed) const;

public:
	bool bSnapFinalValues = false;
	double CameraDistance = 4.0;

	void PostInitProperties() override;
	void BeginDestroy() override;

	void ReadFromEditorViewportSettings();
	void ConfigureGridSnapping(const bool bInPositionGridEnabled, const bool bInRotationGridEnabled);

	FTransform4D SnapSingleTransform(const FTransform4D& InTransform) const;
	FTransform4D SnapTransformChange(const FTransform4D& InOldTransform, const FTransform4D& InTransformChange) const;
};
