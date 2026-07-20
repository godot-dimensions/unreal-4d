/*
 * Vertex buffer for FTetraMesh4DVertexFactory.
 *
 * Defines the per-vertex data layout for tetrahedron-based 4D meshes.
 */

#pragma once

#include "../Unreal4DRenderingDefines.h"

#include "RenderResource.h"

struct FTetraMesh4DVertex
{
	uint32 VertexId = 0;

	FVector4f TetraPosition0 = FVector4f(0.f, 0.f, 0.f, -1.f);
	FVector4f TetraPosition1 = FVector4f(0.f, 0.f, 0.f, 1.f);
	FVector4f TetraPosition2 = FVector4f(0.f, 0.f, 0.f, 1.f);
	FVector4f TetraPosition3 = FVector4f(0.f, 0.f, 0.f, 1.f);

	FVector4f TetraNormal0 = FVector4f(0.f, 0.f, 0.f, 1.f);
	FVector4f TetraNormal1 = FVector4f(0.f, 0.f, 0.f, 1.f);
	FVector4f TetraNormal2 = FVector4f(0.f, 0.f, 0.f, 1.f);
	FVector4f TetraNormal3 = FVector4f(0.f, 0.f, 0.f, 1.f);

	FVector2f TetraUV0 = FVector2f::ZeroVector;
	FVector2f TetraUV1 = FVector2f::ZeroVector;
	FVector2f TetraUV2 = FVector2f::ZeroVector;
	FVector2f TetraUV3 = FVector2f::ZeroVector;

	FVector4f TetraParams0 = FVector4f(1.f, 1.f, 1.f, 1.f);
	FVector4f TetraParams1 = FVector4f(0.f, 0.f, 0.f, 0.f);
};

class FTetraMesh4DVertexBuffer
	: public FVertexBuffer
{

public:
	TArray<FTetraMesh4DVertex> Vertices;

	virtual void InitRHI(FRHICommandListBase& RHICmdList) override;
};
