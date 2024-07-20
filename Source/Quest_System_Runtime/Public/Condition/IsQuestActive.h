// Copyright 2024 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Condition/QuestCondition.h"
#include "IsQuestActive.generated.h"

/**
 * 
 */
UCLASS()
class QUEST_SYSTEM_RUNTIME_API UIsQuestActive : public UQuestCondition
{
	GENERATED_BODY()

public:
	UIsQuestActive();

	virtual bool IsConditionMet_Implementation() const override;
};
