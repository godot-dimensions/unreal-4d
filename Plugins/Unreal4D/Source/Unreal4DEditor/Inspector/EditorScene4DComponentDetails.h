#pragma once

#include "../Unreal4DEditorDefines.h"

#include "IDetailCustomization.h"

// Custom inspector code for UScene4DComponent, which provides a custom
// UI for editing the 4D transform properties of the component.
// Conditionally shows basis/rotation/scale properties based on a dropdown.
class FEditorScene4DComponentDetails : public IDetailCustomization
{
private:
	TSharedPtr<IPropertyHandle> LocalPositionProp;
	TSharedPtr<IPropertyHandle> RotationModeProp;
	TSharedPtr<IPropertyHandle> LocalBasisProp;
	TSharedPtr<IPropertyHandle> LocalRotationDegreesProp;
	TSharedPtr<IPropertyHandle> LocalScaleProp;
	TSharedPtr<IPropertyHandle> LocalUniformScaleProp;

public:
	static TSharedRef<IDetailCustomization> MakeInstance()
	{
		return MakeShared<FEditorScene4DComponentDetails>();
	}

	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
};
