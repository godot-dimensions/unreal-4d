#include "RenderingServer4D.h"

#include "Unreal4DRendering/Rasterization/CameraUniform4D.h"
#include "Engine/World.h"

CameraUniform4DShaderParameters URenderingServer4D::BuildCameraUniform4DShaderParameters(const FTransform4D& InCameraTransform4D, const FTransform& InCameraTransform3D)
{
	const FMatrix cameraRotation4D = InCameraTransform4D.Basis.GetTransposed();
	// the 3D camera rotation is also sent to the GPU so sliced
	// vertices can be rotated back to compensate for the camera's 3D rotation.
	// This is required because the 3D camera rotation will be applied again later
	// by the Unreal Engine rendering pipeline, outside the custom vertex factory path.
	const FMatrix cameraRotation3D = InCameraTransform3D.GetRotation().ToMatrix();
	const FMatrix44f camera4DRotation(
		FPlane4f(
			(float)cameraRotation4D.M[0][0],
			(float)cameraRotation4D.M[1][0],
			(float)cameraRotation4D.M[2][0],
			(float)cameraRotation4D.M[3][0]
		),
		FPlane4f(
			(float)cameraRotation4D.M[0][1],
			(float)cameraRotation4D.M[1][1],
			(float)cameraRotation4D.M[2][1],
			(float)cameraRotation4D.M[3][1]
		),
		FPlane4f(
			(float)cameraRotation4D.M[0][2],
			(float)cameraRotation4D.M[1][2],
			(float)cameraRotation4D.M[2][2],
			(float)cameraRotation4D.M[3][2]
		),
		FPlane4f(
			(float)cameraRotation4D.M[0][3],
			(float)cameraRotation4D.M[1][3],
			(float)cameraRotation4D.M[2][3],
			(float)cameraRotation4D.M[3][3]
		)
	);
	const FMatrix44f camera3DRotation(
		FPlane4f(
			(float)cameraRotation3D.M[0][0],
			(float)cameraRotation3D.M[1][0],
			(float)cameraRotation3D.M[2][0],
			(float)cameraRotation3D.M[3][0]
		),
		FPlane4f(
			(float)cameraRotation3D.M[0][1],
			(float)cameraRotation3D.M[1][1],
			(float)cameraRotation3D.M[2][1],
			(float)cameraRotation3D.M[3][1]
		),
		FPlane4f(
			(float)cameraRotation3D.M[0][2],
			(float)cameraRotation3D.M[1][2],
			(float)cameraRotation3D.M[2][2],
			(float)cameraRotation3D.M[3][2]
		),
		FPlane4f(
			(float)cameraRotation3D.M[0][3],
			(float)cameraRotation3D.M[1][3],
			(float)cameraRotation3D.M[2][3],
			(float)cameraRotation3D.M[3][3]
		)
	);
	const FVector4f camera4DPosition = FVector4f(InCameraTransform4D.Origin);
	CameraUniform4DShaderParameters cameraUniform4DShaderParameters;
	cameraUniform4DShaderParameters.Camera4DPosition = camera4DPosition;
	cameraUniform4DShaderParameters.Camera4DRotation = camera4DRotation;
	cameraUniform4DShaderParameters.Camera3DRotation = camera3DRotation;
	return cameraUniform4DShaderParameters;
}

void URenderingServer4D::CreateCameraRenderState()
{
	if (CameraRenderState.IsValid())
	{
		return;
	}

	CameraRenderState = MakeShared<FCamera4DRenderState, ESPMode::ThreadSafe>();
	BeginInitResource(&CameraRenderState->Resource);
}

void URenderingServer4D::BeginReleaseCameraRenderState()
{
	if (bCameraRenderStateReleaseRequested || !CameraRenderState.IsValid())
	{
		return;
	}

	// enqueue asynchronous release of the camera render resource
	// on the render thread.
	BeginReleaseResource(&CameraRenderState->Resource);
	// start a render-command fence so we can later verify that
	// the previously enqueued camera render resource release has completed.
	CameraRenderStateReleaseFence.BeginFence();
	bCameraRenderStateReleaseRequested = true;
}

void URenderingServer4D::UpdateDirtyMeshBounds()
{
	const FTransform4D cameraTransform4D = GetCurrentCameraTransform4D();
	const FTransform cameraTransform3D = GetCurrentCameraTransform3D();
	// Bounds and component state are UObject-owned and inexpensive, so keep this work on the game thread.
	for (const FDirtyMesh& dirtyMesh : DirtyMeshes)
	{
		if (UMesh4DComponent* mesh = dirtyMesh.MeshComponent.Get())
		{
			mesh->UpdateBoundsForCamera(cameraTransform4D, cameraTransform3D);
		}
	}
	DirtyMeshes.Reset();
}

void URenderingServer4D::UpdateCameraUniform()
{
	if (!CameraRenderState.IsValid())
	{
		return;
	}
	const FTransform cameraTransform3D = GetCurrentCameraTransform3D();
	const FTransform4D cameraTransform4D = GetCurrentCameraTransform4D();
	CameraUniform4DShaderParameters cameraUniform4DShaderParameters = BuildCameraUniform4DShaderParameters(cameraTransform4D, cameraTransform3D);
	const FCamera4DRenderStatePtr cameraRenderStateRef = CameraRenderState;
	// asynchronously push updated 4D camera data to the render thread
	// to refresh the camera uniform buffer used for rendering 4D geometry.
	ENQUEUE_RENDER_COMMAND(UpdateCameraUniform4D)(
		[cameraRenderStateRef, cameraUniform4DShaderParameters](FRHICommandListImmediate& RHICmdList)
		{
			if (cameraRenderStateRef.IsValid())
			{
				cameraRenderStateRef->Resource.UpdateUniformData(
					cameraUniform4DShaderParameters,
					RHICmdList);
			}
		});
}

bool URenderingServer4D::AreSliceHyperplanesEqual(const FTransform4D& InSliceA, const FTransform4D& InSliceB)
{
	// Compare slice hyperplanes
	const FVector4 sliceNormalA = UBasis4D::Basis4D_GetRow(InSliceA.Basis, 3);
	const FVector4 sliceNormalB = UBasis4D::Basis4D_GetRow(InSliceB.Basis, 3);
	// Compare hyperplane normals.
	const bool bNormalsEqual = sliceNormalA.Equals(sliceNormalB, UNREAL4D_COMPARISON_EPSILON);
	// Compare hyperplane offsets. Use same normal to avoid compounding floating-point error.
	const bool bOffsetsEqual = FMath::IsNearlyEqual(Dot4(sliceNormalA, InSliceA.Origin), Dot4(sliceNormalA, InSliceB.Origin), UNREAL4D_COMPARISON_EPSILON);
	return bNormalsEqual && bOffsetsEqual;
}

void URenderingServer4D::OnWorldPreSendAllEndOfFrameUpdates(UWorld* InWorld)
{
	check(IsInGameThread());
	if (InWorld != GetWorld())
	{
		return;
	}
	UpdateCameraUniform();
	// Check if the slice has changed such that we need to rebuild all meshes
	{
		const FTransform4D& cameraTransform4D = GetCurrentCameraTransform4D();

		// Bounds depend on both the slice normal and its offset through 4D space.
		if (!AreSliceHyperplanesEqual(cameraTransform4D, CameraTransform4DPrevious))
		{
			DirtyAllMeshBounds();
		}

		CameraTransform4DPrevious = cameraTransform4D;
	}
	UpdateDirtyMeshBounds();
}

void URenderingServer4D::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	// Create the shared camera render state that owns the GPU
	// uniform buffer for the current 4D camera.
	if (!CameraRenderState.IsValid())
	{
		CreateCameraRenderState();
	}
	FWorldDelegates::OnWorldPreSendAllEndOfFrameUpdates.AddUObject(this, &URenderingServer4D::OnWorldPreSendAllEndOfFrameUpdates);
}

void URenderingServer4D::Deinitialize()
{
	Super::Deinitialize();
	RegisteredMeshes.Empty();
	DirtyMeshes.Empty();
	RegisteredCameras.Empty();
	FWorldDelegates::OnWorldPreSendAllEndOfFrameUpdates.RemoveAll(this);
}

bool URenderingServer4D::DoesSupportWorldType(const EWorldType::Type InWorldType) const
{
	switch (InWorldType)
	{
		case EWorldType::Game:
		case EWorldType::Editor:
		case EWorldType::PIE:
		case EWorldType::EditorPreview:
		{
			return true;
		}
		default:
		{
			return false;
		}
	}
}

/*
 * Use the UObject destruction lifecycle to safely begin releasing
 * the camera render state owned by URenderingServer4D.
 */
void URenderingServer4D::BeginDestroy()
{
	BeginReleaseCameraRenderState();
	Super::BeginDestroy();
}

/*
 * Wait for CameraRenderStateReleaseFence to complete before allowing
 * final destruction, ensuring that all render-thread work using the camera render state
 * has finished and the GPU resource has been safely released.
 */
bool URenderingServer4D::IsReadyForFinishDestroy()
{
	if (bCameraRenderStateReleaseRequested && !CameraRenderStateReleaseFence.IsFenceComplete())
	{
		return false;
	}
	if (bCameraRenderStateReleaseRequested)
	{
		CameraRenderState.Reset();
	}
	return Super::IsReadyForFinishDestroy();
}

// Cameras.

void URenderingServer4D::RegisterCamera(UCamera4DComponent* InCamera)
{
	RegisteredCameras.Add(InCamera);
	if (InCamera->IsCurrent())
	{
		MakeCameraCurrent(InCamera);
	}
}

void URenderingServer4D::UnregisterCamera(UCamera4DComponent* InCamera)
{
	UCamera4DComponent* wasCurrentCamera = GetCurrentCamera();
	if (InCamera == wasCurrentCamera)
	{
		RegisteredCameras.RemoveAt(0);
		if (RegisteredCameras.Num() > 0)
		{
			MakeCameraCurrent(RegisteredCameras[0].Get());
		}
	}
	else
	{
		RegisteredCameras.Remove(InCamera);
	}
}

void URenderingServer4D::MakeCameraCurrent(UCamera4DComponent* InCamera)
{
	if (unlikely(!InCamera->IsCurrent()))
	{
		// Will set the camera's variable and call MakeCameraCurrent again.
		InCamera->MakeCurrent();
		return;
	}
	UCamera4DComponent* oldCurrentCamera = GetCurrentCamera();
	if (InCamera == oldCurrentCamera)
	{
		return; // Nothing to do.
	}
	if (oldCurrentCamera != nullptr && oldCurrentCamera->IsCurrent())
	{
		oldCurrentCamera->ClearCurrent(false);
	}
	if (RegisteredCameras.Contains(InCamera))
	{
		RegisteredCameras.Remove(InCamera);
	}
	RegisteredCameras.Insert(InCamera, 0);
}

void URenderingServer4D::ClearCameraCurrent(UCamera4DComponent* InCamera)
{
	if (unlikely(InCamera->IsCurrent()))
	{
		InCamera->ClearCurrent(false);
	}
	UCamera4DComponent* currentCamera = GetCurrentCamera();
	if (InCamera == currentCamera)
	{
		RegisteredCameras.RemoveAt(0);
		if (RegisteredCameras.Num() > 0)
		{
			UCamera4DComponent* nextCamera = RegisteredCameras[0].Get();
			nextCamera->MakeCurrent();
		}
	}
}

UCamera4DComponent* URenderingServer4D::GetCurrentCamera() const
{
	return RegisteredCameras.Num() > 0 ? RegisteredCameras[0].Get() : nullptr;
}

FTransform URenderingServer4D::GetCurrentCameraTransform3D() const
{
	// TODO: Cache camera transform.
	if (bIsUsingOverrideCameraTransform)
	{
		return OverrideCameraTransform3D;
	}
	// Get the 3D transform of the current camera's root component.
	UCamera4DComponent* currentCamera = GetCurrentCamera();
	if (currentCamera != nullptr)
	{
		UScene4DComponent* cameraRoot4DComponent = Cast<UScene4DComponent>(currentCamera->GetOwner()->GetRootComponent());
		if (cameraRoot4DComponent != nullptr)
		{
			return cameraRoot4DComponent->GetComponentTransform();
		}
	}
	return FTransform::Identity;
}

FTransform4D URenderingServer4D::GetCurrentCameraTransform4D() const
{
	// TODO: Cache camera transform.
	if (bIsUsingOverrideCameraTransform)
	{
		return OverrideCameraTransform4D;
	}
	// Get the 4D transform of the current camera.
	UCamera4DComponent* currentCamera = GetCurrentCamera();
	if (currentCamera != nullptr)
	{
		return currentCamera->GetGlobalTransform4D();
	}
	return FTransform4D::Identity();
}

void URenderingServer4D::SetOverrideCameraTransforms(const FTransform4D& InCameraTransform4D, const FTransform& InCameraTransform3D)
{
	OverrideCameraTransform4D = InCameraTransform4D;
	OverrideCameraTransform3D = InCameraTransform3D;
	bIsUsingOverrideCameraTransform = true;
}

void URenderingServer4D::RegisterMesh(UMesh4DComponent* InMesh)
{
	if (IsValid(InMesh) && !RegisteredMeshes.Contains(InMesh))
	{
		FRegisteredMesh& registeredMesh = RegisteredMeshes.AddDefaulted_GetRef();
		registeredMesh.MeshComponent = InMesh;
		DirtyMeshBounds(InMesh);
		OnMeshRegistered.Broadcast(InMesh);
	}
}

void URenderingServer4D::UnregisterMesh(UMesh4DComponent* InMesh)
{
	if (IsValid(InMesh))
	{
		const int32 index = RegisteredMeshes.IndexOfByKey(InMesh);
		const bool bWasRegistered = index != INDEX_NONE;
		if (index != INDEX_NONE)
		{
			RegisteredMeshes.RemoveAtSwap(index);
		}
		const int32 dirtyIndex = DirtyMeshes.IndexOfByKey(InMesh);
		if (dirtyIndex != INDEX_NONE)
		{
			DirtyMeshes.RemoveAtSwap(dirtyIndex);
		}
		if (bWasRegistered)
		{
			OnMeshUnregistered.Broadcast(InMesh);
		}
	}
}

TArray<TWeakObjectPtr<UMesh4DComponent>> URenderingServer4D::GetRegisteredMeshes() const
{
	TArray<TWeakObjectPtr<UMesh4DComponent>> meshes;
	meshes.Reserve(RegisteredMeshes.Num());
	for (const FRegisteredMesh& registeredMesh : RegisteredMeshes)
	{
		meshes.Add(registeredMesh.MeshComponent);
	}
	return meshes;
}

void URenderingServer4D::DirtyMeshBounds(UMesh4DComponent* InMesh)
{
	if (IsValid(InMesh) && !DirtyMeshes.Contains(InMesh))
	{
		FDirtyMesh& dirtyMesh = DirtyMeshes.AddDefaulted_GetRef();
		dirtyMesh.MeshComponent = InMesh;
	}
}

void URenderingServer4D::DirtyAllMeshBounds()
{
	DirtyMeshes.Reset(RegisteredMeshes.Num());
	for (const FRegisteredMesh& registeredMesh : RegisteredMeshes)
	{
		FDirtyMesh& dirtyMesh = DirtyMeshes.AddDefaulted_GetRef();
		dirtyMesh.MeshComponent = registeredMesh.MeshComponent;
	}
}

FVector4 URenderingServer4D::Vector3DTo4D(const FVector& InVector3D) const
{
	const FTransform4D& cameraTransform4D = GetCurrentCameraTransform4D();
	const FTransform& cameraTransform3D = GetCurrentCameraTransform3D();
	return cameraTransform4D.Basis.TransformFVector4(FVector4(cameraTransform3D.InverseTransformVector(InVector3D), 0.0f));
}

FVector URenderingServer4D::Vector4DTo3D(const FVector4& InVector4D) const
{
	const FTransform4D& cameraTransform4D = GetCurrentCameraTransform4D();
	const FTransform& cameraTransform3D = GetCurrentCameraTransform3D();
	const FVector4 vectorSliceSpace = cameraTransform4D.Basis.Inverse().TransformFVector4(InVector4D);
	return cameraTransform3D.TransformVector(FVector(vectorSliceSpace.X, vectorSliceSpace.Y, vectorSliceSpace.Z));
}

FVector4 URenderingServer4D::Position3DTo4D(const FVector& InPosition3D) const
{
	const FTransform4D cameraTransform4D = GetCurrentCameraTransform4D();
	const FTransform cameraTransform3D = GetCurrentCameraTransform3D();
	return cameraTransform4D.TransformPosition(FVector4(cameraTransform3D.InverseTransformPosition(InPosition3D), 0.0f));
}

FVector URenderingServer4D::Position4DTo3D(const FVector4& InPosition4D) const
{
	const FTransform4D cameraTransform4D = GetCurrentCameraTransform4D();
	const FTransform cameraTransform3D = GetCurrentCameraTransform3D();
	const FVector4 positionSliceSpace = cameraTransform4D.InverseTransformPosition(InPosition4D);
	return cameraTransform3D.TransformPosition(FVector(positionSliceSpace.X, positionSliceSpace.Y, positionSliceSpace.Z));
}

FVector4 URenderingServer4D::Position4DToSlice4D(const FVector4& InPosition4D) const
{
	const FTransform4D cameraTransform4D = GetCurrentCameraTransform4D();
	return cameraTransform4D.InverseTransformPosition(InPosition4D);
}

float URenderingServer4D::SliceDistance4D(const FVector4& InPosition4D) const
{
	return FMath::Abs(Position4DToSlice4D(InPosition4D).W);
}
