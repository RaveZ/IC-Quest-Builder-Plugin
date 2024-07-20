// Copyright 2024 Ivan Chandra. All Rights Reserved.


#include "QuestSystemGraph.h"
#include "QuestSystem.h"
#include "QuestComponent.h"
#include "QuestSystemGraphNode.h"
#include "QuestSystemGraphEdge.h"

#define LOCTEXT_NAMESPACE "QuestSystemGraph"

UQuestSystemGraph::UQuestSystemGraph()
{
	bEdgeEnabled = false;
#if WITH_EDITORONLY_DATA

	bCanRenameNode = true;
#endif
}

UQuestSystemGraph::~UQuestSystemGraph()
{
	
}



#if WITH_EDITORONLY_DATA
void UQuestSystemGraph::GetAllGraphs(TArray<UEdGraph*>& Graphs) const
{
	

	for (int32 i = 0; i < QuestGraphPages.Num(); ++i)
	{
		UEdGraph* Graph = QuestGraphPages[i];
		if (Graph)
		{
			Graphs.Add(Graph);
			Graph->GetAllChildrenGraphs(Graphs);
		}
	}
	
}
#endif // WITH_EDITORONLY_DATA

#undef LOCTEXT_NAMESPACE