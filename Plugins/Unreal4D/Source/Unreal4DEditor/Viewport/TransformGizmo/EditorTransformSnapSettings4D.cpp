#include "EditorTransformSnapSettings4D.h"

#include "../../../Unreal4DCore/Math/Basis4D.h"
#include "../../../Unreal4DCore/Math/Euler4D.h"
#include "../../../Unreal4DCore/Math/VectorFuncs4D.h"

#include "Editor.h"
#include "Framework/Application/SlateApplication.h"
#include "Settings/LevelEditorViewportSettings.h"

void UEditorTransformSnapSettings4D::PostInitProperties()
{
	Super::PostInitProperties();
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		return;
	}
	ReadFromEditorViewportSettings();
	if (!_editorViewportSettingsChangedHandle.IsValid())
	{
		_editorViewportSettingsChangedHandle = FCoreUObjectDelegates::OnObjectPropertyChanged.AddUObject(
			this,
			&UEditorTransformSnapSettings4D::HandleEditorViewportSettingsChanged
		);
	}
}

void UEditorTransformSnapSettings4D::BeginDestroy()
{
	if (_editorViewportSettingsChangedHandle.IsValid())
	{
		FCoreUObjectDelegates::OnObjectPropertyChanged.Remove(_editorViewportSettingsChangedHandle);
		_editorViewportSettingsChangedHandle.Reset();
	}
	Super::BeginDestroy();
}

void UEditorTransformSnapSettings4D::HandleEditorViewportSettingsChanged(UObject* InObject, FPropertyChangedEvent& InPropertyChangedEvent)
{
	if (InObject == GetMutableDefault<ULevelEditorViewportSettings>())
	{
		ReadFromEditorViewportSettings();
	}
}

void UEditorTransformSnapSettings4D::ReadFromEditorViewportSettings()
{
	const ULevelEditorViewportSettings* viewportSettings = GetDefault<ULevelEditorViewportSettings>();
	if (!viewportSettings || !GEditor)
	{
		return;
	}
	_bPositionSnapInvertKeybind = viewportSettings->GridEnabled;
	_bRotationSnapInvertKeybind = viewportSettings->RotGridEnabled;
	_bScaleSnapInvertKeybind = viewportSettings->SnapScaleEnabled;
	_positionSnapDistanceCentimeters = GEditor->GetGridSize();
	const FRotator rotGridSize = GEditor->GetRotGridSize();
	_rotationSnapAngleRadians = FMath::DegreesToRadians((double)rotGridSize.Pitch);
	_scaleSnapRatio = GEditor->GetScaleGridSize();
}

bool UEditorTransformSnapSettings4D::IsSnapModifierPressedInEditor()
{
    if (!FSlateApplication::IsInitialized())
    {
        return false;
    }
    const FModifierKeysState mods = FSlateApplication::Get().GetModifierKeys();
    return mods.IsControlDown() || mods.IsCommandDown();
}

FVector4 UEditorTransformSnapSettings4D::SnapPosition(const FVector4& InPosition, const bool bInIsModifierPressed) const {
	if (bInIsModifierPressed == _bPositionSnapInvertKeybind) {
		return InPosition;
	}
	double activeSnapDist = _positionSnapDistanceCentimeters;
	if (_positionSnapCameraDistanceMode != EEditorTransformSnapCameraDistanceMode4D::Constant) {
		const double camDistMetersClamped = FMath::Max(FMath::Abs(CameraDistance * 0.01), 0.000001);
		switch (_positionSnapCameraDistanceMode) {
			case EEditorTransformSnapCameraDistanceMode4D::Constant: {
				// Do nothing, already set above.
			} break;
			case EEditorTransformSnapCameraDistanceMode4D::DynamicPowerOfTen: {
				const double distLogTen = FMath::Loge(camDistMetersClamped) / FMath::Loge(10.0);
				activeSnapDist *= FMath::Pow(10.0, FMath::Floor(distLogTen));
			} break;
			case EEditorTransformSnapCameraDistanceMode4D::DynamicPowerOfTwo: {
				const double distLogTwo = FMath::Log2(camDistMetersClamped);
				activeSnapDist *= FMath::Pow(2.0, FMath::Floor(distLogTwo));
			} break;
		}
	}
	return UVectorFuncs4D::Vector4_Snap(InPosition, activeSnapDist);
}

FMatrix UEditorTransformSnapSettings4D::SnapRotation(const FMatrix& InBasis, const bool bInIsModifierPressed) const {
	if (bInIsModifierPressed == _bRotationSnapInvertKeybind) {
		return InBasis;
	}
	FMatrix snappedBasis = InBasis;
	const FVector4 scaleAbs = UBasis4D::Basis4D_GetScaleAbs(InBasis);
	// Rotation inherently can't account for negative scale, so just flip W if needed, and flip it back after.
	const double determinant = InBasis.Determinant();
	if (determinant < 0.0f) {
		FVector4 row3 = UBasis4D::Basis4D_GetRow(snappedBasis, 3);
		UBasis4D::Basis4D_SetRow(snappedBasis, 3, -row3);
	}
	const FEuler4D euler = FEuler4D::RadiansFromBasis(snappedBasis);
	snappedBasis = euler.Snapped(_rotationSnapAngleRadians).RadiansToBasis();
	if (determinant < 0.0f) {
		FVector4 row3 = UBasis4D::Basis4D_GetRow(snappedBasis, 3);
		UBasis4D::Basis4D_SetRow(snappedBasis, 3, -row3);
	}
	return UBasis4D::Basis4D_ScaledLocal(snappedBasis, scaleAbs);
}

FTransform4D UEditorTransformSnapSettings4D::SnapScale(const FTransform4D& InTransform, const bool bInIsModifierPressed) const {
	if (bInIsModifierPressed == _bScaleSnapInvertKeybind) {
		return InTransform;
	}
	const FVector4 scaleAbs = UBasis4D::Basis4D_GetScaleAbs(InTransform.Basis);
	const FVector4 snappedScaleAbs = UVectorFuncs4D::Vector4_Snap(scaleAbs, _scaleSnapRatio);
	return InTransform.ScaledLocal(snappedScaleAbs / scaleAbs);
}

FTransform4D UEditorTransformSnapSettings4D::SnapTransformChange(const FTransform4D& InOldTransform, const FTransform4D& InTransformChange) const {
	if (bSnapFinalValues) {
		// Snap the final values after applying the change.
		return SnapSingleTransform(InOldTransform * InTransformChange);
	}
	// Snap the change itself before applying it. We need to avoid scaling the value output by the snap function,
	// so always snap the final scale value, but snap the change in position and rotation.
	const FVector4 oldLocalScaleAbs = UBasis4D::Basis4D_GetScaleAbs(InOldTransform.Basis);
	const bool bIsModifierPressed = IsSnapModifierPressedInEditor();
	FTransform4D composed = FTransform4D(
			SnapRotation(InTransformChange.Basis, bIsModifierPressed),
			SnapPosition(oldLocalScaleAbs * InTransformChange.Origin, bIsModifierPressed) / oldLocalScaleAbs);
	// Always snap scale after composing.
	return SnapScale(InOldTransform * composed, bIsModifierPressed);
}

FTransform4D UEditorTransformSnapSettings4D::SnapSingleTransform(const FTransform4D& InTransform) const {
	const bool bIsModifierPressed = IsSnapModifierPressedInEditor();
	const FTransform4D posRotSnapped = FTransform4D(
			SnapRotation(InTransform.Basis, bIsModifierPressed),
			SnapPosition(InTransform.Origin, bIsModifierPressed));
	return SnapScale(posRotSnapped, bIsModifierPressed);
}
