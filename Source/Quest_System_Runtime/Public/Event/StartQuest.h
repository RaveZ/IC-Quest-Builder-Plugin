// Copyright 2024 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Event/QuestEvent.h"
#include "StartQuest.generated.h"

/**
 * 
 */
UCLASS()
class QUEST_SYSTEM_RUNTIME_API UStartQuest : public UQuestEvent
{
	GENERATED_BODY()
public:
	UStartQuest();

	/** Start Quest From Specific Node ID. if Node ID Not Found Quest Will Start From Root Node*/
	UPROPERTY(EditAnywhere, Category = "QuestEvent", meta = (DisplayName = "StartFromNodeID?"))
	bool bStartFromNodeID = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QuestEvent", meta = (EditCondition = "bStartFromNodeID == true", EditConditionHides))
	FName NodeID;

	virtual void BeginEvent_Implementation() override;
};
