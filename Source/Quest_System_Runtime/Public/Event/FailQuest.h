// Copyright 2024 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Event/QuestEvent.h"
#include "FailQuest.generated.h"

/**
 * 
 */
UCLASS()
class QUEST_SYSTEM_RUNTIME_API UFailQuest : public UQuestEvent
{
	GENERATED_BODY()
public:
	UFailQuest();

	virtual void BeginEvent_Implementation() override;
};
