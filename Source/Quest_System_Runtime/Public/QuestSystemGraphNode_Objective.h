// Copyright 2024 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "QuestSystemGraphNode.h"
#include "QuestSystemGraphNode_Objective.generated.h"

class UQuestTask;

UCLASS(Blueprintable)
class QUEST_SYSTEM_RUNTIME_API UQuestSystemGraphNode_Objective : public UQuestSystemGraphNode
{
	GENERATED_BODY()

public:
	/**List of task for your objective*/
	UPROPERTY(Instanced, EditAnywhere, BlueprintReadOnly, Category = "Objective")
		TArray<UQuestTask*> Tasks;
	

	/**This events will be triggered if you failed the objective node. Ignoring The EventRunType*/
	UPROPERTY(Instanced, EditAnywhere, BlueprintReadOnly, Category = "Events", meta = (DisplayName = "Events (If Objective Failed)"))
		TArray<UQuestEvent*> ObjectiveFailedEvents;

	UFUNCTION(BlueprintCallable, Category = "Objective")
		bool IsAllTaskCompleted();


#if WITH_EDITOR
	virtual FText GetNodeTitle() const override;
	virtual void SetNodeTitle(const FText& NewTitle);
	virtual FText GetNodeDescription() const override;
#endif
};
