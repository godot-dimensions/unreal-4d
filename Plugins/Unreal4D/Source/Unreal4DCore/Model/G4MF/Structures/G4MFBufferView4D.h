#pragma once

#include "G4MFItem4D.h"

#include "G4MFBufferView4D.generated.h"

// UG4MFBufferView4D is a class that represents a slice or view into a G4MF buffer.
// As an analogy with computer storage drives, if a buffer is a disk and
// an accessor is a file system, then a buffer view is a partition.
UCLASS(Category = "Unreal 4D|Model|G4MF")
class UNREAL4DCORE_API UG4MFBufferView4D : public UG4MFItem4D
{
	GENERATED_BODY()

public:
	UPROPERTY()
	int64 ByteOffset = 0; // MUST NOT be negative.

	UPROPERTY()
	int64 ByteLength = 0; // MUST NOT be negative.

	UPROPERTY()
	int32 BufferIndex = 0; // MUST NOT be negative.

	TArray64<uint8> ReadBufferViewData(const UG4MFState4D* InG4MFState) const;
	static int32 WriteNewBufferViewIntoState(UG4MFState4D* InG4MFState, const TArray64<uint8>& InInputData, const int64 InAlignment = 1, const bool InDeduplicate = true, const int32 InBufferIndex = 0);

	static UG4MFBufferView4D* FromJson(UG4MFState4D* InG4MFState, const FJsonObject* InJson);
	TSharedPtr<FJsonObject> ToJson() const;
};
