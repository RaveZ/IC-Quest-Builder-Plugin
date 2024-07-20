// Copyright 2024 Ivan Chandra. All Rights Reserved.


#include "AssetTypeActions_QuestSystem.h"
#include "QuestSystemGraph.h"
#include "AssetEditor_QuestSystem.h"


#define LOCTEXT_NAMESPACE "AssetTypeActions_QuestEditor"
FAssetTypeActions_QuestSystem::FAssetTypeActions_QuestSystem(EAssetTypeCategories::Type InAssetCategory)
    : MyAssetCategory(InAssetCategory)
{
}

FText FAssetTypeActions_QuestSystem::GetName() const
{
    return LOCTEXT("FQuestSystemAssetTypeActionsName", "Quest Builder Editor");
}

FColor FAssetTypeActions_QuestSystem::GetTypeColor() const
{
    return FColor(129, 196, 115);
}

UClass* FAssetTypeActions_QuestSystem::GetSupportedClass() const
{
    return UQuestSystemGraph::StaticClass();
}

void FAssetTypeActions_QuestSystem::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor)
{
	const EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;

	for (auto ObjIt = InObjects.CreateConstIterator(); ObjIt; ++ObjIt)
	{
		if (UQuestSystemGraph* Graph = Cast<UQuestSystemGraph>(*ObjIt))
		{
			FQuest_System_EditorModule& QuestSystemEditorModule = FModuleManager::GetModuleChecked<FQuest_System_EditorModule>("Quest_System_Editor");
			QuestSystemEditorModule.CheckClassCache();

			TSharedRef<FAssetEditor_QuestSystem> NewGraphEditor(new FAssetEditor_QuestSystem());
			NewGraphEditor->InitQuestSystemGraphAssetEditor(Mode, EditWithinLevelEditor, Graph);
		}
	}
}

uint32 FAssetTypeActions_QuestSystem::GetCategories()
{
    return MyAssetCategory;
}

//////////////////////////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE