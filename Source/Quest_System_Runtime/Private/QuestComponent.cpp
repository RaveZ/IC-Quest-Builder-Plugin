// Copyright 2024 Ivan Chandra. All Rights Reserved.


#include "QuestComponent.h"
#include "QuestSystem.h"
#include "Condition/QuestCondition.h"
#include "Event/QuestEvent.h"
#include "Task/QuestTask.h"
#include "UObject/UObjectIterator.h"
#include "QuestSaveObject.h"
#include "QuestSystemGraph.h"
#include "QuestSystemGraphNode_State.h"
#include "QuestSystemGraphNode_Root.h"
#include "QuestSystemGraphNode_Objective.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Controller.h"




#define LOCTEXT_NAMESPACE "QuestComponent"
// Sets default values for this component's properties
UQuestComponent::UQuestComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	// ...
}

bool UQuestComponent::RunQuestGraphs(TArray<UQuestSystemGraph*> QuestAssets)
{
	if (QuestAssets.IsEmpty())
	{
		return false;
	}

	//Set Dependency
	OwningController = Cast<AController>(GetOwner());

	for (UQuestSystemGraph* QuestGraph : QuestAssets)
	{
		if (QuestGraph)
		{
			QuestGraph->OwningController = OwningController;
			QuestGraph->QuestComponent = this;

			//Add QuestGraph to array
			QuestGraphs.Add(QuestGraph);

			for (UQuestSystem* Quest : QuestGraph->QuestSystems)
			{
				QuestSystemMap.Emplace(Quest->QuestID, Quest);

				if (!bQuestLoaded)
				{
					Quest->QuestState = EQuestState::E_Locked;
					Quest->CurrentNode = nullptr;
					Quest->VisitedNodeIDs.Empty();
				}
			}
		}
	}
	bool bSuccess = true;
	

	//CheckQuestCondition
	for (UQuestSystemGraph* QuestGraph : QuestGraphs)
	{
		for (UQuestSystem* QuestSystem : QuestGraph->QuestSystems)
		{
			EvaluateQuestCondition(QuestSystem);
			if (QuestSystem->QuestState == EQuestState::E_Active)
			{
				if (QuestSystem->CurrentNode != nullptr)
				{
					BeginNode(QuestSystem);
				}
				else if (QuestSystem->CurrentNode == nullptr)
				{
					RestartQuest(QuestSystem->QuestID);
				}
			}
		}
	}
	
	OnQuestObjectiveUpdated.Broadcast();
	return bSuccess;
}


void UQuestComponent::ActivateQuest(FName QuestID)
{
	if (UQuestSystem* QuestSystem = QuestSystemMap.FindRef(QuestID))
	{
		if (QuestSystem->QuestState != EQuestState::E_Active)
		{
			QuestSystem->QuestState = EQuestState::E_Active;
			OnQuestAdded.Broadcast(QuestSystem);
			RestartQuest(QuestSystem->QuestID);
		}

	}
}




void UQuestComponent::RestartQuest(FName QuestID, FName NodeID, bool bStartFromNodeID)
{
	if (UQuestSystem* QuestSystem = QuestSystemMap.FindRef(QuestID))
	{
		QuestSystem->QuestState = EQuestState::E_Active;


		bool bNodeFound = false;
		if (NodeID.IsNone())
			bStartFromNodeID = false;

		if (bStartFromNodeID)
		{	
			if (UQuestSystemGraphNode* FoundedNode = QuestSystem->NodeMap.FindRef(NodeID))
			{
				bNodeFound = true;
				//reset visited node children where the node started from
				bool bCanDelete = false;
				for (int i = 0; i < QuestSystem->VisitedNodeIDs.Num(); i++)
				{
					if (QuestSystem->VisitedNodeIDs[i] == NodeID)
					{
						bCanDelete = true;
					}
					if (bCanDelete)
					{
						QuestSystem->VisitedNodeIDs.RemoveAt(i);
						i--;//ensure the next element not skipped
					}
				}

				//reset task data from the founded node
				TArray<UQuestSystemGraphNode*> NodeStack;
				NodeStack.Add(FoundedNode);
				while (NodeStack.Num() > 0)
				{
					UQuestSystemGraphNode* CurrentNodeStack = NodeStack.Pop();
					if (UQuestSystemGraphNode_Objective* ObjectiveNode = Cast<UQuestSystemGraphNode_Objective>(CurrentNodeStack))
					{
						for (UQuestTask* Task : ObjectiveNode->Tasks)
						{
							if (Task)
							{
								Task->TaskState = ETaskState::E_Active;
								Task->CurrentAmount = 0;
							}
						}
					}
					for (UQuestEvent* Event : CurrentNodeStack->Events)
					{
						if (Event)
						{
							Event->StartLaunched = false;
							Event->EndLaunched = false;
						}
					}
					// Traverse child node
					for (int i = 0; i < CurrentNodeStack->ChildrenNodes.Num(); i++)
					{
						NodeStack.Add(CurrentNodeStack->ChildrenNodes[i]);
					}
				}

				QuestSystem->CurrentNode = FoundedNode;
			}
		}
		

		if(!bNodeFound)
		{
			//reset all task data
			for (UQuestSystemGraphNode* Node : QuestSystem->AllNodes)
			{
				if (UQuestSystemGraphNode_Objective* ObjectiveNode = Cast<UQuestSystemGraphNode_Objective>(Node))
				{
					for (UQuestTask* Task : ObjectiveNode->Tasks)
					{
						if (Task)
						{
							Task->TaskState = ETaskState::E_Active;
							Task->CurrentAmount = 0;
						}
						
					}
				}
				for (UQuestEvent* Event : Node->Events)
				{
					if (Event)
					{
						Event->StartLaunched = false;
						Event->EndLaunched = false;
					}
				}
			}
			QuestSystem->VisitedNodeIDs.Empty();
			QuestSystem->CurrentNode = QuestSystem->RootNodes[0];
		}
		

		BeginNode(QuestSystem);
	}
}

void UQuestComponent::FailQuest(FName QuestID)
{
	if (UQuestSystem* QuestSystem = QuestSystemMap.FindRef(QuestID))
	{
		if (QuestSystem->QuestState != EQuestState::E_Fail)
		{
			QuestSystem->QuestState = EQuestState::E_Fail;
			OnQuestFailed.Broadcast(QuestSystem);
		}
		QuestSystem->CurrentNode = nullptr;

	}
}

void UQuestComponent::CompleteQuest(FName QuestID)
{
	if (UQuestSystem* QuestSystem = QuestSystemMap.FindRef(QuestID))
	{
		if (QuestSystem->QuestState != EQuestState::E_Complete)
		{
			QuestSystem->QuestState = EQuestState::E_Complete;
			OnQuestCompleted.Broadcast(QuestSystem);
		}
		QuestSystem->CurrentNode = nullptr;

	}
}

UQuestSystem* UQuestComponent::FindQuestFromNodeID(FName NodeIDParam) const
{
	UQuestSystem* FoundedQuest = nullptr;
	for (const auto& It : QuestSystemMap)
	{
		FName Key = It.Key;
		UQuestSystem* QuestSystem = It.Value;

		if (UQuestSystemGraphNode* QuestNode = QuestSystem->NodeMap.FindRef(NodeIDParam))
		{
			FoundedQuest = QuestSystem;
		}
	}

	return FoundedQuest;

}

bool UQuestComponent::IsQuestActiveFromNodeID(FName NodeID) const
{
	if (UQuestSystem* FoundedQuest = FindQuestFromNodeID(NodeID))
	{
		if (FoundedQuest->QuestState == EQuestState::E_Active)
		{
			return true;
		}
	}

	return false;
}

bool UQuestComponent::IsQuestActiveFromQuestID(FName QuestID) const
{
	if (UQuestSystem* FoundedQuest = QuestSystemMap.FindRef(QuestID))
	{
		if (FoundedQuest->QuestState == EQuestState::E_Active)
		{
			return true;
		}
	}

	return false;
}

void UQuestComponent::UpdateQuest()
{
	for (const auto& It : QuestSystemMap)
	{
		FName Key = It.Key;
		UQuestSystem* QuestSystem = It.Value;
		
		EvaluateQuestCondition(QuestSystem);
		if (QuestSystem->CurrentNode != nullptr)
		{
			if (UQuestSystemGraphNode_Objective* ObjectiveNode = Cast<UQuestSystemGraphNode_Objective>(QuestSystem->CurrentNode))
			{
				if (ObjectiveNode->IsAllTaskCompleted())
				{
					//evaluate next node condition
					EvaluateNextNodeCondition(QuestSystem);
				}
			}
		}
		else if (QuestSystem->QuestState == EQuestState::E_Active && QuestSystem->CurrentNode == nullptr)
		{
			RestartQuest(QuestSystem->QuestID);
		}
		
	}
}

void UQuestComponent::EvaluateNextNodeCondition(UQuestSystem* QuestParam)
{
	UQuestSystemGraphNode* NextNode = nullptr;
	for (UQuestSystemGraphNode* Node : QuestParam->CurrentNode->ChildrenNodes)
	{
		if (NextNode == nullptr && Node->Conditions.IsEmpty())
		{
			NextNode = Node;
		}
		else if (!Node->Conditions.IsEmpty())
		{
			int SumConditionMet = 0;
			for (UQuestCondition* Condition : Node->Conditions)
			{
				if (Condition && Condition->IsConditionMet())
				{
					SumConditionMet++;
				}
			}
			if (SumConditionMet >= Node->Conditions.Num())
			{
				NextNode = Node;
				//break loop to ensure the met condition doesn't get overrided
				break;
			}
		}
	}

	if (NextNode)
	{
		//End of a Node, Launch Event with the End and Both Type
		for (UQuestEvent* Event : QuestParam->CurrentNode->Events)
		{
			if (Event)
			{
				if (Event->EventRunType == EEventRunType::E_Both || Event->EventRunType == EEventRunType::E_End)
				{
					if (!Event->EndLaunched)
					{
						Event->BeginEvent();
						Event->EndLaunched = true;
					}
				}
			}
			
		}
		//Go to next node if exist or condition met
		GoToNextNode(QuestParam, NextNode);

	}
	
}

void UQuestComponent::EvaluateQuestCondition(UQuestSystem* QuestParam)
{
	int SumConditionMet = 0;
	for (UQuestCondition* Condition : QuestParam->Conditions)
	{
		if (Condition && Condition->IsConditionMet())
		{
			SumConditionMet++;
		}
	}

	switch(QuestParam->QuestState)
	{
		case EQuestState::E_Locked:
			if (SumConditionMet >= QuestParam->Conditions.Num())
			{
				if (QuestParam->bAutoActivateQuest)
				{
					QuestParam->QuestState = EQuestState::E_Active;
					OnQuestAdded.Broadcast(QuestParam);
					return;
				}
				QuestParam->QuestState = EQuestState::E_Unlocked;
			}
		case EQuestState::E_Unlocked:
			if (SumConditionMet >= QuestParam->Conditions.Num())
			{
				if (QuestParam->bAutoActivateQuest)
				{
					QuestParam->QuestState = EQuestState::E_Active;
					OnQuestAdded.Broadcast(QuestParam);
					return;
				}
			}

	}
	
	
}

void UQuestComponent::GoToNextNode(UQuestSystem* QuestParam, UQuestSystemGraphNode* NodeParam)
{
	QuestParam->CurrentNode = NodeParam;
	BeginNode(QuestParam);
}

void UQuestComponent::BeginNode(UQuestSystem* QuestParam)
{
	//add visited node
	QuestParam->VisitedNodeIDs.AddUnique(QuestParam->CurrentNode->ID);

	if (UQuestSystemGraphNode_Root* RootNode = Cast<UQuestSystemGraphNode_Root>(QuestParam->CurrentNode))
	{
		EvaluateNextNodeCondition(QuestParam);
		return;
	}

	//Start of the Node, Launch Event with the Start and Both Type
	for (UQuestEvent* Event : QuestParam->CurrentNode->Events)
	{
		if (Event)
		{
			if (Event->EventRunType == EEventRunType::E_Both || Event->EventRunType == EEventRunType::E_Start)
			{
				if (!Event->StartLaunched)
				{
					Event->BeginEvent();
					Event->StartLaunched = true;
				}
			}
		}
	}

	//Begin Node Functionality
	if (UQuestSystemGraphNode_Objective* ObjectiveNode = Cast<UQuestSystemGraphNode_Objective>(QuestParam->CurrentNode))
	{
		for (UQuestTask* Task : ObjectiveNode->Tasks)
		{
			if (Task)
			{
				Task->TaskBegin();
			}

		}
	}

	if (UQuestSystemGraphNode_State* StateNode = Cast<UQuestSystemGraphNode_State>(QuestParam->CurrentNode))
	{
		StateNode->BeginState();
	}
}




void UQuestComponent::Save_Implementation(const FString& SaveFileName, int SlotIndex)
{
	//Create New Save
	if (UQuestSaveObject* QuestSaveObject = Cast<UQuestSaveObject>(UGameplayStatics::CreateSaveGameObject(UQuestSaveObject::StaticClass())))
	{
		//Start Saving Data
		for (const auto& It : QuestSystemMap)
		{
			FName Key = It.Key;
			UQuestSystem* QuestSystem = It.Value;
				TMap<FName, FNodeData> NodeMapData;
				for (UQuestSystemGraphNode* Node : QuestSystem->AllNodes)
				{
					TArray<FTaskData> TaskDataArray;
					if (UQuestSystemGraphNode_Objective* ObjectiveNode = Cast<UQuestSystemGraphNode_Objective>(Node))
					{
						for (UQuestTask* Task : ObjectiveNode->Tasks)
						{
							FTaskData TaskData;
							if (Task)
							{
								TaskData.CurrentAmount = Task->CurrentAmount;
								TaskData.TaskState = Task->TaskState;
							}
							TaskDataArray.Add(TaskData);
						}
					}

					TArray<FEventData> EventDataArray;
					for (UQuestEvent* Event : Node->Events)
					{
						FEventData EventData;
						if (Event)
						{
							EventData.StartLaunched = Event->StartLaunched;
							EventData.EndLaunched = Event->EndLaunched;
						}
						EventDataArray.Add(EventData);
					}

					FNodeData NodeData;
					if (Node)
					{
						NodeData.NodeID = Node->ID;
						NodeData.TaskDatas = TaskDataArray;
						NodeData.EventDatas = EventDataArray;
					}
					NodeMapData.Emplace(Node->ID, NodeData);
				}
				FQuestData QuestData;
				if (QuestSystem->CurrentNode)
				{
					QuestData.CurrentNodeID = QuestSystem->CurrentNode->ID;
				}
				QuestData.QuestState = QuestSystem->QuestState;
				QuestData.NodeMapData = NodeMapData;
				QuestData.VisitedNodeIDs = QuestSystem->VisitedNodeIDs;
				QuestSaveObject->QuestMapData.Emplace(QuestSystem->QuestID, QuestData);
			
		}

		UGameplayStatics::SaveGameToSlot(QuestSaveObject, SaveFileName, SlotIndex);
	}
	


}

void UQuestComponent::Load_Implementation(const FString& SaveFileName, int SlotIndex) 
{
	TMap<FName, UQuestSystem*> QuestMap;
	for (TObjectIterator<UQuestSystemGraph> It; It; ++It)
	{
		for (UQuestSystem* Quest : It->QuestSystems)
		{
			QuestMap.Add(Quest->QuestID, Quest);
		}
	}

	if (UQuestSaveObject* LoadedData = Cast<UQuestSaveObject>(UGameplayStatics::LoadGameFromSlot(SaveFileName, SlotIndex)))
	{
		bQuestLoaded = true;
		for (const auto& QuestIt : LoadedData->QuestMapData)
		{
			FName QuestID = QuestIt.Key;
			FQuestData QuestData = QuestIt.Value;

			UQuestSystem* FoundedQuest = QuestMap.FindRef(QuestID);
			if (FoundedQuest != nullptr)
			{
				//Fill Quest Data Here
				FoundedQuest->QuestState = QuestData.QuestState;
				FoundedQuest->CurrentNode = FoundedQuest->NodeMap.FindRef(QuestData.CurrentNodeID);
				FoundedQuest->VisitedNodeIDs = QuestData.VisitedNodeIDs;

				for (const auto& NodeIt : QuestData.NodeMapData)
				{
					FName NodeID = NodeIt.Key;
					FNodeData NodeData = NodeIt.Value;

					UQuestSystemGraphNode* FoundedNode = FoundedQuest->NodeMap.FindRef(NodeID);
					if (FoundedNode != nullptr)
					{
						//Fill Node Data Here
						if (UQuestSystemGraphNode_Objective* ObjectiveNode = Cast<UQuestSystemGraphNode_Objective>(FoundedNode))
						{
							for (int i = 0; i < ObjectiveNode->Tasks.Num(); i++)
							{
								if (ObjectiveNode->Tasks[i] != nullptr && i < NodeData.TaskDatas.Num())
								{
									//fill task data here
									ObjectiveNode->Tasks[i]->CurrentAmount = NodeData.TaskDatas[i].CurrentAmount;
									ObjectiveNode->Tasks[i]->TaskState = NodeData.TaskDatas[i].TaskState;
								}
							}
						}

						for (int i = 0; i < FoundedNode->Events.Num(); i++)
						{
							if (FoundedNode->Events[i] != nullptr && i < NodeData.EventDatas.Num())
							{
								//fill event data here
								FoundedNode->Events[i]->StartLaunched = NodeData.EventDatas[i].StartLaunched;
								FoundedNode->Events[i]->EndLaunched = NodeData.EventDatas[i].EndLaunched;
							}
						}
					}
				}
			}
		}
	}

}

// Called when the game starts
void UQuestComponent::BeginPlay()
{
	Super::BeginPlay();
	QuestSystemMap.Empty();
	//OnQuestObjectiveUpdated.AddDynamic(this, &UQuestComponent::OnUpdateQuest);
	// ...
	
}


// Called every frame
void UQuestComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

#undef LOCTEXT_NAMESPACE