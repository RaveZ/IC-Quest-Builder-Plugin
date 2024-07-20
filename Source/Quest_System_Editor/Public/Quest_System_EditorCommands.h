// Copyright 2024 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "Quest_System_EditorStyle.h"

class FQuest_System_EditorCommands : public TCommands<FQuest_System_EditorCommands>
{
public:

	FQuest_System_EditorCommands()
		: TCommands<FQuest_System_EditorCommands>(TEXT("Quest_System_Editor"), NSLOCTEXT("Contexts", "Quest_System_Editor", "Quest_System_Editor Plugin"), NAME_None, FQuest_System_EditorStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > OpenPluginWindow;

	// New documents
	TSharedPtr< FUICommandInfo > AddNewQuestGraph;
	
	//Toolbar
	TSharedPtr<FUICommandInfo> NewTask;
	TSharedPtr<FUICommandInfo> NewQuestCondition;
	TSharedPtr<FUICommandInfo> NewQuestEvent;
	TSharedPtr<FUICommandInfo> QuestSetting;
};