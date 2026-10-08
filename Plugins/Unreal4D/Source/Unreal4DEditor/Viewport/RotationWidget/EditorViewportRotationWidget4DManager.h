#pragma once

#include "../../Unreal4DEditorDefines.h"

class FBlueprintEditor;
class FUnreal4DEditorMode;
class ILevelEditor;
class SLevelViewport;
class SSCSEditorViewport;
class SWidget;

// Separate manager type for instantiation inside of the editor module, to avoid over-reliance
// on static variables inside the widget, or filling the editor module with too much stuff.
struct FEditorViewportRotationWidget4DManager final
{
	struct FEditorLevelViewportBinding4D
	{
		TWeakPtr<SLevelViewport> LevelViewport;
		TSharedPtr<SWidget> PaddingWidget;
	};

	struct FEditorBlueprintViewportBinding4D
	{
		TWeakPtr<SSCSEditorViewport> BlueprintViewport;
		TSharedPtr<SWidget> PaddingWidget;
	};

	TArray<FEditorLevelViewportBinding4D> LevelBindings;
	TArray<FEditorBlueprintViewportBinding4D> BlueprintBindings;

	TSharedRef<SWidget> AttachToSlateEditor(const TSharedPtr<SEditorViewport>& InSlateEditor);

public:
	void AttachToLevelEditor(const TSharedPtr<ILevelEditor>& InLevelEditor);
	void AttachToBlueprintAssetEditor(const TSharedPtr<FBlueprintEditor>& InBlueprintEditor);
	void DetachFromAllEditorViewports();
};
