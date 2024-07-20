// Copyright 2024 Ivan Chandra. All Rights Reserved.


#include "AssetGraphSchema_QuestSystem.h"
#include "ToolMenus.h"
#include "Quest_System_Editor.h"
#include "AIGraphTypes.h"
#include "ConnectionDrawPolicy_QuestSystem.h"
#include "Framework/Commands/GenericCommands.h"
#include "Settings/EditorStyleSettings.h"
#include "QuestSystemEditorUtils.h"
#include "EdNode_QuestSystemNode.h"
#include "EdNode_QuestSystemTask.h"
#include "EdNode_QuestSystemState.h"
#include "EdNode_QuestSystemEdge.h"
#include "EdNode_QuestSystemRoot.h"
#include "GraphEditorActions.h"
#include "QuestSystemGraph.h"
#include "QuestSystemGraphEdge.h"
#include "QuestSystemGraphNode_Root.h"
#include "QuestSystemGraphNode_State.h"
#include "QuestSystemGraphNode_Objective.h"
#include "Task/QuestTask.h"


#define LOCTEXT_NAMESPACE "AssetSchema_QuestSystemGraph"

namespace
{
	// Maximum distance a drag can be off a node edge to require 'push off' from node
	const int32 NodeDistance = 60;
}

int32 UAssetGraphSchema_QuestSystem::CurrentCacheRefreshID = 0;

class FNodeVisitorCycleChecker
{
public:
	/** Check whether a loop in the graph would be caused by linking the passed-in nodes */
	bool CheckForLoop(UEdGraphNode* StartNode, UEdGraphNode* EndNode)
	{

		VisitedNodes.Add(StartNode);

		return TraverseNodes(EndNode);
	}

private:
	bool TraverseNodes(UEdGraphNode* Node)
	{
		VisitedNodes.Add(Node);

		for (auto MyPin : Node->Pins)
		{
			if (MyPin->Direction == EGPD_Output)
			{
				for (auto OtherPin : MyPin->LinkedTo)
				{
					UEdGraphNode* OtherNode = OtherPin->GetOwningNode();
					if (VisitedNodes.Contains(OtherNode))
					{
						// Only  an issue if this is a back-edge
						return false;
					}
					else if (!FinishedNodes.Contains(OtherNode))
					{
						// Only should traverse if this node hasn't been traversed
						if (!TraverseNodes(OtherNode))
							return false;
					}
				}
			}
		}

		VisitedNodes.Remove(Node);
		FinishedNodes.Add(Node);
		return true;
	};


	TSet<UEdGraphNode*> VisitedNodes;
	TSet<UEdGraphNode*> FinishedNodes;
};

UEdGraphNode* FAssetSchemaAction_QuestSystem_NewNode::PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode)
{
	UEdGraphNode* ResultNode = NULL;

	//// If there is a template, we actually use it
	if (NodeTemplate != NULL)
	{
		const FScopedTransaction Transaction(LOCTEXT("AddNode", "Add Node"));
		ParentGraph->Modify();
		if (FromPin)
		{
			FromPin->Modify();
		}

		NodeTemplate->SetFlags(RF_Transactional);

	//	// set outer to be the graph so it doesn't go away
		NodeTemplate->Rename(NULL, ParentGraph, REN_NonTransactional);
		ParentGraph->AddNode(NodeTemplate, true);

		NodeTemplate->CreateNewGuid();
		NodeTemplate->PostPlacedNewNode();

		// For input pins, new node will generally overlap node being dragged off
		// Work out if we want to visually push away from connected node
		int32 XLocation = Location.X;
		if (FromPin && FromPin->Direction == EGPD_Input)
		{
			UEdGraphNode* PinNode = FromPin->GetOwningNode();
			const float XDelta = FMath::Abs(PinNode->NodePosX - Location.X);

			if (XDelta < NodeDistance)
			{
				// Set location to edge of current node minus the max move distance
				// to force node to push off from connect node enough to give selection handle
				XLocation = PinNode->NodePosX - NodeDistance;
			}
		}

		NodeTemplate->NodePosX = XLocation;
		NodeTemplate->NodePosY = Location.Y;
		NodeTemplate->SnapToGrid(GetDefault<UEditorStyleSettings>()->GridSnapSize);

		// setup pins after placing node in correct spot, since pin sorting will happen as soon as link connection change occurs
		NodeTemplate->AllocateDefaultPins();
		NodeTemplate->AutowireNewNode(FromPin);

		ResultNode = NodeTemplate;
	}

	return ResultNode;

	/*UEdGraphNode* ResultNode = nullptr;

	if (NodeTemplate != nullptr)
	{
		const FScopedTransaction Transaction(LOCTEXT("GenericGraphEditorNewNode", "Generic Graph Editor: New Node"));
		ParentGraph->Modify();
		if (FromPin != nullptr)
			FromPin->Modify();

		NodeTemplate->Rename(nullptr, ParentGraph);
		ParentGraph->AddNode(NodeTemplate, true, bSelectNewNode);

		NodeTemplate->CreateNewGuid();
		NodeTemplate->PostPlacedNewNode();
		NodeTemplate->AllocateDefaultPins();
		NodeTemplate->AutowireNewNode(FromPin);

		NodeTemplate->NodePosX = Location.X;
		NodeTemplate->NodePosY = Location.Y;

		NodeTemplate->QuestSystemGraphNode->SetFlags(RF_Transactional);
		NodeTemplate->SetFlags(RF_Transactional);

		ResultNode = NodeTemplate;
	}

	return ResultNode;*/
}

void FAssetSchemaAction_QuestSystem_NewNode::AddReferencedObjects(FReferenceCollector& Collector)
{
	FEdGraphSchemaAction::AddReferencedObjects(Collector);
	Collector.AddReferencedObject(NodeTemplate);
}

UEdGraphNode* FAssetSchemaAction_QuestSystem_NewEdge::PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode)
{
	UEdGraphNode* ResultNode = nullptr;

	if (NodeTemplate != nullptr)
	{
		const FScopedTransaction Transaction(LOCTEXT("GenericGraphEditorNewEdge", "Generic Graph Editor: New Edge"));
		ParentGraph->Modify();
		if (FromPin != nullptr)
			FromPin->Modify();

		NodeTemplate->Rename(nullptr, ParentGraph);
		ParentGraph->AddNode(NodeTemplate, true, bSelectNewNode);

		NodeTemplate->CreateNewGuid();
		NodeTemplate->PostPlacedNewNode();
		NodeTemplate->AllocateDefaultPins();
		NodeTemplate->AutowireNewNode(FromPin);

		NodeTemplate->NodePosX = Location.X;
		NodeTemplate->NodePosY = Location.Y;

		NodeTemplate->QuestSystemGraphEdge->SetFlags(RF_Transactional);
		NodeTemplate->SetFlags(RF_Transactional);

		ResultNode = NodeTemplate;
	}

	return ResultNode;
}

void FAssetSchemaAction_QuestSystem_NewEdge::AddReferencedObjects(FReferenceCollector& Collector)
{
	FEdGraphSchemaAction::AddReferencedObjects(Collector);
}


void UAssetGraphSchema_QuestSystem::GetBreakLinkToSubMenuActions(UToolMenu* Menu, UEdGraphPin* InGraphPin)
{
	// Make sure we have a unique name for every entry in the list
	TMap< FString, uint32 > LinkTitleCount;

	FToolMenuSection& Section = Menu->FindOrAddSection("GenericGraphAssetGraphSchemaPinActions");

	// Add all the links we could break from
	for (TArray<class UEdGraphPin*>::TConstIterator Links(InGraphPin->LinkedTo); Links; ++Links)
	{
		UEdGraphPin* Pin = *Links;
		FString TitleString = Pin->GetOwningNode()->GetNodeTitle(ENodeTitleType::ListView).ToString();
		FText Title = FText::FromString(TitleString);
		if (Pin->PinName != TEXT(""))
		{
			TitleString = FString::Printf(TEXT("%s (%s)"), *TitleString, *Pin->PinName.ToString());

			// Add name of connection if possible
			FFormatNamedArguments Args;
			Args.Add(TEXT("NodeTitle"), Title);
			Args.Add(TEXT("PinName"), Pin->GetDisplayName());
			Title = FText::Format(LOCTEXT("BreakDescPin", "{NodeTitle} ({PinName})"), Args);
		}

		uint32& Count = LinkTitleCount.FindOrAdd(TitleString);

		FText Description;
		FFormatNamedArguments Args;
		Args.Add(TEXT("NodeTitle"), Title);
		Args.Add(TEXT("NumberOfNodes"), Count);

		if (Count == 0)
		{
			Description = FText::Format(LOCTEXT("BreakDesc", "Break link to {NodeTitle}"), Args);
		}
		else
		{
			Description = FText::Format(LOCTEXT("BreakDescMulti", "Break link to {NodeTitle} ({NumberOfNodes})"), Args);
		}
		++Count;

		Section.AddMenuEntry(NAME_None, Description, Description, FSlateIcon(), FUIAction(
			FExecuteAction::CreateUObject(this, &UAssetGraphSchema_QuestSystem::BreakSinglePinLink, const_cast<UEdGraphPin*>(InGraphPin), *Links)));
	}
}

void UAssetGraphSchema_QuestSystem::CreateDefaultNodesForGraph(UEdGraph& Graph) const
{
	UQuestSystemGraph* QuestGraph = CastChecked<UQuestSystemGraph>(Graph.GetOuter());

	FGraphNodeCreator<UEdNode_QuestSystemRoot> NodeCreator(Graph);
	UEdNode_QuestSystemRoot* RootNode = NodeCreator.CreateNode();
	RootNode->QuestSystemGraphNode = NewObject<UQuestSystemGraphNode>(RootNode, UQuestSystemGraphNode_Root::StaticClass());
	RootNode->QuestSystemGraphNode->ID = FQuestSystemEditorUtils::FindUniqueNodeName("RootNode");
	RootNode->QuestSystemGraphNode->QuestGraph = QuestGraph;
	RootNode->QuestSystemGraphNode->QuestGraph->QuestSystems = QuestGraph->QuestSystems;
	NodeCreator.Finalize();
	SetNodeMetaData(RootNode, FNodeMetadata::DefaultGraphNode);
}

EGraphType UAssetGraphSchema_QuestSystem::GetGraphType(const UEdGraph* TestEdGraph) const
{
	return GT_StateMachine;
}

void UAssetGraphSchema_QuestSystem::GetGraphContextActions(FGraphContextMenuBuilder& ContextMenuBuilder) const
{
	UQuestSystemGraph* Graph = CastChecked<UQuestSystemGraph>(ContextMenuBuilder.CurrentGraph->GetOuter());


	const bool bNoParent = (ContextMenuBuilder.FromPin == NULL);

	const FText AddToolTip = LOCTEXT("NewGenericGraphNodeTooltip", "Add node here");

	TSet<TSubclassOf<UQuestSystemGraphNode> > Visited;

	/*FText Desc = Graph->NodeType.GetDefaultObject()->ContextMenuName;

	if (Desc.IsEmpty())
	{
		FString Title = Graph->NodeType->GetName();
		Title.RemoveFromEnd("_C");
		Desc = FText::FromString(Title);
	}*/

	


	//to check if we create another instance derived from UQuestSystemGraphNode and they will added automatically
	/*for (TObjectIterator<UClass> It; It; ++It)
	{

		if (It->IsChildOf(UQuestSystemGraphNode::StaticClass()) && !It->HasAnyClassFlags(CLASS_Abstract) && !Visited.Contains(*It))
		{
			TSubclassOf<UQuestSystemGraphNode> NodeType = *It;

			if (It->GetName().StartsWith("REINST") || It->GetName().StartsWith("SKEL"))
				continue;

			if (!Graph->GetClass()->IsChildOf(NodeType.GetDefaultObject()->CompatibleGraphType))
				continue;

			Desc = NodeType.GetDefaultObject()->ContextMenuName;

			if (Desc.IsEmpty())
			{
				FString Title = NodeType->GetName();
				Title.RemoveFromEnd("_C");
				Desc = FText::FromString(Title);
			}

			TSharedPtr<FAssetSchemaAction_QuestSystem_NewNode> Action(new FAssetSchemaAction_QuestSystem_NewNode(LOCTEXT("QuestSystemGraphNodeAction", "Task Node"), Desc, AddToolTip, 0));
			Action->NodeTemplate = NewObject<UEdNode_QuestSystemNode>(ContextMenuBuilder.OwnerOfTemporaries);
			Action->NodeTemplate->QuestSystemGraphNode = NewObject<UQuestSystemGraphNode>(Action->NodeTemplate, NodeType);
			Action->NodeTemplate->QuestSystemGraphNode->QuestGraph = Graph;
			ContextMenuBuilder.AddAction(Action);

			Visited.Add(NodeType);
		}
	}*/


	//Task
	FQuest_System_EditorModule& EditorModule = FModuleManager::GetModuleChecked<FQuest_System_EditorModule>(TEXT("Quest_System_Editor"));
	FGraphNodeClassHelper* ClassCache = EditorModule.GetClassCache().Get();

	FCategorizedGraphActionListBuilder TasksBuilder(TEXT("Task"));

	TArray<FGraphNodeClassData> NodeClasses;
	ClassCache->GatherClasses(UQuestTask::StaticClass(), NodeClasses);
	

	for (auto& NodeClass : NodeClasses)
	{
		if (NodeClass.GetClass(true) == UQuestTask::StaticClass())
			continue;


		UE_LOG(LogTemp, Warning, TEXT("NodeClass: %s"), *FName::NameToDisplayString(NodeClass.ToString(), false));

		const FText NodeTypeName = FText::FromString(FName::NameToDisplayString(NodeClass.ToString(), false));

		TSharedPtr<FAssetSchemaAction_QuestSystem_NewNode> AddOpAction = UAssetGraphSchema_QuestSystem::AddNewNodeAction(TasksBuilder, NodeClass.GetCategory(), NodeTypeName, FText::GetEmpty());
		UClass* GraphNodeClass = UEdNode_QuestSystemTask::StaticClass();


		UEdNode_QuestSystemNode* OpNode = NewObject<UEdNode_QuestSystemNode>(ContextMenuBuilder.OwnerOfTemporaries, GraphNodeClass);
		OpNode->ClassData = NodeClass;
		AddOpAction->NodeTemplate = OpNode;
		AddOpAction->NodeTemplate->QuestSystemGraphNode = NewObject<UQuestSystemGraphNode>(AddOpAction->NodeTemplate, UQuestSystemGraphNode_Objective::StaticClass());// NodeClass.GetClass(true));
		if (UQuestSystemGraphNode_Objective* ObjectiveNode = Cast< UQuestSystemGraphNode_Objective>(AddOpAction->NodeTemplate->QuestSystemGraphNode))
		{
			if (NodeClass.GetClass(true)->IsChildOf(UQuestTask::StaticClass()))
			{
				ObjectiveNode->Tasks.Add(NewObject<UQuestTask>(AddOpAction->NodeTemplate, NodeClass.GetClass(true)));
			}
			AddOpAction->NodeTemplate->QuestSystemGraphNode->QuestGraph = Graph;
			AddOpAction->NodeTemplate->QuestSystemGraphNode->ID = FQuestSystemEditorUtils::FindUniqueNodeName("TaskNode");
			AddOpAction->NodeTemplate->QuestSystemGraphNode->QuestGraph->QuestSystems = Graph->QuestSystems;
		}
	}
	ContextMenuBuilder.Append(TasksBuilder);


	FCategorizedGraphActionListBuilder StateBuilder(TEXT("Quest State"));
	//State : Complete Quest
	TSharedPtr<FAssetSchemaAction_QuestSystem_NewNode> AddOpAction = UAssetGraphSchema_QuestSystem::AddNewNodeAction(StateBuilder, FText::GetEmpty(), LOCTEXT("QuestSystemGraphNodeAction", "Complete Quest"), LOCTEXT("QuestSystemGraphNodeTooltip", "Complete The Quest"));
	UEdNode_QuestSystemNode* OpNode = NewObject<UEdNode_QuestSystemNode>(ContextMenuBuilder.OwnerOfTemporaries, UEdNode_QuestSystemState::StaticClass());
	AddOpAction->NodeTemplate = OpNode;
	AddOpAction->NodeTemplate->QuestSystemGraphNode = NewObject<UQuestSystemGraphNode>(AddOpAction->NodeTemplate, UQuestSystemGraphNode_State::StaticClass());
	AddOpAction->NodeTemplate->QuestSystemGraphNode->QuestGraph = Graph;
	AddOpAction->NodeTemplate->QuestSystemGraphNode->ID = FQuestSystemEditorUtils::FindUniqueNodeName("StateNode");
	AddOpAction->NodeTemplate->QuestSystemGraphNode->QuestGraph->QuestSystems = Graph->QuestSystems;
	if (UQuestSystemGraphNode_State* NodeState = Cast< UQuestSystemGraphNode_State>(AddOpAction->NodeTemplate->QuestSystemGraphNode))
	{
		NodeState->QuestState = EQuestState::E_Complete;
	}
	//State : Fail Quest
	AddOpAction = UAssetGraphSchema_QuestSystem::AddNewNodeAction(StateBuilder, FText::GetEmpty(), LOCTEXT("QuestSystemGraphNodeAction", "Fail Quest"), LOCTEXT("QuestSystemGraphNodeTooltip", "Fail The Quest"));
	OpNode = NewObject<UEdNode_QuestSystemNode>(ContextMenuBuilder.OwnerOfTemporaries, UEdNode_QuestSystemState::StaticClass());
	AddOpAction->NodeTemplate = OpNode;
	AddOpAction->NodeTemplate->QuestSystemGraphNode = NewObject<UQuestSystemGraphNode>(AddOpAction->NodeTemplate, UQuestSystemGraphNode_State::StaticClass());
	AddOpAction->NodeTemplate->QuestSystemGraphNode->QuestGraph = Graph;
	AddOpAction->NodeTemplate->QuestSystemGraphNode->ID = FQuestSystemEditorUtils::FindUniqueNodeName("StateNode");
	AddOpAction->NodeTemplate->QuestSystemGraphNode->QuestGraph->QuestSystems = Graph->QuestSystems;
	if (UQuestSystemGraphNode_State* NodeState = Cast< UQuestSystemGraphNode_State>(AddOpAction->NodeTemplate->QuestSystemGraphNode))
	{
		NodeState->QuestState = EQuestState::E_Fail;
	}
	ContextMenuBuilder.Append(StateBuilder);
}

void UAssetGraphSchema_QuestSystem::GetContextMenuActions(UToolMenu* Menu, UGraphNodeContextMenuContext* Context) const
{
	if (Context->Pin) {
		FToolMenuSection& Section = Menu->AddSection("QuestSystemGraphAssetGraphSchemaNodeActions", LOCTEXT("PinActionsMenuHeader", "Pin Actions"));
		// Only display the 'Break Links' option if there is a link to break!
			if (Context->Pin->LinkedTo.Num() > 0)
			{
				Section.AddMenuEntry(FGraphEditorCommands::Get().BreakPinLinks);

				// add sub menu for break link to
				if (Context->Pin->LinkedTo.Num() > 1)
				{
					Section.AddSubMenu(
						"BreakLinkTo",
						LOCTEXT("BreakLinkTo", "Break Link To..."),
						LOCTEXT("BreakSpecificLinks", "Break a specific link..."),
						FNewToolMenuDelegate::CreateUObject((UAssetGraphSchema_QuestSystem* const)this, &UAssetGraphSchema_QuestSystem::GetBreakLinkToSubMenuActions, const_cast<UEdGraphPin*>(Context->Pin)));
				}
				else
				{
					((UAssetGraphSchema_QuestSystem* const)this)->GetBreakLinkToSubMenuActions(Menu, const_cast<UEdGraphPin*>(Context->Pin));
				}
			}
	}
	else if (Context->Node)
	{
		{
			FToolMenuSection& Section = Menu->AddSection("GenericGraphAssetGraphSchemaNodeActions", LOCTEXT("ClassActionsMenuHeader", "Node Actions"));
			Section.AddMenuEntry(FGenericCommands::Get().Rename);
			Section.AddMenuEntry(FGenericCommands::Get().Delete);
			Section.AddMenuEntry(FGenericCommands::Get().Cut);
			Section.AddMenuEntry(FGenericCommands::Get().Copy);
			Section.AddMenuEntry(FGenericCommands::Get().Duplicate);

			Section.AddMenuEntry(FGraphEditorCommands::Get().BreakNodeLinks);
		}
	}

	Super::GetContextMenuActions(Menu, Context);
}

const FPinConnectionResponse UAssetGraphSchema_QuestSystem::CanCreateConnection(const UEdGraphPin* A, const UEdGraphPin* B) const
{
	
	// Make sure the pins are not on the same node
	if (A->GetOwningNode() == B->GetOwningNode())
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("PinErrorSameNode", "Can't connect node to itself"));
	}

	const UEdGraphPin* Out = A;
	const UEdGraphPin* In = B;

	UEdNode_QuestSystemNode* EdNode_Out = Cast<UEdNode_QuestSystemNode>(Out->GetOwningNode());
	UEdNode_QuestSystemNode* EdNode_In = Cast<UEdNode_QuestSystemNode>(In->GetOwningNode());

	if (EdNode_Out == nullptr || EdNode_In == nullptr)
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("PinError", "Not a valid UQuestGraphEdNode"));
	}

	//Determine if we can have cycles or not
	bool bAllowCycles = false;
	auto EdGraph = Cast<UEdGraph_QuestSystemGraph>(Out->GetOwningNode()->GetGraph());
	if (EdGraph != nullptr)
	{
		bAllowCycles = EdGraph->GetQuestSystemGraph()->bCanBeCyclical;
	}

	// check for cycles
	FNodeVisitorCycleChecker CycleChecker;
	if (!bAllowCycles && !CycleChecker.CheckForLoop(Out->GetOwningNode(), In->GetOwningNode()))
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("PinErrorCycle", "Can't create a graph cycle"));
	}

	FText ErrorMessage;
	/*if (!EdNode_Out->QuestSystemGraphNode->CanCreateConnectionTo(EdNode_In->QuestSystemGraphNode, EdNode_Out->GetOutputPin()->LinkedTo.Num(), ErrorMessage))
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, ErrorMessage);
	}*/
	/*if (!EdNode_In->QuestSystemGraphNode->CanCreateConnectionFrom(EdNode_Out->QuestSystemGraphNode, EdNode_In->GetInputPin()->LinkedTo.Num(), ErrorMessage))
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, ErrorMessage);
	}*/


	if (EdNode_Out->QuestSystemGraphNode->GetOwningQuestGraph()->bEdgeEnabled)
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_MAKE_WITH_CONVERSION_NODE, LOCTEXT("PinConnect", "Connect nodes with edge"));
	}
	else
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_MAKE, LOCTEXT("PinConnect", "Connect nodes"));
	}
}

bool UAssetGraphSchema_QuestSystem::TryCreateConnection(UEdGraphPin* A, UEdGraphPin* B) const
{
	// We don't actually care about the pin, we want the node that is being dragged between
	UEdNode_QuestSystemNode* NodeA = Cast<UEdNode_QuestSystemNode>(A->GetOwningNode());
	UEdNode_QuestSystemNode* NodeB = Cast<UEdNode_QuestSystemNode>(B->GetOwningNode());

	// Check that this edge doesn't already exist
	if (NodeA->GetOutputPin())
	{
		for (UEdGraphPin* TestPin : NodeA->GetOutputPin()->LinkedTo)
		{
			UEdGraphNode* ChildNode = TestPin->GetOwningNode();
			if (UEdNode_QuestSystemEdge* EdNode_Edge = Cast<UEdNode_QuestSystemEdge>(ChildNode))
			{
				ChildNode = EdNode_Edge->GetEndNode();
			}

			if (ChildNode == NodeB)
				return false;
		}
	}
	if (UEdNode_QuestSystemState* StateNode = Cast<UEdNode_QuestSystemState>(NodeA))
	{
		Super::TryCreateConnection(NodeB->GetOutputPin(), NodeA->GetInputPin());
		return true;
	}
	else if (UEdNode_QuestSystemRoot* RootNode = Cast<UEdNode_QuestSystemRoot>(NodeB))
	{
		Super::TryCreateConnection(NodeB->GetOutputPin(), NodeA->GetInputPin());
		return true;
	}
	else if (NodeA && NodeB)
	{
		// Always create connections from node A to B, don't allow adding in reverse
		Super::TryCreateConnection(NodeA->GetOutputPin(), NodeB->GetInputPin());
		return true;
	}
	else
	{
		return false;
	}
}

bool UAssetGraphSchema_QuestSystem::CreateAutomaticConversionNodeAndConnections(UEdGraphPin* A, UEdGraphPin* B) const
{
	UEdNode_QuestSystemNode* NodeA = Cast<UEdNode_QuestSystemNode>(A->GetOwningNode());
	UEdNode_QuestSystemNode* NodeB = Cast<UEdNode_QuestSystemNode>(B->GetOwningNode());

	// Are nodes and pins all valid?
	if (!NodeA || !NodeA->GetOutputPin() || !NodeB || !NodeB->GetInputPin())
		return false;

	UQuestSystemGraph* Graph = NodeA->QuestSystemGraphNode->GetOwningQuestGraph();

	FVector2D InitPos((NodeA->NodePosX + NodeB->NodePosX) / 2, (NodeA->NodePosY + NodeB->NodePosY) / 2);

	FAssetSchemaAction_QuestSystem_NewEdge Action;
	Action.NodeTemplate = NewObject<UEdNode_QuestSystemEdge>(NodeA->GetGraph());
	Action.NodeTemplate->SetEdge(NewObject<UQuestSystemGraphEdge>(Action.NodeTemplate, UQuestSystemGraphEdge::StaticClass()));
	UEdNode_QuestSystemEdge* EdgeNode = Cast<UEdNode_QuestSystemEdge>(Action.PerformAction(NodeA->GetGraph(), nullptr, InitPos, false));

	// Always create connections from node A to B, don't allow adding in reverse
	EdgeNode->CreateConnections(NodeA, NodeB);

	return true;
}

FConnectionDrawingPolicy* UAssetGraphSchema_QuestSystem::CreateConnectionDrawingPolicy(int32 InBackLayerID, int32 InFrontLayerID, float InZoomFactor, const FSlateRect& InClippingRect, FSlateWindowElementList& InDrawElements, UEdGraph* InGraphObj) const
{
	return new FConnectionDrawPolicy_QuestSystem(InBackLayerID, InFrontLayerID, InZoomFactor, InClippingRect, InDrawElements, InGraphObj);
}

FLinearColor UAssetGraphSchema_QuestSystem::GetPinTypeColor(const FEdGraphPinType& PinType) const
{
	return FColor::Cyan;
}

void UAssetGraphSchema_QuestSystem::BreakNodeLinks(UEdGraphNode& TargetNode) const
{
	const FScopedTransaction Transaction(NSLOCTEXT("UnrealEd", "GraphEd_BreakNodeLinks", "Break Node Links"));

	Super::BreakNodeLinks(TargetNode);
}

void UAssetGraphSchema_QuestSystem::BreakPinLinks(UEdGraphPin& TargetPin, bool bSendsNodeNotifcation) const
{
	const FScopedTransaction Transaction(NSLOCTEXT("UnrealEd", "GraphEd_BreakPinLinks", "Break Pin Links"));

	Super::BreakPinLinks(TargetPin, bSendsNodeNotifcation);
}

void UAssetGraphSchema_QuestSystem::BreakSinglePinLink(UEdGraphPin* SourcePin, UEdGraphPin* TargetPin) const
{
	const FScopedTransaction Transaction(NSLOCTEXT("UnrealEd", "GraphEd_BreakSinglePinLink", "Break Pin Link"));

	Super::BreakSinglePinLink(SourcePin, TargetPin);
}

UEdGraphPin* UAssetGraphSchema_QuestSystem::DropPinOnNode(UEdGraphNode* InTargetNode, const FName& InSourcePinName, const FEdGraphPinType& InSourcePinType, EEdGraphPinDirection InSourcePinDirection) const
{
	UEdNode_QuestSystemNode* EdNode = Cast<UEdNode_QuestSystemNode>(InTargetNode);
	switch (InSourcePinDirection)
	{
	case EGPD_Input:
		return EdNode->GetOutputPin();
	case EGPD_Output:
		return EdNode->GetInputPin();
	default:
		return nullptr;
	}
}

bool UAssetGraphSchema_QuestSystem::SupportsDropPinOnNode(UEdGraphNode* InTargetNode, const FEdGraphPinType& InSourcePinType, EEdGraphPinDirection InSourcePinDirection, FText& OutErrorMessage) const
{
	return Cast<UEdNode_QuestSystemNode>(InTargetNode) != nullptr;
}

bool UAssetGraphSchema_QuestSystem::IsCacheVisualizationOutOfDate(int32 InVisualizationCacheID) const
{
	return CurrentCacheRefreshID != InVisualizationCacheID;
}

int32 UAssetGraphSchema_QuestSystem::GetCurrentVisualizationCacheID() const
{
	return CurrentCacheRefreshID;
}

void UAssetGraphSchema_QuestSystem::ForceVisualizationCacheClear() const
{
	++CurrentCacheRefreshID;
}

TSharedPtr<FAssetSchemaAction_QuestSystem_NewNode> UAssetGraphSchema_QuestSystem::AddNewNodeAction(FGraphActionListBuilderBase& ContextMenuBuilder, const FText& Category, const FText& MenuDesc, const FText& Tooltip)
{
	TSharedPtr<FAssetSchemaAction_QuestSystem_NewNode> NewAction = TSharedPtr<FAssetSchemaAction_QuestSystem_NewNode>(new FAssetSchemaAction_QuestSystem_NewNode(Category, MenuDesc, Tooltip, 0));
	ContextMenuBuilder.AddAction(NewAction);

	return NewAction;
}


#undef LOCTEXT_NAMESPACE