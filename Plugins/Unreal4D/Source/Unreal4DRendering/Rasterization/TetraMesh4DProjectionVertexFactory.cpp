#include "TetraMesh4DProjectionVertexFactory.h"

#include "MaterialDomain.h"

void FTetraMesh4DProjectionVertexFactoryShaderParameters::Bind(const FShaderParameterMap& ParameterMap)
{
}

void FTetraMesh4DProjectionVertexFactoryShaderParameters::GetElementShaderBindings(
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
	const FTetraMesh4DProjectionVertexFactory* ProjectionVertexFactory = static_cast<const FTetraMesh4DProjectionVertexFactory*>(VertexFactory);

	ensure(ProjectionVertexFactory->GetCameraUniformBuffer() != nullptr);
	ensure(ProjectionVertexFactory->GetMeshUniformBuffer() != nullptr);

	if (FRHIUniformBuffer* CameraUniformBuffer = ProjectionVertexFactory->GetCameraUniformBuffer())
	{
		ShaderBindings.Add(Shader->GetUniformBufferParameter<CameraUniform4DShaderParameters>(), CameraUniformBuffer);
	}

	if (FRHIUniformBuffer* MeshUniformBuffer = ProjectionVertexFactory->GetMeshUniformBuffer())
	{
		ShaderBindings.Add(Shader->GetUniformBufferParameter<FMesh4DUniformShaderParameters>(), MeshUniformBuffer);
	}
}

FTetraMesh4DProjectionVertexFactory::FTetraMesh4DProjectionVertexFactory(ERHIFeatureLevel::Type InFeatureLevel)
	: FVertexFactory(InFeatureLevel)
{
}

void FTetraMesh4DProjectionVertexFactory::SetData(const FVertexDataType& InData)
{
	VertexData = InData;
}

void FTetraMesh4DProjectionVertexFactory::InitRHI(FRHICommandListBase& RHICmdList)
{
	check(HasValidFeatureLevel());

	FVertexDeclarationElementList defaultElements;
	defaultElements.Add(AccessStreamComponent(VertexData.VertexPosition4D, 0));
	InitDeclaration(defaultElements, EVertexInputStreamType::Default);

}

bool FTetraMesh4DProjectionVertexFactory::ShouldCompilePermutation(const FVertexFactoryShaderPermutationParameters& Parameters)
{
	return Parameters.MaterialParameters.MaterialDomain == MD_Surface;
}

void FTetraMesh4DProjectionVertexFactory::SetCameraRenderState(const FCamera4DRenderStatePtr& InCameraRenderState)
{
	CameraRenderState = InCameraRenderState;
}

void FTetraMesh4DProjectionVertexFactory::SetMeshRenderState(const FTetraMesh4DRenderStatePtr& InMeshRenderState)
{
	MeshRenderState = InMeshRenderState;
}

FRHIUniformBuffer* FTetraMesh4DProjectionVertexFactory::GetCameraUniformBuffer() const
{
	return CameraRenderState.IsValid() ? CameraRenderState->Resource.GetUniformBuffer() : nullptr;
}

FRHIUniformBuffer* FTetraMesh4DProjectionVertexFactory::GetMeshUniformBuffer() const
{
	return MeshRenderState.IsValid() ? MeshRenderState->Resource.GetUniformBuffer() : nullptr;
}

IMPLEMENT_TYPE_LAYOUT(FTetraMesh4DProjectionVertexFactoryShaderParameters);
IMPLEMENT_VERTEX_FACTORY_PARAMETER_TYPE(FTetraMesh4DProjectionVertexFactory, SF_Vertex, FTetraMesh4DProjectionVertexFactoryShaderParameters);

IMPLEMENT_VERTEX_FACTORY_TYPE(
	FTetraMesh4DProjectionVertexFactory,
	"/Plugin/Unreal4D/TetraMesh4DProjectionVertexFactory.ush",
	EVertexFactoryFlags::UsedWithMaterials | EVertexFactoryFlags::SupportsPrimitiveIdStream
);
