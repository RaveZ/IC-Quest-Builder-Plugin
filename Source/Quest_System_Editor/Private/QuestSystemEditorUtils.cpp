// Copyright 2024 Ivan Chandra. All Rights Reserved.

#include "QuestSystemEditorUtils.h"
#include "QuestSystemGraphNode.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "QuestSystemGraph.h"


#define LOCTEXT_NAMESPACE "QuestSystemEditorUtils"

FQuestSystemEditorUtils::FQuestSystemEditorUtils()
{
}

FQuestSystemEditorUtils::~FQuestSystemEditorUtils()
{
}

FName FQuestSystemEditorUtils::FindUniqueQuestName(const FString& InBaseName)
{
	// Start with the base name
	FString UniqueName = InBaseName;

	// Iterate through the quest graph pages until we find a unique name
	for (int i = 0;; i++)
	{
		bool bIsUnique = true;

		// Check if the current name already exists in the quest graph pages
		for (TObjectIterator<UQuestSystem> It; It; ++It)
		{
			if (UQuestSystem* QuestSystem = Cast<UQuestSystem>(*It))
			{
				FString FullName = QuestSystem->QuestID.ToString();

				// If the current name matches a quest graph page name, it's not unique
				if (FullName == UniqueName)
				{
					bIsUnique = false;
					break;
				}
			}
		}

		// If the current name is unique, return it
		if (bIsUnique)
		{
			return FName(*UniqueName);
		}

		// If the current name is not unique, append a number to it and try again
		UniqueName = FString::Printf(TEXT("%s_%d"), *InBaseName, i + 1);
	}
}

FName FQuestSystemEditorUtils::FindUniqueNodeName(const FString& InBaseName)
{
	// Start with the base name
	FString UniqueName = InBaseName;// Iterate through the quest graph pages until we find a unique name
	
	for (int i = 0;; i++)
	{
		bool bIsUnique = true;

		// Check if the current name already exists in the quest graph pages
		for (TObjectIterator<UQuestSystemGraphNode> It; It; ++It)
		{
			if (UQuestSystemGraphNode* Node = Cast<UQuestSystemGraphNode>(*It))
			{
				FString NodeID = Node->ID.ToString();

				// If the current name matches a quest graph page name, it's not unique
				if (NodeID == UniqueName)
				{
					bIsUnique = false;
					break;
				}
			}
		}

		// If the current name is unique, return it
		if (bIsUnique)
		{
			return FName(*UniqueName);
		}

		// If the current name is not unique, append a number to it and try again
		UniqueName = FString::Printf(TEXT("%s_%d"), *InBaseName, i + 1);
	}
}

UEdGraph* FQuestSystemEditorUtils::CreateNewGraph(UObject* ParentScope, const FName& GraphName, TSubclassOf<class UEdGraph> GraphClass, TSubclassOf<class UEdGraphSchema> SchemaClass)
{
	UEdGraph* NewGraph = nullptr;

	
	// Construct a new graph with a default name
	NewGraph = NewObject<UEdGraph>(ParentScope, GraphClass, NAME_None, RF_Transactional);
	
	NewGraph->Schema = SchemaClass;
	NewGraph->GetSchema()->CreateDefaultNodesForGraph(*NewGraph);
	NewGraph->Rename(*(GraphName.ToString()), ParentScope, REN_DoNotDirty | REN_ForceNoResetLoaders);

	if (UEdGraph_QuestSystemGraph* QuestGraph = Cast<UEdGraph_QuestSystemGraph>(NewGraph))
	{
		QuestGraph->QuestSystem = NewObject<UQuestSystem>(ParentScope, UQuestSystem::StaticClass(), NAME_None, RF_Transactional);
		QuestGraph->QuestSystem->QuestGraph = QuestGraph->GetQuestSystemGraph();
		QuestGraph->QuestSystem->QuestID = GraphName;
		if (UQuestSystemGraph* QuestSystemGraph = Cast<UQuestSystemGraph>(QuestGraph->GetQuestSystemGraph()))
		{
			QuestSystemGraph->QuestSystems.Add(QuestGraph->QuestSystem);
		}

	}

	
	return NewGraph;
}



void FQuestSystemEditorUtils::RemoveNode(UQuestSystemGraph* QuestSystemGraph, UEdNode_QuestSystemNode* Node, bool bDontRecompile)
{
	check(Node);

	const UEdGraphSchema* Schema = nullptr;

	// Ensure we mark parent graph modified
	if (UEdGraph* GraphObj = Node->GetGraph())
	{
		GraphObj->Modify();
		Schema = GraphObj->GetSchema();
	}

	Node->Modify();

	// Timelines will be removed from the blueprint if the node is a UK2Node_Timeline
	if (Schema)
	{
		Schema->BreakNodeLinks(*Node);
	}

	Node->DestroyNode();

	/*if (!bDontRecompile && (Blueprint != nullptr))
	{
		FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);
	}*/
}

void FQuestSystemEditorUtils::RemoveGraph(UQuestSystemGraph* QuestSystemGraph, UEdGraph* GraphToRemove)
{
	GraphToRemove->Modify();

	for (UObject* TestOuter = GraphToRemove->GetOuter(); TestOuter; TestOuter = TestOuter->GetOuter())
	{
		if (TestOuter == QuestSystemGraph)
		{
			QuestSystemGraph->QuestGraphPages.Remove(GraphToRemove);
			if (UEdGraph_QuestSystemGraph* QuestGraphToRemove = Cast<UEdGraph_QuestSystemGraph>(GraphToRemove))
			{
				QuestSystemGraph->QuestSystems.Remove(QuestGraphToRemove->QuestSystem);
			}

			// Can't just call Remove, the object is wrapped in a struct
			for (int EditedDocIdx = 0; EditedDocIdx < QuestSystemGraph->LastEditedDocuments.Num(); ++EditedDocIdx)
			{
				if (QuestSystemGraph->LastEditedDocuments[EditedDocIdx].EditedObjectPath.ResolveObject() == GraphToRemove)
				{
					QuestSystemGraph->LastEditedDocuments.RemoveAt(EditedDocIdx);
					break;
				}
			}
		}
		else if (UEdGraph* OuterGraph = Cast<UEdGraph>(TestOuter))
		{
			// remove ourselves
			OuterGraph->Modify();
			OuterGraph->SubGraphs.Remove(GraphToRemove);
		}
		else if (!(Cast<UEdGraphNode>(TestOuter) && Cast<UEdGraphNode>(TestOuter)->GetSubGraphs().Num() > 0))
		{
			break;
		}
	}

	

	// Handle subgraphs held in graph
	TArray<UEdGraphNode*> AllNodes;
	GraphToRemove->GetNodesOfClass<UEdGraphNode>(AllNodes);

	for (UEdGraphNode* GraphNode : AllNodes)
	{
		for (UEdGraph* SubGraph : GraphNode->GetSubGraphs())
		{
			if (SubGraph && SubGraph->GetOuter()->IsA(UEdGraphNode::StaticClass()))
			{
				FQuestSystemEditorUtils::RemoveGraph(QuestSystemGraph, SubGraph);
			}
		}
	}

	GraphToRemove->GetSchema()->HandleGraphBeingDeleted(*GraphToRemove);

	GraphToRemove->Rename(nullptr, QuestSystemGraph ? QuestSystemGraph->GetOuter() : nullptr, REN_DoNotDirty | REN_DontCreateRedirectors);
	GraphToRemove->ClearFlags(RF_Standalone | RF_Public);
	GraphToRemove->RemoveFromRoot();

	
}

void FQuestSystemEditorUtils::AddQuestGraphPage(UQuestSystemGraph* QuestSystemGraph, UEdGraph* Graph)
{
#if WITH_EDITORONLY_DATA
	QuestSystemGraph->QuestGraphPages.Add(Graph);
	Graph->MarkPackageDirty();
#endif	//#if WITH_EDITORONLY_DATA
}


UEdGraph* FQuestSystemEditorUtils::FindQuestGraph(const UQuestSystemGraph* QuestSystemGraph)
{

	for (UEdGraph* CurrentGraph : QuestSystemGraph->QuestGraphPages)
	{
		if (CurrentGraph->GetFName().ToString().Contains(TEXT("QuestGraph")))
		{
			UE_LOG(LogTemp, Warning, TEXT("Current graph name: %s"), *CurrentGraph->GetFName().ToString());
			return CurrentGraph;
		}
	}

	return nullptr;
}


#undef LOCTEXT_NAMESPACE
