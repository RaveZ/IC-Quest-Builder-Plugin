// Copyright 2024 Ivan Chandra. All Rights Reserved.


#include "AssetEditorToolbar_QuestSystem.h"
#include "Quest_System_EditorCommands.h"
#include "Quest_System_EditorStyle.h"
#include "AssetEditor_QuestSystem.h"

#define LOCTEXT_NAMESPACE "AssetEditorToolbar_QuestSystem"
void FAssetEditorToolbar_QuestSystem::AddQuestSystemToolbar(TSharedPtr<FExtender> Extender)
{
	check(QuestSystemEditor.IsValid());
	TSharedPtr<FAssetEditor_QuestSystem> QuestSystemEditorPtr = QuestSystemEditor.Pin();
	
	TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);
	ToolbarExtender->AddToolBarExtension("Asset", EExtensionHook::After, QuestSystemEditorPtr->GetToolkitCommands(), FToolBarExtensionDelegate::CreateSP(this, &FAssetEditorToolbar_QuestSystem::FillQuestSystemToolbar));
	QuestSystemEditorPtr->AddToolbarExtender(ToolbarExtender);
}

void FAssetEditorToolbar_QuestSystem::FillQuestSystemToolbar(FToolBarBuilder& ToolbarBuilder)
{
	check(QuestSystemEditor.IsValid());
	TSharedPtr<FAssetEditor_QuestSystem> QuestSystemEditorPtr = QuestSystemEditor.Pin();

	ToolbarBuilder.BeginSection("Quest System");
	{
		const FText NewTaskLabel = LOCTEXT("NewTask_Label", "New Task");
		const FText NewTaskTooltip = LOCTEXT("NewTask_ToolTip", "Create a new task node Blueprint from a base class");
		const FSlateIcon NewTaskIcon = FSlateIcon(FAppStyle::GetAppStyleSetName(), "BTEditor.Graph.NewTask");

		
		ToolbarBuilder.AddToolBarButton(FQuest_System_EditorCommands::Get().NewTask,
			NAME_None,
			NewTaskLabel,
			NewTaskTooltip,
			NewTaskIcon
		);
	}
	ToolbarBuilder.EndSection();

	ToolbarBuilder.BeginSection("Quest System");
	{
		const FText NewConditionLabel = LOCTEXT("NewQuestCondition_Label", "New Quest Condition");
		const FText NewConditionTooltip = LOCTEXT("NewQuestCondition_ToolTip", "Create a new Quest Condition Blueprint from a base class");
		const FSlateIcon NewConditionIcon = FSlateIcon(FAppStyle::GetAppStyleSetName(), "BTEditor.Graph.BTNode.Decorator.Conditional.Icon");


		ToolbarBuilder.AddToolBarButton(FQuest_System_EditorCommands::Get().NewQuestCondition,
			NAME_None,
			NewConditionLabel,
			NewConditionTooltip,
			NewConditionIcon
		);
	}
	ToolbarBuilder.EndSection();

	ToolbarBuilder.BeginSection("Quest System");
	{
		const FText NewEventLabel = LOCTEXT("NewQuestEvent_Label", "New Quest Event");
		const FText NewEventTooltip = LOCTEXT("NewQuestEvent_ToolTip", "Create a new Quest Event Blueprint from a base class");
		const FSlateIcon NewEventIcon = FSlateIcon(FAppStyle::GetAppStyleSetName(), "GraphEditor.CustomEvent_16x");


		ToolbarBuilder.AddToolBarButton(FQuest_System_EditorCommands::Get().NewQuestEvent,
			NAME_None,
			NewEventLabel,
			NewEventTooltip,
			NewEventIcon
		);
	}
	ToolbarBuilder.EndSection();

	ToolbarBuilder.BeginSection("Quest System");
	{
		const FText QuestSettingLabel = LOCTEXT("QuestSetting_Label", "Quest Builder Setting");
		const FText QuestSettingTooltip = LOCTEXT("QuestSetting_ToolTip", "Access Quest Builder Setting");
		const FSlateIcon QuestSettingIcon = FSlateIcon(FAppStyle::GetAppStyleSetName(), "ProjectSettings.TabIcon");


		ToolbarBuilder.AddToolBarButton(FQuest_System_EditorCommands::Get().QuestSetting,
			NAME_None,
			QuestSettingLabel,
			QuestSettingTooltip,
			QuestSettingIcon
		);
	}
	ToolbarBuilder.EndSection();

	
}



#undef LOCTEXT_NAMESPACE
