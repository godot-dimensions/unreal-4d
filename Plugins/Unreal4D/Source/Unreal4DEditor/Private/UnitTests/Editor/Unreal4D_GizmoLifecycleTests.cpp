#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Misc/ScopeExit.h"
#include "AssetEditorModeManager.h"
#include "BlueprintEditor.h"
#include "Editor.h"
#include "EditorViewportClient.h"
#include "Engine/Blueprint.h"
#include "EngineUtils.h"
#include "InteractiveGizmoManager.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "PreviewScene.h"
#include "Selection.h"
#include "Settings/LevelEditorViewportSettings.h"
#include "SSCSEditorViewport.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "Tools/EdModeInteractiveToolsContext.h"
#include "Unreal4DCore/Scene/Actors/Box4DActor.h"
#include "Unreal4DEditor/Unreal4DEditorMode.h"
#include "Unreal4DEditor/Viewport/TransformGizmo/EditorTransformGizmo4D.h"
#include "Unreal4DEditor/Viewport/TransformGizmo/EditorTransformGizmo4DFactory.h"
#include "Unreal4DEditor/Viewport/TransformGizmo/Meshes/EditorTransformGizmoActor4D.h"
#include "UObject/UObjectIterator.h"
#include "UObject/Package.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUnreal4D_GizmoLifecycle, "Unreal4D.Editor.GizmoLifecycle",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FUnreal4D_GizmoLifecycle::RunTest(const FString& Parameters)
{
	FPreviewScene firstScene(FPreviewScene::ConstructionValues().SetCreateDefaultLighting(false));
	FPreviewScene secondScene(FPreviewScene::ConstructionValues().SetCreateDefaultLighting(false));
	const TSharedRef<FAssetEditorModeManager> firstToolsOwner = MakeShared<FAssetEditorModeManager>();
	const TSharedRef<FAssetEditorModeManager> secondToolsOwner = MakeShared<FAssetEditorModeManager>();
	FAssetEditorModeManager& firstTools = firstToolsOwner.Get();
	FAssetEditorModeManager& secondTools = secondToolsOwner.Get();
	firstTools.SetPreviewScene(&firstScene);
	secondTools.SetPreviewScene(&secondScene);
	const TSharedRef<FEditorViewportClient> firstViewport = MakeShared<FEditorViewportClient>(&firstTools, &firstScene);
	const TSharedRef<FEditorViewportClient> secondViewport = MakeShared<FEditorViewportClient>(&secondTools, &secondScene);
	UEditorTransformGizmo4DFactory::RegisterViewportClientForModeTools(&firstTools, firstViewport);
	UEditorTransformGizmo4DFactory::RegisterViewportClientForModeTools(&secondTools, secondViewport);
	firstTools.ActivateMode(FUnreal4DEditorMode::ModeID);
	secondTools.ActivateMode(FUnreal4DEditorMode::ModeID);
	ON_SCOPE_EXIT
	{
		firstTools.DestroyMode(FUnreal4DEditorMode::ModeID);
		secondTools.DestroyMode(FUnreal4DEditorMode::ModeID);
	};
	FEdMode* firstMode = firstTools.GetActiveMode(FUnreal4DEditorMode::ModeID);
	FEdMode* secondMode = secondTools.GetActiveMode(FUnreal4DEditorMode::ModeID);
	if (!TestNotNull(TEXT("The first preview owns a 4D editor mode."), firstMode)
			|| !TestNotNull(TEXT("The second preview owns a separate 4D editor mode."), secondMode))
	{
		return false;
	}
	UInteractiveGizmoManager* firstManager = firstTools.GetInteractiveToolsContext()->GizmoManager;
	UInteractiveGizmoManager* secondManager = secondTools.GetInteractiveToolsContext()->GizmoManager;
	auto findGizmo = [](UInteractiveGizmoManager* Manager)
	{
		return Cast<UEditorTransformGizmo4D>(Manager->FindGizmoByInstanceIdentifier(UEditorTransformGizmo4DFactory::GizmoIdentifier));
	};
	auto visualCount = [](UWorld* World)
	{
		int32 count = 0;
		for (TActorIterator<AEditorGizmoActor4D> actor(World); actor; ++actor)
		{
			++count;
		}
		return count;
	};
	firstTools.SetShowWidget(true);
	firstViewport->ShowWidget(true);
	ABox4DActor* firstActor = firstScene.GetWorld()->SpawnActor<ABox4DActor>();
	firstTools.GetSelectedActors()->Select(firstActor);
	firstTools.ActorSelectionChangeNotify();
	firstMode->Tick(&firstViewport.Get(), 0.0f);
	UEditorTransformGizmo4D* firstGizmo = findGizmo(firstManager);
	if (!TestNotNull(TEXT("Selecting a 4D actor builds a gizmo without an auxiliary editor mode."), firstGizmo))
	{
		return false;
	}
	TestEqual(TEXT("The visual actor is in the first preview world."), visualCount(firstScene.GetWorld()), 1);
	TestFalse(TEXT("The native mode widget is hidden while the 4D gizmo is active."), firstTools.GetShowWidget());
	TestFalse(TEXT("The native preview widget is hidden too."), firstViewport->GetShowWidget());
	firstMode->Tick(&firstViewport.Get(), 0.0f);
	TestTrue(TEXT("An unchanged selection retains the same gizmo."), findGizmo(firstManager) == firstGizmo);

	ABox4DActor* secondActor = secondScene.GetWorld()->SpawnActor<ABox4DActor>();
	secondTools.GetSelectedComponents()->Select(secondActor->GetRootComponent());
	secondTools.ActorSelectionChangeNotify();
	secondMode->Tick(&secondViewport.Get(), 0.0f);
	UEditorTransformGizmo4D* secondGizmo = findGizmo(secondManager);
	TestNotNull(TEXT("Selecting a preview component builds a gizmo in its own context."), secondGizmo);
	TestTrue(TEXT("The two contexts have different gizmo instances."), firstGizmo != secondGizmo);
	TestEqual(TEXT("The second visual actor is in the second preview world."), visualCount(secondScene.GetWorld()), 1);

	firstTools.SetWidgetMode(UE::Widget::WM_Rotate);
	firstMode->Tick(&firstViewport.Get(), 0.0f);
	TestTrue(TEXT("Changing widget mode rebuilds the 4D handles."), findGizmo(firstManager) != firstGizmo);
	TestTrue(TEXT("Changing the first context leaves the other gizmo alive."), findGizmo(secondManager) == secondGizmo);
	TestEqual(TEXT("Rebuilding does not leave duplicate visual actors."), visualCount(firstScene.GetWorld()), 1);

	UEditorTransformProxy4D* proxy = nullptr;
	for (TObjectIterator<UEditorTransformProxy4D> candidate; candidate; ++candidate)
	{
		if (candidate->GetOuter() == findGizmo(firstManager))
		{
			proxy = *candidate;
			break;
		}
	}
	if (!TestNotNull(TEXT("The gizmo owns a transform proxy."), proxy))
	{
		return false;
	}
	ULevelEditorViewportSettings* settings = GetMutableDefault<ULevelEditorViewportSettings>();
	const bool savedGridEnabled = settings->GridEnabled;
	ON_SCOPE_EXIT { settings->GridEnabled = savedGridEnabled; };
	FTransform4D transform = FTransform4D::Identity();
	transform.Origin = FVector4(GEditor->GetGridSize() * 0.37, 0.0, 0.0, 0.0);
	settings->GridEnabled = false;
	firstMode->Tick(&firstViewport.Get(), 0.0f);
	TestTrue(TEXT("Disabling snapping in the toolbar is reflected on the next tick."),
		proxy->SnapSettings->SnapSingleTransform(transform).Origin.Equals(transform.Origin));
	settings->GridEnabled = true;
	firstMode->Tick(&firstViewport.Get(), 0.0f);
	TestFalse(TEXT("Enabling snapping updates an existing gizmo without rebuilding it."),
		proxy->SnapSettings->SnapSingleTransform(transform).Origin.Equals(transform.Origin));

	firstTools.GetSelectedActors()->DeselectAll();
	AActor* actor3D = firstScene.GetWorld()->SpawnActor<AActor>();
	USceneComponent* component3D = NewObject<USceneComponent>(actor3D);
	actor3D->SetRootComponent(component3D);
	component3D->RegisterComponent();
	firstTools.GetSelectedComponents()->Select(component3D);
	// The editor updates this cache when changing selection through its normal UI.
	firstTools.SetSelectionHasSceneComponent(true);
	firstTools.ActorSelectionChangeNotify();
	firstMode->Tick(&firstViewport.Get(), 0.0f);
	TestNull(TEXT("Clearing the selection removes its gizmo."), findGizmo(firstManager));
	TestEqual(TEXT("Clearing the selection removes its visual actor."), visualCount(firstScene.GetWorld()), 0);
	TestTrue(TEXT("The native mode widget is restored."), firstTools.GetShowWidget());
	TestTrue(TEXT("The native preview widget is restored."), firstViewport->GetShowWidget());
	TestTrue(TEXT("Clearing one context leaves the other gizmo alive."), findGizmo(secondManager) == secondGizmo);

	// Preview reconstruction replaces component objects before the next viewport tick.
	secondTools.GetSelectedComponents()->DeselectAll();
	ABox4DActor* replacement = secondScene.GetWorld()->SpawnActor<ABox4DActor>();
	secondTools.GetSelectedComponents()->Select(replacement->GetRootComponent());
	secondTools.ActorSelectionChangeNotify();
	TestNull(TEXT("Selection notification releases the old preview gizmo immediately."), findGizmo(secondManager));
	secondScene.GetWorld()->DestroyActor(secondActor);
	secondMode->Tick(&secondViewport.Get(), 0.0f);
	TestNotNull(TEXT("The next tick binds the replacement component."), findGizmo(secondManager));
	TestEqual(TEXT("Preview reconstruction retains exactly one visual actor."), visualCount(secondScene.GetWorld()), 1);
	secondTools.DestroyMode(FUnreal4DEditorMode::ModeID);
	TestNull(TEXT("Mode destruction shuts down its gizmo."), findGizmo(secondManager));
	TestEqual(TEXT("Mode destruction removes the final visual actor."), visualCount(secondScene.GetWorld()), 0);
	TestTrue(TEXT("Mode destruction restores the preview widget."), secondViewport->GetShowWidget());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUnreal4D_BlueprintGizmoLifecycle, "Unreal4D.Editor.BlueprintGizmoLifecycle",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter | EAutomationTestFlags::NonNullRHI)

bool FUnreal4D_BlueprintGizmoLifecycle::RunTest(const FString& Parameters)
{
	// Exercise the real Blueprint selection notifier using an unsaved, temporary asset.
	UPackage* package = CreatePackage(*FString::Printf(TEXT("/Temp/Unreal4D_GizmoTest_%s"), *FGuid::NewGuid().ToString()));
	package->SetFlags(RF_Transient);
	UBlueprint* blueprint = FKismetEditorUtilities::CreateBlueprint(ABox4DActor::StaticClass(), package,
		TEXT("GizmoTest"), BPTYPE_Normal);
	if (!TestNotNull(TEXT("The test Blueprint was created."), blueprint))
	{
		return false;
	}
	blueprint->bForceFullEditor = true;
	UAssetEditorSubsystem* editors = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
	ON_SCOPE_EXIT
	{
		package->SetDirtyFlag(false);
		editors->CloseAllEditorsForAsset(blueprint);
	};
	if (!TestTrue(TEXT("The actual Blueprint editor opens."), editors->OpenEditorForAsset(blueprint)))
	{
		return false;
	}
	FBlueprintEditor* editor = static_cast<FBlueprintEditor*>(editors->FindEditorForAsset(blueprint, false));
	if (!TestNotNull(TEXT("The asset has a Blueprint editor."), editor))
	{
		return false;
	}
	editor->UpdatePreviewActor(blueprint, true);
	if (!TestNotNull(TEXT("The Blueprint preview actor exists."), editor->GetPreviewActor())
			|| !TestTrue(TEXT("The Blueprint viewport exists."), editor->GetSubobjectViewport().IsValid()))
	{
		return false;
	}
	const TSharedPtr<FEditorViewportClient> viewport = editor->GetSubobjectViewport()->GetViewportClient();
	FEditorModeTools* tools = viewport->GetModeTools();
	auto selectRoot = [&]()
	{
		return editor->FindAndSelectSubobjectEditorTreeNode(editor->GetPreviewActor()->GetRootComponent(), false).IsValid();
	};
	TestTrue(TEXT("The component tree selects the preview's native 4D root."), selectRoot());
	FEdMode* mode = tools->GetActiveMode(FUnreal4DEditorMode::ModeID);
	if (!TestNotNull(TEXT("Blueprint selection activates the Unreal4D mode."), mode))
	{
		return false;
	}
	mode->Tick(viewport.Get(), 0.0f);
	UInteractiveGizmoManager* manager = tools->GetInteractiveToolsContext()->GizmoManager;
	auto findGizmo = [&]() { return manager->FindGizmoByInstanceIdentifier(UEditorTransformGizmo4DFactory::GizmoIdentifier); };
	TestNotNull(TEXT("Blueprint selection creates the 4D gizmo."), findGizmo());
	TestFalse(TEXT("The native Blueprint widget is hidden."), viewport->GetShowWidget());
	int32 visualCount = 0;
	for (TActorIterator<AEditorGizmoActor4D> actor(editor->GetPreviewActor()->GetWorld()); actor; ++actor) { ++visualCount; }
	TestEqual(TEXT("The gizmo visual is in the Blueprint preview world."), visualCount, 1);
	viewport->SetWidgetMode(UE::Widget::WM_Rotate);
	mode->Tick(viewport.Get(), 0.0f);
	TestNotNull(TEXT("The Blueprint toolbar can switch to the rotation gizmo."), findGizmo());
	editor->UpdatePreviewActor(blueprint, true);
	TestTrue(TEXT("The component can be selected after preview reconstruction."), selectRoot());
	mode->Tick(viewport.Get(), 0.0f);
	TestNotNull(TEXT("The gizmo rebinds after Blueprint preview reconstruction."), findGizmo());
	tools->DestroyMode(FUnreal4DEditorMode::ModeID);
	TestNull(TEXT("Closing the Blueprint mode destroys the gizmo."), findGizmo());
	TestTrue(TEXT("Closing the Blueprint mode restores its native widget."), viewport->GetShowWidget());
	return true;
}

#endif
