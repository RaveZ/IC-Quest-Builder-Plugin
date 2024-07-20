// Copyright 2024 Ivan Chandra. All Rights Reserved.

#include "QuestSystemNodeFactory.h"
#include <EdGraph/EdGraphNode.h>
#include "SGraphNode_QuestSystemEdge.h"
#include "SGraphNode_QuestSystemNode.h"
#include "EdNode_QuestSystemNode.h"
#include "EdNode_QuestSystemEdge.h"


TSharedPtr<class SGraphNode> FQuestSystemNodeFactory::CreateNode(UEdGraphNode* Node) const
{
	if (UEdNode_QuestSystemNode* EdNode_GraphNode = Cast<UEdNode_QuestSystemNode>(Node))
	{
		return SNew(SGraphNode_QuestSystemNode, EdNode_GraphNode);
	}
	return nullptr;
}
