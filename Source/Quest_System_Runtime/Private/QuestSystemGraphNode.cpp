// Copyright 2024 Ivan Chandra. All Rights Reserved.


#include "QuestSystemGraphNode.h"
#include "QuestSystemGraphEdge.h"
#include "QuestSystem.h"
#include "QuestSystemGraph.h"

#define LOCTEXT_NAMESPACE "QuestSystemGraphNode"

UQuestSystemGraphNode::UQuestSystemGraphNode()
{
#if WITH_EDITORONLY_DATA
	CompatibleGraphType = UQuestSystemGraph::StaticClass();

	BackgroundColor = FLinearColor::Black;
#endif
}

UQuestSystemGraphNode::~UQuestSystemGraphNode()
{
}

UQuestSystemGraphEdge* UQuestSystemGraphNode::GetEdge(UQuestSystemGraphNode* ChildNode)
{
	return Edges.Contains(ChildNode) ? Edges.FindChecked(ChildNode) : nullptr;
}


FText UQuestSystemGraphNode::GetDescription_Implementation() const
{
	return Description;
}


FString UQuestSystemGraphNode::GetShortTypeName(const UObject* Ob)
{
	if ((Ob == nullptr) || (Ob->GetClass() == nullptr))
	{
		return TEXT("None");
	}

	if (Ob->GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint))
	{
		return Ob->GetClass()->GetName().LeftChop(2);
	}

	FString TypeDesc = Ob->GetClass()->GetName();
	const int32 ShortNameIdx = TypeDesc.Find(TEXT("_"), ESearchCase::CaseSensitive);
	if (ShortNameIdx != INDEX_NONE)
	{
		TypeDesc.MidInline(ShortNameIdx + 1, MAX_int32, false);
	}

	return TypeDesc;
}

#if WITH_EDITOR
bool UQuestSystemGraphNode::IsNameEditable() const
{
	return true;
}

FLinearColor UQuestSystemGraphNode::GetBackgroundColor() const
{
	return BackgroundColor;
}

FText UQuestSystemGraphNode::GetNodeTitle() const
{
	return FText::FromString(UQuestSystemGraphNode::GetShortTypeName(this));
}

FText UQuestSystemGraphNode::GetNodeDescription() const
{
	return FText::FromString(TEXT("This is NodeBase"));
}

void UQuestSystemGraphNode::SetNodeTitle(const FText& NewTitle)
{
}

bool UQuestSystemGraphNode::CanCreateConnection(UQuestSystemGraphNode* Other, FText& ErrorMessage)
{
	return true;
}

bool UQuestSystemGraphNode::CanCreateConnectionTo(UQuestSystemGraphNode* Other, int32 NumberOfChildrenNodes, FText& ErrorMessage)
{
	if (ChildrenLimitType == ENodeLimits::Limited && NumberOfChildrenNodes >= ChildrenLimit)
	{
		ErrorMessage = FText::FromString("Children limit exceeded");
		return false;
	}

	return CanCreateConnection(Other, ErrorMessage);
	
}

bool UQuestSystemGraphNode::CanCreateConnectionFrom(UQuestSystemGraphNode* Other, int32 NumberOfParentNodes, FText& ErrorMessage)
{
	if (ParentLimitType == ENodeLimits::Limited && NumberOfParentNodes >= ParentLimit)
	{
		ErrorMessage = FText::FromString("Parent limit exceeded");
		return false;
	}

	return true;
}

#endif

bool UQuestSystemGraphNode::IsLeafNode() const
{
	return ChildrenNodes.Num() == 0;
}

UQuestSystemGraph* UQuestSystemGraphNode::GetOwningQuestGraph() const
{
	if (!QuestGraph)
		return nullptr;

	return QuestGraph;
}

UQuestSystem* UQuestSystemGraphNode::GetOwningQuestSystem() const
{
	return QuestSystem;
}


AController* UQuestSystemGraphNode::GetOwningController() const
{
	if (!QuestGraph)
		return nullptr;
	

	return QuestGraph->OwningController;
}

UQuestComponent* UQuestSystemGraphNode::GetQuestComponent() const
{
	if (!QuestGraph)
		return nullptr;

	return QuestGraph->QuestComponent;
}

#undef LOCTEXT_NAMESPACE


