/*
 * Scene proxy implementation for GPU-based 4D mesh rendering.
 */

#pragma once

#include "TetraMesh4DRenderResource.h"
#include "TetraMesh4DVertexFactory.h"

#include "Materials/MaterialInterface.h"
#include "PrimitiveSceneProxy.h"

class UBodySetup;

/*
 * Render-thread representation of UMesh4DComponent.
 *
 * FPrimitiveSceneProxy is the bridge between the game-thread component and the Unreal Engine
 * rendering pipeline. It owns the render-side resources and is responsible for submitting
 * this mesh for rendering by building mesh batches, selecting the appropriate vertex factory,
 * vertex/index buffers, material, and draw path.
 *
 */
class UNREAL4DRENDERING_API FTetraMesh4DComponentGPUSceneProxy : public FPrimitiveSceneProxy
{
public:
	FTetraMesh4DComponentGPUSceneProxy(
		const UPrimitiveComponent* InComponent,
		UMaterialInterface* InMaterial,
		const FCamera4DRenderStatePtr& InCameraRenderState,
		const FTetraMesh4DRenderStatePtr& InMeshRenderState,
		const FTetraMesh4DRenderResourcePtr& InRenderResource);

	virtual ~FTetraMesh4DComponentGPUSceneProxy() override;

	virtual void GetDynamicMeshElements(
		const TArray<const FSceneView*>& Views,
		const FSceneViewFamily& ViewFamily,
		uint32 VisibilityMap,
		FMeshElementCollector& Collector) const override;

	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override;
	virtual void DrawStaticElements(FStaticPrimitiveDrawInterface* PDI) override;

	virtual uint32 GetMemoryFootprint() const override
	{
		return sizeof(*this);
	}

	virtual SIZE_T GetTypeHash() const override
	{
		// Use the address of a function-local static variable as a stable hash
		// shared by all instances of this proxy type.
		// This pattern is commonly used across Unreal Engine scene proxy implementations.
		static int32 uniquePointer;
		return reinterpret_cast<SIZE_T>(&uniquePointer);
	}

	FTetraMesh4DVertexFactory VertexFactory;

private:

	FCamera4DRenderStatePtr CameraRenderState;
	FTetraMesh4DRenderStatePtr MeshRenderState;
	FTetraMesh4DRenderResourcePtr RenderResource = nullptr;
	FMaterialRenderProxy* MaterialProxy = nullptr;
	FMaterialRelevance MaterialRelevance;
	UBodySetup* BodySetup = nullptr;
};
