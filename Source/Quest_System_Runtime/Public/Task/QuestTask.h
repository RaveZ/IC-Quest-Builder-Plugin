// Copyright 2024 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Tickable.h"
#include "UObject/NoExportTypes.h"
#include "QuestTask.generated.h"


class UQuestSystemGraph;
class UQuestSystem;
class UQuestComponent;
class UQuestSystemGraphNode_Objective;




UENUM(BlueprintType)
enum class ETaskState : uint8
{
	E_Active		UMETA(DisplayName = "ACTIVE"),
	E_Complete		UMETA(DisplayName = "COMPLETE"),
};
UCLASS(Blueprintable, EditInlineNew, HideDropdown, HideCategories = ("Hidden"))
class QUEST_SYSTEM_RUNTIME_API UQuestTask : public UObject, public FTickableGameObject
{
	GENERATED_BODY()

public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Task", meta = (MultiLine = true))
		FText Description;

	/**mark the task as optional and skip this task if the other non - optional task has been completed */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Task")
		bool bOptional = false;

	/**Hide the task from widget*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Task")
		bool bHidden = false;
	
	/**This task use amount required to complete task?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task", meta = (DisplayName = "bUseAmount?"))
		bool bUseAmount = false;

	/**Amount required for your task*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task", meta = (EditCondition = "bUseAmount == true", EditConditionHides))
		int RequiredAmount = 1;

	/**Is Task Tickable?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task", meta = (DisplayName = "TaskTickable?"))
		bool bIsTaskTickable = false;

	UPROPERTY(BlueprintReadOnly, Category = "Task")
		UQuestSystemGraph* QuestGraph;

	UPROPERTY(BlueprintReadOnly, Category = "Task")
		UQuestSystem* QuestSystem;


	UPROPERTY(BlueprintReadOnly, Category = "Task")
		UQuestSystemGraphNode_Objective* ObjectiveNode;

	UPROPERTY(BlueprintReadWrite, Category = "Task")
		ETaskState TaskState = ETaskState::E_Active;

	UPROPERTY(BlueprintReadWrite, Category = "Task")
		int CurrentAmount = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Task")
		float DistanceToTarget;

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Task")
		void TaskBegin();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Task")
		void TaskTick(float DeltaTime);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Task")
		void OnTaskCompleted();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Task")
		void OnObjectiveFailed();

	UFUNCTION(BlueprintCallable, Category = "Task")
		bool CompleteTask();

	/**Fail The objective node*/
	UFUNCTION(BlueprintCallable, Category = "Task")
		void FailObjective();

	UFUNCTION(BlueprintCallable, Category = "Task")
		AController* GetOwningController() const;

	UFUNCTION(BlueprintCallable, Category = "Task")
		UQuestComponent* GetQuestComponent() const;

	UFUNCTION(BlueprintCallable, Category = "Task")
		UQuestSystemGraph* GetOwningQuestGraph() const;

	UFUNCTION(BlueprintCallable, Category = "Task")
		UQuestSystem* GetOwningQuestSystem() const;

	/**Add amount and check if the required amount has fulfilled, then complete the task*/
	UFUNCTION(BlueprintCallable, Category = "Task")
		void AddAmount(int AmountToAdd);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Task")
		FText GetDescription() const;

	virtual FText GetDescription_Implementation() const;

	// FTickableGameObject Begin
	virtual void Tick(float DeltaTime) override;

	virtual bool IsTickable() const override { return bIsTaskTickable; }

	virtual ETickableTickType GetTickableTickType() const override
	{
		return ETickableTickType::Conditional;
	}
	virtual TStatId GetStatId() const override
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(FMyTickableThing, STATGROUP_Tickables);
	}



private:
	// The last frame number we were ticked.
	// We don't want to tick multiple times per frame 
	uint32 LastFrameNumberWeTicked = INDEX_NONE;
};
