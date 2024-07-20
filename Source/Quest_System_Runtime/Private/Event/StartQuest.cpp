// Copyright 2024 Ivan Chandra. All Rights Reserved.


#include "Event/StartQuest.h"
#include "QuestSystem.h"
#include "QuestComponent.h"
#include "QuestSystemGraph.h"
#include "QuestSystemGraphNode.h"

UStartQuest::UStartQuest()
{
	bUseQuestID = true;
}

void UStartQuest::BeginEvent_Implementation()
{
	if (!QuestID.IsNone())
	{
		GetQuestComponent()->ActivateQuest(QuestID);
	}
}
