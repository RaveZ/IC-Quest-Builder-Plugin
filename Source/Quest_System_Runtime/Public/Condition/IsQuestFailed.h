// Copyright 2024 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Condition/QuestCondition.h"
#include "IsQuestFailed.generated.h"

/**
 * 
 */
UCLASS()
class QUEST_SYSTEM_RUNTIME_API UIsQuestFailed : public UQuestCondition
{
	GENERATED_BODY()

public:
	UIsQuestFailed();


	virtual bool IsConditionMet_Implementation() const override;
};
