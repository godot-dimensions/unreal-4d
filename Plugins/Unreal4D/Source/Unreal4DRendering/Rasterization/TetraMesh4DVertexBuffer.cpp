#include "TetraMesh4DVertexBuffer.h"

#include "RHIResourceUtils.h"

void FTetraMesh4DVertexBuffer::InitRHI(FRHICommandListBase& RHICmdList)
{
	// Called on the render thread by the render resource system.
	// Uses CreateBufferWithArray rather than RHILockBuffer/RHIUnlockBuffer: BUF_Static
	// buffers must be fully initialized at creation time, and Lock() returns
	// BufferRHI=nullptr for static buffers, which crashes in RHIUnlockBuffer.

	if (Vertices.IsEmpty())
	{
		return;
	}

	const int64 BufferSizeBytes64 = (int64)Vertices.Num() * (int64)sizeof(FTetraMesh4DVertex);
	if (!ensure(BufferSizeBytes64 <= MAX_uint32))
	{
		return;
	}

	const FRHIBufferCreateDesc CreateDesc =
		FRHIBufferCreateDesc::Create(
			TEXT("TetraMesh4DVertexBuffer"),
			(uint32)BufferSizeBytes64, // InSize
			(uint32)sizeof(FTetraMesh4DVertex), // InStride
			BUF_Static | BUF_VertexBuffer)
		.SetInitialState(ERHIAccess::VertexOrIndexBuffer);

	VertexBufferRHI = UE::RHIResourceUtils::CreateBufferWithArray(RHICmdList, CreateDesc, TConstArrayView<FTetraMesh4DVertex>(Vertices));
}
