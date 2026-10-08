#include "EditorCamera4DComponentDetails.h"

#include "../../Unreal4DCore/Scene/Components/Camera4DComponent.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"

void FEditorCamera4DComponentDetails::CustomizeDetails(IDetailLayoutBuilder& InDetailBuilder)
{
	IDetailCategoryBuilder& cat = InDetailBuilder.EditCategory("Camera 4D");
	cat.SetSortOrder(-900);
	DepthFadeModeProp = InDetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UCamera4DComponent, DepthFadeMode));
	DepthFadeStartProp = InDetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UCamera4DComponent, DepthFadeStart));

	PerpFadeModeProp = InDetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UCamera4DComponent, PerpFadeMode));
	PerpFadeColorNegativeProp = InDetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UCamera4DComponent, PerpFadeColorNegative));
	PerpFadeColorPositiveProp = InDetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UCamera4DComponent, PerpFadeColorPositive));
	PerpFadeDistanceProp = InDetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UCamera4DComponent, PerpFadeDistance));
	PerpFadeSlopeProp = InDetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UCamera4DComponent, PerpFadeSlope));
	// Hide all properties first, then selectively show.
	InDetailBuilder.HideProperty(DepthFadeModeProp);
	InDetailBuilder.HideProperty(DepthFadeStartProp);
	InDetailBuilder.HideProperty(PerpFadeModeProp);
	InDetailBuilder.HideProperty(PerpFadeColorNegativeProp);
	InDetailBuilder.HideProperty(PerpFadeColorPositiveProp);
	InDetailBuilder.HideProperty(PerpFadeDistanceProp);
	InDetailBuilder.HideProperty(PerpFadeSlopeProp);
	cat.AddProperty(DepthFadeModeProp);
	uint8 depthFadeModeValue = 0;
	if (DepthFadeModeProp->GetValue(depthFadeModeValue) == FPropertyAccess::Success)
	{
		const ECamera4DDepthFadeMode mode = static_cast<ECamera4DDepthFadeMode>(depthFadeModeValue);
		if (mode != ECamera4DDepthFadeMode::Disabled)
		{
			cat.AddProperty(DepthFadeStartProp);
		}
	}
	cat.AddProperty(PerpFadeModeProp);
	uint8 perpFadeModeValue = 0;
	if (PerpFadeModeProp->GetValue(perpFadeModeValue) == FPropertyAccess::Success)
	{
		const ECamera4DPerpFadeMode mode = static_cast<ECamera4DPerpFadeMode>(perpFadeModeValue);
		if (mode == ECamera4DPerpFadeMode::HueShift || mode == ECamera4DPerpFadeMode::Transparency_HueShift)
		{
			cat.AddProperty(PerpFadeColorNegativeProp);
			cat.AddProperty(PerpFadeColorPositiveProp);
		}
		if (mode != ECamera4DPerpFadeMode::Disabled)
		{
			cat.AddProperty(PerpFadeDistanceProp);
			cat.AddProperty(PerpFadeSlopeProp);
		}
	}
	// Bind change callbacks to refresh the details.
	DepthFadeModeProp->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&InDetailBuilder, this]()
	{
		InDetailBuilder.ForceRefreshDetails();
	}));
	PerpFadeModeProp->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&InDetailBuilder, this]()
	{
		InDetailBuilder.ForceRefreshDetails();
	}));
}
