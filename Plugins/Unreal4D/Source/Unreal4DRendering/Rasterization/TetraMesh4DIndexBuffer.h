/*
 * Index buffer for tetrahedron-based 4D meshes used by FTetraMesh4DVertexFactory.
 */
#pragma once

#include "../Unreal4DRenderingDefines.h"

#include "RenderResource.h"

class FTetraMesh4DIndexBuffer : public FIndexBuffer
{
public:

	virtual void InitRHI(FRHICommandListBase& RHICmdList) override;

	TArray<uint32> Indices;
};
