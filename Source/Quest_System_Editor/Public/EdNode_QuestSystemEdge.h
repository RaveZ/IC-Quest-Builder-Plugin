// Copyright 2024 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include <QuestSystemGraph.h>
#include "EdNode_QuestSystemEdge.generated.h"


class UQuestSystemGraphNode;
class UQuestSystemGraphEdge;
class UEdNode_QuestSystemNode;

UCLASS()
class QUEST_SYSTEM_EDITOR_API UEdNode_QuestSystemEdge : public UEdGraphNode
{
	GENERATED_BODY()
public:
	UEdNode_QuestSystemEdge();

	UPROPERTY()
		class UEdGraph* Graph;

	UPROPERTY(VisibleAnywhere, Instanced, Category = "QuestSystemGraph")
		UQuestSystemGraphEdge* QuestSystemGraphEdge;

	void SetEdge(UQuestSystemGraphEdge* Edge);

	virtual void AllocateDefaultPins() override;

	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;

	virtual void PinConnectionListChanged(UEdGraphPin* Pin) override;

	virtual void PrepareForCopying() override;

	virtual UEdGraphPin* GetInputPin() const { return Pins[0]; }
	virtual UEdGraphPin* GetOutputPin() const { return Pins[1]; }

	void CreateConnections(UEdNode_QuestSystemNode* Start, UEdNode_QuestSystemNode* End);

	UEdNode_QuestSystemNode* GetStartNode();
	UEdNode_QuestSystemNode* GetEndNode();
};
