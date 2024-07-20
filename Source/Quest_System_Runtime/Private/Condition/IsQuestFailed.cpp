// Copyright 2024 Ivan Chandra. All Rights Reserved.


#include "Condition/IsQuestFailed.h"
#include "QuestComponent.h"
#include "QuestSystem.h"

UIsQuestFailed::UIsQuestFailed()
{
    bUseQuestID = true;
}

bool UIsQuestFailed::IsConditionMet_Implementation() const
{
    if (UQuestSystem* FoundedQuest = GetQuestComponent()->QuestSystemMap.FindRef(QuestID))
    {
        switch (FoundedQuest->QuestState)
        {
        case EQuestState::E_Fail:
            return true;
        default:
            return false;
        }
    }
    return false;
}
