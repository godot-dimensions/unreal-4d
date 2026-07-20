#include "TetraMesh4DVertexFactory.h"

#include "MaterialDomain.h"

FTetraMesh4DUniformRenderResource::FTetraMesh4DUniformRenderResource()
{
	CachedMesh4DUniformParameters.Position4D = FVector4f::Zero();
	CachedMesh4DUniformParameters.RotAndScale4D = FMatrix44f::Identity;
	CachedMesh4DUniformParameters.RotAndScale4DInverseTranspose = FMatrix44f::Identity;
}

void FTetraMesh4DUniformRenderResource::InitRHI(FRHICommandListBase& RHICmdList)
{
	// CachedMesh4DUniformParameters is initialized in the UMesh4DComponent::CreateMeshRenderState function
	// after creating FTetraMesh4DRenderState
	Mesh4DUniformBufferRef = TUniformBufferRef<FMesh4DUniformShaderParameters>::CreateUniformBufferImmediate(
		CachedMesh4DUniformParameters,
		UniformBuffer_MultiFrame);
}

void FTetraMesh4DUniformRenderResource::ReleaseRHI()
{
	Mesh4DUniformBufferRef.SafeRelease();
}

void FTetraMesh4DUniformRenderResource::UpdateUniformData(const FMesh4DUniformShaderParameters& InParameters, FRHICommandList& RHICmdList)
{
	CachedMesh4DUniformParameters = InParameters;

	ensure(Mesh4DUniformBufferRef.IsValid());
	if (Mesh4DUniformBufferRef.IsValid())
	{
		Mesh4DUniformBufferRef.UpdateUniformBufferImmediate(RHICmdList, CachedMesh4DUniformParameters);
	}
}

/*
* used to bind shader parameters that are not bound through a uniform buffer (or SRV, UAV), such as pushing constants.
*/
void FTetraMesh4DVertexFactoryShaderParameters::Bind(const FShaderParameterMap& ParameterMap)
{
}

/*
 * Binds per-draw camera and mesh uniform buffers required by the vertex factory shader.
 */
void FTetraMesh4DVertexFactoryShaderParameters::GetElementShaderBindings(
	const FSceneInterface* Scene,
	const FSceneView* View,
	const FMeshMaterialShader* Shader,
	const EVertexInputStreamType InputStreamType,
	ERHIFeatureLevel::Type FeatureLevel,
	const FVertexFactory* VertexFactory,
	const FMeshBatchElement& BatchElement,
	FMeshDrawSingleShaderBindings& ShaderBindings,
	FVertexInputStreamArray& VertexStreams) const
{
	const FTetraMesh4DVertexFactory* vertexFactory = static_cast<const FTetraMesh4DVertexFactory*>(VertexFactory);

	ensure(vertexFactory->GetCameraUniformBuffer() != nullptr);
	ensure(vertexFactory->GetMeshUniformBuffer() != nullptr);

	if (FRHIUniformBuffer* CameraUniformBuffer = vertexFactory->GetCameraUniformBuffer())
	{
		ShaderBindings.Add(Shader->GetUniformBufferParameter<CameraUniform4DShaderParameters>(), CameraUniformBuffer);
	}

	if (FRHIUniformBuffer* MeshUniformBuffer = vertexFactory->GetMeshUniformBuffer())
	{
		ShaderBindings.Add(Shader->GetUniformBufferParameter<FMesh4DUniformShaderParameters>(), MeshUniformBuffer);
	}
}

FTetraMesh4DVertexFactory::FTetraMesh4DVertexFactory(ERHIFeatureLevel::Type InFeatureLevel)
	: FVertexFactory(InFeatureLevel)
{
}

void FTetraMesh4DVertexFactory::SetData(const FVertexDataType& InData)
{
	VertexData = InData;
}

void FTetraMesh4DVertexFactory::InitRHI(FRHICommandListBase& RHICmdList)
{
	check(HasValidFeatureLevel());

	FVertexDeclarationElementList defaultElements;
	defaultElements.Add(AccessStreamComponent(VertexData.VertexIdComponent, 0));
	defaultElements.Add(AccessStreamComponent(VertexData.TetraPosition0Component, 1));
	defaultElements.Add(AccessStreamComponent(VertexData.TetraPosition1Component, 2));
	defaultElements.Add(AccessStreamComponent(VertexData.TetraPosition2Component, 3));
	defaultElements.Add(AccessStreamComponent(VertexData.TetraPosition3Component, 4));
	defaultElements.Add(AccessStreamComponent(VertexData.TetraNormal0Component, 5));
	defaultElements.Add(AccessStreamComponent(VertexData.TetraNormal1Component, 6));
	defaultElements.Add(AccessStreamComponent(VertexData.TetraNormal2Component, 7));
	defaultElements.Add(AccessStreamComponent(VertexData.TetraNormal3Component, 8));
	defaultElements.Add(AccessStreamComponent(VertexData.TetraParams0Component, 9));
	defaultElements.Add(AccessStreamComponent(VertexData.TetraParams1Component, 10));
	defaultElements.Add(AccessStreamComponent(VertexData.TetraUV0Component, 11));
	defaultElements.Add(AccessStreamComponent(VertexData.TetraUV1Component, 12));
	defaultElements.Add(AccessStreamComponent(VertexData.TetraUV2Component, 13));
	defaultElements.Add(AccessStreamComponent(VertexData.TetraUV3Component, 14));
	InitDeclaration(defaultElements, EVertexInputStreamType::Default);

	FVertexDeclarationElementList positionAndNormalOnlyElements;
	positionAndNormalOnlyElements.Add(AccessStreamComponent(VertexData.VertexIdComponent, 0));
	positionAndNormalOnlyElements.Add(AccessStreamComponent(VertexData.TetraPosition0Component, 1));
	positionAndNormalOnlyElements.Add(AccessStreamComponent(VertexData.TetraPosition1Component, 2));
	positionAndNormalOnlyElements.Add(AccessStreamComponent(VertexData.TetraPosition2Component, 3));
	positionAndNormalOnlyElements.Add(AccessStreamComponent(VertexData.TetraPosition3Component, 4));
	positionAndNormalOnlyElements.Add(AccessStreamComponent(VertexData.TetraNormal0Component, 5));
	positionAndNormalOnlyElements.Add(AccessStreamComponent(VertexData.TetraNormal1Component, 6));
	positionAndNormalOnlyElements.Add(AccessStreamComponent(VertexData.TetraNormal2Component, 7));
	positionAndNormalOnlyElements.Add(AccessStreamComponent(VertexData.TetraNormal3Component, 8));
	positionAndNormalOnlyElements.Add(AccessStreamComponent(VertexData.TetraParams0Component, 9));
	InitDeclaration(positionAndNormalOnlyElements, EVertexInputStreamType::PositionAndNormalOnly);

	FVertexDeclarationElementList positionOnlyElements;
	positionOnlyElements.Add(AccessStreamComponent(VertexData.VertexIdComponent, 0));
	positionOnlyElements.Add(AccessStreamComponent(VertexData.TetraPosition0Component, 1));
	positionOnlyElements.Add(AccessStreamComponent(VertexData.TetraPosition1Component, 2));
	positionOnlyElements.Add(AccessStreamComponent(VertexData.TetraPosition2Component, 3));
	positionOnlyElements.Add(AccessStreamComponent(VertexData.TetraPosition3Component, 4));
	positionOnlyElements.Add(AccessStreamComponent(VertexData.TetraNormal0Component, 5));
	positionOnlyElements.Add(AccessStreamComponent(VertexData.TetraNormal1Component, 6));
	positionOnlyElements.Add(AccessStreamComponent(VertexData.TetraNormal2Component, 7));
	positionOnlyElements.Add(AccessStreamComponent(VertexData.TetraNormal3Component, 8));
	InitDeclaration(positionOnlyElements, EVertexInputStreamType::PositionOnly);
}

bool FTetraMesh4DVertexFactory::ShouldCompilePermutation(const FVertexFactoryShaderPermutationParameters& Parameters)
{
	return Parameters.MaterialParameters.MaterialDomain == MD_Surface;
}

void FTetraMesh4DVertexFactory::SetCameraRenderState(const FCamera4DRenderStatePtr& InCameraRenderState)
{
	CameraRenderState = InCameraRenderState;
}

void FTetraMesh4DVertexFactory::SetMeshRenderState(const FTetraMesh4DRenderStatePtr& InMeshRenderState)
{
	MeshRenderState = InMeshRenderState;
}

FRHIUniformBuffer* FTetraMesh4DVertexFactory::GetCameraUniformBuffer() const
{
	return CameraRenderState.IsValid() ? CameraRenderState->Resource.GetUniformBuffer() : nullptr;
}

FRHIUniformBuffer* FTetraMesh4DVertexFactory::GetMeshUniformBuffer() const
{
	return MeshRenderState.IsValid() ? MeshRenderState->Resource.GetUniformBuffer() : nullptr;
}

IMPLEMENT_TYPE_LAYOUT(FTetraMesh4DVertexFactoryShaderParameters);
IMPLEMENT_VERTEX_FACTORY_PARAMETER_TYPE(FTetraMesh4DVertexFactory, SF_Vertex, FTetraMesh4DVertexFactoryShaderParameters);

/*
 * Registers the vertex factory type and associates it with its TetraMesh4DVertexFactory.ush implementation.
 */
IMPLEMENT_VERTEX_FACTORY_TYPE(
	FTetraMesh4DVertexFactory,
	"/Plugin/Unreal4D/TetraMesh4DVertexFactory.ush",
	EVertexFactoryFlags::UsedWithMaterials
	| EVertexFactoryFlags::SupportsStaticLighting
	| EVertexFactoryFlags::SupportsDynamicLighting
	| EVertexFactoryFlags::SupportsCachingMeshDrawCommands
	| EVertexFactoryFlags::SupportsPrimitiveIdStream
	| EVertexFactoryFlags::SupportsTriangleSorting
	| EVertexFactoryFlags::SupportsPositionOnly
);

/*
 * Registers the mesh 4D uniform parameter struct for shader access.
 */
IMPLEMENT_GLOBAL_SHADER_PARAMETER_STRUCT(FMesh4DUniformShaderParameters, "Mesh4D");
