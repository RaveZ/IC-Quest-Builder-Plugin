// Copyright 2024 Ivan Chandra. All Rights Reserved.

#include "Quest_System_Editor.h"
#include "Quest_System_EditorStyle.h"
#include "Quest_System_EditorCommands.h"
#include "AssetTypeActions_QuestSystem.h"
#include "QuestSystemGraphNode.h"
#include "Task/QuestTask.h"
#include "IAssetTools.h"
#include "LevelEditor.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "ToolMenus.h"
#include "GraphNodeFactory.h"
#include <IAssetTools.h>

static const FName Quest_System_EditorTabName("Quest_System_Editor");

#define LOCTEXT_NAMESPACE "FQuest_System_EditorModule"

void FQuest_System_EditorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FQuest_System_EditorStyle::Initialize();
	FQuest_System_EditorStyle::ReloadTextures();

	FQuest_System_EditorCommands::Register();
	
	GraphPanelNodeFactory_QuestEditor = MakeShareable(new FQuestSystemNodeFactory());
	FEdGraphUtilities::RegisterVisualNodeFactory(GraphPanelNodeFactory_QuestEditor);
	

	//register asset type to advanced asset(may be able to create new asset in content drawer)
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

	QuestEditorAssetCategoryBit = AssetTools.RegisterAdvancedAssetCategory(FName(TEXT("Quest Builder")), LOCTEXT("QuestBuilderAssetCategory", "Quest Builder"));

	RegisterAssetTypeAction(AssetTools, MakeShareable(new FAssetTypeActions_QuestSystem(QuestEditorAssetCategoryBit)));

	
}

void FQuest_System_EditorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		IAssetTools& AssetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();
		for (int32 Index = 0; Index < CreatedAssetTypeActions.Num(); ++Index)
		{
			AssetTools.UnregisterAssetTypeActions(CreatedAssetTypeActions[Index].ToSharedRef());
		}
	}
	if (GraphPanelNodeFactory_QuestEditor.IsValid())
	{
		FEdGraphUtilities::UnregisterVisualNodeFactory(GraphPanelNodeFactory_QuestEditor);
		GraphPanelNodeFactory_QuestEditor.Reset();
	}

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FQuest_System_EditorStyle::Shutdown();

	FQuest_System_EditorCommands::Unregister();

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(Quest_System_EditorTabName);

	ClassCache.Reset();
}

void FQuest_System_EditorModule::RegisterAssetTypeAction(IAssetTools& AssetTools, TSharedRef<IAssetTypeActions> Action)
{
	AssetTools.RegisterAssetTypeActions(Action);
	CreatedAssetTypeActions.Add(Action);
}

TSharedRef<SDockTab> FQuest_System_EditorModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
		//Load Property Module
		FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

		//FDetailsViewArgs is a struct of settings to customize our Details View Widget
		FDetailsViewArgs Args;
		Args.bHideSelectionTip = true;

	// Return the property widget as the content of the tab
		FText WidgetText = FText::Format(
			LOCTEXT("WindowWidgetText", "Add code to {0} in {1} to override this window's contents"),
			FText::FromString(TEXT("FQuest_System_EditorModule::OnSpawnPluginTab")),
			FText::FromString(TEXT("Quest_System_Editor.cpp"))
			);

		return SNew(SDockTab)
			.TabRole(ETabRole::NomadTab)
			[
				// Put your tab content here!
				SNew(SBox)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(WidgetText)
				]
			];
}

void FQuest_System_EditorModule::PluginButtonClicked()
{
	FGlobalTabmanager::Get()->TryInvokeTab(Quest_System_EditorTabName);
}

void FQuest_System_EditorModule::CheckClassCache()
{
	if (!ClassCache.IsValid())
	{
		ClassCache = MakeShareable(new FGraphNodeClassHelper(UQuestTask::StaticClass()));
		FGraphNodeClassHelper::AddObservedBlueprintClasses(UQuestTask::StaticClass());
		ClassCache->UpdateAvailableBlueprintClasses();
	}
}

void FQuest_System_EditorModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(FQuest_System_EditorCommands::Get().OpenPluginWindow, PluginCommands);
		}
	}

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("Settings");
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FQuest_System_EditorCommands::Get().OpenPluginWindow));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FQuest_System_EditorModule, Quest_System_Editor)

