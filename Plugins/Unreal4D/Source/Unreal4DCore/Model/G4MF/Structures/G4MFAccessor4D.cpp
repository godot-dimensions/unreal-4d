#include "G4MFAccessor4D.h"

#include "../../../Math/Math4D.h"
#include "../G4MFState4D.h"

// Private functions for determining the minimal component type.

bool UG4MFAccessor4D::DoubleBitsEqual(const double InA, const double InB)
{
	union {
		double d;
		uint64_t bits;
	} a{ InA }, b{ InB };
	return a.bits == b.bits;
}

void UG4MFAccessor4D::MinimalComponentBitsForDoubleOnly(const double InValue, uint32 &OutFloatBits)
{
	if (OutFloatBits == 8 && !DoubleBitsEqual(UMath4D::Float8ToDouble(UMath4D::DoubleToFloat8(InValue)), InValue))
	{
		OutFloatBits = 16;
	}
	if (OutFloatBits == 16 && !DoubleBitsEqual(UMath4D::Float16ToDouble(UMath4D::DoubleToFloat16(InValue)), InValue))
	{
		OutFloatBits = 32;
	}
	if (OutFloatBits == 32 && !DoubleBitsEqual((double)(float)InValue, InValue))
	{
		OutFloatBits = 64;
	}
}

void UG4MFAccessor4D::MinimalComponentBitsForDouble(const double InValue, uint32 &OutFloatBits, uint32 &OutIntBits, uint32 &OutUintBits)
{
	if (OutFloatBits == 8 && !DoubleBitsEqual(UMath4D::Float8ToDouble(UMath4D::DoubleToFloat8(InValue)), InValue))
	{
		OutFloatBits = 16;
	}
	if (OutFloatBits == 16 && !DoubleBitsEqual(UMath4D::Float16ToDouble(UMath4D::DoubleToFloat16(InValue)), InValue))
	{
		OutFloatBits = 32;
	}
	if (OutFloatBits == 32 && !DoubleBitsEqual((double)(float)InValue, InValue))
	{
		OutFloatBits = 64;
	}
	// Converting a non-finite or out-of-range floating-point value to an integer
	// is undefined behavior in C++. The upper bound is 2^63 because INT64_MAX
	// rounds to 2^63 when represented as a double.
	if (!FMath::IsFinite(InValue) || InValue < (double)INT64_MIN || InValue >= 9223372036854775808.0)
	{
		OutIntBits = CANT_USE_COMP_TYPE;
		OutUintBits = CANT_USE_COMP_TYPE;
		return;
	}
	const int64 valueAsInt = (int64)InValue;
	if (InValue != (double)valueAsInt)
	{
		// Can't be represented as an int, so just set these to a very big number.
		OutIntBits = CANT_USE_COMP_TYPE;
		OutUintBits = CANT_USE_COMP_TYPE;
		return;
	}
	MinimalComponentBitsForInt64(valueAsInt, OutIntBits, OutUintBits);
}

void UG4MFAccessor4D::MinimalComponentBitsForInt64(const int64 InValue, uint32 &OutIntBits, uint32 &OutUintBits)
{
	if (OutIntBits == 8 && !(InValue >= INT8_MIN && InValue <= INT8_MAX))
	{
		OutIntBits = 16;
	}
	if (OutIntBits == 16 && !(InValue >= INT16_MIN && InValue <= INT16_MAX))
	{
		OutIntBits = 32;
	}
	if (OutIntBits == 32 && !(InValue >= INT32_MIN && InValue <= INT32_MAX))
	{
		OutIntBits = 64;
	}
	if (InValue < 0)
	{
		// Can't be represented as an unsigned int, so just set this to a very big number.
		OutUintBits = CANT_USE_COMP_TYPE;
		return;
	}
	if (OutUintBits == 8 && InValue > UINT8_MAX)
	{
		OutUintBits = 16;
	}
	if (OutUintBits == 16 && InValue > UINT16_MAX)
	{
		OutUintBits = 32;
	}
	if (OutUintBits == 32 && InValue > UINT32_MAX)
	{
		OutUintBits = 64;
	}
}

FString UG4MFAccessor4D::MinimalComponentTypeGivenBits(const uint32 InFloatBits, const uint32 InIntBits, const uint32 InUintBits)
{
	// Only use floats if they are more efficient than integers.
	if (InFloatBits < InIntBits && InFloatBits < InUintBits)
	{
		return FString::Printf(TEXT("float%u"), InFloatBits);
	}
	if (InIntBits < InUintBits)
	{
		return FString::Printf(TEXT("int%u"), InIntBits);
	}
	// Prefer unsigned ints if all are equally efficient.
	return FString::Printf(TEXT("uint%u"), InUintBits);
}

void UG4MFAccessor4D::SetVectorSize(const int32 InVectorSize)
{
	if (InVectorSize < 1)
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("G4MFAccessor4D: Vector size must be a positive integer. Refusing to set."));
		return;
	}
	_vectorSize = InVectorSize;
}

// General helper functions.

bool UG4MFAccessor4D::IsEqualExact(const UG4MFAccessor4D* InOther) const
{
	if (InOther == nullptr || !IsValid(InOther)) return false;
	return (BufferViewIndex == InOther->BufferViewIndex &&
			_vectorSize == InOther->GetVectorSize() &&
			ComponentType == InOther->ComponentType);
}

int64 UG4MFAccessor4D::GetBytesPerComponent() const
{
	// Extract numeric digits from component type string. Ex: "float32" -> 32.
	int32 bits = 0;
	for (TCHAR character : ComponentType)
	{
		if (FChar::IsDigit(character))
		{
			bits = bits * 10 + (character - TEXT('0'));
		}
	}
	// Bits to bytes. Ex: 32 bits -> 4 bytes.
	return bits / 8;
}

int64 UG4MFAccessor4D::GetBytesPerVector() const
{
	return GetBytesPerComponent() * _vectorSize;
}

// Determine the minimal component type for encoding the given data.
// Add more types only as needed, otherwise this will be a mess.

FString UG4MFAccessor4D::MinimalComponentTypeForColors(const TArray<FLinearColor> &InInputData)
{
	uint32 minFloatBits = 8;
	for (const FLinearColor &color : InInputData)
	{
		MinimalComponentBitsForDoubleOnly(color.R, minFloatBits);
		MinimalComponentBitsForDoubleOnly(color.G, minFloatBits);
		MinimalComponentBitsForDoubleOnly(color.B, minFloatBits);
		MinimalComponentBitsForDoubleOnly(color.A, minFloatBits);
	}
	return MinimalComponentTypeGivenBits(minFloatBits, CANT_USE_COMP_TYPE, CANT_USE_COMP_TYPE);
}

template<typename T>
FString UG4MFAccessor4D::MinimalComponentTypeForFloats(const TArray<T> &InInputData)
{
	uint32 minFloatBits = 8;
	uint32 minIntBits = 8;
	uint32 minUintBits = 8;
	for (const T num : InInputData)
	{
		MinimalComponentBitsForDouble(num, minFloatBits, minIntBits, minUintBits);
	}
	return MinimalComponentTypeGivenBits(minFloatBits, minIntBits, minUintBits);
}

template<typename T>
FString UG4MFAccessor4D::MinimalComponentTypeForInts(const TArray<T> &InInputData)
{
	uint32 minIntBits = 8;
	uint32 minUintBits = 8;
	for (const T num : InInputData)
	{
		MinimalComponentBitsForInt64(num, minIntBits, minUintBits);
	}
	return MinimalComponentTypeGivenBits(CANT_USE_COMP_TYPE, minIntBits, minUintBits);
}

template<typename T>
FString UG4MFAccessor4D::MinimalComponentTypeForUints(const TArray<T> &InInputData)
{
	uint32 minIntBits = 8;
	uint32 minUintBits = 8;
	for (const T num : InInputData)
	{
		MinimalComponentBitsForInt64(num, minIntBits, minUintBits);
	}
	if (minUintBits == CANT_USE_COMP_TYPE)
	{
		// Edge case that happens when a uint64 overflows into negative when cast to int64.
		return TEXT("uint64");
	}
	return MinimalComponentTypeGivenBits(CANT_USE_COMP_TYPE, CANT_USE_COMP_TYPE, minUintBits);
}

FString UG4MFAccessor4D::MinimalComponentTypeForVector3s(const TArray<FVector> &InInputData)
{
	uint32 minFloatBits = 8;
	uint32 minIntBits = 8;
	uint32 minUintBits = 8;
	for (const FVector& vec : InInputData)
	{
		MinimalComponentBitsForDouble(vec.X, minFloatBits, minIntBits, minUintBits);
		MinimalComponentBitsForDouble(vec.Y, minFloatBits, minIntBits, minUintBits);
		MinimalComponentBitsForDouble(vec.Z, minFloatBits, minIntBits, minUintBits);
	}
	return MinimalComponentTypeGivenBits(minFloatBits, minIntBits, minUintBits);
}

FString UG4MFAccessor4D::MinimalComponentTypeForVector4s(const TArray<FVector4> &InInputData)
{
	uint32 minFloatBits = 8;
	uint32 minIntBits = 8;
	uint32 minUintBits = 8;
	for (const FVector4& vec : InInputData)
	{
		MinimalComponentBitsForDouble(vec.X, minFloatBits, minIntBits, minUintBits);
		MinimalComponentBitsForDouble(vec.Y, minFloatBits, minIntBits, minUintBits);
		MinimalComponentBitsForDouble(vec.Z, minFloatBits, minIntBits, minUintBits);
		MinimalComponentBitsForDouble(vec.W, minFloatBits, minIntBits, minUintBits);
	}
	return MinimalComponentTypeGivenBits(minFloatBits, minIntBits, minUintBits);
}

// Decode functions.

TArray64<uint8> UG4MFAccessor4D::LoadBytesFromBufferView(const UG4MFState4D* InG4MFState) const
{
	if (InG4MFState == nullptr) { return TArray64<uint8>(); }
	if (!InG4MFState->G4MFBufferViews.IsValidIndex(BufferViewIndex))
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF import: The buffer view index is out of bounds. Returning an empty byte array."));
		return TArray64<uint8>();
	}
	const UG4MFBufferView4D* bufferView = InG4MFState->G4MFBufferViews[BufferViewIndex];
	if (bufferView == nullptr)
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF import: The buffer view is null. Returning an empty byte array."));
		return TArray64<uint8>();
	}
	const TArray64<uint8> rawBytes = bufferView->ReadBufferViewData(InG4MFState);
	const int64 bytesPerVector = GetBytesPerVector();
	if (bytesPerVector <= 0)
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF import: Failed to determine the number of bytes per vector. Returning an empty byte array."));
		return TArray64<uint8>();
	}
	if ((rawBytes.Num() % bytesPerVector) != 0)
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF import: The buffer view size was not a multiple of the vector size. Returning an empty byte array."));
		return TArray64<uint8>();
	}
	return rawBytes;
}

template<typename T>
TArray<T> UG4MFAccessor4D::DecodeNumbersFromBytes(const UG4MFState4D* InG4MFState) const
{
	TArray64<uint8> rawBytes = LoadBytesFromBufferView(InG4MFState);
	TArray<T> numbers;
	if (rawBytes.Num() == 0)
	{
		return numbers; // Return the empty array.
	}
	const uint8* dataPtr = rawBytes.GetData();
	const int64 byteCount = rawBytes.Num();
	// This is guaranteed to be non-zero and positive if LoadBytesFromBufferView returned a non-empty array.
	const int64 bytesPerComponent = GetBytesPerComponent();
	check(bytesPerComponent > 0 && (byteCount % bytesPerComponent) == 0);
	const int64 componentCount = byteCount / bytesPerComponent;
	if (componentCount > MAX_int32)
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF import: Accessor contains too many components for Unreal's 32-bit result arrays. Returning an empty array."));
		return numbers;
	}
	numbers.SetNumUninitialized(componentCount);
	if (ComponentType.StartsWith(TEXT("float")))
	{
		for (int64 numberIndex = 0; numberIndex < componentCount; numberIndex++)
		{
			const int64 byteIndex = numberIndex * bytesPerComponent;
			switch (bytesPerComponent)
			{
				case 1: {
					const uint8 value8 = dataPtr[byteIndex];
					numbers[numberIndex] = (T)UMath4D::Float8ToDouble(value8);
				} break;
				case 2: {
					uint16 value16;
					FMemory::Memcpy(&value16, dataPtr + byteIndex, sizeof(value16));
					numbers[numberIndex] = (T)UMath4D::Float16ToDouble(value16);
				} break;
				case 4: {
					float value32;
					FMemory::Memcpy(&value32, dataPtr + byteIndex, sizeof(value32));
					numbers[numberIndex] = (T)value32;
				} break;
				case 8: {
					double value64;
					FMemory::Memcpy(&value64, dataPtr + byteIndex, sizeof(value64));
					numbers[numberIndex] = (T)value64;
				} break;
				default: {
					UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF import: Unsupported float component size %lld. Returning an empty array."), bytesPerComponent);
					return {};
				} break;
			}
		}
	}
	else if (ComponentType.StartsWith(TEXT("int")))
	{
		for (int64 numberIndex = 0; numberIndex < componentCount; numberIndex++)
		{
			const int64 byteIndex = numberIndex * bytesPerComponent;
			switch (bytesPerComponent)
			{
				case 1: {
					int8 value8;
					FMemory::Memcpy(&value8, dataPtr + byteIndex, sizeof(value8));
					numbers[numberIndex] = (T)value8;
				} break;
				case 2: {
					int16 value16;
					FMemory::Memcpy(&value16, dataPtr + byteIndex, sizeof(value16));
					numbers[numberIndex] = (T)value16;
				} break;
				case 4: {
					int32 value32;
					FMemory::Memcpy(&value32, dataPtr + byteIndex, sizeof(value32));
					numbers[numberIndex] = (T)value32;
				} break;
				case 8: {
					int64 value64;
					FMemory::Memcpy(&value64, dataPtr + byteIndex, sizeof(value64));
					numbers[numberIndex] = (T)value64;
				} break;
				default: {
					UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF import: Unsupported int component size %lld. Returning an empty array."), bytesPerComponent);
					return {};
				} break;
			}
		}
	}
	else if (ComponentType.StartsWith(TEXT("uint")))
	{
		for (int64 numberIndex = 0; numberIndex < componentCount; numberIndex++)
		{
			const int64 byteIndex = numberIndex * bytesPerComponent;
			switch (bytesPerComponent)
			{
				case 1: {
					uint8 value8 = dataPtr[byteIndex];
					numbers[numberIndex] = (T)value8;
				} break;
				case 2: {
					uint16 value16;
					FMemory::Memcpy(&value16, dataPtr + byteIndex, sizeof(value16));
					numbers[numberIndex] = (T)value16;
				} break;
				case 4: {
					uint32 value32;
					FMemory::Memcpy(&value32, dataPtr + byteIndex, sizeof(value32));
					numbers[numberIndex] = (T)value32;
				} break;
				case 8: {
					uint64 value64;
					FMemory::Memcpy(&value64, dataPtr + byteIndex, sizeof(value64));
					numbers[numberIndex] = (T)value64;
				} break;
				default: {
					UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF import: Unsupported uint component size %lld. Returning an empty array."), bytesPerComponent);
					return {};
				} break;
			}
		}
	}
	else
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF import: Unsupported component type %s. Returning an empty array."), *ComponentType);
		return {};
	}
	return numbers;
}

TArray<FLinearColor> UG4MFAccessor4D::DecodeColorsFromBytes(const UG4MFState4D* InG4MFState) const
{
	const TArray<float> numbers = DecodeNumbersFromBytes<float>(InG4MFState);
	TArray<FLinearColor> colors;
	if (_vectorSize == 1)
	{
		const int64 colorCount = numbers.Num();
		colors.Reserve(colorCount);
		for (int64 I = 0; I < colorCount; I++)
		{
			const float Gray = numbers[I];
			colors.Add(FLinearColor(Gray, Gray, Gray, 1.0f));
		}
	}
	else if (_vectorSize == 3)
	{
		const int64 colorCount = numbers.Num() / 3;
		colors.Reserve(colorCount);
		for (int64 I = 0; I < colorCount; I++)
		{
			const int64 NumOffset = I * 3;
			colors.Add(FLinearColor(numbers[NumOffset + 0], numbers[NumOffset + 1], numbers[NumOffset + 2], 1.0f));
		}
	}
	else if (_vectorSize == 4)
	{
		const int64 colorCount = numbers.Num() / 4;
		colors.Reserve(colorCount);
		for (int64 I = 0; I < colorCount; I++)
		{
			const int64 NumOffset = I * 4;
			colors.Add(FLinearColor(numbers[NumOffset + 0], numbers[NumOffset + 1], numbers[NumOffset + 2], numbers[NumOffset + 3]));
		}
	}
	else
	{
		UE_LOG(LogUnreal4DCore, Warning, TEXT("G4MFAccessor4D: Cannot decode colors from accessor with vector size %lld. Returning an empty array."), _vectorSize);
	}
	return colors;
}

TArray<FVector> UG4MFAccessor4D::DecodeVector3sFromBytes(const UG4MFState4D* InG4MFState) const
{
	const TArray<double> numbers = DecodeNumbersFromBytes<double>(InG4MFState);
	TArray<FVector> vectors;
	const int64 vectorCount = numbers.Num() / _vectorSize;
	const int64 clampedVectorSize = FMath::Min(_vectorSize, 3);
	vectors.Reserve(vectorCount);
	for (int64 vectorIndex = 0; vectorIndex < vectorCount; vectorIndex++)
	{
		const int64 numOffset = vectorIndex * _vectorSize;
		FVector vec = FVector(0.0, 0.0, 0.0);
		for (int64 componentIndex = 0; componentIndex < clampedVectorSize; componentIndex++)
		{
			vec[componentIndex] = numbers[numOffset + componentIndex];
		}
		vectors.Add(vec);
	}
	return vectors;
}

TArray<FVector4> UG4MFAccessor4D::DecodeVector4sFromBytes(const UG4MFState4D* InG4MFState) const
{
	const TArray<double> numbers = DecodeNumbersFromBytes<double>(InG4MFState);
	TArray<FVector4> vectors;
	const int64 vectorCount = numbers.Num() / _vectorSize;
	const int64 clampedVectorSize = FMath::Min(_vectorSize, 4);
	vectors.Reserve(vectorCount);
	for (int64 vectorIndex = 0; vectorIndex < vectorCount; vectorIndex++)
	{
		const int64 numOffset = vectorIndex * _vectorSize;
		FVector4 vec = FVector4(0.0, 0.0, 0.0, 0.0);
		for (int64 componentIndex = 0; componentIndex < clampedVectorSize; componentIndex++)
		{
			vec[componentIndex] = numbers[numOffset + componentIndex];
		}
		vectors.Add(vec);
	}
	return vectors;
}

// Encode functions.

template<typename T>
TArray64<uint8> UG4MFAccessor4D::EncodeNumbersAsBytes(const TArray<T>& InInputNumbers) const
{
	TArray64<uint8> ret;
	const int64 inputSize = InInputNumbers.Num();
	if (inputSize == 0)
	{
		return ret; // Return the empty array.
	}
	const int64 bytesPerComponent = GetBytesPerComponent();
	if (bytesPerComponent < 1)
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF export: Invalid component type '%s'."), *ComponentType);
		return ret; // Return the empty array.
	}
	const int64 rawByteSize = bytesPerComponent * inputSize;
	ret.SetNumUninitialized(rawByteSize);
	uint8* retWritePtr = ret.GetData();
	if (ComponentType.StartsWith(TEXT("float")))
	{
		for (int64 numberIndex = 0; numberIndex < inputSize; numberIndex++)
		{
			const int64 byteOffset = numberIndex * bytesPerComponent;
			switch (bytesPerComponent)
			{
				case 1: {
					const uint8 value8 = UMath4D::DoubleToFloat8((double)InInputNumbers[numberIndex]);
					FMemory::Memcpy(retWritePtr + byteOffset, &value8, sizeof(value8));
				} break;
				case 2: {
					const uint16 value16 = UMath4D::DoubleToFloat16((double)InInputNumbers[numberIndex]);
					FMemory::Memcpy(retWritePtr + byteOffset, &value16, sizeof(value16));
				} break;
				case 4: {
					const float value32 = (float)InInputNumbers[numberIndex];
					FMemory::Memcpy(retWritePtr + byteOffset, &value32, sizeof(value32));
				} break;
				case 8: {
					const double value64 = (double)InInputNumbers[numberIndex];
					FMemory::Memcpy(retWritePtr + byteOffset, &value64, sizeof(value64));
				} break;
				default: {
					UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF export: Unsupported float component of size '%lld'. Returning an empty array."), bytesPerComponent);
					return {};
				} break;
			}
		}
	}
	else if (ComponentType.StartsWith(TEXT("int")))
	{
		for (int64 numberIndex = 0; numberIndex < inputSize; numberIndex++)
		{
			const int64 byteOffset = numberIndex * bytesPerComponent;
			switch (bytesPerComponent)
			{
				case 1: {
					const int8 value8 = (int8)InInputNumbers[numberIndex];
					FMemory::Memcpy(retWritePtr + byteOffset, &value8, sizeof(value8));
				} break;
				case 2: {
					const int16 value16 = (int16)InInputNumbers[numberIndex];
					FMemory::Memcpy(retWritePtr + byteOffset, &value16, sizeof(value16));
				} break;
				case 4: {
					const int32 value32 = (int32)InInputNumbers[numberIndex];
					FMemory::Memcpy(retWritePtr + byteOffset, &value32, sizeof(value32));
				} break;
				case 8: {
					const int64 value64 = (int64)InInputNumbers[numberIndex];
					FMemory::Memcpy(retWritePtr + byteOffset, &value64, sizeof(value64));
				} break;
				default: {
					UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF export: Unsupported int component of size '%lld' bytes. Returning an empty array."), bytesPerComponent);
					return {};
				} break;
			}
		}
	}
	else if (ComponentType.StartsWith(TEXT("uint")))
	{
		for (int64 numberIndex = 0; numberIndex < inputSize; numberIndex++)
		{
			const int64 byteOffset = numberIndex * bytesPerComponent;
			switch (bytesPerComponent)
			{
				case 1: {
					const uint8 value8 = (uint8)InInputNumbers[numberIndex];
					FMemory::Memcpy(retWritePtr + byteOffset, &value8, sizeof(value8));
				} break;
				case 2: {
					const uint16 value16 = (uint16)InInputNumbers[numberIndex];
					FMemory::Memcpy(retWritePtr + byteOffset, &value16, sizeof(value16));
				} break;
				case 4: {
					const uint32 value32 = (uint32)InInputNumbers[numberIndex];
					FMemory::Memcpy(retWritePtr + byteOffset, &value32, sizeof(value32));
				} break;
				case 8: {
					const uint64 value64 = (uint64)InInputNumbers[numberIndex];
					FMemory::Memcpy(retWritePtr + byteOffset, &value64, sizeof(value64));
				} break;
				default: {
					UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF export: Unsupported uint component of size '%lld' bytes. Returning an empty array."), bytesPerComponent);
					return {};
				} break;
			}
		}
	}
	else
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF import: Unsupported component type %s. Returning an empty array."), *ComponentType);
		return {};
	}
	return ret;
}

int32 UG4MFAccessor4D::StoreAccessorDataIntoState(UG4MFState4D* InG4MFState, const TArray64<uint8> &InDataBytes, const bool bInDeduplicate)
{
	if (InG4MFState == nullptr) { return -1; }
	if (InDataBytes.IsEmpty())
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF export: Cannot store nothing."));
		return -1;
	}
	const int64 bytesPerComponent = GetBytesPerComponent();
	if (bytesPerComponent < 1)
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF export: Cannot store accessor data with an invalid component type '%s'."), *ComponentType);
		return -1;
	}
	// Write the data into a new buffer view.
	const int32 newBufferViewIndex = UG4MFBufferView4D::WriteNewBufferViewIntoState(InG4MFState, InDataBytes, GetBytesPerComponent(), bInDeduplicate);
	if (newBufferViewIndex < 0)
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF export: Accessor failed to write new buffer view into G4MF state."));
		return -1;
	}
	BufferViewIndex = newBufferViewIndex;
	// Add the new accessor to the state, but check for duplicates first.
	const int32 accessorCount = InG4MFState->G4MFAccessors.Num();
	if (bInDeduplicate)
	{
		for (int32 accessorIndex = 0; accessorIndex < accessorCount; accessorIndex++)
		{
			const UG4MFAccessor4D* existingAccessor = InG4MFState->G4MFAccessors[accessorIndex];
			if (IsEqualExact(existingAccessor))
			{
				// An identical accessor already exists in the state, so just return the index.
				return accessorIndex;
			}
		}
	}
	InG4MFState->G4MFAccessors.Add(this);
	return accessorCount;
}

UG4MFAccessor4D* UG4MFAccessor4D::MakeNewAccessorWithoutData(UG4MFState4D* InOuterState, const FString &InComponentType, const int32 InVectorSize)
{
	UG4MFAccessor4D* ret = NewObject<UG4MFAccessor4D>(InOuterState);
	ret->ComponentType = InComponentType;
	ret->SetVectorSize(InVectorSize);
	return ret;
}

int32 UG4MFAccessor4D::EncodeNewAccessorFromColors(UG4MFState4D* InG4MFState, const TArray<FLinearColor>& InInputData, const bool bInDeduplicate)
{
	const FString componentType = MinimalComponentTypeForColors(InInputData);
	int32 vectorSize = 3;
	for (int32 colorIndex = 0; colorIndex < InInputData.Num(); colorIndex++)
	{
		if (!FMath::IsNearlyEqual(InInputData[colorIndex].A, 1.0f, KINDA_SMALL_NUMBER))
		{
			vectorSize = 4; // Need to store alpha channel.
			break;
		}
	}
	TArray<float> numbers;
	numbers.Reserve(InInputData.Num() * vectorSize);
	for (int64 colorIndex = 0; colorIndex < InInputData.Num(); colorIndex++)
	{
		const FLinearColor& color = InInputData[colorIndex];
		numbers.Add(color.R);
		numbers.Add(color.G);
		numbers.Add(color.B);
		if (vectorSize == 4)
		{
			numbers.Add(color.A);
		}
	}
	UG4MFAccessor4D* ret = MakeNewAccessorWithoutData(InG4MFState, componentType, vectorSize);
	TArray64<uint8> encodedBytes = ret->EncodeNumbersAsBytes<float>(numbers);
	if (encodedBytes.IsEmpty())
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF export: Accessor failed to encode data as bytes (was the input data empty?)."));
		return -1;
	}
	return ret->StoreAccessorDataIntoState(InG4MFState, encodedBytes, bInDeduplicate);
}

template<typename T>
int32 UG4MFAccessor4D::EncodeNewAccessorFromFloats(UG4MFState4D* InG4MFState, const TArray<T>& InInputData, const bool bInDeduplicate)
{
	const FString componentType = MinimalComponentTypeForFloats(TArray<T>(InInputData.GetData(), InInputData.Num()));
	UG4MFAccessor4D* ret = MakeNewAccessorWithoutData(InG4MFState, componentType, 1);
	TArray64<uint8> encodedBytes = ret->EncodeNumbersAsBytes<T>(InInputData);
	if (encodedBytes.IsEmpty())
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF export: Accessor failed to encode data as bytes (was the input data empty?)."));
		return -1;
	}
	return ret->StoreAccessorDataIntoState(InG4MFState, encodedBytes, bInDeduplicate);
}

template<typename T>
int32 UG4MFAccessor4D::EncodeNewAccessorFromInts(UG4MFState4D* InG4MFState, const TArray<T>& InInputData, const bool bInDeduplicate)
{
	const FString componentType = MinimalComponentTypeForInts(TArray<T>(InInputData.GetData(), InInputData.Num()));
	UG4MFAccessor4D* ret = MakeNewAccessorWithoutData(InG4MFState, componentType, 1);
	TArray64<uint8> encodedBytes = ret->EncodeNumbersAsBytes<T>(InInputData);
	if (encodedBytes.IsEmpty())
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF export: Accessor failed to encode data as bytes (was the input data empty?)."));
		return -1;
	}
	return ret->StoreAccessorDataIntoState(InG4MFState, encodedBytes, bInDeduplicate);
}

template<typename T>
int32 UG4MFAccessor4D::EncodeNewAccessorFromUints(UG4MFState4D* InG4MFState, const TArray<T>& InInputData, const bool bInDeduplicate)
{
	const FString componentType = MinimalComponentTypeForUints(TArray<T>(InInputData.GetData(), InInputData.Num()));
	UG4MFAccessor4D* ret = MakeNewAccessorWithoutData(InG4MFState, componentType, 1);
	TArray64<uint8> encodedBytes = ret->EncodeNumbersAsBytes<T>(InInputData);
	if (encodedBytes.IsEmpty())
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF export: Accessor failed to encode data as bytes (was the input data empty?)."));
		return -1;
	}
	return ret->StoreAccessorDataIntoState(InG4MFState, encodedBytes, bInDeduplicate);
}

int32 UG4MFAccessor4D::EncodeNewAccessorFromVector3s(UG4MFState4D* InG4MFState, const TArray<FVector>& InInputData, const bool bInDeduplicate)
{
	const FString componentType = MinimalComponentTypeForVector3s(InInputData);
	UG4MFAccessor4D* ret = MakeNewAccessorWithoutData(InG4MFState, componentType, 3);
	TArray<double> numbers;
	numbers.Reserve(InInputData.Num() * 3);
	for (int64 I = 0; I < InInputData.Num(); I++)
	{
		const FVector4 &vector = InInputData[I];
		numbers.Add(vector.X);
		numbers.Add(vector.Y);
		numbers.Add(vector.Z);
	}
	TArray64<uint8> encodedBytes = ret->EncodeNumbersAsBytes<double>(numbers);
	if (encodedBytes.IsEmpty())
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF export: Accessor failed to encode data as bytes (was the input data empty?)."));
		return -1;
	}
	return ret->StoreAccessorDataIntoState(InG4MFState, encodedBytes, bInDeduplicate);
}

int32 UG4MFAccessor4D::EncodeNewAccessorFromVector4s(UG4MFState4D* InG4MFState, const TArray<FVector4> &InInputData, const bool bInDeduplicate)
{
	const FString componentType = MinimalComponentTypeForVector4s(InInputData);
	UG4MFAccessor4D* ret = MakeNewAccessorWithoutData(InG4MFState, componentType, 4);
	TArray<double> numbers;
	numbers.Reserve(InInputData.Num() * 4);
	for (int64 I = 0; I < InInputData.Num(); I++)
	{
		const FVector4 &vector = InInputData[I];
		numbers.Add(vector.X);
		numbers.Add(vector.Y);
		numbers.Add(vector.Z);
		numbers.Add(vector.W);
	}
	TArray64<uint8> encodedBytes = ret->EncodeNumbersAsBytes<double>(numbers);
	if (encodedBytes.IsEmpty())
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF export: Accessor failed to encode data as bytes (was the input data empty?)."));
		return -1;
	}
	return ret->StoreAccessorDataIntoState(InG4MFState, encodedBytes, bInDeduplicate);
}

// JSON serialization conversion.

UG4MFAccessor4D* UG4MFAccessor4D::FromJson(UG4MFState4D* InG4MFState, const FJsonObject* InJson)
{
	if (InJson == nullptr) { return nullptr; }
	const FName uniqueName = InG4MFState->ReserveUniqueFNameFromJson(InJson);
	UG4MFAccessor4D* ret = NewObject<UG4MFAccessor4D>(InG4MFState, uniqueName);
	ret->ReadItemEntriesFromJson(InJson);
	if (InJson->HasField(TEXT("bufferView")))
	{
		ret->BufferViewIndex = InJson->GetIntegerField(TEXT("bufferView"));
	}
	if (InJson->HasField(TEXT("componentType")))
	{
		ret->ComponentType = InJson->GetStringField(TEXT("componentType"));
	}
	if (InJson->HasField(TEXT("vectorSize")))
	{
		ret->SetVectorSize(InJson->GetIntegerField(TEXT("vectorSize")));
	}
	return ret;
}

TSharedPtr<FJsonObject> UG4MFAccessor4D::ToJson() const
{
	TSharedPtr<FJsonObject> json = WriteItemEntriesToJson();
	if (BufferViewIndex != -1)
	{
		json->SetNumberField(TEXT("bufferView"), BufferViewIndex);
	}
	if (!ComponentType.IsEmpty())
	{
		json->SetStringField(TEXT("componentType"), ComponentType);
	}
	if (_vectorSize > 1)
	{
		json->SetNumberField(TEXT("vectorSize"), _vectorSize);
	}
	return json;
}

// Explicit template instantiations. It is safe to just have all of these
// variations here and the linker will automatically remove unused variations.

template FString UG4MFAccessor4D::MinimalComponentTypeForFloats<float>(const TArray<float>& InInputData);
template FString UG4MFAccessor4D::MinimalComponentTypeForFloats<double>(const TArray<double>& InInputData);
template FString UG4MFAccessor4D::MinimalComponentTypeForInts<int8>(const TArray<int8>& InInputData);
template FString UG4MFAccessor4D::MinimalComponentTypeForInts<int16>(const TArray<int16>& InInputData);
template FString UG4MFAccessor4D::MinimalComponentTypeForInts<int32>(const TArray<int32>& InInputData);
template FString UG4MFAccessor4D::MinimalComponentTypeForInts<int64>(const TArray<int64>& InInputData);
template FString UG4MFAccessor4D::MinimalComponentTypeForUints<uint8>(const TArray<uint8>& InInputData);
template FString UG4MFAccessor4D::MinimalComponentTypeForUints<uint16>(const TArray<uint16>& InInputData);
template FString UG4MFAccessor4D::MinimalComponentTypeForUints<uint32>(const TArray<uint32>& InInputData);
template FString UG4MFAccessor4D::MinimalComponentTypeForUints<uint64>(const TArray<uint64>& InInputData);

template TArray<float> UG4MFAccessor4D::DecodeNumbersFromBytes<float>(const UG4MFState4D* InG4MFState) const;
template TArray<double> UG4MFAccessor4D::DecodeNumbersFromBytes<double>(const UG4MFState4D* InG4MFState) const;
template TArray<int8> UG4MFAccessor4D::DecodeNumbersFromBytes<int8>(const UG4MFState4D* InG4MFState) const;
template TArray<int16> UG4MFAccessor4D::DecodeNumbersFromBytes<int16>(const UG4MFState4D* InG4MFState) const;
template TArray<int32> UG4MFAccessor4D::DecodeNumbersFromBytes<int32>(const UG4MFState4D* InG4MFState) const;
template TArray<int64> UG4MFAccessor4D::DecodeNumbersFromBytes<int64>(const UG4MFState4D* InG4MFState) const;
template TArray<uint8> UG4MFAccessor4D::DecodeNumbersFromBytes<uint8>(const UG4MFState4D* InG4MFState) const;
template TArray<uint16> UG4MFAccessor4D::DecodeNumbersFromBytes<uint16>(const UG4MFState4D* InG4MFState) const;
template TArray<uint32> UG4MFAccessor4D::DecodeNumbersFromBytes<uint32>(const UG4MFState4D* InG4MFState) const;
template TArray<uint64> UG4MFAccessor4D::DecodeNumbersFromBytes<uint64>(const UG4MFState4D* InG4MFState) const;

template TArray64<uint8> UG4MFAccessor4D::EncodeNumbersAsBytes<float>(const TArray<float>& InInputNumbers) const;
template TArray64<uint8> UG4MFAccessor4D::EncodeNumbersAsBytes<double>(const TArray<double>& InInputNumbers) const;
template TArray64<uint8> UG4MFAccessor4D::EncodeNumbersAsBytes<int8>(const TArray<int8>& InInputNumbers) const;
template TArray64<uint8> UG4MFAccessor4D::EncodeNumbersAsBytes<int16>(const TArray<int16>& InInputNumbers) const;
template TArray64<uint8> UG4MFAccessor4D::EncodeNumbersAsBytes<int32>(const TArray<int32>& InInputNumbers) const;
template TArray64<uint8> UG4MFAccessor4D::EncodeNumbersAsBytes<int64>(const TArray<int64>& InInputNumbers) const;
template TArray64<uint8> UG4MFAccessor4D::EncodeNumbersAsBytes<uint8>(const TArray<uint8>& InInputNumbers) const;
template TArray64<uint8> UG4MFAccessor4D::EncodeNumbersAsBytes<uint16>(const TArray<uint16>& InInputNumbers) const;
template TArray64<uint8> UG4MFAccessor4D::EncodeNumbersAsBytes<uint32>(const TArray<uint32>& InInputNumbers) const;
template TArray64<uint8> UG4MFAccessor4D::EncodeNumbersAsBytes<uint64>(const TArray<uint64>& InInputNumbers) const;

template int32 UG4MFAccessor4D::EncodeNewAccessorFromFloats<float>(UG4MFState4D* InG4MFState, const TArray<float>& InInputData, const bool bInDeduplicate);
template int32 UG4MFAccessor4D::EncodeNewAccessorFromFloats<double>(UG4MFState4D* InG4MFState, const TArray<double>& InInputData, const bool bInDeduplicate);
template int32 UG4MFAccessor4D::EncodeNewAccessorFromInts<int8>(UG4MFState4D* InG4MFState, const TArray<int8>& InInputData, const bool bInDeduplicate);
template int32 UG4MFAccessor4D::EncodeNewAccessorFromInts<int16>(UG4MFState4D* InG4MFState, const TArray<int16>& InInputData, const bool bInDeduplicate);
template int32 UG4MFAccessor4D::EncodeNewAccessorFromInts<int32>(UG4MFState4D* InG4MFState, const TArray<int32>& InInputData, const bool bInDeduplicate);
template int32 UG4MFAccessor4D::EncodeNewAccessorFromInts<int64>(UG4MFState4D* InG4MFState, const TArray<int64>& InInputData, const bool bInDeduplicate);
template int32 UG4MFAccessor4D::EncodeNewAccessorFromUints<uint8>(UG4MFState4D* InG4MFState, const TArray<uint8>& InInputData, const bool bInDeduplicate);
template int32 UG4MFAccessor4D::EncodeNewAccessorFromUints<uint16>(UG4MFState4D* InG4MFState, const TArray<uint16>& InInputData, const bool bInDeduplicate);
template int32 UG4MFAccessor4D::EncodeNewAccessorFromUints<uint32>(UG4MFState4D* InG4MFState, const TArray<uint32>& InInputData, const bool bInDeduplicate);
template int32 UG4MFAccessor4D::EncodeNewAccessorFromUints<uint64>(UG4MFState4D* InG4MFState, const TArray<uint64>& InInputData, const bool bInDeduplicate);
