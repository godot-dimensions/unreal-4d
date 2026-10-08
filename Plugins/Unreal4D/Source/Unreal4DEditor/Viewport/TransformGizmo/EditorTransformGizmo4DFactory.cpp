#include "EditorTransformGizmo4DFactory.h"

#include "../../../Unreal4DCore/Scene/Components/Scene4DComponent.h"
#include "../../Unreal4DEditorModule.h"
#include "EditorTransformGizmo4D.h"

#include "EditorModeManager.h"
#include "Selection.h"
#include "InteractiveGizmoManager.h"
#include "GameFramework/Actor.h"
#include "Settings/LevelEditorViewportSettings.h"

TMap<FEditorModeTools*, FEditorViewportClient*> UEditorTransformGizmo4DFactory::GViewportClientsByModeTools;

FEditorTransformGizmo4DPartVisibilitySettings UEditorTransformGizmo4DFactory::WidgetModeToVisibilitySettings(const UE::Widget::EWidgetMode InWidgetMode, const EEditorTransform4DKeepMode InKeepMode)
{
	// The default is to have bIsMoveScreenRelativeEnabled and
	// bIsMoveLinearEnabled true and everything else false.
	FEditorTransformGizmo4DPartVisibilitySettings ret = FEditorTransformGizmo4DPartVisibilitySettings();
	switch (InWidgetMode)
	{
		case UE::Widget::WM_None: // Default select mode will give linear move only.
		case UE::Widget::WM_2D: // Not applicable, treat as default.
		case UE::Widget::WM_Max: // Invalid, treat as default.
		{
			// Default settings already set.
		} break;
		case UE::Widget::WM_Translate:
		case UE::Widget::WM_TranslateRotateZ: // Dunno what this is, treat as translate.
		{
			ret.bIsMovePlanarEnabled = true;
		} break;
		case UE::Widget::WM_Rotate:
		{
			ret.bIsMoveScreenRelativeEnabled = false;
			ret.bIsMoveLinearEnabled = false;
			ret.bIsRotatePlanarEnabled = true;
		} break;
		case UE::Widget::WM_Scale:
		{
			ret.bIsMoveScreenRelativeEnabled = false;
			ret.bIsMoveLinearEnabled = false;
			if (InKeepMode != EEditorTransform4DKeepMode::Orthonormal)
			{
				ret.bIsScaleUniformEnabled = true;
				ret.bIsScaleLinearEnabled = true;
				if (InKeepMode != EEditorTransform4DKeepMode::ConformalChangesOnly && InKeepMode != EEditorTransform4DKeepMode::ConformalChangesAndResults)
				{
					ret.bIsScalePlanarEnabled = true;
				}
			}
		} break;
	}
	return ret;
}

void UEditorTransformGizmo4DFactory::RegisterViewportClientForModeTools(FEditorModeTools* InModeTools, FEditorViewportClient* InViewportClient)
{
	if (InModeTools == nullptr)
	{
		return;
	}
	if (InViewportClient == nullptr)
	{
		GViewportClientsByModeTools.Remove(InModeTools);
		return;
	}
	GViewportClientsByModeTools.Add(InModeTools, InViewportClient);
}

FEditorViewportClient* UEditorTransformGizmo4DFactory::FindViewportClientForModeTools(FEditorModeTools* InModeTools)
{
	if (InModeTools == nullptr)
	{
		return nullptr;
	}
	if (FEditorViewportClient** viewportClient = GViewportClientsByModeTools.Find(InModeTools))
	{
		return *viewportClient;
	}
	return nullptr;
}

TArray<UScene4DComponent*> UEditorTransformGizmo4DFactory::CollectSelected4DComponents(FEditorModeTools* InModeTools)
{
	TArray<UScene4DComponent*> ret;
	if (InModeTools == nullptr)
	{
		return ret;
	}
	// Both actor selection and component selection need to be handled.
	USelection* selectedComponents = InModeTools->GetSelectedComponents();
	if (selectedComponents != nullptr)
	{
		TArray<UActorComponent*> selectedActorComponents;
		selectedComponents->GetSelectedObjects<UActorComponent>(selectedActorComponents);
		for (UActorComponent* comp : selectedActorComponents)
		{
			UScene4DComponent* comp4D = Cast<UScene4DComponent>(comp);
			if (comp4D != nullptr)
			{
				ret.Add(comp4D);
			}
		}
	}
	// Only when no components are directly selected, check selected actors and grab their root 4D components.
	if (ret.Num() == 0)
	{
		USelection* selectedActors = InModeTools->GetSelectedActors();
		if (selectedActors != nullptr)
		{
			TArray<AActor*> selectedActorObjects;
			selectedActors->GetSelectedObjects<AActor>(selectedActorObjects);
			for (AActor* actor : selectedActorObjects)
			{
				UScene4DComponent* rootComponent4D = Cast<UScene4DComponent>(actor->GetRootComponent());
				if (rootComponent4D != nullptr)
				{
					ret.Add(rootComponent4D);
				}
			}
		}
	}
	return ret;
}

bool UEditorTransformGizmo4DFactory::CanBuildGizmoForSelection(FEditorModeTools* InModeTools) const
{
	return CollectSelected4DComponents(InModeTools).Num() > 0;
}

TArray<UInteractiveGizmo*> UEditorTransformGizmo4DFactory::BuildGizmoForSelection(FEditorModeTools* InModeTools, UInteractiveGizmoManager* InGizmoManager) const
{
	TArray<UInteractiveGizmo*> retGizmos;
	TArray<AActor*> selectedActors;
	InModeTools->GetSelectedActors()->GetSelectedObjects<AActor>(selectedActors);
	// Collect all 4D root components from selected actors.
	TArray<UScene4DComponent*> selected4DComponents = CollectSelected4DComponents(InModeTools);
	// We only want to show the 4D gizmo if 4D objects are selected.
	if (selected4DComponents.Num() == 0)
	{
		return retGizmos;
	}
	// Note: UGizmoEdMode::RecreateGizmo() may be called twice per selection change:
	// once from ActorSelectionChangeNotify and once from OnWidgetModeChanged.
	// This is harmless - the second call destroys and rebuilds the gizmo.
	// Attempting to debounce here doesn't work because DestroyGizmo() is called
	// before BuildGizmoForSelection, so skipping the build leaves no gizmo.
	const ECoordSystem coordSystem = InModeTools->GetCoordSystem();
	const UE::Widget::EWidgetMode widgetMode = InModeTools->GetWidgetMode();
	const bool bPreserveNonUniformScale = GetDefault<ULevelEditorViewportSettings>()->PreserveNonUniformScale;
	const EEditorTransform4DKeepMode keepMode = bPreserveNonUniformScale ? EEditorTransform4DKeepMode::ConformalChangesOnly : EEditorTransform4DKeepMode::Freeform;
	// Ensure the gizmo builder is registered.
	if (!InGizmoManager->IsGizmoTypeRegistered(TEXT("TransformGizmo4D")))
	{
		InGizmoManager->RegisterGizmoType(TEXT("TransformGizmo4D"), FUnreal4DEditorModule::TransformGizmo4DBuilder);
	}
	// Create a new 4D transform gizmo, a single gizmo for all selected objects.
	// In order to fit into the lifecycle of Unreal's InteractiveToolsFramework,
	// we need to use the gizmo manager's CreateGizmo to properly register it,
	// and let it manage the gizmo's lifecycle, so the next time this factory
	// is called the original object will be deleted and we need to create a new one.
	UEditorTransformGizmo4D* gizmo4D = Cast<UEditorTransformGizmo4D>(InGizmoManager->CreateGizmo(TEXT("TransformGizmo4D")));
	if (gizmo4D == nullptr)
	{
		return retGizmos;
	}
	// Configure the gizmo.
	const FEditorTransformGizmo4DPartVisibilitySettings visibilitySettings = WidgetModeToVisibilitySettings(widgetMode, keepMode);
	FEditorViewportClient* viewportClient = FindViewportClientForModeTools(InModeTools);
	gizmo4D->Setup4D(selected4DComponents, visibilitySettings, coordSystem, InModeTools, viewportClient);
	retGizmos.Add(gizmo4D);
	return retGizmos;
}

void UEditorTransformGizmo4DFactory::ConfigureGridSnapping(bool bInPositionGridEnabled, bool bInRotationGridEnabled, const TArray<UInteractiveGizmo*>& InGizmos) const
{
	// Configure snapping for 4D gizmos.
	for (UInteractiveGizmo* gizmo : InGizmos)
	{
		UEditorTransformGizmo4D* gizmo4D = Cast<UEditorTransformGizmo4D>(gizmo);
		if (gizmo4D != nullptr)
		{
			// TODO: Implement snapping.
		}
	}
}
