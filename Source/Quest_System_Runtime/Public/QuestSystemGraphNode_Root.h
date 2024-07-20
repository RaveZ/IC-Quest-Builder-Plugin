// Copyright 2024 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "QuestSystemGraphNode.h"
#include "UObject/NoExportTypes.h"
#include "QuestSystemGraphNode_Root.generated.h"



UCLASS(HideCategories = ("Events", "Conditions"))
class QUEST_SYSTEM_RUNTIME_API UQuestSystemGraphNode_Root : public UQuestSystemGraphNode
{
	GENERATED_BODY()

public:

#if WITH_EDITOR
	virtual FText GetNodeDescription() const override;
#endif
	
};
