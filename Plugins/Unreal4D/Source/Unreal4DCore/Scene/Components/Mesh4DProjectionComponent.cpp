#include "Mesh4DProjectionComponent.h"

#include "Mesh4DComponent.h"
#include "../../Model/Mesh/Tetra/TetraMesh4D.h"
#include "../../Render/RenderingServer4D.h"
#include "Materials/Material.h"
#include "Engine/World.h"
#include "MaterialDomain.h"
#include "Unreal4DRendering/Rasterization/TetraMesh4DProjectionComponentSceneProxy.h"

UMesh4DProjectionComponent::UMesh4DProjectionComponent(const FObjectInitializer& InInit)
	: Super(InInit)
{
	CastShadow = false;
	bCastDynamicShadow = false;
	bCastStaticShadow = false;
	SetCollisionEnabled(ECollisionEnabled::NoCollision);
	UpdateCustomPrimitiveData();
}

void UMesh4DProjectionComponent::SetSourceComponent(UMesh4DComponent* InSourceComponent)
{
	if (SourceComponent == InSourceComponent)
	{
		return;
	}

	SourceComponent = InSourceComponent;

	if (IsRegistered())
	{
		MarkRenderStateDirty();
	}
}

void UMesh4DProjectionComponent::SetProjectionEnabled(bool bInProjectionEnabled)
{
	if (bProjectionEnabled == bInProjectionEnabled)
	{
		return;
	}

	bProjectionEnabled = bInProjectionEnabled;
	SetVisibility(bProjectionEnabled, true);
	SetHiddenInGame(!bProjectionEnabled, true);
}

void UMesh4DProjectionComponent::SetProjectionAppearance(const FLinearColor& InColor, float InOpacity, float InScale, float InDistance)
{
	const float ClampedOpacity = FMath::Clamp(InOpacity, 0.0f, 1.0f);
	const float ClampedScale = FMath::Max(InScale, 0.0f);

	ProjectionColor = InColor;
	ProjectionOpacity = ClampedOpacity;
	ProjectionScale = ClampedScale;
	ProjectionDistance = FMath::Max(InDistance, 1.0f);

	UpdateCustomPrimitiveData();
}

void UMesh4DProjectionComponent::UpdateCustomPrimitiveData()
{
	SetCustomPrimitiveDataFloat(0, ProjectionColor.R);
	SetCustomPrimitiveDataFloat(1, ProjectionColor.G);
	SetCustomPrimitiveDataFloat(2, ProjectionColor.B);
	SetCustomPrimitiveDataFloat(3, ProjectionOpacity);
	SetCustomPrimitiveDataFloat(4, ProjectionScale);
	SetCustomPrimitiveDataFloat(5, ProjectionDistance);
}

void UMesh4DProjectionComponent::SetProjectionBoundsTransform(const FTransform& InBoundsTransform)
{
	CachedBoundsTransform = InBoundsTransform;
	UpdateBounds();
	MarkRenderTransformDirty();
}

void UMesh4DProjectionComponent::CreateRenderState_Concurrent(FRegisterComponentContext* Context)
{
	if (SourceComponent != nullptr)
	{
		if (UMesh4D* Mesh4D = SourceComponent->GetMesh4DForProjection())
		{
			if (Mesh4D->IsMeshDataValid())
			{
				if (UTetraMesh4D* TetraMesh4D = Cast<UTetraMesh4D>(Mesh4D))
				{
					TetraMesh4D->GetOrCreateRenderResource();
				}
			}
		}
	}

	Super::CreateRenderState_Concurrent(Context);
}

FPrimitiveSceneProxy* UMesh4DProjectionComponent::CreateSceneProxy()
{
	if (SourceComponent == nullptr)
	{
		return nullptr;
	}

	UTetraMesh4D* TetraMesh4D = Cast<UTetraMesh4D>(SourceComponent->GetMesh4DForProjection());
	if (TetraMesh4D == nullptr || !TetraMesh4D->IsMeshDataValid())
	{
		return nullptr;
	}

	UWorld* World = GetWorld();
	URenderingServer4D* RenderingServer4D = World ? World->GetSubsystem<URenderingServer4D>() : nullptr;
	if (RenderingServer4D == nullptr)
	{
		return nullptr;
	}

	const TSharedPtr<FCamera4DRenderState, ESPMode::ThreadSafe> CameraRenderState = RenderingServer4D->GetCameraRenderState();
	const TSharedPtr<FTetraMesh4DRenderState, ESPMode::ThreadSafe> MeshRenderState = SourceComponent->GetMeshRenderStateForProjection();
	if (!CameraRenderState.IsValid() || !MeshRenderState.IsValid())
	{
		return nullptr;
	}

	FTetraMesh4DRenderResourcePtr RenderResource = TetraMesh4D->GetOrCreateRenderResource();
	if (!RenderResource.IsValid())
	{
		return nullptr;
	}

	const uint32 NumProjectionPrimitives = RenderResource->GetNumProjectionPrimitives();
	if (NumProjectionPrimitives == 0)
	{
		return nullptr;
	}

	// Store the material reference so that it isnt GC'd
	Material = GetMaterial(0);

	return new FTetraMesh4DProjectionComponentSceneProxy(
		this,
		Material,
		CameraRenderState,
		MeshRenderState,
		RenderResource);
}

UMaterialInterface* UMesh4DProjectionComponent::GetMaterial(int32 ElementIndex) const
{
	if (SourceComponent != nullptr)
	{
		return SourceComponent->GetProjectionMaterialForProjectionComponent();
	}

	return UMaterial::GetDefaultMaterial(MD_Surface);
}

int32 UMesh4DProjectionComponent::GetNumMaterials() const
{
	return 1;
}

void UMesh4DProjectionComponent::GetUsedMaterials(TArray<UMaterialInterface*>& OutMaterials, bool bGetDebugMaterials) const
{
	OutMaterials.Add(GetMaterial(0));
}

FBoxSphereBounds UMesh4DProjectionComponent::CalcBounds(const FTransform& LocalToWorld) const
{
	if (SourceComponent == nullptr)
	{
		return FBoxSphereBounds(FVector::ZeroVector, FVector::ZeroVector, 0.0f);
	}

	return SourceComponent->GetLocalMeshBoundsForProjection().TransformBy(CachedBoundsTransform);
}
