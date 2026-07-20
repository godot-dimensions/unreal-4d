#include "Mesh4DComponent.h"

#include "Mesh4DProjectionComponent.h"

#include "../../Model/Mesh/Tetra/TetraMesh4D.h"
#include "../../Render/RenderingServer4D.h"
#include "Unreal4DRendering/Rasterization/TetraMesh4DComponentGPUSceneProxy.h"

#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "MaterialDomain.h"
#include "Materials/Material.h"

namespace
{
	static FMesh4DUniformShaderParameters MakeMesh4DUniformShaderParameters(const FMesh4DTransformCache& InData)
	{
		FMesh4DUniformShaderParameters Parameters;
		Parameters.Position4D = InData.Position4D;
		Parameters.RotAndScale4D = InData.RotAndScale4D;
		Parameters.RotAndScale4DInverseTranspose = InData.RotAndScale4DInverseTranspose;
		return Parameters;
	}
}

UMesh4DComponent::UMesh4DComponent(const FObjectInitializer& InInit)
	: Super(InInit)
{
	CastShadow = true;
	bCastDynamicShadow = true;
	SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetGenerateOverlapEvents(false);
}

void UMesh4DComponent::BindMesh4DDelegate()
{
	if (Mesh4D == nullptr || BoundMesh4D == Mesh4D)
	{
		return;
	}

	Mesh4DCacheClearedHandle = Mesh4D->OnMesh4DCacheCleared.AddUObject(this, &UMesh4DComponent::OnUpdatedMesh4DData);
	BoundMesh4D = Mesh4D;
}

void UMesh4DComponent::UnbindMesh4DDelegate()
{
	if (BoundMesh4D == nullptr)
	{
		return;
	}

	BoundMesh4D->OnMesh4DCacheCleared.Remove(Mesh4DCacheClearedHandle);
	Mesh4DCacheClearedHandle.Reset();
	BoundMesh4D = nullptr;
}

void UMesh4DComponent::CreateMeshRenderState()
{
	if (MeshRenderState.IsValid())
	{
		return;
	}

	UpdateCachedUniformData();

	MeshRenderState = MakeShared<FTetraMesh4DRenderState, ESPMode::ThreadSafe>();

	// Enqueues asynchronous initialization of the mesh render resource on the render thread.
	BeginInitResource(&MeshRenderState->Resource);
}

void UMesh4DComponent::BeginReleaseMeshRenderState()
{
	if (bMeshRenderStateReleaseRequested || !MeshRenderState.IsValid())
	{
		return;
	}

	// Enqueues asynchronous release of the mesh render resource on the render thread.
	BeginReleaseResource(&MeshRenderState->Resource);

	// Starts a render-command fence so IsReadyForFinishDestroy() can verify that
	// all previously enqueued render-thread work using this resource has completed.
	MeshRenderStateReleaseFence.BeginFence();
	bMeshRenderStateReleaseRequested = true;
}


void UMesh4DComponent::EnsureProjectionComponentCreated()
{
	if (!EnableProjection)
	{
		return;
	}

	if (ProjectionComponent != nullptr)
	{
		if (IsValid(ProjectionComponent) && ProjectionComponent->IsRegistered())
		{
			return;
		}

		ProjectionComponent = nullptr;
	}

	AActor* Owner = GetOwner();
	if (Owner == nullptr)
	{
		return;
	}

	const FName ProjectionComponentName = MakeUniqueObjectName(
		Owner,
		UMesh4DProjectionComponent::StaticClass(),
		TEXT("Unreal4DMesh4DProjectionComponent"));

	ProjectionComponent = NewObject<UMesh4DProjectionComponent>(
		Owner,
		UMesh4DProjectionComponent::StaticClass(),
		ProjectionComponentName,
		RF_Transient);

	if (ProjectionComponent == nullptr)
	{
		return;
	}

	ProjectionComponent->CreationMethod = EComponentCreationMethod::Native;
	ProjectionComponent->SetSourceComponent(this);
	ProjectionComponent->SetupAttachment(this);
	ProjectionComponent->SetRelativeTransform(FTransform::Identity);

	ProjectionComponent->SetVisibility(EnableProjection, true);
	ProjectionComponent->SetHiddenInGame(!EnableProjection, true);

	ProjectionComponent->RegisterComponent();
}

void UMesh4DComponent::UpdateProjectionComponentState()
{
	if (EnableProjection)
	{
		EnsureProjectionComponentCreated();
	}

	if (ProjectionComponent == nullptr)
	{
		return;
	}

	ProjectionComponent->SetProjectionEnabled(EnableProjection);
	ProjectionComponent->SetProjectionAppearance(ProjectionColor, ProjectionOpacity, ProjectionScale, ProjectionDistance);
	ProjectionComponent->SetHiddenInGame(bHiddenInGame);

	FTransform ProjectionBoundsTransform = CachedBoundsTransform;
	ProjectionBoundsTransform.SetScale3D(FVector::OneVector);
	ProjectionComponent->SetProjectionBoundsTransform(ProjectionBoundsTransform);
}

void UMesh4DComponent::UpdateCachedUniformData()
{
	const FTransform4D Transform4D = GetGlobalTransform4D();

	CachedUniformData.Position4D = FVector4f(Transform4D.Origin);
	CachedUniformData.RotAndScale4D = FMatrix44f(
		FPlane4f(
			(float)Transform4D.Basis.M[0][0],
			(float)Transform4D.Basis.M[1][0],
			(float)Transform4D.Basis.M[2][0],
			(float)Transform4D.Basis.M[3][0]
		),
		FPlane4f(
			(float)Transform4D.Basis.M[0][1],
			(float)Transform4D.Basis.M[1][1],
			(float)Transform4D.Basis.M[2][1],
			(float)Transform4D.Basis.M[3][1]
		),
		FPlane4f(
			(float)Transform4D.Basis.M[0][2],
			(float)Transform4D.Basis.M[1][2],
			(float)Transform4D.Basis.M[2][2],
			(float)Transform4D.Basis.M[3][2]
		),
		FPlane4f(
			(float)Transform4D.Basis.M[0][3],
			(float)Transform4D.Basis.M[1][3],
			(float)Transform4D.Basis.M[2][3],
			(float)Transform4D.Basis.M[3][3]
		)
	);

	const FMatrix RotAndScaleMatInversedAndTransposed = Transform4D.Basis.Inverse().GetTransposed();
	CachedUniformData.RotAndScale4DInverseTranspose = FMatrix44f(
		FPlane4f(
			(float)RotAndScaleMatInversedAndTransposed.M[0][0],
			(float)RotAndScaleMatInversedAndTransposed.M[1][0],
			(float)RotAndScaleMatInversedAndTransposed.M[2][0],
			(float)RotAndScaleMatInversedAndTransposed.M[3][0]
		),
		FPlane4f(
			(float)RotAndScaleMatInversedAndTransposed.M[0][1],
			(float)RotAndScaleMatInversedAndTransposed.M[1][1],
			(float)RotAndScaleMatInversedAndTransposed.M[2][1],
			(float)RotAndScaleMatInversedAndTransposed.M[3][1]
		),
		FPlane4f(
			(float)RotAndScaleMatInversedAndTransposed.M[0][2],
			(float)RotAndScaleMatInversedAndTransposed.M[1][2],
			(float)RotAndScaleMatInversedAndTransposed.M[2][2],
			(float)RotAndScaleMatInversedAndTransposed.M[3][2]
		),
		FPlane4f(
			(float)RotAndScaleMatInversedAndTransposed.M[0][3],
			(float)RotAndScaleMatInversedAndTransposed.M[1][3],
			(float)RotAndScaleMatInversedAndTransposed.M[2][3],
			(float)RotAndScaleMatInversedAndTransposed.M[3][3]
		)
	);
}

void UMesh4DComponent::OnUpdatedMesh4DData()
{
	RebuildCachedLocalMeshBounds();
	UpdateProjectionComponentState();

	MarkBoundsDirty();
	MarkRenderTransformDirty();
	MarkRenderStateDirty();

	if (ProjectionComponent != nullptr)
	{
		ProjectionComponent->MarkRenderStateDirty();
	}
}

void UMesh4DComponent::OnTransform4DChanged()
{
	UpdateCachedUniformData();

	MarkBoundsDirty();
	MarkRenderTransformDirty();
	MarkRenderDynamicDataDirty();
}

#if WITH_EDITOR

void UMesh4DComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const FName PropertyName = PropertyChangedEvent.GetPropertyName();

	const bool bMesh4DChanged =
		PropertyName == GET_MEMBER_NAME_CHECKED(UMesh4DComponent, Mesh4D);

	const bool bSliceMaterialChanged =
		PropertyName == GET_MEMBER_NAME_CHECKED(UMesh4DComponent, Material);

	const bool bProjectionMaterialChanged =
		PropertyName == GET_MEMBER_NAME_CHECKED(UMesh4DComponent, ProjectionMaterial);

	const bool bProjectionEnabledChanged =
		PropertyName == GET_MEMBER_NAME_CHECKED(UMesh4DComponent, EnableProjection);

	const bool bProjectionAppearanceChanged =
		PropertyName == GET_MEMBER_NAME_CHECKED(UMesh4DComponent, ProjectionColor)
		|| PropertyName == GET_MEMBER_NAME_CHECKED(UMesh4DComponent, ProjectionOpacity)
		|| PropertyName == GET_MEMBER_NAME_CHECKED(UMesh4DComponent, ProjectionScale)
		|| PropertyName == GET_MEMBER_NAME_CHECKED(USceneComponent, bHiddenInGame);

	if (bMesh4DChanged)
	{
		UnbindMesh4DDelegate();
		BindMesh4DDelegate();

		OnUpdatedMesh4DData();
	}

	if (bSliceMaterialChanged)
	{
		MarkRenderStateDirty();
	}

	if (bProjectionMaterialChanged)
	{
		if (ProjectionComponent != nullptr)
		{
			UpdateProjectionComponentState();
			ProjectionComponent->MarkRenderStateDirty();
		}
	}

	if (bProjectionEnabledChanged)
	{
		UpdateProjectionComponentState();
	}

	if (bProjectionAppearanceChanged)
	{
		if (EnableProjection)
		{
			EnsureProjectionComponentCreated();
		}

		if (ProjectionComponent != nullptr)
		{
			ProjectionComponent->SetProjectionAppearance(
				ProjectionColor,
				ProjectionOpacity,
				ProjectionScale,
				ProjectionDistance);
		}
	}
}

#endif //~ WITH_EDITOR

void UMesh4DComponent::OnRegister()
{
	CreateMeshRenderState();

	Super::OnRegister();

	if (UWorld* World = GetWorld())
	{
		if (URenderingServer4D* Server = World->GetSubsystem<URenderingServer4D>())
		{
			Server->RegisterMesh(this);
		}
	}

	BindMesh4DDelegate();

	RebuildCachedLocalMeshBounds();
	UpdateProjectionComponentState();

}

void UMesh4DComponent::OnUnregister()
{
	Super::OnUnregister();

	if (UWorld* World = GetWorld())
	{
		if (URenderingServer4D* Server = World->GetSubsystem<URenderingServer4D>())
		{
			Server->UnregisterMesh(this);
		}
	}

	UnbindMesh4DDelegate();

	if (ProjectionComponent != nullptr)
	{
		ProjectionComponent->DestroyComponent();
		ProjectionComponent = nullptr;
	}

}

void UMesh4DComponent::CreateRenderState_Concurrent(FRegisterComponentContext* Context)
{
	// Pre-create the GPU render resource while still on the game thread.
	// FScene::AddPrimitive is called with a FRegisterComponentContext and deferred;
	// FRegisterComponentContext::Process can then dispatch CreateSceneProxy() inside
	// a ParallelFor on task threads. Pre-creating here keeps task-thread proxy
	// creation on the fast path where it only copies the shared resource pointer.
	if (Mesh4D != nullptr && Mesh4D->IsMeshDataValid())
	{
		Mesh4D->GetOrCreateRenderResource();
	}

	Super::CreateRenderState_Concurrent(Context);

	// If the render state is re-created `SendRenderDynamicData_Concurrent` will not be called and so we must additionally update our uniform data in `CreateRenderState_Concurrent`
	UpdateUniformData_Concurrent();
}

/*
 * Pushes updated per-mesh uniform data to the render thread.
 */
void UMesh4DComponent::SendRenderDynamicData_Concurrent()
{
	Super::SendRenderDynamicData_Concurrent();

	UpdateUniformData_Concurrent();
}

void UMesh4DComponent::UpdateUniformData_Concurrent()
{
	if (!ensure(MeshRenderState.IsValid()))
	{
		return;
	}

	const FMesh4DUniformShaderParameters UniformParameters = MakeMesh4DUniformShaderParameters(CachedUniformData);
	const TSharedPtr<FTetraMesh4DRenderState, ESPMode::ThreadSafe> MeshRenderStateRef = MeshRenderState;

	// Enqueues an asynchronous render-thread command to update the per-mesh
	// uniform buffer with the latest cached 4D transform data.
	ENQUEUE_RENDER_COMMAND(Unreal4DUpdateMesh4DUniform)(
		[MeshRenderStateRef, UniformParameters](FRHICommandList& RHICmdList)
		{
			if (MeshRenderStateRef.IsValid())
			{
				MeshRenderStateRef->Resource.UpdateUniformData(UniformParameters, RHICmdList);
			}
		});
}

void UMesh4DComponent::BeginDestroy()
{
	Super::BeginDestroy();

	BeginReleaseMeshRenderState();
}

bool UMesh4DComponent::IsReadyForFinishDestroy()
{
	if (bMeshRenderStateReleaseRequested && !MeshRenderStateReleaseFence.IsFenceComplete())
	{
		return false;
	}

	if (bMeshRenderStateReleaseRequested)
	{
		MeshRenderState.Reset();
	}

	return Super::IsReadyForFinishDestroy();
}

void UMesh4DComponent::RebuildCachedLocalMeshBounds()
{
	if (Mesh4D == nullptr)
	{
		CachedLocalMeshBounds = FBoxSphereBounds(FVector::ZeroVector, FVector::ZeroVector, 0.0f);
		LocalBoundsSphereRadius4D = 0.0f;
		return;
	}

	FVector4 MeshLocalBoundsOrigin = FVector4::Zero();
	FVector4 MeshLocalBoundsExtents = FVector4::Zero();
	Mesh4D->GetLocalBounds(MeshLocalBoundsOrigin, MeshLocalBoundsExtents);

	const float SphereRadius = MeshLocalBoundsExtents.Size();

	CachedLocalMeshBounds = FBoxSphereBounds(MeshLocalBoundsOrigin, FVector(SphereRadius), SphereRadius);
	LocalBoundsSphereRadius4D = SphereRadius;
}

void UMesh4DComponent::UpdateBoundsForCamera(const FTransform4D& InCameraTransform4D, const FTransform& InCameraTransform3D)
{
	check(IsInGameThread());
	UpdateCachedBoundsTransform(InCameraTransform4D, InCameraTransform3D);
	UpdateBounds();
	UpdateProjectionComponentState();
	MarkRenderTransformDirty();
}

void UMesh4DComponent::MarkBoundsDirty()
{
	if (UWorld* World = GetWorld())
	{
		if (URenderingServer4D* RenderingServer = World->GetSubsystem<URenderingServer4D>())
		{
			RenderingServer->DirtyMeshBounds(this);
		}
	}
}

/*
 * Updates the CPU-side bounds transform for the current 4D slice.
 *
 * Converts the object's 4D transform into camera-relative slice space, maps it
 * back to UE world space, and scales the bounds using the radius of the sliced
 * 4D bounding hypersphere.
 */
void UMesh4DComponent::UpdateCachedBoundsTransform(const FTransform4D& CameraTransform4D, const FTransform& CameraTransform3D)
{
	// TODO: 4D -> 3D mesh bounds calculation is currently overly conservative, leading to large bounds even when the object is not in the slice;
	// improve 4D -> 3D mesh bounds calculation.

	const FTransform4D GlobalTransform4D = GetGlobalTransform4D();

	const FTransform4D RelToCamera4D = CameraTransform4D.Inverse() * GlobalTransform4D;

	FTransform BoundsTransform = RelToCamera4D.To3D() * CameraTransform3D;

	const FVector4 Scale4D = GlobalTransform4D.GetScaleAbs();
	const float MaxScale = FMath::Max(FMath::Max3(Scale4D.X, Scale4D.Y, Scale4D.Z), Scale4D.W);

	const float EffectiveRadius = LocalBoundsSphereRadius4D * MaxScale;
	const float DistanceW = FMath::Abs((float)RelToCamera4D.Origin.W);

	BoundsTransform.SetRotation(FQuat::Identity);

	if (LocalBoundsSphereRadius4D <= UE_SMALL_NUMBER || DistanceW >= EffectiveRadius)
	{
		BoundsTransform.SetScale3D(FVector::ZeroVector);
		CachedBoundsTransform = BoundsTransform;
		return;
	}

	const float SliceRadius = FMath::Sqrt(
		FMath::Max(0.0f, EffectiveRadius * EffectiveRadius - DistanceW * DistanceW));

	const float SliceScale = SliceRadius / LocalBoundsSphereRadius4D;

	BoundsTransform.SetScale3D(FVector(SliceScale));

	CachedBoundsTransform = BoundsTransform;
}

UMesh4D* UMesh4DComponent::GetMesh4DForProjection() const
{
	return Mesh4D;
}

UMaterialInterface* UMesh4DComponent::GetProjectionMaterialForProjectionComponent() const
{
	if (ProjectionMaterial != nullptr)
	{
		return ProjectionMaterial;
	}

	return UMaterial::GetDefaultMaterial(MD_Surface);
}

FLinearColor UMesh4DComponent::GetProjectionColorForProjectionComponent() const
{
	return ProjectionColor;
}

float UMesh4DComponent::GetProjectionOpacityForProjectionComponent() const
{
	return ProjectionOpacity;
}

float UMesh4DComponent::GetProjectionScaleForProjectionComponent() const
{
	return ProjectionScale;
}

TSharedPtr<FTetraMesh4DRenderState, ESPMode::ThreadSafe> UMesh4DComponent::GetMeshRenderStateForProjection() const
{
	return MeshRenderState;
}

const FBoxSphereBounds& UMesh4DComponent::GetLocalMeshBoundsForProjection() const
{
	return CachedLocalMeshBounds;
}

FPrimitiveSceneProxy* UMesh4DComponent::CreateSceneProxy()
{
	if (Mesh4D == nullptr || !Mesh4D->IsMeshDataValid())
	{
		return nullptr;
	}

	UTetraMesh4D* TetraMesh4D = Cast<UTetraMesh4D>(Mesh4D);
	if (TetraMesh4D == nullptr)
	{
		ensure(false);
		return nullptr;
	}

	UWorld* World = GetWorld();
	URenderingServer4D* RenderingServer4D = World ? World->GetSubsystem<URenderingServer4D>() : nullptr;
	if (RenderingServer4D == nullptr)
	{
		return nullptr;
	}

	const TSharedPtr<FCamera4DRenderState, ESPMode::ThreadSafe> CameraRenderState = RenderingServer4D->GetCameraRenderState();
	ensure(CameraRenderState.IsValid() && MeshRenderState.IsValid());
	if (!CameraRenderState.IsValid() || !MeshRenderState.IsValid())
	{
		return nullptr;
	}

	FTetraMesh4DRenderResourcePtr RenderResource = TetraMesh4D->GetOrCreateRenderResource();
	ensure(RenderResource.IsValid());
	if (!RenderResource.IsValid())
	{
		return nullptr;
	}

	return new FTetraMesh4DComponentGPUSceneProxy(this, GetMaterial(0), CameraRenderState, MeshRenderState, RenderResource);
}

UMaterialInterface* UMesh4DComponent::GetMaterial(int32 ElementIndex) const
{
	if (Material)
	{
		return Material;
	}
	return UMaterial::GetDefaultMaterial(MD_Surface);
}

void UMesh4DComponent::SetMaterial(int32 ElementIndex, UMaterialInterface* InMaterial)
{
	if ((Material != InMaterial) && (ElementIndex == 0))
	{
		Material = InMaterial;

		MarkRenderStateDirty();
	}
}

int32 UMesh4DComponent::GetNumMaterials() const
{
	return 1;
}

void UMesh4DComponent::GetUsedMaterials(TArray<UMaterialInterface*>& OutMaterials, bool bGetDebugMaterials) const
{
	OutMaterials.Add(GetMaterial(0));
}

/*
 * Computes a temporary conservative bounds approximation for the 4D mesh.
 *
 * The box extent is built as a cube with equal X/Y/Z half-extents, using the maximum
 * absolute coordinate across all 4D axes. The sphere radius is computed from the
 * maximum possible 4D distance from the origin.
 *
 * This is a simple test approximation intended to keep the primitive visible until
 * a more accurate 4D-aware bounds implementation is introduced.
 */
FBoxSphereBounds UMesh4DComponent::CalcBounds(const FTransform& LocalToWorld) const
{
	return CachedLocalMeshBounds.TransformBy(CachedBoundsTransform);
}
