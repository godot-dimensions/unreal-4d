#include "G4MFTexture4D.h"

#include "../G4MFState4D.h"

UG4MFTexture4D* UG4MFTexture4D::FromJson(UG4MFState4D* InG4MFState, const FJsonObject* InJson)
{
	// General code for all UG4MFItem4D-derived classes.
	if (InJson == nullptr) { return nullptr; }
	const FName uniqueName = InG4MFState->ReserveUniqueFNameFromJson(InJson);
	UG4MFTexture4D* ret = NewObject<UG4MFTexture4D>(InG4MFState, uniqueName);
	ret->ReadItemEntriesFromJson(InJson);
	// UG4MFTexture4D-specific code.
	if (InJson->HasField(TEXT("files")))
	{
		TArray<TSharedPtr<FJsonValue>> imageFileIndicesJsonArray = InJson->GetArrayField(TEXT("files"));
		for (const TSharedPtr<FJsonValue>& imageFileIndexJsonValue : imageFileIndicesJsonArray)
		{
			int32 imageFileIndexInt;
			if (imageFileIndexJsonValue->TryGetNumber(imageFileIndexInt))
			{
				ret->ImageFileIndices.Add(imageFileIndexInt);
			}
		}
	}
	if (InJson->HasField(TEXT("placeholder")))
	{
		ret->PlaceholderColor = UG4MFItem4D::JsonArrayToColor(InJson->GetArrayField(TEXT("placeholder")));
	}
	if (InJson->HasField(TEXT("sampler")))
	{
		const FJsonObject* samplerJsonObject = InJson->GetObjectField(TEXT("sampler")).Get();
		if (samplerJsonObject != nullptr)		{
			if (samplerJsonObject->HasField(TEXT("magFilter")))
			{
				ret->SamplerMagFilter = samplerJsonObject->GetStringField(TEXT("magFilter"));
			}
			if (samplerJsonObject->HasField(TEXT("minFilter")))
			{
				ret->SamplerMinFilter = samplerJsonObject->GetStringField(TEXT("minFilter"));
			}
			if (samplerJsonObject->HasField(TEXT("mipmapFilter")))
			{
				ret->SamplerMipmapFilter = samplerJsonObject->GetStringField(TEXT("mipmapFilter"));
			}
			if (samplerJsonObject->HasField(TEXT("wrap")))
			{
				TArray<TSharedPtr<FJsonValue>> wrapJsonArray = samplerJsonObject->GetArrayField(TEXT("wrap"));
				for (const TSharedPtr<FJsonValue>& wrapJsonValue : wrapJsonArray)
				{
					ret->SamplerWrapModes.Add(wrapJsonValue->AsString());
				}
			} else {
				// Default to repeat if no wrap modes are specified.
				ret->SamplerWrapModes.Add(TEXT("repeat"));
			}
		}
	}
	if (InJson->HasField(TEXT("size")))
	{
		TArray<TSharedPtr<FJsonValue>> sizeJsonArray = InJson->GetArrayField(TEXT("size"));
		for (const TSharedPtr<FJsonValue>& sizeJsonValue : sizeJsonArray)
		{
			int32 sizeInt;
			if (sizeJsonValue->TryGetNumber(sizeInt))
			{
				ret->TextureSize.Add(sizeInt);
			}
		}
	}
	return ret;
}

TSharedPtr<FJsonObject> UG4MFTexture4D::ToJson() const
{
	TSharedPtr<FJsonObject> json = WriteItemEntriesToJson();
	if (!ImageFileIndices.IsEmpty())
	{
		TArray<TSharedPtr<FJsonValue>> imageFileIndicesJsonArray;
		for (int32 imageFileIndexInt : ImageFileIndices)
		{
			imageFileIndicesJsonArray.Add(MakeShared<FJsonValueNumber>(imageFileIndexInt));
		}
		json->SetArrayField(TEXT("files"), imageFileIndicesJsonArray);
	}
	// Required property, always write.
	json->SetArrayField(TEXT("placeholder"), UG4MFItem4D::ColorToJsonArray(PlaceholderColor, false));
	{
		TSharedPtr<FJsonObject> samplerJsonObject = MakeShared<FJsonObject>();
		// Only write sampler properties if they are not empty and are not the default value.
		if (!SamplerMagFilter.IsEmpty() && SamplerMagFilter != TEXT("linear"))
		{
			samplerJsonObject->SetStringField(TEXT("magFilter"), SamplerMagFilter);
		}
		if (!SamplerMinFilter.IsEmpty() && SamplerMinFilter != TEXT("linear"))
		{
			samplerJsonObject->SetStringField(TEXT("minFilter"), SamplerMinFilter);
		}
		if (!SamplerMipmapFilter.IsEmpty() && SamplerMipmapFilter != TEXT("linear"))
		{
			samplerJsonObject->SetStringField(TEXT("mipmapFilter"), SamplerMipmapFilter);
		}
		if (!SamplerWrapModes.IsEmpty() && !(SamplerWrapModes.Num() == 1 && SamplerWrapModes[0] == TEXT("repeat")))
		{
			TArray<TSharedPtr<FJsonValue>> wrapJsonArray;
			for (const FString& wrapMode : SamplerWrapModes)
			{
				wrapJsonArray.Add(MakeShared<FJsonValueString>(wrapMode));
			}
			samplerJsonObject->SetArrayField(TEXT("wrap"), wrapJsonArray);
		}
		if (samplerJsonObject->Values.Num() > 0)
		{
			json->SetObjectField(TEXT("sampler"), samplerJsonObject);
		}
		else
		{
			samplerJsonObject.Reset();
		}
	}
	// Required property, always write.
	TArray<TSharedPtr<FJsonValue>> sizeJsonArray;
	for (int32 sizeInt : TextureSize)
	{
		sizeJsonArray.Add(MakeShared<FJsonValueNumber>(sizeInt));
	}
	json->SetArrayField(TEXT("size"), sizeJsonArray);
	return json;
}
