#include "TetraMesh4DProjectionComponentSceneProxy.h"

#include "MaterialDomain.h"
#include "Materials/Material.h"
#include "Misc/EngineVersionComparison.h"
#include "SceneManagement.h"
#include "SceneInterface.h"

namespace
{
	static void InitProjectionVertexFactory(
		FTetraMesh4DProjectionVertexFactory& InOutVertexFactory,
		FTetraMesh4DProjectionVertexBuffer& InOutVertexBuffer,
		const FCamera4DRenderStatePtr& InCameraRenderState,
		const FTetraMesh4DRenderStatePtr& InMeshRenderState)
	{
		FTetraMesh4DProjectionVertexFactory::FVertexDataType VertexData;
		const uint32 Stride = sizeof(FTetraMesh4DProjectionVertex);

		VertexData.VertexPosition4D = FVertexStreamComponent(&InOutVertexBuffer, STRUCT_OFFSET(FTetraMesh4DProjectionVertex, VertexPosition4D), Stride, VET_Float4);

		InOutVertexFactory.SetData(VertexData);
		InOutVertexFactory.SetCameraRenderState(InCameraRenderState);
		InOutVertexFactory.SetMeshRenderState(InMeshRenderState);
		BeginInitResource(&InOutVertexFactory);
	}
}

FTetraMesh4DProjectionComponentSceneProxy::FTetraMesh4DProjectionComponentSceneProxy(
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
	UMaterial* Material = EffectiveMaterial->GetMaterial();

	MaterialProxy = EffectiveMaterial->GetRenderProxy();
	check(MaterialProxy != nullptr);

	const EShaderPlatform ShaderPlatform = GetScene().GetShaderPlatform();
	MaterialRelevance = Material->GetRelevance_Concurrent(ShaderPlatform);

	InitProjectionVertexFactory(VertexFactory, RenderResource->GetProjectionVertexBuffer(), CameraRenderState, MeshRenderState);

	bCastDynamicShadow = false;
	bCastStaticShadow = false;
}

FTetraMesh4DProjectionComponentSceneProxy::~FTetraMesh4DProjectionComponentSceneProxy()
{
	VertexFactory.ReleaseResource();
}

FPrimitiveViewRelevance FTetraMesh4DProjectionComponentSceneProxy::GetViewRelevance(const FSceneView* View) const
{
	FPrimitiveViewRelevance Result;

	Result.bDrawRelevance = IsShown(View);
	Result.bShadowRelevance = false;
#if UE_VERSION_OLDER_THAN(5, 8, 0)
	Result.bStaticRelevance = true;
	Result.bDynamicRelevance = false;
#else
	Result.bStaticRelevance = false;
	Result.bDynamicRelevance = true;
#endif
	Result.bRenderInMainPass = ShouldRenderInMainPass();
	Result.bUsesLightingChannels = GetLightingChannelMask() != GetDefaultLightingChannelMask();
	Result.bRenderCustomDepth = ShouldRenderCustomDepth();
	Result.bTranslucentSelfShadow = false;
	Result.bEditorPrimitiveRelevance = UseEditorCompositing(View);

	MaterialRelevance.SetPrimitiveViewRelevance(Result);
	return Result;
}

void FTetraMesh4DProjectionComponentSceneProxy::BuildMeshBatch(FMeshBatch& Mesh) const
{
	check(VertexFactory.IsInitialized());
	check(RenderResource.IsValid());
	check(RenderResource->IsProjectionInitialized());

	FMeshBatchElement& BatchElement = Mesh.Elements[0];

	Mesh.VertexFactory = &VertexFactory;
	Mesh.MaterialRenderProxy = MaterialProxy;
	Mesh.CastShadow = false;
	Mesh.ReverseCulling = IsLocalToWorldDeterminantNegative();
	Mesh.Type = PT_TriangleList;
	Mesh.DepthPriorityGroup = SDPG_World;
	Mesh.bCanApplyViewModeOverrides = true;
	Mesh.LCI = nullptr;
	Mesh.CastRayTracedShadow = false;
	Mesh.bUseForMaterial = true;
	Mesh.bUseForDepthPass = false;
	Mesh.bUseAsOccluder = false;
	Mesh.LODIndex = 0;
	Mesh.SegmentIndex = 0;

	const FTetraMesh4DIndexBuffer& IndexBuffer = RenderResource->GetProjectionIndexBuffer();
	const uint32 NumProjectionVertices = RenderResource->GetNumProjectionVertices();
	const uint32 NumProjectionPrimitives = RenderResource->GetNumProjectionPrimitives();

	BatchElement.IndexBuffer = &IndexBuffer;
	BatchElement.bForceInstanceCulling = false;
	BatchElement.bFetchInstanceCountFromScene = false;
	BatchElement.bIsInstanceRuns = false;
#if UE_VERSION_OLDER_THAN(5, 8, 0)
	BatchElement.bIsSplineProxy = false;
#endif
	BatchElement.bPreserveInstanceOrder = false;
	BatchElement.bUserDataIsColorVertexBuffer = false;
	BatchElement.FirstIndex = 0;
	BatchElement.NumPrimitives = NumProjectionPrimitives;
	BatchElement.MinVertexIndex = 0;
	BatchElement.MaxVertexIndex = NumProjectionVertices > 0 ? NumProjectionVertices - 1 : 0;
	BatchElement.PrimitiveUniformBuffer = GetUniformBuffer();
}

void FTetraMesh4DProjectionComponentSceneProxy::DrawStaticElements(FStaticPrimitiveDrawInterface* PDI)
{
#if UE_VERSION_OLDER_THAN(5, 8, 0)
	FMeshBatch Mesh;
	BuildMeshBatch(Mesh);
	PDI->DrawMesh(Mesh, 0.0f);
#endif
}

void FTetraMesh4DProjectionComponentSceneProxy::GetDynamicMeshElements(
	const TArray<const FSceneView*>& Views,
	const FSceneViewFamily& ViewFamily,
	uint32 VisibilityMap,
	FMeshElementCollector& Collector) const
{
#if !UE_VERSION_OLDER_THAN(5, 8, 0)
	// Use the same UE 5.8 submission workaround as the sliced mesh proxy.
	for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
	{
		if (VisibilityMap & (1 << ViewIndex))
		{
			FMeshBatch& Mesh = Collector.AllocateMesh();
			BuildMeshBatch(Mesh);
			Collector.AddMesh(ViewIndex, Mesh);
		}
	}
#endif
}
