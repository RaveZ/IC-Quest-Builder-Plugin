// Copyright 2024 Ivan Chandra. All Rights Reserved.


#include "Event/FailQuest.h"
#include "QuestSystem.h"
#include "QuestComponent.h"
#include "QuestSystemGraph.h"
#include "QuestSystemGraphNode.h"

UFailQuest::UFailQuest()
{
	bUseQuestID = true;
}

void UFailQuest::BeginEvent_Implementation()
{
	if (!QuestID.IsNone())
	{
		GetQuestComponent()->FailQuest(QuestID);
	}
	
}
