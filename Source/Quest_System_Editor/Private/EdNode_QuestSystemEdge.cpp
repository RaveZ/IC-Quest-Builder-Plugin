// Copyright 2024 Ivan Chandra. All Rights Reserved.


#include "EdNode_QuestSystemEdge.h"
#include "QuestSystemGraphEdge.h"
#include "EdNode_QuestSystemNode.h"

#define LOCTEXT_NAMESPACE "EdNode_QuestSystemGraphEdge"

UEdNode_QuestSystemEdge::UEdNode_QuestSystemEdge()
{
	bCanRenameNode = true;
}

void UEdNode_QuestSystemEdge::SetEdge(UQuestSystemGraphEdge* Edge)
{
	QuestSystemGraphEdge = Edge;
}

void UEdNode_QuestSystemEdge::AllocateDefaultPins()
{
	UEdGraphPin* Inputs = CreatePin(EGPD_Input, TEXT("Edge"), FName(), TEXT("In"));
	Inputs->bHidden = true;
	UEdGraphPin* Outputs = CreatePin(EGPD_Output, TEXT("Edge"), FName(), TEXT("Out"));
	Outputs->bHidden = true;
}

FText UEdNode_QuestSystemEdge::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	if (QuestSystemGraphEdge)
	{
		return QuestSystemGraphEdge->GetNodeTitle();
	}
	return FText();
}

void UEdNode_QuestSystemEdge::PinConnectionListChanged(UEdGraphPin* Pin)
{
	if (Pin->LinkedTo.Num() == 0)
	{
		// Commit suicide; transitions must always have an input and output connection
		Modify();

		// Our parent graph will have our graph in SubGraphs so needs to be modified to record that.
		if (UEdGraph* ParentGraph = GetGraph())
		{
			ParentGraph->Modify();
		}

		DestroyNode();
	}
}

void UEdNode_QuestSystemEdge::PrepareForCopying()
{
	QuestSystemGraphEdge->Rename(nullptr, this, REN_DontCreateRedirectors | REN_DoNotDirty);
}

void UEdNode_QuestSystemEdge::CreateConnections(UEdNode_QuestSystemNode* Start, UEdNode_QuestSystemNode* End)
{
	Pins[0]->Modify();
	Pins[0]->LinkedTo.Empty();

	Start->GetOutputPin()->Modify();
	Pins[0]->MakeLinkTo(Start->GetOutputPin());

	// This to next
	Pins[1]->Modify();
	Pins[1]->LinkedTo.Empty();

	End->GetInputPin()->Modify();
	Pins[1]->MakeLinkTo(End->GetInputPin());
}

UEdNode_QuestSystemNode* UEdNode_QuestSystemEdge::GetStartNode()
{
	if (Pins[0]->LinkedTo.Num() > 0)
	{
		return Cast<UEdNode_QuestSystemNode>(Pins[0]->LinkedTo[0]->GetOwningNode());
	}
	else
	{
		return nullptr;
	}
}

UEdNode_QuestSystemNode* UEdNode_QuestSystemEdge::GetEndNode()
{
	if (Pins[1]->LinkedTo.Num() > 0)
	{
		return Cast<UEdNode_QuestSystemNode>(Pins[1]->LinkedTo[0]->GetOwningNode());
	}
	else
	{
		return nullptr;
	}
}

#undef LOCTEXT_NAMESPACE