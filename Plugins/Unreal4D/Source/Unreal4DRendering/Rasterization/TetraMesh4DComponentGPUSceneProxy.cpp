#include "TetraMesh4DComponentGPUSceneProxy.h"

#include "Components/PrimitiveComponent.h"
#include "MaterialDomain.h"
#include "Materials/Material.h"
#include "PhysicsEngine/BodySetup.h"
#include "SceneManagement.h"
#include "SceneInterface.h"
#include "SceneView.h"

FTetraMesh4DComponentGPUSceneProxy::FTetraMesh4DComponentGPUSceneProxy(
	const UPrimitiveComponent* InComponent,
	UMaterialInterface* InMaterial,
	const FCamera4DRenderStatePtr& InCameraRenderState,
	const FTetraMesh4DRenderStatePtr& InMeshRenderState,
	const FTetraMesh4DRenderResourcePtr& InRenderResource)
	: FPrimitiveSceneProxy(InComponent)
	, VertexFactory(GMaxRHIFeatureLevel)
	, CameraRenderState(InCameraRenderState)
	, MeshRenderState(InMeshRenderState)
	, RenderResource(InRenderResource)
{
	check(CameraRenderState.IsValid());
	check(MeshRenderState.IsValid());
	check(RenderResource.IsValid());

	UMaterialInterface* EffectiveMaterial = InMaterial ? InMaterial : UMaterial::GetDefaultMaterial(MD_Surface);
	UMaterial* Mat = EffectiveMaterial->GetMaterial();

	MaterialProxy = EffectiveMaterial->GetRenderProxy();
	const EShaderPlatform ShaderPlatform = GetScene().GetShaderPlatform();
	MaterialRelevance = Mat->GetRelevance_Concurrent(ShaderPlatform);

	FTetraMesh4DVertexFactory::FVertexDataType vertexData;
	const uint32 stride = sizeof(FTetraMesh4DVertex);
	FTetraMesh4DVertexBuffer& vertexBuffer = RenderResource->GetVertexBuffer();

	vertexData.VertexIdComponent = FVertexStreamComponent(&vertexBuffer, STRUCT_OFFSET(FTetraMesh4DVertex, VertexId), stride, VET_UInt);
	vertexData.TetraPosition0Component = FVertexStreamComponent(&vertexBuffer, STRUCT_OFFSET(FTetraMesh4DVertex, TetraPosition0), stride, VET_Float4);
	vertexData.TetraPosition1Component = FVertexStreamComponent(&vertexBuffer, STRUCT_OFFSET(FTetraMesh4DVertex, TetraPosition1), stride, VET_Float4);
	vertexData.TetraPosition2Component = FVertexStreamComponent(&vertexBuffer, STRUCT_OFFSET(FTetraMesh4DVertex, TetraPosition2), stride, VET_Float4);
	vertexData.TetraPosition3Component = FVertexStreamComponent(&vertexBuffer, STRUCT_OFFSET(FTetraMesh4DVertex, TetraPosition3), stride, VET_Float4);
	vertexData.TetraNormal0Component = FVertexStreamComponent(&vertexBuffer, STRUCT_OFFSET(FTetraMesh4DVertex, TetraNormal0), stride, VET_Float4);
	vertexData.TetraNormal1Component = FVertexStreamComponent(&vertexBuffer, STRUCT_OFFSET(FTetraMesh4DVertex, TetraNormal1), stride, VET_Float4);
	vertexData.TetraNormal2Component = FVertexStreamComponent(&vertexBuffer, STRUCT_OFFSET(FTetraMesh4DVertex, TetraNormal2), stride, VET_Float4);
	vertexData.TetraNormal3Component = FVertexStreamComponent(&vertexBuffer, STRUCT_OFFSET(FTetraMesh4DVertex, TetraNormal3), stride, VET_Float4);
	vertexData.TetraUV0Component = FVertexStreamComponent(&vertexBuffer, STRUCT_OFFSET(FTetraMesh4DVertex, TetraUV0), stride, VET_Float2);
	vertexData.TetraUV1Component = FVertexStreamComponent(&vertexBuffer, STRUCT_OFFSET(FTetraMesh4DVertex, TetraUV1), stride, VET_Float2);
	vertexData.TetraUV2Component = FVertexStreamComponent(&vertexBuffer, STRUCT_OFFSET(FTetraMesh4DVertex, TetraUV2), stride, VET_Float2);
	vertexData.TetraUV3Component = FVertexStreamComponent(&vertexBuffer, STRUCT_OFFSET(FTetraMesh4DVertex, TetraUV3), stride, VET_Float2);
	vertexData.TetraParams0Component = FVertexStreamComponent(&vertexBuffer, STRUCT_OFFSET(FTetraMesh4DVertex, TetraParams0), stride, VET_Float4);
	vertexData.TetraParams1Component = FVertexStreamComponent(&vertexBuffer, STRUCT_OFFSET(FTetraMesh4DVertex, TetraParams1), stride, VET_Float4);

	VertexFactory.SetData(vertexData);
	VertexFactory.SetCameraRenderState(CameraRenderState);
	VertexFactory.SetMeshRenderState(MeshRenderState);
	BeginInitResource(&VertexFactory);

	bCastDynamicShadow = true;
	bCastStaticShadow = true;

	BodySetup = const_cast<UPrimitiveComponent*>(InComponent)->GetBodySetup();
}

/*
 * Submits mesh batches for the dynamic draw path.
 *
 * Used when GPU resources or draw state may change per frame. In this path,
 * mesh batches are rebuilt every frame, including rebinding of vertex factory,
 * buffers, and per-draw uniform data, allowing fully dynamic geometry and state.
 */
void FTetraMesh4DComponentGPUSceneProxy::GetDynamicMeshElements(
	const TArray<const FSceneView*>& Views,
	const FSceneViewFamily& ViewFamily,
	uint32 VisibilityMap,
	FMeshElementCollector& Collector) const
{

#if WITH_EDITOR

		for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
		{
			if (VisibilityMap & (1 << ViewIndex))
			{
				// Render collision
				if (ViewFamily.EngineShowFlags.Collision && BodySetup)
				{
					const FColor SimpleCollisionColor = FColor(157, 149, 223, 255);
					BodySetup->AggGeom.GetAggGeom(FTransform(GetLocalToWorld()), GetSelectionColor(SimpleCollisionColor, IsSelected(), IsHovered()).ToFColor(true), nullptr, false, false, false, ViewIndex, Collector);
				}

				// Render bounds
				if (ViewFamily.EngineShowFlags.Bounds)
				{
					RenderBounds(
						Collector.GetPDI(ViewIndex),
						ViewFamily.EngineShowFlags,
						GetBounds(),
						IsSelected()
					);
				}
			}
		}

#endif //~ WITH_EDITOR

	//------------------------------------------------------------------------------
	// NOTE:
	// Dynamic mesh submission is temporarily disabled.
	//
	// This function is currently used only for debug rendering. The actual
	// 4D mesh is rendered through the static draw path (DrawStaticElements).
	//
	// Dynamic mesh path is kept here for future use but is not used at the moment
	// to avoid duplicate submissions and rendering issues.
	//------------------------------------------------------------------------------

	//for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
	//{
	//	if (!(VisibilityMap & (1 << ViewIndex)))
	//	{
	//		continue;
	//	}

	//	FMeshBatch& Mesh = Collector.AllocateMesh();
	//	FMeshBatchElement& BatchElement = Mesh.Elements[0];

	//	Mesh.VertexFactory = &VertexFactory;
	//	Mesh.MaterialRenderProxy = MaterialProxy;
	//	Mesh.ReverseCulling = false;
	//	Mesh.Type = PT_TriangleList;
	//	Mesh.DepthPriorityGroup = SDPG_World;
	//	Mesh.bCanApplyViewModeOverrides = true;
	//	Mesh.LCI = nullptr;
	//	Mesh.CastShadow = true;
	//	Mesh.bUseForMaterial = true;
	//	Mesh.bUseForDepthPass = true;
	//	Mesh.bUseAsOccluder = false;

	//	BatchElement.IndexBuffer = &RenderResource->GetIndexBuffer();
	//	BatchElement.FirstIndex = 0;
	//	BatchElement.NumPrimitives = RenderResource->GetNumPrimitives();
	//	BatchElement.MinVertexIndex = 0;
	//	BatchElement.MaxVertexIndex = RenderResource->GetNumVertices() > 0 ? RenderResource->GetNumVertices() - 1 : 0;
	//	BatchElement.PrimitiveUniformBuffer = GetUniformBuffer();
	//	BatchElement.PrimitiveIdMode = PrimID_FromPrimitiveSceneInfo;

	//	Collector.AddMesh(ViewIndex, Mesh);
	//}
}

FTetraMesh4DComponentGPUSceneProxy::~FTetraMesh4DComponentGPUSceneProxy()
{
	VertexFactory.ReleaseResource();
}

/*
 * Submits mesh batches for the static draw path.
 *
 * Used when the set of GPU resources (vertex factory, buffers, uniform data)
 * and draw configuration remain stable across frames. In this path, resource
 * bindings and draw commands are established once during scene proxy creation,
 * avoiding per-frame rebinding and draw setup on the render thread.
 */
FPrimitiveViewRelevance FTetraMesh4DComponentGPUSceneProxy::GetViewRelevance(const FSceneView* View) const
{
	FPrimitiveViewRelevance Result;

	Result.bDrawRelevance = IsShown(View);
	Result.bShadowRelevance = IsShadowCast(View);

	// only static relevance is enabled for now.
	Result.bStaticRelevance = true;
	Result.bDynamicRelevance = (View->Family->EngineShowFlags.Collision && BodySetup)
							|| View->Family->EngineShowFlags.Bounds;

	Result.bRenderInMainPass = ShouldRenderInMainPass();
	Result.bUsesLightingChannels = GetLightingChannelMask() != GetDefaultLightingChannelMask();
	Result.bRenderCustomDepth = ShouldRenderCustomDepth();
	Result.bTranslucentSelfShadow = bCastVolumetricTranslucentShadow;
	Result.bVelocityRelevance = IsMovable() & Result.bOpaque & Result.bRenderInMainPass;
	Result.bEditorPrimitiveRelevance = UseEditorCompositing(View);
	MaterialRelevance.SetPrimitiveViewRelevance(Result);
	return Result;
}

/*
 * Submits static mesh batches for the static draw path.
 *
 * Used when the mesh draw state is stable across frames. In this path, mesh submission
 * is performed once at scene proxy creation time instead of being rebuilt every frame,
 * which avoids repeated per-frame setup of draw data and resource bindings.
 */
void FTetraMesh4DComponentGPUSceneProxy::DrawStaticElements(FStaticPrimitiveDrawInterface* PDI)
{
	check(VertexFactory.IsInitialized());
	check(RenderResource != nullptr);
	check(RenderResource->GetVertexBuffer().IsInitialized());
	check(RenderResource->GetIndexBuffer().IsInitialized());

	FMeshBatch Mesh;
	FMeshBatchElement& BatchElement = Mesh.Elements[0];

	Mesh.VertexFactory = &VertexFactory;
	Mesh.MaterialRenderProxy = MaterialProxy;
	Mesh.CastShadow = true;
	Mesh.ReverseCulling = false;
	Mesh.Type = PT_TriangleList;
	Mesh.DepthPriorityGroup = SDPG_World;
	Mesh.bCanApplyViewModeOverrides = true;
	Mesh.LCI = nullptr;
	Mesh.CastRayTracedShadow = false;
	Mesh.bUseForMaterial = true;
	Mesh.bUseForDepthPass = true;
	Mesh.bUseAsOccluder = false;
	Mesh.LODIndex = 0;
	Mesh.SegmentIndex = 0;

	BatchElement.IndexBuffer = &RenderResource->GetIndexBuffer();
	BatchElement.bForceInstanceCulling = false;
	BatchElement.bFetchInstanceCountFromScene = false;
	BatchElement.bIsInstanceRuns = false;
	BatchElement.bIsSplineProxy = false;
	BatchElement.bPreserveInstanceOrder = false;
	BatchElement.bUserDataIsColorVertexBuffer = false;
	BatchElement.FirstIndex = 0;
	BatchElement.NumPrimitives = RenderResource->GetNumPrimitives();
	BatchElement.MinVertexIndex = 0;
	BatchElement.MaxVertexIndex = RenderResource->GetNumVertices() > 0 ? RenderResource->GetNumVertices() - 1 : 0;
	BatchElement.PrimitiveUniformBuffer = GetUniformBuffer();

	PDI->DrawMesh(Mesh, 0.0f);
}
