// Copyright 2024 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "QuestSystemGraphEdge.generated.h"


class UQuestSystemGraph;
class UQuestSystemGraphNode;

UCLASS(Blueprintable)
class QUEST_SYSTEM_RUNTIME_API UQuestSystemGraphEdge : public UObject
{
	GENERATED_BODY()
public:
	UQuestSystemGraphEdge();
	virtual ~UQuestSystemGraphEdge();

	UPROPERTY(VisibleAnywhere, Category = "GenericGraphNode")
		UQuestSystemGraph* Graph;

	UPROPERTY(BlueprintReadOnly, Category = "GenericGraphEdge")
		UQuestSystemGraphNode* StartNode;

	UPROPERTY(BlueprintReadOnly, Category = "GenericGraphEdge")
		UQuestSystemGraphNode* EndNode;

	UFUNCTION(BlueprintPure, Category = "GenericGraphEdge")
		UQuestSystemGraph* GetGraph() const;

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditDefaultsOnly, Category = "GenericGraphNode_Editor")
		bool bShouldDrawTitle = false;

	UPROPERTY(EditDefaultsOnly, Category = "GenericGraphNode_Editor")
		FText NodeTitle;

	UPROPERTY(EditDefaultsOnly, Category = "GenericGraphEdge")
		FLinearColor EdgeColour = FLinearColor(0.9f, 0.9f, 0.9f, 1.0f);
#endif

#if WITH_EDITOR
	virtual FText GetNodeTitle() const { return NodeTitle; }
	FLinearColor GetEdgeColour() { return EdgeColour; }

	virtual void SetNodeTitle(const FText& NewTitle);
#endif
};
