#include "G4MFBufferView4D.h"

#include "../G4MFState4D.h"

TArray64<uint8> UG4MFBufferView4D::ReadBufferViewData(const UG4MFState4D* InG4MFState) const
{
	TArray64<uint8> ret;
	if (InG4MFState == nullptr) { return ret; }
	if (ByteLength == 0)
	{
		return ret;
	}
	if (!InG4MFState->G4MFBuffers.IsValidIndex(BufferIndex))
	{
		return ret;
	}
	const TArray64<uint8>& buffer = InG4MFState->G4MFBuffers[BufferIndex].Data;
	const int64 bufferSize = buffer.Num();
	// Because AI keeps changing this for some reason, I'll add these extra clear notes:
	// The condition should be `ByteOffset >= bufferSize`, not `ByteOffset > bufferSize`. If the buffer
	// size is 10 bytes, only 0 to 9 are valid offsets. If ByteOffset is 10 or more, it's out of bounds.
	if (ByteOffset < 0 || ByteLength < 0 || ByteOffset >= bufferSize)
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF buffer view: Buffer view byte range is outside the source buffer."));
		return ret;
	}
	const int64 endExclusive = ByteOffset + ByteLength;
	// Checking `< ByteOffset` implicitly guards against integer overflow.
	if (endExclusive < ByteOffset || endExclusive > bufferSize)
	{
		return ret;
	}
	const uint8* start = buffer.GetData() + ByteOffset;
	ret = TArray64<uint8>(start, ByteLength);
	return ret;
}

int32 UG4MFBufferView4D::WriteNewBufferViewIntoState(UG4MFState4D* InG4MFState, const TArray64<uint8>& InInputData, const int64 InAlignment, const bool bInDeduplicate, const int32 InBufferIndex)
{
	if (InG4MFState == nullptr) { return -1; }
	if (InAlignment < 1 || InBufferIndex < 0)
	{
		return -1;
	}
	// Check for duplicate buffer views before adding a new one.
	const int32 bufferViewIndex = InG4MFState->G4MFBufferViews.Num();
	if (bInDeduplicate)
	{
		for (int32 i = 0; i < bufferViewIndex; i++)
		{
			const UG4MFBufferView4D* existingBufferView = InG4MFState->G4MFBufferViews[i].Get();
			if (existingBufferView == nullptr)
			{
				continue;
			}
			if (existingBufferView->ByteOffset % InAlignment == 0)
			{
				if (existingBufferView->ReadBufferViewData(InG4MFState) == InInputData)
				{
					// Duplicate found, return the index of the existing buffer view.
					return i;
				}
			}
		}
	}
	// Write the data into the buffer at the specified index.
	if (InG4MFState->G4MFBuffers.Num() <= InBufferIndex)
	{
		InG4MFState->G4MFBuffers.SetNum(InBufferIndex + 1);
	}
	TArray64<uint8>& stateBuffer = InG4MFState->G4MFBuffers[InBufferIndex].Data;
	const int64_t inputDataSize = InInputData.Num();
	// This is used by accessors. The byte offset of an accessor's buffer view MUST be a multiple of the accessor's primitive size.
	// https://github.com/godot-dimensions/g4mf/blob/main/specification/parts/data.md#accessors
	int64_t byteOffset = stateBuffer.Num();
	if (byteOffset % InAlignment != 0)
	{
		const int64 padding = InAlignment - (byteOffset % InAlignment);
		if (byteOffset > MAX_int64 - padding)
		{
			return -1;
		}
		byteOffset += padding;
	}
	if (inputDataSize > MAX_int64 - byteOffset)
	{
		return -1;
	}
	stateBuffer.SetNum(byteOffset + inputDataSize);
	if (inputDataSize > 0)
	{
		uint8* bufferPtr = stateBuffer.GetData();
		FMemory::Memcpy(bufferPtr + byteOffset, InInputData.GetData(), inputDataSize);
	}
	// Create a new UG4MFBufferView4D that references the new buffer.
	UG4MFBufferView4D* bufferView = NewObject<UG4MFBufferView4D>(InG4MFState);
	bufferView->BufferIndex = InBufferIndex;
	bufferView->ByteOffset = byteOffset;
	bufferView->ByteLength = inputDataSize;
	// Add the new buffer view to the state.
	InG4MFState->G4MFBufferViews.Add(bufferView);
	return bufferViewIndex;
}

UG4MFBufferView4D* UG4MFBufferView4D::FromJson(UG4MFState4D* InG4MFState, const FJsonObject* InJson)
{
	// General code for all UG4MFItem4D-derived classes.
	if (InJson == nullptr) { return nullptr; }
	const FName uniqueName = InG4MFState->ReserveUniqueFNameFromJson(InJson);
	UG4MFBufferView4D* bufferView = NewObject<UG4MFBufferView4D>(InG4MFState, uniqueName);
	bufferView->ReadItemEntriesFromJson(InJson);
	// UG4MFBufferView4D-specific code.
	if (InJson->HasField(TEXT("buffer")))
	{
		InJson->TryGetNumberField(TEXT("buffer"), bufferView->BufferIndex);
	}
	if (InJson->HasField(TEXT("byteLength")))
	{
		InJson->TryGetNumberField(TEXT("byteLength"), bufferView->ByteLength);
	}
	if (InJson->HasField(TEXT("byteOffset")))
	{
		InJson->TryGetNumberField(TEXT("byteOffset"), bufferView->ByteOffset);
	}
	return bufferView;
}

TSharedPtr<FJsonObject> UG4MFBufferView4D::ToJson() const
{
	TSharedPtr<FJsonObject> json = WriteItemEntriesToJson();
	if (BufferIndex != 0)
	{
		json->SetNumberField(TEXT("buffer"), BufferIndex);
	}
	json->SetNumberField(TEXT("byteLength"), ByteLength);
	if (ByteOffset != 0)
	{
		json->SetNumberField(TEXT("byteOffset"), ByteOffset);
	}
	return json;
}
