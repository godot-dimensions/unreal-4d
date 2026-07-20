#include "TetraMesh4DIndexBuffer.h"

#include "RHIResourceUtils.h"

void FTetraMesh4DIndexBuffer::InitRHI(FRHICommandListBase& RHICmdList)
{
	// Called on the render thread by the render resource system.
	// Uses CreateBufferWithArray rather than RHILockBuffer/RHIUnlockBuffer: BUF_Static
	// buffers must be fully initialized at creation time, and Lock() returns
	// BufferRHI=nullptr for static buffers, which crashes in RHIUnlockBuffer.

	if (Indices.IsEmpty())
	{
		return;
	}

	const int64 BufferSizeBytes64 = (int64)Indices.Num() * (int64)sizeof(uint32);
	if (!ensure(BufferSizeBytes64 <= MAX_uint32))
	{
		return;
	}

	const FRHIBufferCreateDesc CreateDesc =
		FRHIBufferCreateDesc::CreateIndex(
			TEXT("TetraMesh4DIndexBuffer"),
			(uint32)BufferSizeBytes64, // InSize
			(uint32)sizeof(uint32)) // InStride
		.SetInitialState(ERHIAccess::VertexOrIndexBuffer);

	IndexBufferRHI = UE::RHIResourceUtils::CreateBufferWithArray(RHICmdList, CreateDesc, TConstArrayView<uint32>(Indices));
}
