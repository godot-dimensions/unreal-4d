#include "Unreal4DEditorModule.h"

// Inspector.
#include "../Unreal4DCore/Scene/Components/Scene4DComponent.h"
#include "Inspector/EditorScene4DComponentDetails.h"
#include "Inspector/EditorCamera4DComponentDetails.h"
#include "Inspector/EditorMesh4DComponentDetails.h"
#include "Inspector/EditorTransform4DDetails.h"

// Editor mode.
#include "Unreal4DEditorCommands.h"
#include "Unreal4DEditorMode.h"
#include "BlueprintEditor.h"
#include "EditorModeRegistry.h"
#include "EditorModeManager.h"
#include "EditorModes.h"
#include "LevelEditorViewport.h"

// Gizmo mode integration.
#include "Viewport/TransformGizmo/EditorTransformGizmo4DFactory.h"
#include "Viewport/TransformGizmo/EditorTransformGizmo4D.h"
#include "GizmoEdMode.h"
#include "InteractiveGizmoManager.h"
#include "LevelEditor.h"
#include "Tools/EdModeInteractiveToolsContext.h"
#include "Toolkits/ToolkitManager.h"

// Model import factory.
#include "Asset/Factory/G4MFModelActor4DFactory.h"
#include "Asset/Factory/G4MFModelAsset4DFactory.h"
#include "Asset/TypeActions/FAssetTypeActions_G4MFModelAsset4D.h"
#include "Asset/TypeActions/FAssetTypeActions_Mesh4D.h"
#include "Subsystems/PlacementSubsystem.h"
#include "AssetToolsModule.h"
#include "IAssetTools.h"

// Globals.
#include "PropertyEditorModule.h"
#include "TimerManager.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "CoreGlobals.h"
#include "Editor/UnrealEdEngine.h"
#include "Selection.h"
#include "UnrealEdGlobals.h"

//------------------------------------------------------------------------------

IMPLEMENT_MODULE(FUnreal4DEditorModule, Unreal4DEditor);

DEFINE_LOG_CATEGORY(LogUnreal4DEditor);

TObjectPtr<class UEditorTransformGizmo4DBuilder> FUnreal4DEditorModule::TransformGizmo4DBuilder;
TObjectPtr<class UEditorTransformGizmo4DFactory> FUnreal4DEditorModule::TransformGizmo4DFactory;

////////////////////////////////////////////////////////////////////////////////
void FUnreal4DEditorModule::StartupModule()
{
	// Create the builder and factory for the gizmo.
	// Both are required for integration in Unreal's interactive tools framework.
	TransformGizmo4DBuilder = NewObject<UEditorTransformGizmo4DBuilder>();
	TransformGizmo4DFactory = NewObject<UEditorTransformGizmo4DFactory>();
	TransformGizmo4DBuilder->AddToRoot();
	TransformGizmo4DFactory->AddToRoot();

	// Register details customizations, property editors, editor subsystems, etc.
	FPropertyEditorModule& propModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	propModule.RegisterCustomClassLayout(
		"Scene4DComponent",
		FOnGetDetailCustomizationInstance::CreateStatic(&FEditorScene4DComponentDetails::MakeInstance)
	);
	propModule.RegisterCustomClassLayout(
		"Camera4DComponent",
		FOnGetDetailCustomizationInstance::CreateStatic(&FEditorCamera4DComponentDetails::MakeInstance)
	);
	propModule.RegisterCustomClassLayout(
		"Mesh4DComponent",
		FOnGetDetailCustomizationInstance::CreateStatic(&FEditorMesh4DComponentDetails::MakeInstance)
	);
	propModule.RegisterCustomPropertyTypeLayout(
		"Transform4D",
		FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FEditorTransform4DCustomization::MakeInstance)
	);
	propModule.NotifyCustomizationModuleChanged();

	FLevelEditorModule& LevelEditor = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	LevelEditorCreatedHandle = LevelEditor.OnLevelEditorCreated().AddRaw(this, &FUnreal4DEditorModule::OnLevelEditorCreated);
	BlueprintSelectionNotifier.Register();
	// Register 4D editor commands so they appear in Editor Preferences > Keyboard Shortcuts.
	FUnreal4DEditorCommands::Register();
	// Register the 4D editor mode so it can be activated later.
	FEditorModeRegistry::Get().RegisterMode<FUnreal4DEditorMode>(
		FUnreal4DEditorMode::ModeID,
		FText::FromString(TEXT("Unreal 4D Editor Mode")),
		FSlateIcon(),
		/*bVisible=*/ false,
		/*PriorityOrder=*/ 0
	);

	// Register the asset import factory and actor factory.
	if (GUnrealEd)
	{
		GUnrealEd->GetEditorSubsystem<UPlacementSubsystem>()->RegisterAssetFactory(NewObject<UG4MFModelActor4DFactory>());
	}

	// Register asset type actions so the Content Browser knows about G4MF model assets.
	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
		AssetTools.RegisterAssetTypeActions(MakeShareable(new FAssetTypeActions_G4MFModelAsset4D()));
		AssetTools.RegisterAssetTypeActions(MakeShareable(new FAssetTypeActions_Mesh4D()));
	}
}

////////////////////////////////////////////////////////////////////////////////
void FUnreal4DEditorModule::ShutdownModule()
{
	// Delete the builder and factory.
	if (!GExitPurge)
	{
		if (TransformGizmo4DBuilder != nullptr)
		{
			TransformGizmo4DBuilder->RemoveFromRoot();
		}
		if (TransformGizmo4DFactory != nullptr)
		{
			TransformGizmo4DFactory->RemoveFromRoot();
		}
	}
	TransformGizmo4DBuilder = nullptr;
	TransformGizmo4DFactory = nullptr;
	// Detach the viewport rotation widget from all viewports.
	ViewportRotationWidgetManager.DetachFromAllEditorViewports();
	// Unregister the 4D editor mode.
	FEditorModeRegistry::Get().UnregisterMode(FUnreal4DEditorMode::ModeID);
	if (FModuleManager::Get().IsModuleLoaded("LevelEditor"))
	{
		FLevelEditorModule& LevelEditor = FModuleManager::GetModuleChecked<FLevelEditorModule>("LevelEditor");
		if (LevelEditorCreatedHandle.IsValid())
		{
			LevelEditor.OnLevelEditorCreated().Remove(LevelEditorCreatedHandle);
			LevelEditorCreatedHandle.Reset();
		}
	}
	if (GEditor != nullptr)
	{
		if (AssetEditorOpenedHandle.IsValid())
		{
			if (UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>())
			{
				AssetEditorSubsystem->OnAssetOpenedInEditor().Remove(AssetEditorOpenedHandle);
			}
			AssetEditorOpenedHandle.Reset();
		}
		if (BeginObjectMovementHandle.IsValid())
		{
			GEditor->OnBeginObjectMovement().Remove(BeginObjectMovementHandle);
			BeginObjectMovementHandle.Reset();
		}
		if (EndObjectMovementHandle.IsValid())
		{
			GEditor->OnEndObjectMovement().Remove(EndObjectMovementHandle);
			EndObjectMovementHandle.Reset();
		}
	}
	BlueprintSelectionNotifier.Unregister();
	ModeToolsWithGizmoFactory.Empty();
	if (SelectedActorsPtr.IsValid() && SelectionChangedHandle.IsValid())
	{
		SelectedActorsPtr->SelectionChangedEvent.Remove(SelectionChangedHandle);
		SelectionChangedHandle.Reset();
		SelectedActorsPtr.Reset();
	}

	// Unregister details customizations.
	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule& propModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
		propModule.UnregisterCustomClassLayout("Scene4DComponent");
		propModule.UnregisterCustomClassLayout("Camera4DComponent");
		propModule.UnregisterCustomClassLayout("Mesh4DComponent");
		propModule.UnregisterCustomPropertyTypeLayout("Transform4D");
		propModule.NotifyCustomizationModuleChanged();
	}

	FUnreal4DEditorCommands::RestoreAllViewportNavChords();
	FUnreal4DEditorCommands::Unregister();
}

void FUnreal4DEditorModule::SetupGizmoForModeTools(FEditorModeTools& InModeTools)
{
	// Ensure the default mode is active before activating 4D modes.
	if (!InModeTools.IsModeActive(FBuiltinEditorModes::EM_Default))
	{
		InModeTools.ActivateMode(FBuiltinEditorModes::EM_Default);
	}
	// Activate GizmoEdMode after the default mode for custom 4D gizmo support.
	const FEditorModeID GizmoModeID = GetDefault<UGizmoEdMode>()->GetID();
	InModeTools.ActivateMode(GizmoModeID);
	// Activate the 4D editor mode after the 4D gizmo mode so it can receive input events.
	InModeTools.ActivateMode(FUnreal4DEditorMode::ModeID);
	// Get the gizmo manager from the mode tools so we can register our gizmo builder and factory.
	UEdMode* EdMode = InModeTools.GetActiveScriptableMode(GizmoModeID);
	UGizmoEdMode* GizmoEdMode = Cast<UGizmoEdMode>(EdMode);
	UInteractiveGizmoManager* GizmoManager = InModeTools.GetInteractiveToolsContext()->GizmoManager;
	if (GizmoEdMode == nullptr || GizmoManager == nullptr)
	{
		return;
	}
	// If the mode and gizmo manager are valid, register the gizmo builder and factory.
	if (!GizmoManager->IsGizmoTypeRegistered(TEXT("TransformGizmo4D")))
	{
		GizmoManager->RegisterGizmoType(TEXT("TransformGizmo4D"), TransformGizmo4DBuilder);
	}
	// Avoid re-adding the same factory repeatedly for the same mode tools instance.
	if (!ModeToolsWithGizmoFactory.Contains(&InModeTools))
	{
		GizmoEdMode->AddFactory(TransformGizmo4DFactory);
		ModeToolsWithGizmoFactory.Add(&InModeTools);
	}
}

void FUnreal4DEditorModule::OnAssetEditorOpened(UObject* InAsset, IAssetEditorInstance* InEditorInstance)
{
	if (InAsset == nullptr || InEditorInstance == nullptr)
	{
		return;
	}
	const FName editorName = InEditorInstance->GetEditorName();
	if (editorName != FName(TEXT("BlueprintEditor")))
	{
		return;
	}
	// Look up the editor as an IToolkit. IAssetEditorInstance doesn't expose mode manager.
	const TSharedPtr<IToolkit> toolkit = FToolkitManager::Get().FindEditorForAsset(InAsset);
	if (!toolkit.IsValid() || !toolkit->IsBlueprintEditor())
	{
		return;
	}
	const TSharedPtr<FBlueprintEditor> blueprintEditor = StaticCastSharedPtr<FBlueprintEditor>(toolkit);
	if (!blueprintEditor.IsValid())
	{
		return;
	}
	FEditorModeTools& toolkitModeTools = blueprintEditor->GetEditorModeManager();
	SetupGizmoForModeTools(toolkitModeTools);
	// Get the editor mode instance and convert to weak pointer.
	ViewportRotationWidgetManager.AttachToBlueprintAssetEditor(blueprintEditor);
}

void FUnreal4DEditorModule::OnLevelEditorCreated(TSharedPtr<ILevelEditor> InLevelEditor)
{
	ensure(GEditor != nullptr);
	// Connect `OnAssetEditorOpened` to `UAssetEditorSubsystem::OnAssetOpenedInEditor`.
	// This is done in `OnLevelEditorCreated` because by this point Unreal has finished
	// setting up every core part of the editor including GEditor and its subsystems.
	if (!AssetEditorOpenedHandle.IsValid())
	{
		UAssetEditorSubsystem* assetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
		if (assetEditorSubsystem != nullptr)
		{
			AssetEditorOpenedHandle = assetEditorSubsystem->OnAssetOpenedInEditor().AddRaw(this, &FUnreal4DEditorModule::OnAssetEditorOpened);
		}
	}
	if (!BeginObjectMovementHandle.IsValid())
	{
		BeginObjectMovementHandle = GEditor->OnBeginObjectMovement().AddRaw(this, &FUnreal4DEditorModule::OnEditorBeginObjectMovement);
	}
	if (!EndObjectMovementHandle.IsValid())
	{
		EndObjectMovementHandle = GEditor->OnEndObjectMovement().AddRaw(this, &FUnreal4DEditorModule::OnEditorEndObjectMovement);
	}
	// Defer gizmo setup and viewport rotation widget attachment to next tick.
	TWeakPtr<ILevelEditor> LevelEditorWeakPtr = InLevelEditor;
	GEditor->GetTimerManager()->SetTimerForNextTick([this, LevelEditorWeakPtr]()
	{
		if (!LevelEditorWeakPtr.IsValid())
		{
			return;
		}

		FEditorModeID GizmoModeID = GetDefault<UGizmoEdMode>()->GetID();
		FEditorModeTools& ModeTools = GLevelEditorModeTools();
		SetupGizmoForModeTools(ModeTools);
		ViewportRotationWidgetManager.AttachToLevelEditor(LevelEditorWeakPtr.Pin().ToSharedRef());
		// Re-activate GizmoEdMode if it gets deactivated.
		if (USelection* Selection = GEditor->GetSelectedActors())
		{
			SelectedActorsPtr = Selection;
			if (SelectionChangedHandle.IsValid())
			{
				Selection->SelectionChangedEvent.Remove(SelectionChangedHandle);
				SelectionChangedHandle.Reset();
			}
			SelectionChangedHandle = Selection->SelectionChangedEvent.AddLambda([LevelEditorWeakPtr, GizmoModeID](UObject* InObject)
			{
				if (!LevelEditorWeakPtr.IsValid())
				{
					return;
				}
				FEditorModeTools& ModeMgr = GLevelEditorModeTools();

				// Check if GizmoEdMode is active.
				UEdMode* CurrentMode = ModeMgr.GetActiveScriptableMode(GizmoModeID);
				if (!CurrentMode)
				{
					// Defer activation to next tick.
					GEditor->GetTimerManager()->SetTimerForNextTick([LevelEditorWeakPtr, GizmoModeID]()
					{
						if (LevelEditorWeakPtr.IsValid())
						{
							FEditorModeTools& ModeMgr = GLevelEditorModeTools();
							if (!ModeMgr.GetActiveScriptableMode(GizmoModeID))
							{
								if (!ModeMgr.IsModeActive(FBuiltinEditorModes::EM_Default))
								{
									ModeMgr.ActivateMode(FBuiltinEditorModes::EM_Default);
								}
								ModeMgr.ActivateMode(GizmoModeID);
								ModeMgr.ActivateMode(FUnreal4DEditorMode::ModeID);
							}
						}
					});
				}
			});
		}
	});
}

void FUnreal4DEditorModule::OnEditorBeginObjectMovement(UObject& InObject)
{
	UScene4DComponent* component4D = ResolveObjectTo4DComponent(InObject);
	if (component4D == nullptr)
	{
		return;
	}
	// We need to ensure both the 4D component and its actor are marked
	// as transactional (included in the undo/redo system) and also
	// call Modify() on them at the start to record their pre-move state.
	if (!component4D->HasAnyFlags(RF_Transactional))
	{
		component4D->SetFlags(RF_Transactional);
	}
	AActor* ownerActor = component4D->GetOwner();
	if (ownerActor != nullptr)
	{
		if (!ownerActor->HasAnyFlags(RF_Transactional))
		{
			ownerActor->SetFlags(RF_Transactional);
		}
		ownerActor->Modify();
	}
	component4D->Modify();
}

void FUnreal4DEditorModule::OnEditorEndObjectMovement(UObject& InObject)
{
	UScene4DComponent* component4D = ResolveObjectTo4DComponent(InObject);
	if (component4D == nullptr)
	{
		return;
	}
	Sync4DComponentFromEditorTransform(component4D);
}

UScene4DComponent* FUnreal4DEditorModule::ResolveObjectTo4DComponent(UObject& InObject)
{
	// Both of these cases are hit and need to be handled, it depends on what the user has selected.
	AActor* actor = Cast<AActor>(&InObject);
	if (actor != nullptr)
	{
		return Cast<UScene4DComponent>(actor->GetRootComponent());
	}
	return Cast<UScene4DComponent>(&InObject);
}

void FUnreal4DEditorModule::Sync4DComponentFromEditorTransform(UScene4DComponent* InComponent4D)
{
	if (InComponent4D == nullptr)
	{
		return;
	}
	// The 3D transform of a UScene4DComponent always has a scale of 1.0 to avoid issues, so we need
	// to ultimately ignore it and re-apply the 4D scale. We want to restore the full 4D scale,
	// but also let's try to avoid temporary mass diffs when calling SetLocalTransform4Dfrom3D
	// by applying the uniform 4D scale as the 3D scale before restoring the 4D scale.
	const FVector4 localScale4D = InComponent4D->GetLocalScale4D();
	const double uniformScale = InComponent4D->GetLocalUniformScale4D();
	FTransform transform3D = InComponent4D->GetComponentTransform();
	transform3D.SetScale3D(FVector(uniformScale));
	InComponent4D->Modify();
	InComponent4D->SetLocalTransform4Dfrom3D(transform3D);
	InComponent4D->SetLocalScale4D(localScale4D);
}
