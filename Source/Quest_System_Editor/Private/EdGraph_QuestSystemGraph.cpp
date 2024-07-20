// Copyright 2024 Ivan Chandra. All Rights Reserved.


#include "EdGraph_QuestSystemGraph.h"
#include "EdNode_QuestSystemEdge.h"
#include "EdNode_QuestSystemRoot.h"
#include "EdNode_QuestSystemNode.h"
#include "QuestSystemGraphEdge.h"
#include "QuestSystemGraphNode.h"
#include "QuestSystemGraphNode_Objective.h"
#include "Condition/QuestCondition.h"
#include "Event/QuestEvent.h"
#include "Task/QuestTask.h"


UEdGraph_QuestSystemGraph::UEdGraph_QuestSystemGraph()
{
}

UEdGraph_QuestSystemGraph::~UEdGraph_QuestSystemGraph()
{
}

void UEdGraph_QuestSystemGraph::RebuildQuestSystemGraph()
{
	//LOG_INFO(TEXT("UQuestSystemGraphEdGraph::RebuildGenericGraph has been called"));

	UQuestSystemGraph* Graph = GetQuestSystemGraph();

	Clear();

	if (Graph->CategoryDisplayName.IsEmpty())
	{
		Graph->CategoryDisplayName = Graph->GetFName().ToString();
	}

	for (UQuestCondition* QuestCondition : QuestSystem->Conditions)
	{
		if (QuestCondition != nullptr)
		{
			QuestCondition->QuestGraph = Graph;
			QuestCondition->QuestSystem = QuestSystem;
		}
	}

	for (int i = 0; i < Nodes.Num(); ++i)
	{
		if (UEdNode_QuestSystemNode* EdNode = Cast<UEdNode_QuestSystemNode>(Nodes[i]))
		{
			if (EdNode->QuestSystemGraphNode == nullptr)
				continue;

			UQuestSystemGraphNode* QuestSystemGraphNode = EdNode->QuestSystemGraphNode;

			NodeMap.Add(QuestSystemGraphNode, EdNode);

			QuestSystem->AllNodes.Add(QuestSystemGraphNode);
			QuestSystem->NodeMap.Emplace(QuestSystemGraphNode->ID, QuestSystemGraphNode);

			for (int PinIdx = 0; PinIdx < EdNode->Pins.Num(); ++PinIdx)
			{
				UEdGraphPin* Pin = EdNode->Pins[PinIdx];

				if (Pin->Direction != EEdGraphPinDirection::EGPD_Output)
					continue;

				for (int LinkToIdx = 0; LinkToIdx < Pin->LinkedTo.Num(); ++LinkToIdx)
				{
					UQuestSystemGraphNode* ChildNode = nullptr;
					if (UEdNode_QuestSystemNode* EdNode_Child = Cast<UEdNode_QuestSystemNode>(Pin->LinkedTo[LinkToIdx]->GetOwningNode()))
					{
						ChildNode = EdNode_Child->QuestSystemGraphNode;
					}
					else if (UEdNode_QuestSystemEdge* EdNode_Edge = Cast<UEdNode_QuestSystemEdge>(Pin->LinkedTo[LinkToIdx]->GetOwningNode()))
					{
						UEdNode_QuestSystemNode* Child = EdNode_Edge->GetEndNode();
						if (Child != nullptr)
						{
							ChildNode = Child->QuestSystemGraphNode;
						}
					}

					if (ChildNode != nullptr)
					{
						QuestSystemGraphNode->ChildrenNodes.Add(ChildNode);

						ChildNode->ParentNodes.Add(QuestSystemGraphNode);
					}
					else
					{
						//LOG_ERROR(TEXT("UEdGraph_GenericGraph::RebuildGenericGraph can't find child node"));
					}
				}
			}

			if (UQuestSystemGraphNode_Objective* ObjectiveNode = Cast<UQuestSystemGraphNode_Objective>(QuestSystemGraphNode))
			{
				for (UQuestTask* Task : ObjectiveNode->Tasks)
				{
					if (Task != nullptr)
					{
						Task->ObjectiveNode = ObjectiveNode;
						Task->QuestGraph = Graph;
						Task->QuestSystem = QuestSystem;
					}
				}
				for (UQuestEvent* QuestEvent : ObjectiveNode->ObjectiveFailedEvents)
				{
					if (QuestEvent != nullptr)
					{
						QuestEvent->OwningNode = ObjectiveNode;
						QuestEvent->QuestGraph = Graph;
						QuestEvent->QuestSystem = QuestSystem;
					}
				}
			}

			for (UQuestCondition* QuestCondition : QuestSystemGraphNode->Conditions)
			{
				if (QuestCondition != nullptr)
				{
					QuestCondition->QuestGraph = Graph;
					QuestCondition->QuestSystem = QuestSystem;
				}
			}
			
			for (UQuestEvent* QuestEvent : QuestSystemGraphNode->Events)
			{
				if (QuestEvent != nullptr)
				{
					QuestEvent->OwningNode = QuestSystemGraphNode;
					QuestEvent->QuestGraph = Graph;
					QuestEvent->QuestSystem = QuestSystem;
				}
			}

		}
		else if (UEdNode_QuestSystemEdge* EdgeNode = Cast<UEdNode_QuestSystemEdge>(Nodes[i]))
		{
			UEdNode_QuestSystemNode* StartNode = EdgeNode->GetStartNode();
			UEdNode_QuestSystemNode* EndNode = EdgeNode->GetEndNode();
			UQuestSystemGraphEdge* Edge = EdgeNode->QuestSystemGraphEdge;

			if (StartNode == nullptr || EndNode == nullptr || Edge == nullptr)
			{
				//LOG_ERROR(TEXT("UEdGraph_GenericGraph::RebuildGenericGraph add edge failed."));
				continue;
			}

			EdgeMap.Add(Edge, EdgeNode);

			Edge->Graph = Graph;
			Edge->Rename(nullptr, Graph, REN_DontCreateRedirectors | REN_DoNotDirty);
			Edge->StartNode = StartNode->QuestSystemGraphNode;
			Edge->EndNode = EndNode->QuestSystemGraphNode;
			Edge->StartNode->Edges.Add(Edge->EndNode, Edge);
		}

		if (UEdNode_QuestSystemRoot* RootNode = Cast<UEdNode_QuestSystemRoot>(Nodes[i]))
		{
			if (RootNode->QuestSystemGraphNode == nullptr)
				continue;
			QuestSystem->RootNodes.Add(RootNode->QuestSystemGraphNode);
		}

	}

	for (int i = 0; i < QuestSystem->AllNodes.Num(); ++i)
	{
		UQuestSystemGraphNode* Node = QuestSystem->AllNodes[i];
		if (Node->ParentNodes.Num() == 0)
		{
			//use this line below if you want to automatically detect rootnodes based on their parent
			//Graph->RootNodes.Add(Node);
			SortNodes(Node);
		}

		Node->QuestGraph = Graph;
		Node->QuestSystem = QuestSystem;
		Node->Rename(nullptr, Graph, REN_DontCreateRedirectors | REN_DoNotDirty);
	}

	QuestSystem->RootNodes.Sort([&](const UQuestSystemGraphNode& L, const UQuestSystemGraphNode& R)
	{
		UEdNode_QuestSystemNode* EdNode_LNode = NodeMap[&L];
		UEdNode_QuestSystemNode* EdNode_RNode = NodeMap[&R];
		return EdNode_LNode->NodePosX < EdNode_RNode->NodePosX;
	});
}

UQuestSystemGraph* UEdGraph_QuestSystemGraph::GetQuestSystemGraph() const
{
	return CastChecked<UQuestSystemGraph>(GetOuter());
}

bool UEdGraph_QuestSystemGraph::Modify(bool bAlwaysMarkDirty)
{
	bool Rtn = Super::Modify(bAlwaysMarkDirty);

	GetQuestSystemGraph()->Modify();

	for (int32 i = 0; i < Nodes.Num(); ++i)
	{
		Nodes[i]->Modify();
	}

	return Rtn;
}

void UEdGraph_QuestSystemGraph::PostEditUndo()
{
	Super::PostEditUndo();

	NotifyGraphChanged();
	
}

void UEdGraph_QuestSystemGraph::Clear()
{
	QuestSystem->ClearGraph();
	QuestSystem->NodeMap.Reset();
	NodeMap.Reset();
	EdgeMap.Reset();

	for (int i = 0; i < Nodes.Num(); ++i)
	{
		if (UEdNode_QuestSystemNode* EdNode = Cast<UEdNode_QuestSystemNode>(Nodes[i]))
		{
			UQuestSystemGraphNode* QuestGraphNode = EdNode->QuestSystemGraphNode;
			if (QuestGraphNode)
			{
				QuestGraphNode->ParentNodes.Reset();
				QuestGraphNode->ChildrenNodes.Reset();
				QuestGraphNode->Edges.Reset();
			}
		}
	}
}

void UEdGraph_QuestSystemGraph::SortNodes(UQuestSystemGraphNode* RootNode)
{
	int Level = 0;
	TArray<UQuestSystemGraphNode*> CurrLevelNodes = { RootNode };
	TArray<UQuestSystemGraphNode*> NextLevelNodes;
	TSet<UQuestSystemGraphNode*> Visited;

	while (CurrLevelNodes.Num() != 0)
	{
		int32 LevelWidth = 0;
		for (int i = 0; i < CurrLevelNodes.Num(); ++i)
		{
			UQuestSystemGraphNode* Node = CurrLevelNodes[i];
			Visited.Add(Node);

			auto Comp = [&](const UQuestSystemGraphNode& L, const UQuestSystemGraphNode& R)
			{
				UEdNode_QuestSystemNode* EdNode_LNode = NodeMap[&L];
				UEdNode_QuestSystemNode* EdNode_RNode = NodeMap[&R];
				return EdNode_LNode->NodePosX < EdNode_RNode->NodePosX;
			};

			Node->ChildrenNodes.Sort(Comp);
			Node->ParentNodes.Sort(Comp);

			for (int j = 0; j < Node->ChildrenNodes.Num(); ++j)
			{
				UQuestSystemGraphNode* ChildNode = Node->ChildrenNodes[j];
				if (!Visited.Contains(ChildNode))
					NextLevelNodes.Add(Node->ChildrenNodes[j]);
			}
		}

		CurrLevelNodes = NextLevelNodes;
		NextLevelNodes.Reset();
		++Level;
	}
}
