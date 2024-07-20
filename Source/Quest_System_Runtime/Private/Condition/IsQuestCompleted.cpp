// Copyright 2024 Ivan Chandra. All Rights Reserved.


#include "Condition/IsQuestCompleted.h"
#include "QuestComponent.h"
#include "QuestSystem.h"

UIsQuestCompleted::UIsQuestCompleted()
{
    bUseQuestID = true;
}

bool UIsQuestCompleted::IsConditionMet_Implementation() const
{
    if (UQuestSystem* FoundedQuest = GetQuestComponent()->QuestSystemMap.FindRef(QuestID))
    {
        switch (FoundedQuest->QuestState)
        {
        case EQuestState::E_Complete:
            return true;
        default:
            return false;
        }
    }
    return false;
}
