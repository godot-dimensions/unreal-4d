#pragma once

class FCamera4DRenderState;

#include "../Scene/Components/Camera4DComponent.h"
#include "../Scene/Components/Mesh4DComponent.h"

#include "RenderCommandFence.h"
#include "Subsystems/WorldSubsystem.h"
#include "Templates/SharedPointer.h"
#include "UObject/WeakObjectPtr.h"

#include "RenderingServer4D.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMesh4DRegistrationDelegate, UMesh4DComponent*, InMesh);

// World subsystem that:
// - Tracks registered cameras and mesh components (weak refs).
// - Maintains a single "current" camera.
// - Updates meshes at the end of the game frame.
UCLASS(Category = "Unreal 4D|Render")
class UNREAL4DCORE_API URenderingServer4D : public UWorldSubsystem
{
	GENERATED_BODY()

private:
	/** A 4D mesh registered with the rendering server. */
	struct FRegisteredMesh
	{
		TWeakObjectPtr<UMesh4DComponent> MeshComponent;

		bool operator==(UMesh4DComponent* InMesh) const
		{
			return (MeshComponent == InMesh);
		}
	};

	/** A 4D mesh that needs to be updated at the end of the game frame. */
	struct FDirtyMesh
	{
		TWeakObjectPtr<UMesh4DComponent> MeshComponent;

		bool operator==(UMesh4DComponent* InMesh) const
		{
			return (MeshComponent == InMesh);
		}
	};

private:
	TArray<FRegisteredMesh> RegisteredMeshes;
	TArray<FDirtyMesh> DirtyMeshes;

	TArray<TWeakObjectPtr<UCamera4DComponent>> RegisteredCameras;

	// Used by the editor camera.
	FTransform4D OverrideCameraTransform4D = FTransform4D::Identity();
	FTransform OverrideCameraTransform3D = FTransform::Identity;
	bool bIsUsingOverrideCameraTransform = false;

	TSharedPtr<FCamera4DRenderState, ESPMode::ThreadSafe> CameraRenderState;
	FRenderCommandFence CameraRenderStateReleaseFence;
	bool bCameraRenderStateReleaseRequested = false;

	/** Tracks if the slice has materially changed since the prior game frame. */
	FTransform4D CameraTransform4DPrevious = FTransform4D::Identity();

private:
	static CameraUniform4DShaderParameters BuildCameraUniform4DShaderParameters(const FTransform4D& InCameraTransform4D, const FTransform& InCameraTransform3D);
	static bool AreSliceHyperplanesEqual(const FTransform4D& InSliceA, const FTransform4D& InSliceB);
	void UpdateCameraUniform();
	// Per-frame callback (game thread).
	void OnWorldPreSendAllEndOfFrameUpdates(UWorld* InWorld);
	void CreateCameraRenderState();
	void BeginReleaseCameraRenderState();
	void UpdateDirtyMeshBounds();

public:
	// UWorldSubsystem.
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool DoesSupportWorldType(const EWorldType::Type InWorldType) const override;

	virtual void BeginDestroy() override;
	virtual bool IsReadyForFinishDestroy() override;

	// Cameras.
	void RegisterCamera(UCamera4DComponent* InCamera);
	void UnregisterCamera(UCamera4DComponent* InCamera);
	void MakeCameraCurrent(UCamera4DComponent* InCamera);
	void ClearCameraCurrent(UCamera4DComponent* InCamera);
	UCamera4DComponent* GetCurrentCamera() const;

	// Camera transform getters.
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Render")
	FTransform GetCurrentCameraTransform3D() const;

	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Render")
	FTransform4D GetCurrentCameraTransform4D() const;

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Unreal 4D|Render")
	void SetOverrideCameraTransforms(const FTransform4D& InCameraTransform4D, const FTransform& InCameraTransform3D);

	// Meshes.
	void RegisterMesh(UMesh4DComponent* InMesh);
	void UnregisterMesh(UMesh4DComponent* InMesh);
	void DirtyMeshBounds(UMesh4DComponent* InMesh);
	void DirtyAllMeshBounds();

	// RegisterMesh/UnregisterMesh notify synchronously after registry membership and dirty bounds change.
	// Component OnRegister/OnUnregister may still be running; defer component creation or destruction.
	UPROPERTY(Transient, BlueprintAssignable, Category = "Unreal 4D|Render")
	FMesh4DRegistrationDelegate OnMeshRegistered;

	UPROPERTY(Transient, BlueprintAssignable, Category = "Unreal 4D|Render")
	FMesh4DRegistrationDelegate OnMeshUnregistered;

	// Return a snapshot so callbacks cannot invalidate iteration. Resolve weak references before use.
	TArray<TWeakObjectPtr<UMesh4DComponent>> GetRegisteredMeshes() const;

	const TSharedPtr<FCamera4DRenderState, ESPMode::ThreadSafe>& GetCameraRenderState() const
	{
		return CameraRenderState;
	}

	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Render")
	FVector4 Vector3DTo4D(const FVector& InVector3D) const;

	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Render")
	FVector Vector4DTo3D(const FVector4& InVector4D) const;

	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Render")
	FVector4 Position3DTo4D(const FVector& InPosition3D) const;

	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Render")
	FVector Position4DTo3D(const FVector4& InPosition4D) const;

	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Render")
	FVector4 Position4DToSlice4D(const FVector4& InPosition4D) const;

	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Render")
	float SliceDistance4D(const FVector4& InPosition4D) const;
};
