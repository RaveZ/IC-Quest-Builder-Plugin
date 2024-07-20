// Copyright 2024 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "QuestComponent.generated.h"

class UQuestSaveObject;
class USaveGame;
class AController;

//Delegate
/** This Delegate will be called if a quest objective has been completed*/
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FQuestObjectiveUpdated);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FStartNavigateQuest);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FQuestFailed, UQuestSystem*, Quest);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FQuestCompleted, UQuestSystem*, Quest);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FQuestAdded, UQuestSystem*, Quest);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), Blueprintable, HideDropdown, ClassGroup = "Quest")
class QUEST_SYSTEM_RUNTIME_API UQuestComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UQuestComponent();

	UPROPERTY(BlueprintReadOnly, Category = "QuestComponent")
		AController* OwningController;

	/** Array of QuestGraph that is Running*/
	UPROPERTY(BlueprintReadWrite, Category = "QuestComponent")
		TArray<UQuestSystemGraph*> QuestGraphs;

	/** Map of Quest ID and attached Quest System That is Running*/
	UPROPERTY(BlueprintReadWrite, Category = "QuestComponent")
		TMap<FName, UQuestSystem*> QuestSystemMap;

	UPROPERTY(BlueprintReadOnly, Category = "QuestComponent")
		bool bQuestLoaded = false;

	UFUNCTION(BlueprintCallable, Category = "QuestComponent")
		bool RunQuestGraphs(TArray<UQuestSystemGraph*>  QuestAssets);

	UFUNCTION(BlueprintCallable, Category = "QuestComponent")
		void ActivateQuest(FName QuestID);

	UFUNCTION(BlueprintCallable, Category = "QuestComponent")
		void RestartQuest(FName QuestID, FName NodeID = NAME_None, bool bStartFromNodeID = false);
	
	UFUNCTION(BlueprintCallable, Category = "QuestComponent")
		void FailQuest(FName QuestID);

	UFUNCTION(BlueprintCallable, Category = "QuestComponent")
		void CompleteQuest(FName QuestID);

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "QuestComponent")
		FQuestObjectiveUpdated OnQuestObjectiveUpdated;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "QuestComponent")
		FStartNavigateQuest OnStartNavigateQuest;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "QuestComponent")
		FQuestFailed OnQuestFailed;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "QuestComponent")
		FQuestCompleted OnQuestCompleted;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "QuestComponent")
		FQuestAdded OnQuestAdded;

	UFUNCTION(BlueprintCallable, Category = "QuestComponent")
		UQuestSystem* FindQuestFromNodeID(FName NodeIDParam) const;

	UFUNCTION(BlueprintCallable, Category = "QuestComponent")
		bool IsQuestActiveFromNodeID(FName NodeID) const;


	UFUNCTION(BlueprintCallable, Category = "QuestComponent")
		bool IsQuestActiveFromQuestID(FName QuestID) const;

		UFUNCTION(BlueprintCallable, Category = "QuestComponent")
		void UpdateQuest();

	UFUNCTION(BlueprintCallable, Category = "QuestComponent")
		void EvaluateNextNodeCondition(UQuestSystem* QuestParam);

	UFUNCTION(BlueprintCallable, Category = "QuestComponent")
		void EvaluateQuestCondition(UQuestSystem* QuestParam);

	UFUNCTION(BlueprintCallable, Category = "QuestComponent")
		void GoToNextNode(UQuestSystem* QuestParam, UQuestSystemGraphNode* NodeParam);

	UFUNCTION(BlueprintCallable, Category = "QuestComponent")
		void BeginNode(UQuestSystem* QuestParam);



	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "QuestComponent")
		void Save(const FString& SaveFileName, int SlotIndex);

	virtual void Save_Implementation(const FString& SaveFileName, int SlotIndex);
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "QuestComponent")
		void Load(const FString& SaveFileName, int SlotIndex);

	virtual void Load_Implementation(const FString& SaveFileName, int SlotIndex);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
