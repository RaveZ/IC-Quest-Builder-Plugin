// Copyright 2024 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Delegates/Delegate.h"
#include "QuestSystemGraph.h"
#include "Factories/Factory.h"
#include "UObject/Object.h"
#include "UObject/WeakObjectPtrTemplates.h"
#include "WorkflowOrientedApp/WorkflowTabFactory.h"
#include "WorkflowOrientedApp/WorkflowTabManager.h"
#include "WorkflowOrientedApp/WorkflowUObjectDocuments.h"
#include "QuestEditorFactory.generated.h"


#define LOCTEXT_NAMESPACE "QuestEditor"

/////////////////////////////////////////////////////
// FLocalKismetCallbacks

struct FLocalKismetCallbacks
{
	static FText GetObjectName(UObject* Object)
	{
		return (Object != NULL) ? FText::FromString(Object->GetName()) : LOCTEXT("UnknownObjectName", "UNKNOWN");
	}

	static FText GetGraphDisplayName(const UEdGraph* Graph)
	{
		if (Graph)
		{
			if (const UEdGraphSchema* Schema = Graph->GetSchema())
			{
				FGraphDisplayInfo Info;
				Schema->GetGraphDisplayInformation(*Graph, /*out*/ Info);

				return Info.DisplayName;
			}
			else
			{
				// if we don't have a schema, we're dealing with a malformed (or incomplete graph)...
				// possibly in the midst of some transaction - here we return the object's outer path 
				// so we can at least get some context as to which graph we're referring
				return FText::FromString(Graph->GetPathName());
			}
		}

		return LOCTEXT("UnknownGraphName", "UNKNOWN");
	}
};

struct FQuestGraphEditorSummoner : public FDocumentTabFactoryForObjects<UEdGraph>
{
public:
	DECLARE_DELEGATE_RetVal_TwoParams(TSharedRef<SGraphEditor>, FOnCreateGraphEditorWidget, TSharedRef<FTabInfo>, UEdGraph*);
public:
	FQuestGraphEditorSummoner(TSharedPtr<class FAssetEditor_QuestSystem> InQuestEditorPtr, FOnCreateGraphEditorWidget CreateGraphEditorWidgetCallback);

	virtual void OnTabActivated(TSharedPtr<SDockTab> Tab) const override;

	virtual void OnTabBackgrounded(TSharedPtr<SDockTab> Tab) const override;

	virtual void OnTabRefreshed(TSharedPtr<SDockTab> Tab) const override;

	virtual void SaveState(TSharedPtr<SDockTab> Tab, TSharedPtr<FTabPayload> Payload) const override;

protected:
	virtual TAttribute<FText> ConstructTabNameForObject(UEdGraph* DocumentID) const override
	{
		// Extract the name of the graph from DocumentID (replace with your logic)
		FString GraphName = DocumentID->GetName();
		// Create FText with the extracted name
		FText TabName = FText::FromString(GraphName);

		// Return TAttribute containing the constructed FText
		return TAttribute<FText>(TabName);
	}

	virtual TSharedRef<SWidget> CreateTabBodyForObject(const FWorkflowTabSpawnInfo& Info, UEdGraph* DocumentID) const override;

	virtual const FSlateBrush* GetTabIconForObject(const FWorkflowTabSpawnInfo& Info, UEdGraph* DocumentID) const override;

	//virtual TSharedRef<FGenericTabHistory> CreateTabHistoryNode(TSharedPtr<FTabPayload> Payload) override;

protected:
	TWeakPtr<class FAssetEditor_QuestSystem> QuestEditorPtr;
	FOnCreateGraphEditorWidget OnCreateGraphEditorWidget;
};
/**
 * 
 */
UCLASS()
class QUEST_SYSTEM_EDITOR_API UQuestEditorFactory : public UFactory
{
	GENERATED_BODY()
public:
	UQuestEditorFactory();
	virtual ~UQuestEditorFactory();

	UPROPERTY(EditAnywhere, Category = DataAsset)	
		TSubclassOf<UQuestSystemGraph> QuestSystemGraphClass;

	virtual bool ConfigureProperties() override;
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;

};


#undef LOCTEXT_NAMESPACE