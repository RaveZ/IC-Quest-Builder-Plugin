// Copyright 2024 Ivan Chandra. All Rights Reserved.


#include "EdNode_QuestSystemRoot.h"

UEdNode_QuestSystemRoot::UEdNode_QuestSystemRoot()
{
	bIsReadOnly = true;
	bCanRenameNode = false;
}

void UEdNode_QuestSystemRoot::AllocateDefaultPins()
{
	CreatePin(EGPD_Output, "MultipleNode", TEXT("In"));
}

FText UEdNode_QuestSystemRoot::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return NSLOCTEXT("BehaviorTreeEditor", "Root", "ROOT");
	
}
