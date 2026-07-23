#pragma once

#include "../../Unreal4DDefines.h"

#include "../../Math/Transform4D.h"
#include "../../Math/VectorFuncs4D.h"
#include "Containers/Array.h"
#include "Dom/JsonObject.h"
#include "Templates/SharedPointer.h"

#include "G4MFDefines.generated.h"

UENUM()
enum class EG4MFEncodingFormat4D : uint32
{
	Unknown = 0xFFFFFFFFu, // Max value.
	Plain = 0,
	Zstd = 0x6474735Au, // "Zstd" in ASCII. Note that this does not need to match the magic number in Zstd itself (0xFD2FB528).
};

UENUM()
enum class EG4MFExternalDataMode4D : uint8
{
	Automatic,
	EmbedEverything,
	SeparateAllFiles,
	SeparateBinaryBlobs,
	SeparateResourceFiles,
	SeparateModelFiles,
};

// Wrapper struct for nested array support since `TArray<TArray<>>` is not allowed in Unreal.
USTRUCT()
struct FG4MFBuffer4D
{
	GENERATED_BODY()

	// UnrealHeaderTool only supports TArray's default 32-bit allocator, so this cannot be a UPROPERTY.
	TArray64<uint8> Data;

	bool Serialize(FArchive& Ar)
	{
		Ar << Data;
		return !Ar.IsError();
	}
};

template<>
struct TStructOpsTypeTraits<FG4MFBuffer4D> : public TStructOpsTypeTraitsBase2<FG4MFBuffer4D>
{
	enum
	{
		WithSerializer = true,
	};
};

// Nothing about this class is 4D-specific but it should be suffixed with "4D" anyway to avoid name collisions.
USTRUCT()
struct FInt32Array4D
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<int32> Data;
};

// This needs to be forward declared everywhere, because lots of
// UG4MFState4D's sub-structure functions take a UG4MFState4D as the first parameter.
class UG4MFState4D;
