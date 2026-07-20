#include "TetraMesh4DProjectionVertexBuffer.h"

#include "RHIResourceUtils.h"

void FTetraMesh4DProjectionVertexBuffer::InitRHI(FRHICommandListBase& RHICmdList)
{
	if (Vertices.IsEmpty())
	{
		return;
	}

	const int64 BufferSizeBytes64 = (int64)Vertices.Num() * (int64)sizeof(FTetraMesh4DProjectionVertex);
	if (!ensure(BufferSizeBytes64 <= MAX_uint32))
	{
		return;
	}

	const FRHIBufferCreateDesc CreateDesc =
		FRHIBufferCreateDesc::Create(
			TEXT("TetraMesh4DProjectionVertexBuffer"),
			(uint32)BufferSizeBytes64,
			(uint32)sizeof(FTetraMesh4DProjectionVertex),
			BUF_Static | BUF_VertexBuffer)
		.SetInitialState(ERHIAccess::VertexOrIndexBuffer);

	VertexBufferRHI = UE::RHIResourceUtils::CreateBufferWithArray(
		RHICmdList,
		CreateDesc,
		TConstArrayView<FTetraMesh4DProjectionVertex>(Vertices));
}
