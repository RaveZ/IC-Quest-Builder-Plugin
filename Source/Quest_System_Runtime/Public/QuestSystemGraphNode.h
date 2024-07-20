// Copyright 2024 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "QuestSystemGraphNode.generated.h"

class UQuestSystemGraph;
class UQuestSystem;
class UQuestCondition;
class UQuestEvent;
class UQuestSystemGraphEdge;

UENUM(BlueprintType)
enum class ENodeLimits : uint8
{
	Unlimited,
	Limited
};



UCLASS(Blueprintable, EditInlineNew)
class QUEST_SYSTEM_RUNTIME_API UQuestSystemGraphNode : public UObject
{
	GENERATED_BODY()

public:
	UQuestSystemGraphNode();
	virtual ~UQuestSystemGraphNode();


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Detail", meta = (MultiLine = true))
		FText Description;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Detail")
		FName ID;

	UPROPERTY(Instanced, EditAnywhere, BlueprintReadOnly, Category = "Conditions")
		TArray<UQuestCondition*> Conditions;
	
	UPROPERTY(Instanced, EditAnywhere, BlueprintReadOnly, Category = "Events")
		TArray<UQuestEvent*> Events;

	UPROPERTY(BlueprintReadOnly, Category = "QuestSystemGraphNode")
		UQuestSystemGraph* QuestGraph;

	UPROPERTY(BlueprintReadOnly, Category = "QuestSystemGraphNode")
		UQuestSystem* QuestSystem;

	UPROPERTY(BlueprintReadOnly, Category = "QuestSystemGraphNode")
		TArray<UQuestSystemGraphNode*> ParentNodes;

	UPROPERTY(BlueprintReadOnly, Category = "QuestSystemGraphNode")
		TArray<UQuestSystemGraphNode*> ChildrenNodes;

	UPROPERTY(BlueprintReadOnly, Category = "QuestSystemGraphNode")
		TMap<UQuestSystemGraphNode*, UQuestSystemGraphEdge*> Edges;


	UFUNCTION(BlueprintCallable, Category = "QuestSystemGraphNode")
		virtual UQuestSystemGraphEdge* GetEdge(UQuestSystemGraphNode* ChildNode);

	UFUNCTION(BlueprintCallable, Category = "QuestSystemGraphNode")
		bool IsLeafNode() const;


	UFUNCTION(BlueprintCallable, Category = "QuestSystemGraphNode")
		AController* GetOwningController() const;

	UFUNCTION(BlueprintCallable, Category = "QuestSystemGraphNode")
		UQuestComponent* GetQuestComponent() const;

	UFUNCTION(BlueprintCallable, Category = "QuestSystemGraphNode")
		UQuestSystemGraph* GetOwningQuestGraph() const;

	UFUNCTION(BlueprintCallable, Category = "QuestSystemGraphNode")
		UQuestSystem* GetOwningQuestSystem() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "QuestSystemGraphNode")
		FText GetDescription() const;


	virtual FText GetDescription_Implementation() const;


	/** returns short name of object's class (BTTaskNode_Wait -> Wait) */
	static FString GetShortTypeName(const UObject* Ob);



	//////////////////////////////////////////////////////////////////////////
#if WITH_EDITORONLY_DATA


	UPROPERTY(BlueprintReadOnly, Category = "QuestSystemGraphNode")
		TSubclassOf<UQuestSystemGraph> CompatibleGraphType;

	UPROPERTY(BlueprintReadOnly, Category = "QuestSystemGraphNode")
		FLinearColor BackgroundColor;


	UPROPERTY(BlueprintReadOnly, Category = "QuestSystemGraphNode")
		ENodeLimits ParentLimitType;

	UPROPERTY(BlueprintReadOnly, Category = "QuestSystemGraphNode", meta = (ClampMin = "0", EditCondition = "ParentLimitType == ENodeLimits::Limited", EditConditionHides))
		int32 ParentLimit;

	UPROPERTY(BlueprintReadOnly, Category = "QuestSystemGraphNode")
		ENodeLimits ChildrenLimitType;

	UPROPERTY(BlueprintReadOnly, Category = "QuestSystemGraphNode", meta = (ClampMin = "0", EditCondition = "ChildrenLimitType == ENodeLimits::Limited", EditConditionHides))
		int32 ChildrenLimit;



#endif

#if WITH_EDITOR
	virtual bool IsNameEditable() const;

	virtual FLinearColor GetBackgroundColor() const;

	virtual FText GetNodeTitle() const;

	virtual FText GetNodeDescription() const;

	virtual void SetNodeTitle(const FText& NewTitle);

	virtual bool CanCreateConnection(UQuestSystemGraphNode* Other, FText& ErrorMessage);

	virtual bool CanCreateConnectionTo(UQuestSystemGraphNode* Other, int32 NumberOfChildrenNodes, FText& ErrorMessage);
	virtual bool CanCreateConnectionFrom(UQuestSystemGraphNode* Other, int32 NumberOfParentNodes, FText& ErrorMessage);
#endif
};
