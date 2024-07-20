// Copyright 2024 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EdNode_QuestSystemNode.h"
#include "EdGraph/EdGraphNode.h"
#include "EdNode_QuestSystemState.generated.h"

/**
 * 
 */
UCLASS()
class QUEST_SYSTEM_EDITOR_API UEdNode_QuestSystemState : public UEdNode_QuestSystemNode
{
	GENERATED_BODY()
public:
	UEdNode_QuestSystemState();
	//virtual void PostPlacedNewNode() override;
	virtual void AllocateDefaultPins() override;
	//virtual bool HasErrors() const override { return false; }
	//virtual bool RefreshNodeClass() override { return false; }
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	// gets icon resource name for title bar 
	//virtual FName GetNameIcon() const override;
	//virtual FText GetTooltipText() const override;

	/*virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostEditUndo() override;
	virtual FText GetDescription() const override;*/
};
