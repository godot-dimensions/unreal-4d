#include "EditorScene4DComponentDetails.h"

#include "../../Unreal4DCore/Scene/Components/Scene4DComponent.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Widgets/Layout/SGridPanel.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "EditorScene4DComponentDetails"

namespace EditorScene4DComponentDetailsLocal
{
	// The code in this namespace creates a 4x4 matrix editor widget for editing FMatrix properties.
	// Disclaimer: Everything in this namespace is AI-generated, but since this is editor-only UI code
	// and nothing else in the codebase depends on it, I don't think it's necessary to comb through it.
	TSharedPtr<IPropertyHandle> GetMatrixElementHandle(TSharedPtr<IPropertyHandle> InMatrixHandle, int32 InRow, int32 InCol)
	{
		if (!InMatrixHandle.IsValid())
		{
			return nullptr;
		}
		// FMatrix has XPlane, YPlane, ZPlane, WPlane as children (rows).
		// Each plane has X, Y, Z, W components (columns).
		static const FName PlaneNames[] = { "XPlane", "YPlane", "ZPlane", "WPlane" };
		static const FName ComponentNames[] = { "X", "Y", "Z", "W" };
		TSharedPtr<IPropertyHandle> PlaneHandle = InMatrixHandle->GetChildHandle(PlaneNames[InRow]);
		if (PlaneHandle.IsValid())
		{
			return PlaneHandle->GetChildHandle(ComponentNames[InCol]);
		}
		return nullptr;
	}

	TSharedRef<SWidget> CreateMatrixElementWidget(TSharedPtr<IPropertyHandle> InMatrixHandle, int32 InRow, int32 InCol)
	{
		TSharedPtr<IPropertyHandle> ElementHandle = GetMatrixElementHandle(InMatrixHandle, InRow, InCol);
		if (!ElementHandle.IsValid())
		{
			return SNew(STextBlock).Text(LOCTEXT("InvalidElement", "?"));
		}
		// Create a numeric entry box bound to the property.
		return SNew(SBox)
			.MinDesiredWidth(60.0f)
			[
				SNew(SNumericEntryBox<double>)
				.AllowSpin(true)
				.MinSliderValue(-10.0)
				.MaxSliderValue(10.0)
				.SliderExponent(1.0f)
				.Delta(0.0)
				.LinearDeltaSensitivity(50)
				.ShiftMultiplier(0.1)
				.SupportDynamicSliderMinValue(true)
				.SupportDynamicSliderMaxValue(true)
				.Value_Lambda([ElementHandle]() -> TOptional<double>
				{
					double value = 0.0;
					if (ElementHandle.IsValid() && ElementHandle->GetValue(value) == FPropertyAccess::Success)
					{
						return value;
					}
					return TOptional<double>();
				})
				.OnValueCommitted_Lambda([ElementHandle](double NewValue, ETextCommit::Type CommitType)
				{
					if (ElementHandle.IsValid())
					{
						ElementHandle->SetValue(NewValue);
					}
				})
				.OnValueChanged_Lambda([ElementHandle](double NewValue)
				{
					if (ElementHandle.IsValid())
					{
						ElementHandle->SetValue(NewValue, EPropertyValueSetFlags::InteractiveChange);
					}
				})
			];
	}

	TSharedRef<SWidget> CreateMatrix4x4Widget(TSharedPtr<IPropertyHandle> InMatrixHandle)
	{
		TSharedRef<SGridPanel> gridPanel = SNew(SGridPanel);
		// Add column headers.
		static const TCHAR* colHeaders[] = { TEXT("X"), TEXT("Y"), TEXT("Z"), TEXT("W") };
		for (int32 col = 0; col < 4; col++)
		{
			gridPanel->AddSlot(col + 1, 0)
				.HAlign(HAlign_Center)
				.Padding(2.0f)
				[
					SNew(STextBlock)
					.Text(FText::FromString(colHeaders[col]))
					.Font(IDetailLayoutBuilder::GetDetailFontBold())
				];
		}
		// Add row headers and matrix elements.
		static const TCHAR* rowHeaders[] = { TEXT("X"), TEXT("Y"), TEXT("Z"), TEXT("W") };
		for (int32 row = 0; row < 4; row++)
		{
			// Row header.
			gridPanel->AddSlot(0, row + 1)
				.HAlign(HAlign_Right)
				.VAlign(VAlign_Center)
				.Padding(2.0f, 2.0f, 8.0f, 2.0f)
				[
					SNew(STextBlock)
					.Text(FText::FromString(rowHeaders[row]))
					.Font(IDetailLayoutBuilder::GetDetailFontBold())
				];
			// Matrix elements for this row.
			for (int32 col = 0; col < 4; col++)
			{
				gridPanel->AddSlot(col + 1, row + 1).Padding(2.0f)[CreateMatrixElementWidget(InMatrixHandle, row, col)];
			}
		}
		return SNew(SBox).Padding(FMargin(0.0f, 4.0f))[gridPanel];
	}
}

void FEditorScene4DComponentDetails::CustomizeDetails(IDetailLayoutBuilder& InDetailBuilder)
{
	IDetailCategoryBuilder& cat = InDetailBuilder.EditCategory("Transform 4D");
	cat.SetSortOrder(-1000);
	LocalPositionProp = InDetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UScene4DComponent, _localPosition4D));
	RotationModeProp = InDetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UScene4DComponent, _rotationEditMode));
	LocalBasisProp = InDetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UScene4DComponent, _localBasis4D));
	LocalRotationDegreesProp = InDetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UScene4DComponent, _localRotationDegrees4D));
	LocalScaleProp = InDetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UScene4DComponent, _localScale4D));
	LocalUniformScaleProp = InDetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UScene4DComponent, _localUniformScale4D));
	// Hide all rotation/scale related properties first, then selectively show.
	InDetailBuilder.HideProperty(LocalPositionProp);
	InDetailBuilder.HideProperty(RotationModeProp);
	InDetailBuilder.HideProperty(LocalBasisProp);
	InDetailBuilder.HideProperty(LocalRotationDegreesProp);
	InDetailBuilder.HideProperty(LocalScaleProp);
	InDetailBuilder.HideProperty(LocalUniformScaleProp);
	cat.AddProperty(LocalPositionProp);
	cat.AddProperty(RotationModeProp);
	uint8 rotationModeValue = 0;
	if (RotationModeProp->GetValue(rotationModeValue) == FPropertyAccess::Success)
	{
		const ERotationEditMode4D mode = static_cast<ERotationEditMode4D>(rotationModeValue);
		if (mode == ERotationEditMode4D::Basis4D)
		{
			// Add the matrix as a custom 4x4 grid widget instead of the default Location/Rotation/Scale.
			cat.AddCustomRow(LOCTEXT("LocalBasis4D", "Local Basis 4D"))
				.NameContent()[LocalBasisProp->CreatePropertyNameWidget()]
				.ValueContent()
				.MinDesiredWidth(300.0f)[EditorScene4DComponentDetailsLocal::CreateMatrix4x4Widget(LocalBasisProp)];
		}
		else if (mode == ERotationEditMode4D::Euler4D)
		{
			cat.AddProperty(LocalRotationDegreesProp);
			cat.AddProperty(LocalScaleProp);
		}
		else if (mode == ERotationEditMode4D::Euler4D_Uniform_Scale)
		{
			cat.AddProperty(LocalRotationDegreesProp);
			cat.AddProperty(LocalUniformScaleProp);
		}
	}
	// Bind change callback to refresh the details.
	RotationModeProp->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&InDetailBuilder, this]()
	{
		InDetailBuilder.ForceRefreshDetails();
	}));
}

#undef LOCTEXT_NAMESPACE
