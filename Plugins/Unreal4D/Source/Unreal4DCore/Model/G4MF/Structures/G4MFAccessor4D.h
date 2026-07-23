#pragma once

#include "G4MFItem4D.h"

#include "G4MFAccessor4D.generated.h"

// UG4MFAccessor4D is a class that provides a typed way to access the data in a UG4MFBufferView4D.
// As an analogy with computer storage drives, if a buffer is a disk and
// a buffer view is a partition, then an accessor is a file system.
UCLASS(Category = "Unreal 4D|Model|G4MF")
class UNREAL4DCORE_API UG4MFAccessor4D : public UG4MFItem4D
{
	GENERATED_BODY()

private:
	UPROPERTY()
	int32 _vectorSize = 1;

	// Private functions for determining the minimal component type.
	static constexpr uint32 CANT_USE_COMP_TYPE = 1000000; // Any very big number will do.
	static bool DoubleBitsEqual(const double InA, const double InB);
	static void MinimalComponentBitsForDoubleOnly(const double InValue, uint32& OutFloatBits);
	static void MinimalComponentBitsForDouble(const double InValue, uint32& OutFloatBits, uint32& OutIntBits, uint32& OutUintBits);
	static void MinimalComponentBitsForInt64(const int64 InValue, uint32& OutIntBits, uint32& OutUintBits);
	static FString MinimalComponentTypeGivenBits(const uint32 InFloatBits, const uint32 InIntBits, const uint32 InUintBits);

public:
	UPROPERTY()
	FString ComponentType = TEXT("");  // No default, must be explicitly set.

	UPROPERTY()
	int32 BufferViewIndex = -1;

	int32 GetVectorSize() const { return _vectorSize; }
	void SetVectorSize(const int32 InVectorSize);

	// General helper functions.
	bool IsEqualExact(const UG4MFAccessor4D* InOther) const;
	int64 GetBytesPerComponent() const;
	int64 GetBytesPerVector() const;

	// Determine the minimal component type for the given data.
	// Add more types only as needed otherwise this will be a mess.
	static FString MinimalComponentTypeForColors(const TArray<FLinearColor>& InInputData);
	template<typename T>
	static FString MinimalComponentTypeForFloats(const TArray<T>& InInputData);
	template<typename T>
	static FString MinimalComponentTypeForInts(const TArray<T>& InInputData);
	template<typename T>
	static FString MinimalComponentTypeForUints(const TArray<T>& InInputData);
	static FString MinimalComponentTypeForVector3s(const TArray<FVector>& InInputData);
	static FString MinimalComponentTypeForVector4s(const TArray<FVector4>& InInputData);

	// Decode functions.
	TArray64<uint8> LoadBytesFromBufferView(const UG4MFState4D* InG4MFState) const;
	template<typename T>
	TArray<T> DecodeNumbersFromBytes(const UG4MFState4D* InG4MFState) const;
	TArray<FLinearColor> DecodeColorsFromBytes(const UG4MFState4D* InG4MFState) const;
	TArray<FVector> DecodeVector3sFromBytes(const UG4MFState4D* InG4MFState) const;
	TArray<FVector4> DecodeVector4sFromBytes(const UG4MFState4D* InG4MFState) const;

	// Encode functions.
	template<typename T>
	TArray64<uint8> EncodeNumbersAsBytes(const TArray<T>& InInputNumbers) const;
	int32 StoreAccessorDataIntoState(UG4MFState4D* InG4MFState, const TArray64<uint8>& InDataBytes, const bool bInDeduplicate = true);
	static UG4MFAccessor4D* MakeNewAccessorWithoutData(UG4MFState4D* InOuterState, const FString& InComponentType, const int32 InVectorSize = 1);

	static int32 EncodeNewAccessorFromColors(UG4MFState4D* InG4MFState, const TArray<FLinearColor>& InInputData, const bool bInDeduplicate = true);
	template<typename T>
	static int32 EncodeNewAccessorFromFloats(UG4MFState4D* InG4MFState, const TArray<T>& InInputData, const bool bInDeduplicate = true);
	template<typename T>
	static int32 EncodeNewAccessorFromInts(UG4MFState4D* InG4MFState, const TArray<T>& InInputData, const bool bInDeduplicate = true);
	template<typename T>
	static int32 EncodeNewAccessorFromUints(UG4MFState4D* InG4MFState, const TArray<T>& InInputData, const bool bInDeduplicate = true);
	static int32 EncodeNewAccessorFromVector3s(UG4MFState4D* InG4MFState, const TArray<FVector>& InInputData, const bool bInDeduplicate = true);
	static int32 EncodeNewAccessorFromVector4s(UG4MFState4D* InG4MFState, const TArray<FVector4>& InInputData, const bool bInDeduplicate = true);

	// Dictionary conversion.
	static UG4MFAccessor4D* FromJson(UG4MFState4D* InG4MFState, const FJsonObject* InJson);
	TSharedPtr<FJsonObject> ToJson() const;
};
