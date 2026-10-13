#include "ModelActor4D.h"

#include "ModelAsset4D.h"
#include "../../Scene/Components/Scene4DComponent.h"

AModelActor4D::AModelActor4D()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AModelActor4D::DestroyGeneratedComponents()
{
	if (RootComponent == GeneratedRoot)
	{
		SetRootComponent(nullptr);
	}
	for (UActorComponent* comp : GeneratedComponents)
	{
		if (IsValid(comp))
		{
			RemoveInstanceComponent(comp);
			comp->DestroyComponent(/*bPromoteChildren=*/ false);
			// Destroyed objects remain until garbage collection. Free their names so
			// a rebuilt model can use the same node name for a different component class.
			comp->Rename(nullptr, GetTransientPackage(), REN_DontCreateRedirectors | REN_NonTransactional | REN_ForceNoResetLoaders);
		}
	}
	GeneratedComponents.Reset();
	GeneratedRoot = nullptr;
};

void AModelActor4D::RebuildFromAsset()
{
	// Blueprint subclasses and callers may add unrelated components, so only
	// destroy components created by this actor's model-generation process.
	DestroyGeneratedComponents();

	if (!IsValid(Model))
	{
		LastBuiltModel = nullptr;
		return;
	}

	// Snapshot every existing component subobject before generating the model hierarchy.
	// GetComponents() can omit unregistered components, which would cause us to mistake
	// an unrelated component for part of the generated model and destroy it on rebuild.
	TSet<UActorComponent*> preExistingComponents;
	TArray<UObject*> preExistingSubObjects;
	GetObjectsWithOuter(this, preExistingSubObjects, /*bIncludeNestedObjects=*/ true);
	for (UObject* subObject : preExistingSubObjects)
	{
		if (UActorComponent* component = Cast<UActorComponent>(subObject))
		{
			preExistingComponents.Add(component);
		}
	}

	UScene4DComponent* newRoot = Model->GenerateComponentHierarchyForActor(this);

	TArray<UObject*> generatedSubObjects;
	GetObjectsWithOuter(this, generatedSubObjects, /*bIncludeNestedObjects=*/ true);
	for (UObject* subObject : generatedSubObjects)
	{
		UActorComponent* comp = Cast<UActorComponent>(subObject);
		if (IsValid(comp) && !preExistingComponents.Contains(comp))
		{
			GeneratedComponents.Add(comp);
		}
	}

	if (!IsValid(newRoot))
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("AModelActor4D::RebuildFromAsset: GenerateComponentHierarchyForActor returned an invalid root."));
		DestroyGeneratedComponents();
		return;
	}

	// Install the generated root as the Actor's root.
	SetRootComponent(newRoot);
	GeneratedRoot = newRoot;

	// Register components and mark them as Instance-created.
	//    Components from NewObject<> are unregistered. We use GetObjectsWithOuter
	//    (not the attachment tree, which isn't populated until after registration)
	//    to find them all.
	//
	//    AddInstanceComponent sets CreationMethod to Instance, which is required
	//    for visibility in the editor's Details panel. Native components are hidden
	//    unless backed by a UPROPERTY, and UserConstructionScript components are
	//    hidden when bHideConstructionScriptComponentsInDetailsView is true
	//    (the default in BlueprintEditorSettings). Instance is the only
	//    CreationMethod that passes the editor's ShouldAddInstancedActorComponent
	//    filter unconditionally (see SubobjectDataSubsystem.cpp).
	UWorld* world = GetWorld();
	if (world)
	{
		for (UActorComponent* comp : GeneratedComponents)
		{
			if (!IsValid(comp))
			{
				continue;
			}
			// Ensure editor treats generated components as instanced actor components.
			// This avoids intermittent "Native components are editable when declared as a FProperty" issues.
			comp->CreationMethod = EComponentCreationMethod::Instance;
			if (!GetInstanceComponents().Contains(comp))
			{
				AddInstanceComponent(comp);
			}
			// Register the component so it becomes part of the actor's hierarchy and is visible in the editor.
			if (!comp->IsRegistered())
			{
				comp->RegisterComponent();
			}
		}
	}
	else
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("AModelActor4D::RebuildFromAsset: GetWorld() is null - cannot register components!"));
	}

	// Record which Model was used so the OnConstruction guard
	//    can detect when a rebuild is actually needed.
	LastBuiltModel = Model;
}

// AActor hooks.

void AModelActor4D::OnConstruction(const FTransform& InTransform)
{
	Super::OnConstruction(InTransform);
	// Guard: only rebuild when the hierarchy is actually stale.
	// After the initial build, the editor triggers RerunConstructionScripts
	// (via PostEditChange / PostEditMove) which re-enters OnConstruction.
	// Without this check, each rebuild would create new components, which
	// triggers yet another RerunConstructionScripts, causing O(n) cascading
	// reconstructions.  If the root is already valid and matches the current
	// Model, the hierarchy is up-to-date and we can skip the rebuild.
	if (IsValid(GeneratedRoot) && GeneratedRoot == RootComponent && IsValid(Model) && Model == LastBuiltModel)
	{
		return;
	}
	RebuildFromAsset();
}

void AModelActor4D::PostLoad()
{
	Super::PostLoad();
	// Do NOT rebuild here. When loading a saved level, UE has already
	// deserialized the InstanceComponents from disk with their full state
	// (including user-modified 4D transforms and mesh data). GeneratedRoot
	// and LastBuiltModel are also deserialized, so the OnConstruction guard
	// will correctly skip unnecessary rebuilds. Calling RebuildFromAsset()
	// would destroy the deserialized components and regenerate them from
	// the asset, losing any per-instance edits.
}

#if WITH_EDITOR
void AModelActor4D::PostEditChangeProperty(FPropertyChangedEvent& InPropertyChangedEvent)
{
	Super::PostEditChangeProperty(InPropertyChangedEvent);
	const FName propName = InPropertyChangedEvent.GetPropertyName();
	if (propName == GET_MEMBER_NAME_CHECKED(AModelActor4D, Model))
	{
		// Super::PostEditChangeProperty above calls RerunConstructionScripts,
		// which triggers OnConstruction. If that already rebuilt for the new
		// Model (the usual case), LastBuiltModel == Model and we're done.
		// This explicit call is a safety net for paths where construction
		// scripts are suppressed (e.g. FScopedSuspendRerunConstructionScripts).
		if (Model != LastBuiltModel)
		{
			RebuildFromAsset();
		}
	}
}
#endif
