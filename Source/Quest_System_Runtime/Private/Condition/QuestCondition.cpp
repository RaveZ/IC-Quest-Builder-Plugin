// Copyright 2024 Ivan Chandra. All Rights Reserved.


#include "Condition/QuestCondition.h"
#include "UObject/UObjectIterator.h"
#include "QuestComponent.h"
#include "QuestSystem.h"
#include "QuestSystemGraph.h"

#define LOCTEXT_NAMESPACE "QuestCondition"

bool UQuestCondition::IsConditionMet_Implementation() const
{
	return true;
}

AController* UQuestCondition::GetOwningController() const
{
	if (!QuestGraph)
		return nullptr;

	return QuestGraph->OwningController;
}

UQuestComponent* UQuestCondition::GetQuestComponent() const
{
	if (!QuestGraph)
		return nullptr;

	return QuestGraph->QuestComponent;
}

UQuestSystemGraph* UQuestCondition::GetOwningQuestGraph() const
{
	if (!QuestGraph)
		return nullptr;

	return QuestGraph;
}

UQuestSystem* UQuestCondition::GetOwningQuestSystem() const
{
	return QuestSystem;
}

TArray<FName> UQuestCondition::GetAllQuestID()
{
	TArray<FName> QuestIDs;
	for (TObjectIterator<UQuestSystemGraph> It; It; ++It)
	{
		for (UQuestSystem* Quest : It->QuestSystems)
		{
			QuestIDs.Add(Quest->QuestID);
		}
	}

	return QuestIDs;
}

#undef LOCTEXT_NAMESPACE