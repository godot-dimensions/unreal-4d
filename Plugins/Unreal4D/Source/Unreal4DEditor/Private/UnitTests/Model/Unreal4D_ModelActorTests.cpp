#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Misc/ScopeExit.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/MemoryWriter.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"
#include "Unreal4DCore/Model/Asset/G4MFModelAsset4D.h"
#include "Unreal4DCore/Model/Asset/ModelActor4D.h"
#include "Unreal4DCore/Scene/Components/Scene4DComponent.h"
#include "UObject/UnrealType.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUnreal4D_ModelActorGeneratedComponents, "Unreal4D.Model.GeneratedComponentTracking",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FUnreal4D_ModelActorGeneratedComponents::RunTest(const FString& Parameters)
{
	const UWorld::InitializationValues initialization = UWorld::InitializationValues()
		.AllowAudioPlayback(false).CreateNavigation(false).CreateAISystem(false)
		.CreateFXSystem(false).CreatePhysicsScene(false).ShouldSimulatePhysics(false);
	UWorld* world = UWorld::CreateWorld(EWorldType::EditorPreview, false, NAME_None,
		nullptr, true, ERHIFeatureLevel::Num, &initialization);
	GEngine->CreateNewWorldContext(EWorldType::EditorPreview).SetCurrentWorld(world);
	ON_SCOPE_EXIT
	{
		GEngine->DestroyWorldContext(world);
		world->DestroyWorld(false);
	};
	FActorSpawnParameters spawn;
	spawn.bDeferConstruction = true;
	AModelActor4D* actor = world->SpawnActor<AModelActor4D>(AModelActor4D::StaticClass(), FTransform::Identity, spawn);
	// Deliberately leave this unrelated component unregistered: GetComponents() can omit it.
	UScene4DComponent* custom = NewObject<UScene4DComponent>(actor, TEXT("CustomComponent"));
	actor->AddInstanceComponent(custom);
	UG4MFModelAsset4D* model = NewObject<UG4MFModelAsset4D>(actor);
	model->EmbeddedState = NewObject<UG4MFState4D>(model);
	model->ImportDocument = NewObject<UG4MFDocument4D>(model);
	UG4MFNode4D* rootNode = NewObject<UG4MFNode4D>(model->EmbeddedState);
	rootNode->SetItemName(model->EmbeddedState, TEXT("ModelRoot"));
	rootNode->ChildrenIndices.Add(1);
	UG4MFNode4D* childNode = NewObject<UG4MFNode4D>(model->EmbeddedState);
	childNode->SetItemName(model->EmbeddedState, TEXT("ModelChild"));
	childNode->ParentIndex = 0;
	model->EmbeddedState->G4MFNodes = { rootNode, childNode };
	actor->Model = model;
	actor->FinishSpawning(FTransform::Identity);
	FArrayProperty* trackingProperty = FindFProperty<FArrayProperty>(actor->GetClass(), TEXT("GeneratedComponents"));
	auto* generated = trackingProperty->ContainerPtrToValuePtr<TArray<TObjectPtr<UActorComponent>>>(actor);
	TestEqual(TEXT("A new G4MF actor tracks every generated node."), generated->Num(), 2);
	TestTrue(TEXT("The generated root is tracked."), generated->Contains(actor->GetRootComponent()));
	TestFalse(TEXT("The pre-existing component is not treated as generated."), generated->Contains(custom));
	for (UActorComponent* component : *generated)
	{
		TestTrue(TEXT("Generated nodes are instance components."), actor->GetInstanceComponents().Contains(component));
	}
	UScene4DComponent* child = FindObject<UScene4DComponent>(actor, TEXT("ModelChild"));
	if (!TestNotNull(TEXT("The generated child exists."), child)) { return false; }
	FTransform4D editedTransform = child->GetLocalTransform4D();
	editedTransform.Origin = FVector4(123.0, -45.0, 67.0, 89.0);
	child->SetLocalTransform4D(editedTransform);

	// Exercise persistent UObject property serialization, with no PostLoad reconstruction.
	TArray<uint8> bytes;
	FMemoryWriter writer(bytes, true);
	FObjectAndNameAsStringProxyArchive saveArchive(writer, false);
	actor->Serialize(saveArchive);
	generated->Reset();
	FMemoryReader reader(bytes, true);
	FObjectAndNameAsStringProxyArchive loadArchive(reader, false);
	actor->Serialize(loadArchive);
	TestFalse(TEXT("Actor serialization restores the tracked list without compatibility recovery."), generated->IsEmpty());
	TestTrue(TEXT("The serialized list retains the generated root."), generated->Contains(actor->GetRootComponent()));
	TestTrue(TEXT("The serialized list retains the generated child."), generated->Contains(child));
	actor->OnConstruction(actor->GetActorTransform());
	TestTrue(TEXT("Construction after loading keeps the saved hierarchy."), FindObject<UScene4DComponent>(actor, TEXT("ModelChild")) == child);
	TestTrue(TEXT("Construction preserves edited 4D transforms."), child->GetLocalTransform4D().Origin == editedTransform.Origin);

	TWeakObjectPtr<UScene4DComponent> previousChild = child;
	actor->RebuildFromAsset();
	TestFalse(TEXT("Rebuilding destroys the previous generated child."), previousChild.IsValid());
	TestTrue(TEXT("Rebuilding preserves unrelated unregistered components."), IsValid(custom));
	TestEqual(TEXT("Rebuilding replaces the tracked list instead of accumulating nodes."), generated->Num(), 2);
	TestTrue(TEXT("The rebuilt child reuses its model node name."), IsValid(FindObject<UScene4DComponent>(actor, TEXT("ModelChild"))));
	actor->Model = nullptr;
	actor->RebuildFromAsset();
	TestTrue(TEXT("Clearing the model clears generated-component tracking."), generated->IsEmpty());
	TestNull(TEXT("Clearing the model removes its generated root."), actor->GetRootComponent());
	TestTrue(TEXT("Clearing the model still preserves unrelated components."), IsValid(custom));
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
