#include "EditorTransform4DDetails.h"

#include "../../Unreal4DCore/Math/Transform4D.h"

#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "IDetailPropertyRow.h"

////////////////////////////////////////////////////////////////////////////////
void FEditorTransform4DCustomization::OnTransformChanged(const TSharedPtr<IPropertyHandle> InPropertyHandle)
{
	// TODO: Add support for multi-edit.

	void* transform4DPtr = nullptr;
	if (InPropertyHandle->GetValueData(transform4DPtr) == FPropertyAccess::Success)
	{
		// Recompose transform from scale/rotation

		FTransform4D transform4D = *(FTransform4D*)(transform4DPtr);
		transform4D.Basis = FMatrix::Identity;
		transform4D.ScaleLocal(Scale4D);
		transform4D.RotateLocal(Rotation4D.DegToRad());

		FString transform4DString;
		FTransform4D::StaticStruct()->ExportText(transform4DString, &transform4D, nullptr, nullptr, PPF_None, nullptr);

		InPropertyHandle->SetValueFromFormattedString(transform4DString);
	}
}

////////////////////////////////////////////////////////////////////////////////
void FEditorTransform4DCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> InPropertyHandle, FDetailWidgetRow& InHeaderRow, IPropertyTypeCustomizationUtils& InCustomizationUtils)
{
	// TODO: Add support for multi-edit.

	void* Transform4DPtr = nullptr;
	if (InPropertyHandle->GetValueData(Transform4DPtr) == FPropertyAccess::Success)
	{
		// Initialize scale/rotation from transform

		const FTransform4D& transform4D = *(FTransform4D*)(Transform4DPtr);
		Scale4D = transform4D.GetScale();
		Rotation4D = transform4D.GetRotationDegrees();
	}

	ScaleStruct = MakeShareable<FStructOnScope>(new FStructOnScope(TBaseStructure<FVector4>::Get(), (uint8*)(&Scale4D)));
	RotationStruct = MakeShareable<FStructOnScope>(new FStructOnScope(FEuler4D::StaticStruct(), (uint8*)(&Rotation4D)));

	InHeaderRow
	.NameContent()
	[
		InPropertyHandle->CreatePropertyNameWidget()
	]
	.ValueContent()
	[
		InPropertyHandle->CreatePropertyValueWidget()
	];
}

////////////////////////////////////////////////////////////////////////////////
void FEditorTransform4DCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> InPropertyHandle, IDetailChildrenBuilder& InChildBuilder, IPropertyTypeCustomizationUtils& InCustomizationUtils)
{
	// Origin
	InChildBuilder.AddProperty(InPropertyHandle->GetChildHandle(TEXT("Origin")).ToSharedRef());

	const FSimpleDelegate OnTransformChanged = FSimpleDelegate::CreateSP(this, &FEditorTransform4DCustomization::OnTransformChanged, InPropertyHandle.ToSharedPtr());

	// Rotation
	RotationRow = InChildBuilder.AddExternalStructure(RotationStruct.ToSharedRef());
	RotationRow->DisplayName(NSLOCTEXT("Transform4DCustomization", "Rotation", "Rotation"));
	RotationRow->GetPropertyHandle()->SetOnPropertyValueChanged(OnTransformChanged);
	RotationRow->GetPropertyHandle()->SetOnChildPropertyValueChanged(OnTransformChanged);

	// Scale
	ScaleRow = InChildBuilder.AddExternalStructure(ScaleStruct.ToSharedRef());
	ScaleRow->DisplayName(NSLOCTEXT("Transform4DCustomization", "Scale", "Scale"));
	ScaleRow->GetPropertyHandle()->SetOnPropertyValueChanged(OnTransformChanged);
	ScaleRow->GetPropertyHandle()->SetOnChildPropertyValueChanged(OnTransformChanged);
}
