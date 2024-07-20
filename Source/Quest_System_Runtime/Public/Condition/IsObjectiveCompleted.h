// Copyright 2024 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Condition/QuestCondition.h"
#include "IsObjectiveCompleted.generated.h"

/**
 * 
 */
UCLASS()
class QUEST_SYSTEM_RUNTIME_API UIsObjectiveCompleted : public UQuestCondition
{
	GENERATED_BODY()
public:
	UIsObjectiveCompleted();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QuestCondition")
	FName NodeID;

	virtual bool IsConditionMet_Implementation() const override;
};
