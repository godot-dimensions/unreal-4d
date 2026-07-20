/*
 * Vertex buffer for FTetraMesh4DProjectionVertexFactory.
 *
 * Defines the compact per-vertex data layout used by the projected 4D mesh
 * rendering path. Unlike the slicing vertex buffer, each vertex represents a
 * single 4D surface vertex rather than a full tetrahedron context.
 */

#pragma once

#include "../Unreal4DRenderingDefines.h"

#include "RenderResource.h"

struct FTetraMesh4DProjectionVertex
{
	FVector4f VertexPosition4D = FVector4f(0.0f, 0.0f, 0.0f, 0.0f);
};

class FTetraMesh4DProjectionVertexBuffer
	: public FVertexBuffer
{
public:
	TArray<FTetraMesh4DProjectionVertex> Vertices;

	virtual void InitRHI(FRHICommandListBase& RHICmdList) override;
};
