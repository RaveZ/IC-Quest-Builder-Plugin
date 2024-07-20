// Copyright 2024 Ivan Chandra. All Rights Reserved.


#include "Event/RestartQuest.h"
#include "QuestSystem.h"
#include "QuestComponent.h"
#include "QuestSystemGraph.h"
#include "QuestSystemGraphNode.h"
#include "QuestSystemGraphNode_Objective.h"

URestartQuest::URestartQuest()
{
	bUseQuestID = true;
}

void URestartQuest::BeginEvent_Implementation()
{
	if (!QuestID.IsNone())
	{
		GetQuestComponent()->RestartQuest(QuestID, NodeID, bStartFromNodeID);
	}
}

