/*
 * Shared render resource for tetrahedron-based 4D mesh assets.
 *
 * Owns the immutable GPU buffers shared by all scene proxies referencing the
 * same UMesh4D asset instance.
 */

#pragma once

#include "TetraMesh4DIndexBuffer.h"
#include "TetraMesh4DVertexBuffer.h"
#include "TetraMesh4DProjectionVertexBuffer.h"

class FTetraMesh4DRenderResource;

using FTetraMesh4DRenderResourcePtr = TSharedPtr<FTetraMesh4DRenderResource, ESPMode::ThreadSafe>;

class UNREAL4DRENDERING_API FTetraMesh4DRenderResource
{
private:
	static FVector4 MakeFlatNormal4D(const FVector4& InPos0, const FVector4& InPos1, const FVector4& InPos2, const FVector4& InPos3);
	static void AddProjectionFace(FTetraMesh4DProjectionVertexBuffer& OutVertexBuffer, FTetraMesh4DIndexBuffer& OutIndexBuffer, const FVector4* InPositions, const int32 InLocalIndex0, const int32 InLocalIndex1, const int32 InLocalIndex2);

public:
	static FTetraMesh4DRenderResourcePtr MakeTetraMesh4DRenderResource();

	/*
	 * Initializes the shared vertex and index buffers by sampling the provided indexed mesh data.
	 */
	void InitResources(
		const TArray<FVector4>& InVertexValues,
		const TArray<FVector4>& InNormalValues,
		const TArray<FVector>& InTextureMapValues,
		const TArray<int32>& InSimplexCellVertexIndices,
		const TArray<int32>& InSimplexCellNormalIndices,
		const TArray<int32>& InSimplexCellTextureMapIndices);

	/*
	 * Releases the shared vertex and index buffers.
	 *
	 * Must be called on the render thread. Object destruction is deferred through
	 * MakeTetraMesh4DRenderResource() so the buffers are released before the resource
	 * object is deleted.
	 */
	void ReleaseResources_RenderThread();

	const FTetraMesh4DVertexBuffer& GetVertexBuffer() const
	{
		return VertexBuffer;
	}

	const FTetraMesh4DIndexBuffer& GetIndexBuffer() const
	{
		return IndexBuffer;
	}

	const FTetraMesh4DProjectionVertexBuffer& GetProjectionVertexBuffer() const
	{
		return ProjectionVertexBuffer;
	}

	const FTetraMesh4DIndexBuffer& GetProjectionIndexBuffer() const
	{
		return ProjectionIndexBuffer;
	}


	FTetraMesh4DVertexBuffer& GetVertexBuffer()
	{
		return VertexBuffer;
	}

	FTetraMesh4DIndexBuffer& GetIndexBuffer()
	{
		return IndexBuffer;
	}

	FTetraMesh4DProjectionVertexBuffer& GetProjectionVertexBuffer()
	{
		return ProjectionVertexBuffer;
	}

	FTetraMesh4DIndexBuffer& GetProjectionIndexBuffer()
	{
		return ProjectionIndexBuffer;
	}


	uint32 GetNumVertices() const
	{
		return VertexBuffer.Vertices.Num();
	}

	uint32 GetNumIndices() const
	{
		return IndexBuffer.Indices.Num();
	}

	uint32 GetNumPrimitives() const
	{
		return IndexBuffer.Indices.Num() / 3;
	}

	uint32 GetNumProjectionVertices() const
	{
		return ProjectionVertexBuffer.Vertices.Num();
	}

	uint32 GetNumProjectionIndices() const
	{
		return ProjectionIndexBuffer.Indices.Num();
	}

	uint32 GetNumProjectionPrimitives() const
	{
		return ProjectionIndexBuffer.Indices.Num() / 3;
	}


	bool IsInitialized() const
	{
		return VertexBuffer.IsInitialized() && IndexBuffer.IsInitialized();
	}

	bool IsProjectionInitialized() const
	{
		return ProjectionVertexBuffer.IsInitialized() && ProjectionIndexBuffer.IsInitialized();
	}


private:
	void BuildBuffersFromSimplexData(
		const TArray<FVector4>& InVertexValues,
		const TArray<FVector4>& InNormalValues,
		const TArray<FVector>& InTextureMapValues,
		const TArray<int32>& InSimplexCellVertexIndices,
		const TArray<int32>& InSimplexCellNormalIndices,
		const TArray<int32>& InSimplexCellTextureMapIndices);

	FTetraMesh4DVertexBuffer VertexBuffer;
	FTetraMesh4DIndexBuffer IndexBuffer;
	FTetraMesh4DProjectionVertexBuffer ProjectionVertexBuffer;
	FTetraMesh4DIndexBuffer ProjectionIndexBuffer;
};

struct FTetraMesh4DProjectionFaceKey
{
	int32 VertexIndex0 = INDEX_NONE;
	int32 VertexIndex1 = INDEX_NONE;
	int32 VertexIndex2 = INDEX_NONE;

	friend bool operator==(const FTetraMesh4DProjectionFaceKey& InLeftKey, const FTetraMesh4DProjectionFaceKey& InRightKey)
	{
		return InLeftKey.VertexIndex0 == InRightKey.VertexIndex0
			&& InLeftKey.VertexIndex1 == InRightKey.VertexIndex1
			&& InLeftKey.VertexIndex2 == InRightKey.VertexIndex2;
	}

	friend uint32 GetTypeHash(const FTetraMesh4DProjectionFaceKey& InKey)
	{
		uint32 hash = ::GetTypeHash(InKey.VertexIndex0);
		hash = HashCombine(hash, ::GetTypeHash(InKey.VertexIndex1));
		hash = HashCombine(hash, ::GetTypeHash(InKey.VertexIndex2));
		return hash;
	}

	// These parameters need to be mutable, not const.
	static FTetraMesh4DProjectionFaceKey MakeProjectionFaceKey(int32 InVertexIndex0, int32 InVertexIndex1, int32 InVertexIndex2);
};
