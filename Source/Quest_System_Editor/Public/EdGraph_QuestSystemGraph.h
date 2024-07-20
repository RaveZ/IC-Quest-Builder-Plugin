// Copyright 2024 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraph.h"
#include "QuestSystem.h"
#include "EdGraph_QuestSystemGraph.generated.h"

class UQuestSystemGraph;
class UQuestSystemGraphNode;
class UQuestSystemGraphEdge;
class UEdNode_QuestSystemNode;
class UEdNode_QuestSystemEdge;

UCLASS()
class QUEST_SYSTEM_EDITOR_API UEdGraph_QuestSystemGraph : public UEdGraph
{
	GENERATED_BODY()
public:
	UEdGraph_QuestSystemGraph();
	virtual ~UEdGraph_QuestSystemGraph();

	virtual void RebuildQuestSystemGraph();

	UQuestSystemGraph* GetQuestSystemGraph() const;

	virtual bool Modify(bool bAlwaysMarkDirty = true) override;
	virtual void PostEditUndo() override;

	/** QuestSystem linked with this UEdGraph */
	UPROPERTY(VisibleAnywhere, Instanced, Category = "QuestSystemGraph")
		UQuestSystem* QuestSystem;

	UPROPERTY(Transient)
		TMap<UQuestSystemGraphNode*, UEdNode_QuestSystemNode*> NodeMap;

	UPROPERTY(Transient)
		TMap<UQuestSystemGraphEdge*, UEdNode_QuestSystemEdge*> EdgeMap;


	void Clear();

protected:

	void SortNodes(UQuestSystemGraphNode* RootNode);
};
