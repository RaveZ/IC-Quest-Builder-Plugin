// Copyright 2024 Ivan Chandra. All Rights Reserved.


#include "Condition/IsQuestActive.h"
#include "QuestComponent.h"
#include "QuestSystem.h"

UIsQuestActive::UIsQuestActive()
{
    bUseQuestID = true;
}

bool UIsQuestActive::IsConditionMet_Implementation() const
{
    if (UQuestSystem* FoundedQuest = GetQuestComponent()->QuestSystemMap.FindRef(QuestID))
    {
        switch (FoundedQuest->QuestState)
        {
            case EQuestState::E_Active :
                return true;
            default:
                return false;
        }
    }
    return false;
}
