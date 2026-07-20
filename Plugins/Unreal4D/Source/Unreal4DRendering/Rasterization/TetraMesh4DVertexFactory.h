/*
 * FTetraMesh4DVertexFactory
 *
 * Custom vertex factory for GPU-based 4D slice rendering.
 *
 * Defines vertex input layout and shader interface used to transform
 * per-tetrahedron 4D data into attributes consumed by the Unreal Engine
 * material pipeline.
 */

#pragma once

#include "../Unreal4DRenderingDefines.h"

#include "MeshDrawShaderBindings.h"
#include "MeshMaterialShader.h"
#include "CameraUniform4D.h"
#include "TetraMesh4DVertexBuffer.h"
#include "ShaderParameterMacros.h"
#include "ShaderParameterUtils.h"
#include "Templates/SharedPointer.h"
#include "VertexFactory.h"

 /*
  * Per-mesh 4D transform parameters consumed by the vertex factory shader.
  * Passed to the GPU via a uniform buffer.
  */
BEGIN_GLOBAL_SHADER_PARAMETER_STRUCT(FMesh4DUniformShaderParameters, UNREAL4DRENDERING_API)
	SHADER_PARAMETER(FVector4f, Position4D)
	SHADER_PARAMETER(FMatrix44f, RotAndScale4D)
	SHADER_PARAMETER(FMatrix44f, RotAndScale4DInverseTranspose)
END_GLOBAL_SHADER_PARAMETER_STRUCT()

/*
* FRenderResource that encapsulates the uniform buffer for 4D tetra mesh transform.
*/
class UNREAL4DRENDERING_API FTetraMesh4DUniformRenderResource : public FRenderResource
{
public:
	FTetraMesh4DUniformRenderResource();

	virtual void InitRHI(FRHICommandListBase& RHICmdList) override;
	virtual void ReleaseRHI() override;

	void UpdateUniformData(const FMesh4DUniformShaderParameters& InParameters, FRHICommandList& RHICmdList);

	FRHIUniformBuffer* GetUniformBuffer() const
	{
		return Mesh4DUniformBufferRef.GetReference();
	}

	TUniformBufferRef<FMesh4DUniformShaderParameters> GetUniformBufferRef() const
	{
		return Mesh4DUniformBufferRef;
	}

	FMesh4DUniformShaderParameters CachedMesh4DUniformParameters;

private:
	TUniformBufferRef<FMesh4DUniformShaderParameters> Mesh4DUniformBufferRef;
};

/*
* Simple FTetraMesh4DUniformRenderResource wrapper.
* Can be extended in the future to include additional mesh-related render resources.
*/
class UNREAL4DRENDERING_API FTetraMesh4DRenderState
{
public:
	FTetraMesh4DUniformRenderResource Resource;
};

/*
 * Shared ownership container for per-mesh render state used across threads.
 *
 * The game thread (URenderingServer4D subsystem) owns and updates this data on
 * the render thread via enqueued render commands, while the render thread
 * (FTetraMesh4DVertexFactory) consumes it during shader binding.
 *
 * Uses TSharedPtr (thread-safe) to guarantee correct lifetime management across
 * threads and prevent use-after-free when accessing the render state.
 */
using FTetraMesh4DRenderStatePtr = TSharedPtr<FTetraMesh4DRenderState, ESPMode::ThreadSafe>;


/*
 * Shader parameter bindings for the 4D vertex factory.
 * Responsible for providing per-draw shader bindings specific to this vertex factory.
 */
class FTetraMesh4DVertexFactoryShaderParameters : public FVertexFactoryShaderParameters
{
	DECLARE_TYPE_LAYOUT(FTetraMesh4DVertexFactoryShaderParameters, NonVirtual);

public:
	void Bind(const FShaderParameterMap& ParameterMap);

	void GetElementShaderBindings(
		const FSceneInterface* Scene,
		const FSceneView* View,
		const FMeshMaterialShader* Shader,
		const EVertexInputStreamType InputStreamType,
		ERHIFeatureLevel::Type FeatureLevel,
		const FVertexFactory* VertexFactory,
		const FMeshBatchElement& BatchElement,
		FMeshDrawSingleShaderBindings& ShaderBindings,
		FVertexInputStreamArray& VertexStreams) const;
};


/*
 * Custom vertex factory for 4D mesh rendering.
 *
 * A vertex factory defines how raw per-vertex data is interpreted and passed
 * into the GPU vertex processing stage. It is tightly coupled with a specific
 * shader (.ush) implementation that defines the actual transformation logic
 * from input vertex data to outputs consumed by the Unreal Engine material pipeline
 * (e.g. translated world position, normals, and other attributes).
 *
 * This implementation passes per-tetrahedron 4D data and auxiliary parameters
 * to the shader so that the final 3D slice geometry can be generated on the GPU.
 */
class FTetraMesh4DVertexFactory : public FVertexFactory
{
	DECLARE_VERTEX_FACTORY_TYPE(FTetraMesh4DVertexFactory);

public:
	explicit FTetraMesh4DVertexFactory(ERHIFeatureLevel::Type InFeatureLevel);

	struct FVertexDataType
	{
		FVertexStreamComponent VertexIdComponent;
		FVertexStreamComponent TetraPosition0Component;
		FVertexStreamComponent TetraPosition1Component;
		FVertexStreamComponent TetraPosition2Component;
		FVertexStreamComponent TetraPosition3Component;
		FVertexStreamComponent TetraNormal0Component;
		FVertexStreamComponent TetraNormal1Component;
		FVertexStreamComponent TetraNormal2Component;
		FVertexStreamComponent TetraNormal3Component;
		FVertexStreamComponent TetraUV0Component;
		FVertexStreamComponent TetraUV1Component;
		FVertexStreamComponent TetraUV2Component;
		FVertexStreamComponent TetraUV3Component;
		// can be used to store any additional per-vertex data
		FVertexStreamComponent TetraParams0Component;
		FVertexStreamComponent TetraParams1Component;
	};

	void SetData(const FVertexDataType& InData);
	virtual void InitRHI(FRHICommandListBase& RHICmdList) override;
	static bool ShouldCompilePermutation(const FVertexFactoryShaderPermutationParameters& Parameters);

	void SetCameraRenderState(const FCamera4DRenderStatePtr& InCameraRenderState);
	void SetMeshRenderState(const FTetraMesh4DRenderStatePtr& InMeshRenderState);

	FRHIUniformBuffer* GetCameraUniformBuffer() const;
	FRHIUniformBuffer* GetMeshUniformBuffer() const;

private:
	FVertexDataType VertexData;
	FCamera4DRenderStatePtr CameraRenderState;
	FTetraMesh4DRenderStatePtr MeshRenderState;
};
