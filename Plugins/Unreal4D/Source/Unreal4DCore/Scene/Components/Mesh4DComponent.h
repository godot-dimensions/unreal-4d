/*
 * GPU-backed 4D mesh component.
 *
 * Represents a 4D mesh whose rendering is performed on the GPU through
 * FTetraMesh4DComponentGPUSceneProxy and FTetraMesh4DVertexFactory.
 *
 * This component owns the game-thread state for a 4D mesh instance, tracks its
 * current 4D transform, and synchronizes per-mesh render data required by the
 * render thread.
 */

#pragma once

#include "Primitive4DComponent.h"

#include "Unreal4DRendering/Rasterization/TetraMesh4DVertexFactory.h"
#include "RenderCommandFence.h"
#include "Templates/SharedPointer.h"

#include "Mesh4DComponent.generated.h"

class UMesh4D;
class UTetraMesh4D;
class UMesh4DProjectionComponent;
class UMaterialInterface;
class FMesh4DRenderState;

UENUM(BlueprintType, Category = "Unreal 4D|Component|Mesh")
enum class EMesh4DPhysicsMode : uint8
{
	None,
	ConcaveMesh,
	ConvexHull,
};

struct FMesh4DTransformCache
{
	FVector4f Position4D = FVector4f(0.0f, 0.0f, 0.0f, 0.0f);
	FMatrix44f RotAndScale4D = FMatrix44f::Identity;
	FMatrix44f RotAndScale4DInverseTranspose = FMatrix44f::Identity;
};

UCLASS(BlueprintType, Category = "Unreal 4D|Component|Mesh", meta=(BlueprintSpawnableComponent))
class UNREAL4DCORE_API UMesh4DComponent : public UPrimitive4DComponent
{
	GENERATED_BODY()

private:
	UFUNCTION()
	void OnUpdatedMesh4DData();

	void CreateMeshRenderState();
	void BeginReleaseMeshRenderState();
	void UpdateCachedUniformData();
	void EnsureProjectionComponentCreated();
	void UpdateProjectionComponentState();
	void BindMesh4DDelegate();
	void UnbindMesh4DDelegate();

protected:
	virtual UMesh4D* GetCollisionMesh4D() const;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR

	void OnTransform4DChanged() override;
	virtual void OnRegister() override;
	virtual void OnUnregister() override;
	virtual void CreateRenderState_Concurrent(FRegisterComponentContext* Context) override;
	virtual void SendRenderDynamicData_Concurrent() override;
	void UpdateUniformData_Concurrent();
	virtual void BeginDestroy() override;
	virtual bool IsReadyForFinishDestroy() override;
	void RebuildCachedLocalMeshBounds();

public:

	UMesh4DComponent(const FObjectInitializer& InInit);

	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;
	virtual UMaterialInterface* GetMaterial(int32 ElementIndex) const final;
	virtual void SetMaterial(int32 ElementIndex, UMaterialInterface* Material) override;
	virtual int32 GetNumMaterials() const override;
	virtual void GetUsedMaterials(TArray<UMaterialInterface*>& OutMaterials, bool bGetDebugMaterials = false) const override;
	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;
	virtual bool QueryPointCollision(const FVector4& InPoint, FCollisionManifold4D& OutCollision) const override;

	/** Refreshes the camera-relative bounds used by Unreal's renderer and the projection component. */
	virtual void UpdateBoundsForCamera(const FTransform4D& InCameraTransform4D, const FTransform& InCameraTransform3D);

	/** Flags the mesh as needing a camera-relative bounds update at the end of the frame.
	  * @see URenderingServer4D.
	  */
	void MarkBoundsDirty();

	UMesh4D* GetMesh4DForProjection() const;
	virtual UMaterialInterface* GetProjectionMaterialForProjectionComponent() const;
	FLinearColor GetProjectionColorForProjectionComponent() const;
	float GetProjectionOpacityForProjectionComponent() const;
	float GetProjectionScaleForProjectionComponent() const;
	TSharedPtr<FTetraMesh4DRenderState, ESPMode::ThreadSafe> GetMeshRenderStateForProjection() const;
	const FBoxSphereBounds& GetLocalMeshBoundsForProjection() const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mesh 4D")
	TObjectPtr<UMesh4D> Mesh4D = nullptr;

	// Enables 4D point queries against this mesh, independently of 3D collision.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal 4D|Physics|Mesh")
	bool EnableCollision = false;

	// Optional mesh used for 4D collision instead of the rendered mesh.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal 4D|Physics|Mesh", meta = (EditCondition = "EnableCollision", EditConditionHides = true))
	TObjectPtr<UMesh4D> CollisionMesh4D = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Unreal 4D|Component|Mesh|Material")
	TObjectPtr <UMaterialInterface> Material = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Unreal 4D|Component|Mesh|Projection")
	bool EnableProjection = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Unreal 4D|Component|Mesh|Projection", meta = (EditCondition = "EnableProjection", EditConditionHides = true))
	TObjectPtr<UMaterialInterface> ProjectionMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Unreal 4D|Component|Mesh|Projection", meta = (EditCondition = "EnableProjection", EditConditionHides = true))
	FLinearColor ProjectionColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Unreal 4D|Component|Mesh|Projection", meta = (EditCondition = "EnableProjection", EditConditionHides = true))
	float ProjectionOpacity = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Unreal 4D|Component|Mesh|Projection", meta = (EditCondition = "EnableProjection", EditConditionHides = true))
	float ProjectionScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Unreal 4D|Component|Mesh|Projection", meta = (EditCondition = "EnableProjection", EditConditionHides = true))
	float ProjectionDistance = 2000.0f;

private:
	void UpdateCachedBoundsTransform(const FTransform4D& InCameraTransform4D, const FTransform& InCameraTransform3D);

	FMesh4DTransformCache CachedUniformData;

	/*
	 * Thread-safe shared pointer to the per-mesh render state for this component instance.
	 *
	 * Stores render-thread data associated with this 4D mesh, including the uniform buffer
	 * containing its per-instance 4D transform.
	 *
	 * Shared ownership is used to guarantee correct lifetime across the game thread and
	 * render thread, and to prevent use-after-free while render-thread updates are still
	 * in flight via enqueued render commands.
	 */
	TSharedPtr<FTetraMesh4DRenderState, ESPMode::ThreadSafe> MeshRenderState;

	/*
	 * Render-thread fence used to guarantee safe release of MeshRenderState resources.
	 *
	 * Ensures that all previously enqueued render commands using this render state have
	 * completed before destruction is finalized.
	 */
	FRenderCommandFence MeshRenderStateReleaseFence;

	/*
	* Radius of the mesh bounding hypersphere in local 4D space.
	* Used to compute the scale of the 3D bounds based on the slice of the hypersphere
	* relative to the current 4D camera. This allows updating bounds size during
	* slicing when the object or the 4D camera transform changes.
	*/
	float LocalBoundsSphereRadius4D = 0.0f;

	bool bMeshRenderStateReleaseRequested = false;

	/*
	 * Handle for the OnMesh4DCacheCleared delegate bound to the currently referenced Mesh4D asset.
	 * Managed by BindMesh4DDelegate() / UnbindMesh4DDelegate().
	 */
	FDelegateHandle Mesh4DCacheClearedHandle;

	/*
	 * The Mesh4D asset that OnMesh4DCacheCleared is currently bound to.
	 * Kept separately so we can safely unbind even after Mesh4D has been reassigned.
	 */
	UPROPERTY(Transient)
	TObjectPtr<UMesh4D> BoundMesh4D = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UMesh4DProjectionComponent> ProjectionComponent = nullptr;

protected:
	// Cached local-space bounds built from the 4D mesh data (expensive, updated rarely)
	FBoxSphereBounds CachedLocalMeshBounds;

	// CPU-side transform used ONLY for bounds (decoupled from actual component transform)
	FTransform CachedBoundsTransform;
};
