#pragma once

#include "../Unreal4DEditorDefines.h"

#include "IDetailCustomization.h"

// Custom inspector code for 4D cameras.
class FEditorCamera4DComponentDetails : public IDetailCustomization
{
private:
	TSharedPtr<IPropertyHandle> DepthFadeModeProp;
	TSharedPtr<IPropertyHandle> DepthFadeStartProp;

	TSharedPtr<IPropertyHandle> PerpFadeModeProp;
	TSharedPtr<IPropertyHandle> PerpFadeColorNegativeProp;
	TSharedPtr<IPropertyHandle> PerpFadeColorPositiveProp;
	TSharedPtr<IPropertyHandle> PerpFadeDistanceProp;
	TSharedPtr<IPropertyHandle> PerpFadeSlopeProp;

public:
	static TSharedRef<IDetailCustomization> MakeInstance()
	{
		return MakeShared<FEditorCamera4DComponentDetails>();
	}

	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
};
