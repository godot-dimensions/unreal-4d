#pragma once

class IDetailPropertyRow;

//------------------------------------------------------------------------------

#include "../../Unreal4DCore/Math/Euler4D.h"

#include "IPropertyTypeCustomization.h"
#include "UObject/StructOnScope.h"

//______________________________________________________________________________
class FEditorTransform4DCustomization
	: public IPropertyTypeCustomization
{

//==============================================================================
public:

	////////////////////////////////////////////////////////////////////////////////
	static TSharedRef<IPropertyTypeCustomization> MakeInstance()
	{
		return MakeShared<FEditorTransform4DCustomization>();
	}

//==============================================================================
private:

	FVector4 Scale4D = FVector4::One();
	TSharedPtr<FStructOnScope> ScaleStruct;
	IDetailPropertyRow* ScaleRow = nullptr;

	FEuler4D Rotation4D;
	TSharedPtr<FStructOnScope> RotationStruct;
	IDetailPropertyRow* RotationRow = nullptr;

//==============================================================================
private:

	void OnTransformChanged(const TSharedPtr<IPropertyHandle> InPropertyHandle);

//==============================================================================
protected:

	//~ IPropertyTypeCustomization BEGIN

	void CustomizeHeader(TSharedRef<IPropertyHandle> InPropertyHandle, FDetailWidgetRow& InHeaderRow, IPropertyTypeCustomizationUtils& InCustomizationUtils) override;
	void CustomizeChildren(TSharedRef<IPropertyHandle> InPropertyHandle, IDetailChildrenBuilder& InChildBuilder, IPropertyTypeCustomizationUtils& InCustomizationUtils) override;

	//~ IPropertyTypeCustomization END
};
