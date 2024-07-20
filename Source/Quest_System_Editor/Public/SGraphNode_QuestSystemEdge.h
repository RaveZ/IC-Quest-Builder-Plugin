// Copyright 2024 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SGraphNode.h"
#include "Widgets/SCompoundWidget.h"
#include <EdNode_QuestSystemEdge.h>

/**
 * 
 */
class QUEST_SYSTEM_EDITOR_API SGraphNode_QuestSystemEdge : public SGraphNode
{
public:
	SLATE_BEGIN_ARGS(SGraphNode_QuestSystemEdge)
	{}
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs, UEdNode_QuestSystemEdge* InNode);
};
