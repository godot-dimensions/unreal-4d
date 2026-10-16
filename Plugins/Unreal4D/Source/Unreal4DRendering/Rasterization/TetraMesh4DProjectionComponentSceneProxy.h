/*
 * Scene proxy implementation for GPU-based 4D mesh projection rendering.
 */

#pragma once

#include "TetraMesh4DProjectionVertexFactory.h"
#include "TetraMesh4DRenderResource.h"

#include "Materials/MaterialInterface.h"
#include "PrimitiveSceneProxy.h"

/*
 * Render-thread representation of UMesh4DProjectionComponent.
 *
 * This proxy renders the projected 4D mesh through a separate primitive so its
 * bounds and visibility are independent from the sliced mesh primitive.
 */
class UNREAL4DRENDERING_API FTetraMesh4DProjectionComponentSceneProxy : public FPrimitiveSceneProxy
{
public:
	FTetraMesh4DProjectionComponentSceneProxy(
		const UPrimitiveComponent* InComponent,
		UMaterialInterface* InMaterial,
		const FCamera4DRenderStatePtr& InCameraRenderState,
		const FTetraMesh4DRenderStatePtr& InMeshRenderState,
		const FTetraMesh4DRenderResourcePtr& InRenderResource);

	virtual ~FTetraMesh4DProjectionComponentSceneProxy() override;

	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override;
	virtual void DrawStaticElements(FStaticPrimitiveDrawInterface* PDI) override;
	virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const override;

	virtual uint32 GetMemoryFootprint() const override
	{
		return sizeof(*this);
	}

	virtual SIZE_T GetTypeHash() const override
	{
		static int32 UniquePointer;
		return reinterpret_cast<SIZE_T>(&UniquePointer);
	}

private:
	void BuildMeshBatch(FMeshBatch& Mesh) const;

	FTetraMesh4DProjectionVertexFactory VertexFactory;
	FCamera4DRenderStatePtr CameraRenderState;
	FTetraMesh4DRenderStatePtr MeshRenderState;
	FTetraMesh4DRenderResourcePtr RenderResource;
	FMaterialRenderProxy* MaterialProxy = nullptr;
	FMaterialRelevance MaterialRelevance;
};
