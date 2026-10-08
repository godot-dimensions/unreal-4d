#include "EditorMesh4DComponentDetails.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"

void FEditorMesh4DComponentDetails::CustomizeDetails(IDetailLayoutBuilder& InDetailBuilder)
{
	IDetailCategoryBuilder& cat = InDetailBuilder.EditCategory("Mesh 4D");
	cat.SetSortOrder(-900);
}
