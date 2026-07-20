#include "TetraMesh4DRenderResource.h"

#include "RenderingThread.h"

struct FTetraMesh4DRenderResourceDeleter
{
	void operator()(FTetraMesh4DRenderResource* InResource) const
	{
		if (InResource == nullptr)
		{
			return;
		}

		ENQUEUE_RENDER_COMMAND(Unreal4DDeleteMesh4DRenderResource)(
			[InResource](FRHICommandListImmediate& RHICmdList)
			{
				InResource->ReleaseResources_RenderThread();
				delete InResource;
			});
	}
};

FVector4 FTetraMesh4DRenderResource::MakeFlatNormal4D(const FVector4& InPos0, const FVector4& InPos1, const FVector4& InPos2, const FVector4& InPos3)
{
	const FVector4 a = InPos1 - InPos0;
	const FVector4 b = InPos2 - InPos0;
	const FVector4 c = InPos3 - InPos0;
	FVector4 norm;
	// This is the same as UVectorFuncs4D::Vector4_Perpendicular, but we need to avoid a cross-module dependency.
	norm.X = -a.Y * (b.Z * c.W - b.W * c.Z)
			+ a.Z * (b.Y * c.W - b.W * c.Y)
			- a.W * (b.Y * c.Z - b.Z * c.Y);
	norm.Y = +a.X * (b.Z * c.W - b.W * c.Z)
			- a.Z * (b.X * c.W - b.W * c.X)
			+ a.W * (b.X * c.Z - b.Z * c.X);
	norm.Z = -a.X * (b.Y * c.W - b.W * c.Y)
			+ a.Y * (b.X * c.W - b.W * c.X)
			- a.W * (b.X * c.Y - b.Y * c.X);
	norm.W = +a.X * (b.Y * c.Z - b.Z * c.Y)
			- a.Y * (b.X * c.Z - b.Z * c.X)
			+ a.Z * (b.X * c.Y - b.Y * c.X);
	const double lengthSquared = norm.SizeSquared();
	return lengthSquared > 0.0 ? norm / FMath::Sqrt(lengthSquared) : FVector4::Zero();
}

FTetraMesh4DProjectionFaceKey FTetraMesh4DProjectionFaceKey::MakeProjectionFaceKey(int32 InVertexIndex0, int32 InVertexIndex1, int32 InVertexIndex2)
{
	if (InVertexIndex1 < InVertexIndex0)
	{
		Swap(InVertexIndex0, InVertexIndex1);
	}
	if (InVertexIndex2 < InVertexIndex1)
	{
		Swap(InVertexIndex1, InVertexIndex2);
	}
	if (InVertexIndex1 < InVertexIndex0)
	{
		Swap(InVertexIndex0, InVertexIndex1);
	}

	FTetraMesh4DProjectionFaceKey key;
	key.VertexIndex0 = InVertexIndex0;
	key.VertexIndex1 = InVertexIndex1;
	key.VertexIndex2 = InVertexIndex2;
	return key;
}

void FTetraMesh4DRenderResource::AddProjectionFace(FTetraMesh4DProjectionVertexBuffer& OutVertexBuffer, FTetraMesh4DIndexBuffer& OutIndexBuffer, const FVector4* InPositions, const int32 InLocalIndex0, const int32 InLocalIndex1, const int32 InLocalIndex2)
{
	const uint32 baseVertexIndex = static_cast<uint32>(OutVertexBuffer.Vertices.Num());

	FTetraMesh4DProjectionVertex vertex0;
	vertex0.VertexPosition4D = FVector4f(InPositions[InLocalIndex0]);
	OutVertexBuffer.Vertices.Add(vertex0);

	FTetraMesh4DProjectionVertex vertex1;
	vertex1.VertexPosition4D = FVector4f(InPositions[InLocalIndex1]);
	OutVertexBuffer.Vertices.Add(vertex1);

	FTetraMesh4DProjectionVertex vertex2;
	vertex2.VertexPosition4D = FVector4f(InPositions[InLocalIndex2]);
	OutVertexBuffer.Vertices.Add(vertex2);

	OutIndexBuffer.Indices.Add(baseVertexIndex + 0);
	OutIndexBuffer.Indices.Add(baseVertexIndex + 1);
	OutIndexBuffer.Indices.Add(baseVertexIndex + 2);
}

FTetraMesh4DRenderResourcePtr FTetraMesh4DRenderResource::MakeTetraMesh4DRenderResource()
{
	return FTetraMesh4DRenderResourcePtr(new FTetraMesh4DRenderResource(), FTetraMesh4DRenderResourceDeleter());
}

void FTetraMesh4DRenderResource::BuildBuffersFromSimplexData(
	const TArray<FVector4>& InVertexValues,
	const TArray<FVector4>& InNormalValues,
	const TArray<FVector>& InTextureMapValues,
	const TArray<int32>& InSimplexCellVertexIndices,
	const TArray<int32>& InSimplexCellNormalIndices,
	const TArray<int32>& InSimplexCellTextureMapIndices)
{
	check(InSimplexCellVertexIndices.Num() % 4 == 0);
	check(InSimplexCellNormalIndices.IsEmpty() || InSimplexCellNormalIndices.Num() == InSimplexCellVertexIndices.Num());
	check(InSimplexCellTextureMapIndices.IsEmpty() || InSimplexCellTextureMapIndices.Num() == InSimplexCellVertexIndices.Num());

	VertexBuffer.Vertices.Reset();
	IndexBuffer.Indices.Reset();
	ProjectionVertexBuffer.Vertices.Reset();
	ProjectionIndexBuffer.Indices.Reset();

	const int32 tetraCount = InSimplexCellVertexIndices.Num() / 4;
	if (!ensureMsgf(tetraCount <= MAX_int32 / 12, TEXT("Tetrahedron count is too large to build render buffers.")))
	{
		return;
	}
	const bool bHasNormals = !InSimplexCellNormalIndices.IsEmpty();
	const bool bHasUVs = !InSimplexCellTextureMapIndices.IsEmpty();

	VertexBuffer.Vertices.Reserve(tetraCount * 6);
	IndexBuffer.Indices.Reserve(tetraCount * 6);

	// Projection vertices are currently emitted per triangle, so each tetrahedron
	// may contribute up to 12 vertices: 4 faces * 3 vertices.
	// This keeps the projection mesh generation simple and allows each projected
	// face to be deduplicated independently. The vertex format is very small, so
	// the extra memory cost should be negligible for now.
	// If GPU memory becomes a concern later, this can be optimized to use 4 shared
	// projection vertices per tetrahedron with indexed faces.
	ProjectionVertexBuffer.Vertices.Reserve(tetraCount * 12);
	ProjectionIndexBuffer.Indices.Reserve(tetraCount * 12);

	static constexpr int32 ProjectionFaceLocalIndices[4][3] =
	{
		{ 0, 2, 1 },
		{ 0, 1, 3 },
		{ 1, 2, 3 },
		{ 2, 0, 3 }
	};

	TSet<FTetraMesh4DProjectionFaceKey> addedProjectionFaces;
	addedProjectionFaces.Reserve(tetraCount * 4);

	for (int32 tetraIndex = 0; tetraIndex < tetraCount; ++tetraIndex)
	{
		const int32 baseSourceIndex = tetraIndex * 4;
		const int32 sourceVertexIndices[4] =
		{
			InSimplexCellVertexIndices[baseSourceIndex + 0],
			InSimplexCellVertexIndices[baseSourceIndex + 1],
			InSimplexCellVertexIndices[baseSourceIndex + 2],
			InSimplexCellVertexIndices[baseSourceIndex + 3]
		};

		const FVector4 positions[4] =
		{
			InVertexValues[sourceVertexIndices[0]],
			InVertexValues[sourceVertexIndices[1]],
			InVertexValues[sourceVertexIndices[2]],
			InVertexValues[sourceVertexIndices[3]]
		};

		FVector4 normals[4];
		if (bHasNormals)
		{
			normals[0] = InNormalValues[InSimplexCellNormalIndices[baseSourceIndex + 0]];
			normals[1] = InNormalValues[InSimplexCellNormalIndices[baseSourceIndex + 1]];
			normals[2] = InNormalValues[InSimplexCellNormalIndices[baseSourceIndex + 2]];
			normals[3] = InNormalValues[InSimplexCellNormalIndices[baseSourceIndex + 3]];
		}
		else
		{
			const FVector4 flatNormal = MakeFlatNormal4D(positions[0], positions[1], positions[2], positions[3]);
			normals[0] = normals[1] = normals[2] = normals[3] = flatNormal;
		}

		FVector2f uv0, uv1, uv2, uv3;
		if (bHasUVs)
		{
			const FVector& t0 = InTextureMapValues[InSimplexCellTextureMapIndices[baseSourceIndex + 0]];
			const FVector& t1 = InTextureMapValues[InSimplexCellTextureMapIndices[baseSourceIndex + 1]];
			const FVector& t2 = InTextureMapValues[InSimplexCellTextureMapIndices[baseSourceIndex + 2]];
			const FVector& t3 = InTextureMapValues[InSimplexCellTextureMapIndices[baseSourceIndex + 3]];
			uv0 = FVector2f(t0.X, t0.Y);
			uv1 = FVector2f(t1.X, t1.Y);
			uv2 = FVector2f(t2.X, t2.Y);
			uv3 = FVector2f(t3.X, t3.Y);
		}
		else
		{
			uv0 = uv1 = uv2 = uv3 = FVector2f(0.0f, 0.0f);
		}

		const uint32 baseVertexIndex = static_cast<uint32>(VertexBuffer.Vertices.Num());

		for (uint32 vertexId = 0; vertexId < 6; ++vertexId)
		{
			FTetraMesh4DVertex vertex;
			vertex.VertexId = vertexId;
			vertex.TetraPosition0 = FVector4f(positions[0]);
			vertex.TetraPosition1 = FVector4f(positions[1]);
			vertex.TetraPosition2 = FVector4f(positions[2]);
			vertex.TetraPosition3 = FVector4f(positions[3]);
			vertex.TetraNormal0 = FVector4f(normals[0]);
			vertex.TetraNormal1 = FVector4f(normals[1]);
			vertex.TetraNormal2 = FVector4f(normals[2]);
			vertex.TetraNormal3 = FVector4f(normals[3]);
			vertex.TetraUV0 = uv0;
			vertex.TetraUV1 = uv1;
			vertex.TetraUV2 = uv2;
			vertex.TetraUV3 = uv3;
			vertex.TetraParams0 = FVector4f(1.0f, 1.0f, 1.0f, 1.0f);
			vertex.TetraParams1 = FVector4f(0.0f, 0.0f, 0.0f, 0.0f);

			VertexBuffer.Vertices.Add(vertex);
			IndexBuffer.Indices.Add(baseVertexIndex + vertexId);
		}

		for (int32 faceIndex = 0; faceIndex < 4; ++faceIndex)
		{
			const int32 localIndex0 = ProjectionFaceLocalIndices[faceIndex][0];
			const int32 localIndex1 = ProjectionFaceLocalIndices[faceIndex][1];
			const int32 localIndex2 = ProjectionFaceLocalIndices[faceIndex][2];

			// Projection faces are deduplicated by source vertex indices.
			// This follows the Engine4D-style "add each tetra face once" approach:
			// repeated faces are skipped, not removed as internal boundary faces.
			const FTetraMesh4DProjectionFaceKey faceKey = FTetraMesh4DProjectionFaceKey::MakeProjectionFaceKey(
				sourceVertexIndices[localIndex0], sourceVertexIndices[localIndex1], sourceVertexIndices[localIndex2]);

			if (addedProjectionFaces.Contains(faceKey))
			{
				continue;
			}

			addedProjectionFaces.Add(faceKey);
			AddProjectionFace(
				ProjectionVertexBuffer,
				ProjectionIndexBuffer,
				positions,
				localIndex0,
				localIndex1,
				localIndex2);
		}
	}
}

void FTetraMesh4DRenderResource::InitResources(
	const TArray<FVector4>& InVertexValues,
	const TArray<FVector4>& InNormalValues,
	const TArray<FVector>& InTextureMapValues,
	const TArray<int32>& InSimplexCellVertexIndices,
	const TArray<int32>& InSimplexCellNormalIndices,
	const TArray<int32>& InSimplexCellTextureMapIndices)
{
	BuildBuffersFromSimplexData(InVertexValues, InNormalValues, InTextureMapValues, InSimplexCellVertexIndices, InSimplexCellNormalIndices, InSimplexCellTextureMapIndices);

	/*
	 * Initialization is intentionally asynchronous. The mesh asset and scene proxies
	 * keep this resource alive through shared ownership, so the game thread does not
	 * need to block here while InitRHI is pending on the render thread.
	 */
	BeginInitResource(&VertexBuffer);
	BeginInitResource(&IndexBuffer);
	BeginInitResource(&ProjectionVertexBuffer);
	BeginInitResource(&ProjectionIndexBuffer);
}

void FTetraMesh4DRenderResource::ReleaseResources_RenderThread()
{
	check(IsInRenderingThread());

	if (VertexBuffer.IsInitialized())
	{
		VertexBuffer.ReleaseResource();
	}

	if (IndexBuffer.IsInitialized())
	{
		IndexBuffer.ReleaseResource();
	}

	if (ProjectionVertexBuffer.IsInitialized())
	{
		ProjectionVertexBuffer.ReleaseResource();
	}

	if (ProjectionIndexBuffer.IsInitialized())
	{
		ProjectionIndexBuffer.ReleaseResource();
	}

}
