// Copyright 2024 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Condition/QuestCondition.h"
#include "IsQuestCompleted.generated.h"


class UQuestSystem;

UCLASS()
class QUEST_SYSTEM_RUNTIME_API UIsQuestCompleted : public UQuestCondition
{
	GENERATED_BODY()

public:
	UIsQuestCompleted();


	virtual bool IsConditionMet_Implementation() const override;
};
