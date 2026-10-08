#include "EditorViewportRotationWidget4DManager.h"

#include "../../Unreal4DEditorMode.h"
#include "EditorViewportRotationWidget4D.h"

#include "BlueprintEditor.h"
#include "BlueprintEditorTabs.h"
#include "ILevelEditor.h"
#include "SLevelViewport.h"
#include "SSCSEditorViewport.h"

TSharedRef<SWidget> FEditorViewportRotationWidget4DManager::AttachToSlateEditor(const TSharedPtr<SEditorViewport>& InSlateEditor)
{
	ensure(InSlateEditor.IsValid());
	FEditorViewportClient* editorViewportClient = InSlateEditor->GetViewportClient().Get();
	FEditorModeTools* viewportModeTools = editorViewportClient->GetModeTools();
	FUnreal4DEditorMode* editorMode4D = static_cast<FUnreal4DEditorMode*>(viewportModeTools->GetActiveMode(FUnreal4DEditorMode::ModeID));
	// Slate uses a special construction syntax using the SNew macro.
	TSharedRef<SEditorViewportRotationWidget4D> rotationWidget = SNew(SEditorViewportRotationWidget4D)
		.SlateViewport(InSlateEditor.Get());
	rotationWidget->Setup(editorViewportClient, editorMode4D);
	// Disable Unreal's existing 3D rotation axes widget.
	editorViewportClient->bDrawAxes = false;
	// Add the rotation widget inside of another widget for padding.
	TSharedRef<SWidget> paddingWidget = SNew(SOverlay)
		+ SOverlay::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Top)
		.Padding(FMargin(8.0f))
		[
			rotationWidget
		];
	return paddingWidget;
}

void FEditorViewportRotationWidget4DManager::AttachToLevelEditor(const TSharedPtr<ILevelEditor>& InLevelEditor)
{
	if (!InLevelEditor.IsValid())
	{
		return;
	}
	for (const TSharedPtr<SLevelViewport>& levelViewport : InLevelEditor->GetViewports())
	{
		const TSharedPtr<SEditorViewport>& slateEditorViewport = StaticCastSharedPtr<SEditorViewport>(levelViewport);
		TSharedRef<SWidget> paddingWidget = AttachToSlateEditor(slateEditorViewport);
		levelViewport->AddOverlayWidget(paddingWidget, INDEX_NONE);
		// Keep track of the binding for later cleanup.
		FEditorLevelViewportBinding4D& binding = LevelBindings.AddDefaulted_GetRef();
		binding.LevelViewport = levelViewport;
		binding.PaddingWidget = paddingWidget;
	}
}

void FEditorViewportRotationWidget4DManager::AttachToBlueprintAssetEditor(const TSharedPtr<FBlueprintEditor>& InBlueprintEditor)
{
	if (!InBlueprintEditor.IsValid() || !InBlueprintEditor->CanAccessComponentsMode())
    {
        return;
    }
	// The Blueprint SCS editor is not ready on the first tick, so we have to wait one tick to attach the widget.
	GEditor->GetTimerManager()->SetTimerForNextTick([this, InBlueprintEditor]()
	{
		// Though unlikely, it is possible that the blueprint editor is no longer valid.
		if (!InBlueprintEditor.IsValid())
		{
			ensure(false);
			return;
		}
		InBlueprintEditor->GetTabManager()->TryInvokeTab(FBlueprintEditorTabs::SCSViewportID);
		const TSharedPtr<SSCSEditorViewport>& blueprintViewport = InBlueprintEditor->GetSubobjectViewport();
		ensure(blueprintViewport.IsValid());
		TSharedRef<SWidget> paddingWidget = AttachToSlateEditor(StaticCastSharedPtr<SEditorViewport>(blueprintViewport));
		blueprintViewport->AddOverlayWidget(paddingWidget, INDEX_NONE);
		// Keep track of the binding for later cleanup.
		FEditorBlueprintViewportBinding4D& binding = BlueprintBindings.AddDefaulted_GetRef();
		binding.BlueprintViewport = blueprintViewport;
		binding.PaddingWidget = paddingWidget;
	});
}

void FEditorViewportRotationWidget4DManager::DetachFromAllEditorViewports()
{
	for (FEditorLevelViewportBinding4D& levelBinding : LevelBindings)
	{
		// Pin turns a TWeakPtr into a strong TSharedPtr if the object is still valid.
		const TSharedPtr<SLevelViewport> levelViewport = levelBinding.LevelViewport.Pin();
		if (levelViewport.IsValid() && levelBinding.PaddingWidget.IsValid())
		{
			levelViewport->RemoveOverlayWidget(levelBinding.PaddingWidget.ToSharedRef());
		}
	}
	LevelBindings.Empty();
	for (FEditorBlueprintViewportBinding4D& blueprintBinding : BlueprintBindings)
	{
		const TSharedPtr<SSCSEditorViewport> blueprintViewport = blueprintBinding.BlueprintViewport.Pin();
		if (blueprintViewport.IsValid() && blueprintBinding.PaddingWidget.IsValid())
		{
			blueprintViewport->RemoveOverlayWidget(blueprintBinding.PaddingWidget.ToSharedRef());
		}
	}
	BlueprintBindings.Empty();
}
