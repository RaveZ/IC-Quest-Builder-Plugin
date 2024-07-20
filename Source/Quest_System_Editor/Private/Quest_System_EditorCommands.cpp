// Copyright 2024 Ivan Chandra. All Rights Reserved.

#include "Quest_System_EditorCommands.h"

#define LOCTEXT_NAMESPACE "FQuest_System_EditorModule"

void FQuest_System_EditorCommands::RegisterCommands()
{
	UI_COMMAND(OpenPluginWindow, "Quest_System_Editor", "Bring up Quest_System_Editor window", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(NewTask, "Graph Settings", "Graph Settings", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(NewQuestCondition, "New Quest Condition", "New Quest Condition", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(NewQuestEvent, "New Quest Event", "New Quest Event", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(AddNewQuestGraph, "New Quest Graph", "New Quest Graph", EUserInterfaceActionType::Button, FInputChord(EModifierKey::Control, EKeys::Q));
	UI_COMMAND(QuestSetting, "Quest Setting", "Open Quest Setting", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
