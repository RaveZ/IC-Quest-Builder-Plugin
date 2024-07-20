// Copyright 2024 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/NoExportTypes.h"
#include "QuestSystem.generated.h"

class UQuestCondition;
class UQuestSystemGraphNode;
class UQuestSystemGraphEdge;
class UQuestSystemGraphNode_Root;

UENUM(BlueprintType)
enum class EQuestState : uint8
{
	/**Quest is Active*/
	E_Active		UMETA(DisplayName = "ACTIVE"),
	/**Quest is Completed*/
	E_Complete		UMETA(DisplayName = "COMPLETE"),
	/**Quest is Fail*/
	E_Fail			UMETA(DisplayName = "FAIL"),
	/**Quest is Locked, Waiting to be Unlocked*/
	E_Locked		UMETA(DisplayName = "LOCKED"),
	/**Quest is Unlocked, Waiting to be Activated*/
	E_Unlocked		UMETA(DisplayName = "UNLOCKED"),
};

/** Reward Data for Quest*/
USTRUCT(BlueprintType)
struct FRewardData
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Reward Tag"), Category = RewardData)
		FString RewardTag;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Reward Name"), Category = RewardData)
		FString RewardName;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Reward Quantity"), Category = RewardData)
		int RewardQuantity = 0;

};



UCLASS(Blueprintable)
class QUEST_SYSTEM_RUNTIME_API UQuestSystem : public UObject
{
	GENERATED_BODY()
public:

	UPROPERTY(VisibleDefaultsOnly, Category = "QuestAssets")
		UQuestSystemGraph* QuestGraph;

	/** Quest ID */
	UPROPERTY(BlueprintReadOnly, VisibleDefaultsOnly, Category = "QuestSystem")
		FName QuestID;


	/** Description for your quest*/
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "QuestSystem", meta = (MultiLine = true))
		FText Description; 


	/** Reward for your Quest*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "QuestSystem")
		TArray<FRewardData> Rewards;

	/**Make your quest can be aborted*/
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Conditions")
		bool bCanQuestBeAborted = false;
	/**
	* TRUE : After the conditions met, activate this quest. if there are no conditions, activate this quest immediately
	* FALSE : After the conditions met, unlock this quest.
	*/
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Conditions")
		bool bAutoActivateQuest = false;

	/**Conditions to unlock the Quest*/
	UPROPERTY(Instanced, EditAnywhere, BlueprintReadOnly, Category = "Conditions")
		TArray<UQuestCondition*> Conditions;


	/** List of All Visited Node IDs in Sequence*/
	UPROPERTY(BlueprintReadWrite, Category = "QuestSystem")
		TArray<FName> VisitedNodeIDs;

	/**Your quest state, will be changed based on condition*/
	UPROPERTY(BlueprintReadWrite, Category = "QuestSystem")
		EQuestState QuestState = EQuestState::E_Locked;
	
	UPROPERTY(BlueprintReadWrite, Category = "QuestSystem")
		UQuestSystemGraphNode* CurrentNode;

	UPROPERTY(BlueprintReadOnly, Category = "QuestSystem")
		TArray<UQuestSystemGraphNode*> RootNodes;

	UPROPERTY(BlueprintReadOnly, Category = "QuestSystem")
		TArray<UQuestSystemGraphNode*> AllNodes;

	UPROPERTY(BlueprintReadOnly, Category = "QuestSystem")
		TMap<FName, UQuestSystemGraphNode*> NodeMap;


	UFUNCTION(BlueprintCallable, Category = "QuestSystem")
		int GetLevelNum() const;

	UFUNCTION(BlueprintCallable, Category = "QuestSystem")
		void GetNodesByLevel(int Level, TArray<UQuestSystemGraphNode*>& Nodes);

	UFUNCTION(BlueprintCallable, Category = "QuestSystem")
		AController* GetOwningController();

	UFUNCTION(BlueprintCallable, Category = "QuestSystem")
		UQuestComponent* GetQuestComponent();

	UFUNCTION(BlueprintCallable, Category = "QuestSystem")
		UQuestSystemGraph* GetOwningQuestGraph() const;
	void ClearGraph();


	

	

};

