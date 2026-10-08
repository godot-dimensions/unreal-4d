#pragma once

#include "../Unreal4DEditorDefines.h"

#include "IDetailCustomization.h"

// Custom inspector code for 4D meshes.
// Currently it just sets the category priority, but this should be expanded later.
class FEditorMesh4DComponentDetails : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance()
	{
		return MakeShared<FEditorMesh4DComponentDetails>();
	}

	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
};
