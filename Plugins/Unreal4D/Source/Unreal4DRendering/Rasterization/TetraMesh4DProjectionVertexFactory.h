/*
 * FTetraMesh4DProjectionVertexFactory
 *
 * Custom vertex factory for GPU-based 4D mesh projection rendering.
 *
 * The projection path renders the 4D tetra mesh as projected 3D surface
 * triangles by transforming each 4D vertex into 4D camera space and dropping
 * the W coordinate. It intentionally uses a compact vertex layout that stores
 * one 4D position per vertex.
 */

#pragma once

#include "CameraUniform4D.h"
#include "TetraMesh4DProjectionVertexBuffer.h"
#include "TetraMesh4DVertexFactory.h"

#include "MeshDrawShaderBindings.h"
#include "MeshMaterialShader.h"
#include "ShaderParameterUtils.h"
#include "Templates/SharedPointer.h"
#include "VertexFactory.h"

class FTetraMesh4DProjectionVertexFactoryShaderParameters : public FVertexFactoryShaderParameters
{
	DECLARE_TYPE_LAYOUT(FTetraMesh4DProjectionVertexFactoryShaderParameters, NonVirtual);

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

class FTetraMesh4DProjectionVertexFactory : public FVertexFactory
{
	DECLARE_VERTEX_FACTORY_TYPE(FTetraMesh4DProjectionVertexFactory);

public:
	explicit FTetraMesh4DProjectionVertexFactory(ERHIFeatureLevel::Type InFeatureLevel);

	struct FVertexDataType
	{
		FVertexStreamComponent VertexPosition4D;
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
