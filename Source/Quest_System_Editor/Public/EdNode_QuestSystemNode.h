// Copyright 2024 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraphNode.h"
#include "AIGraphTypes.h"
#include "UObject/ObjectMacros.h"
#include "UObject/NoExportTypes.h"
#include "EdGraph_QuestSystemGraph.h"
#include "QuestSystemGraph.h"
#include "EdNode_QuestSystemNode.generated.h"

class UEdNode_QuestSystemEdge;
class UEdGraph_QuestSystemGraph;
class SGraphNode_QuestSystemNode;

UCLASS()
class QUEST_SYSTEM_EDITOR_API UEdNode_QuestSystemNode : public UEdGraphNode
{
	GENERATED_BODY()
public:
	UEdNode_QuestSystemNode();
	virtual ~UEdNode_QuestSystemNode();

	/** instance class */
	UPROPERTY()
	struct FGraphNodeClassData ClassData;

	UPROPERTY(VisibleAnywhere, Instanced, Category = "QuestSystemGraph")
		UQuestSystemGraphNode* QuestSystemGraphNode;
	
	void SetQuestSystemGraphNode(UQuestSystemGraphNode* InNode);
	UEdGraph_QuestSystemGraph* GetQuestSystemGraphEdGraph();

	SGraphNode_QuestSystemNode* SGraphNode;

	//UedGraphnode interface
	virtual void AllocateDefaultPins() override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual bool CanDuplicateNode() const override;
	virtual bool CanUserDeleteNode() const override;
	virtual void PrepareForCopying() override;
	virtual void AutowireNewNode(UEdGraphPin* FromPin) override;
	virtual FText GetDescription() const;

	virtual FLinearColor GetBackgroundColor() const;
	virtual UEdGraphPin* GetInputPin() const;
	virtual UEdGraphPin* GetOutputPin() const;

	/** Check if node instance uses blueprint for its implementation */
	bool UsesBlueprint() const;

#if WITH_EDITOR
	virtual void PostEditUndo() override;
#endif

	/** if set, all modifications (including delete/cut) are disabled */
	UPROPERTY()
	uint32 bIsReadOnly : 1;
};
