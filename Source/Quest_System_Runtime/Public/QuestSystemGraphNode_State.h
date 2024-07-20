// Copyright 2024 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "QuestSystemGraphNode.h"
#include "QuestSystem.h"
#include "QuestSystemGraphNode_State.generated.h"


UCLASS(/*HideCategories = ("Detail")*/)
class QUEST_SYSTEM_RUNTIME_API UQuestSystemGraphNode_State : public UQuestSystemGraphNode
{
	GENERATED_BODY()
public:

    UPROPERTY(BlueprintReadWrite, Category = "State")
        EQuestState QuestState;

    UFUNCTION(BlueprintCallable, Category = "State")
        void BeginState();

#if WITH_EDITOR
    virtual FText GetNodeTitle() const override;
    virtual void SetNodeTitle(const FText& NewTitle);
    virtual FText GetNodeDescription() const override;

    virtual FLinearColor GetBackgroundColor() const override;
#endif
};
