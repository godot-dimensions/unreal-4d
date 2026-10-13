#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Misc/ScopeExit.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Unreal4DCore/Render/RenderingSettings4D.h"
#include "Unreal4DCore/Model/Mesh/Tetra/BoxTetraMesh4D.h"
#include "Unreal4DCore/Scene/Components/Mesh4DComponent.h"
#include "Unreal4DCore/Scene/Components/Mesh4DProjectionComponent.h"
#include "UObject/UnrealType.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUnreal4D_ProjectionMaterialDefaults, "Unreal4D.Rendering.ProjectionMaterialDefaults",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FUnreal4D_ProjectionMaterialDefaults::RunTest(const FString& Parameters)
{
	URenderingSettings4D* settings = GetMutableDefault<URenderingSettings4D>();
	const TSoftObjectPtr<UMaterialInterface> originalSetting = settings->DefaultProjectionMaterial;
	UMaterialInterface* originalDefault = settings->GetDefaultProjectionMaterial();
	if (originalDefault == nullptr)
	{
		originalDefault = UMaterial::GetDefaultMaterial(MD_Surface);
	}
	ON_SCOPE_EXIT
	{
		settings->DefaultProjectionMaterial = originalSetting;
		settings->GetDefaultProjectionMaterial();
	};
	if (!TestNotNull(TEXT("The built-in projection material is publicly accessible."), LoadObject<UMaterialInterface>(nullptr,
		TEXT("/Unreal4D/Materials/MAT_Mesh4DProjection_Default.MAT_Mesh4DProjection_Default"))))
	{
		return false;
	}
	UMesh4DComponent* mesh = NewObject<UMesh4DComponent>();
	UMesh4DProjectionComponent* projection = NewObject<UMesh4DProjectionComponent>();
	projection->SetSourceComponent(mesh);
	TestTrue(TEXT("Plain Unreal4D meshes use the project default."), projection->GetMaterial(0) == originalDefault);
	TArray<UMaterialInterface*> usedMaterials;
	projection->GetUsedMaterials(usedMaterials);
	TestTrue(TEXT("The projection reports the default as a used material."), usedMaterials.Contains(originalDefault));

	UMaterialInstanceDynamic* projectDefault = UMaterialInstanceDynamic::Create(originalDefault, settings);
	UMaterialInstanceDynamic* meshOverride = UMaterialInstanceDynamic::Create(originalDefault, mesh);
	settings->DefaultProjectionMaterial = projectDefault;
	TestTrue(TEXT("Changing the project default affects meshes without overrides."), projection->GetMaterial(0) == projectDefault);
	mesh->ProjectionMaterial = meshOverride;
	TestTrue(TEXT("An explicit mesh material overrides the project default."), projection->GetMaterial(0) == meshOverride);
	settings->DefaultProjectionMaterial = nullptr;
	TestTrue(TEXT("Clearing the project default preserves explicit mesh materials."), projection->GetMaterial(0) == meshOverride);
	mesh->ProjectionMaterial = nullptr;
	TestTrue(TEXT("Clearing both defaults uses Unreal's emergency surface material."),
		projection->GetMaterial(0) == UMaterial::GetDefaultMaterial(MD_Surface));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUnreal4D_ProjectionMaterialRefresh, "Unreal4D.Rendering.ProjectionMaterialRefresh",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FUnreal4D_ProjectionMaterialRefresh::RunTest(const FString& Parameters)
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
	URenderingSettings4D* settings = URenderingSettings4D::Get();
	const TSoftObjectPtr<UMaterialInterface> originalSetting = settings->DefaultProjectionMaterial;
	ON_SCOPE_EXIT
	{
		settings->DefaultProjectionMaterial = originalSetting;
		settings->GetDefaultProjectionMaterial();
	};
	AActor* actor = world->SpawnActor<AActor>();
	UMesh4DComponent* mesh = NewObject<UMesh4DComponent>(actor);
	actor->SetRootComponent(mesh);
	mesh->Mesh4D = NewObject<UBoxTetraMesh4D>(mesh);
	mesh->EnableProjection = true;
	mesh->RegisterComponent();
	world->SendAllEndOfFrameUpdates();
	UMesh4DProjectionComponent* projection = actor->FindComponentByClass<UMesh4DProjectionComponent>();
	if (!TestNotNull(TEXT("Enabling projection creates the registered projection component."), projection))
	{
		return false;
	}
	if (!projection->IsRenderStateCreated())
	{
		AddInfo(TEXT("Render-state refresh requires a rendering RHI; rerun without NullRHI to exercise it."));
		return true;
	}
	TestFalse(TEXT("Initial projection render-state updates are complete."), projection->IsRenderStateDirty());
	UMaterialInstanceDynamic* replacement = UMaterialInstanceDynamic::Create(mesh->GetProjectionMaterialForProjectionComponent(), settings);
	settings->DefaultProjectionMaterial = replacement;
	FPropertyChangedEvent change(FindFProperty<FProperty>(settings->GetClass(),
		GET_MEMBER_NAME_CHECKED(URenderingSettings4D, DefaultProjectionMaterial)), EPropertyChangeType::ValueSet);
	settings->PostEditChangeProperty(change);
	TestTrue(TEXT("Editing the project default invalidates existing projection scene proxies."), projection->IsRenderStateDirty());
	world->SendAllEndOfFrameUpdates();
	TestFalse(TEXT("The projection render state is rebuilt."), projection->IsRenderStateDirty());
	FObjectPropertyBase* cachedMaterial = FindFProperty<FObjectPropertyBase>(projection->GetClass(), TEXT("Material"));
	TestTrue(TEXT("The rebuilt scene proxy retains the new project material."),
		cachedMaterial->GetObjectPropertyValue_InContainer(projection) == replacement);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
