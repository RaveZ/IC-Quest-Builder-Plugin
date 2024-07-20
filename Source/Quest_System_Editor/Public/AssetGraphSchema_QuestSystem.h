// Copyright 2024 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraphSchema.h"
#include "AssetGraphSchema_QuestSystem.generated.h"

/** Action to add a node to the graph */

class UEdNode_QuestSystemNode;
class UEdNode_QuestSystemEdge;

/** Action to add a node to the graph */
USTRUCT()
struct QUEST_SYSTEM_EDITOR_API FAssetSchemaAction_QuestSystem_NewNode : public FEdGraphSchemaAction
{
	GENERATED_USTRUCT_BODY();

public:
	FAssetSchemaAction_QuestSystem_NewNode() : NodeTemplate(nullptr) {}

	FAssetSchemaAction_QuestSystem_NewNode(const FText& InNodeCategory, const FText& InMenuDesc, const FText& InToolTip, const int32 InGrouping)
		: FEdGraphSchemaAction(InNodeCategory, InMenuDesc, InToolTip, InGrouping), NodeTemplate(nullptr) {}

	virtual UEdGraphNode* PerformAction(class UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode = true) override;
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;

	UEdNode_QuestSystemNode* NodeTemplate;
};

USTRUCT()
struct QUEST_SYSTEM_EDITOR_API FAssetSchemaAction_QuestSystem_NewEdge : public FEdGraphSchemaAction
{
	GENERATED_USTRUCT_BODY();

public:
	FAssetSchemaAction_QuestSystem_NewEdge() : NodeTemplate(nullptr) {}

	FAssetSchemaAction_QuestSystem_NewEdge(const FText& InNodeCategory, const FText& InMenuDesc, const FText& InToolTip, const int32 InGrouping)
		: FEdGraphSchemaAction(InNodeCategory, InMenuDesc, InToolTip, InGrouping), NodeTemplate(nullptr) {}

	virtual UEdGraphNode* PerformAction(class UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode = true) override;
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;

	UEdNode_QuestSystemEdge* NodeTemplate;
};

/** Reference to a  event graph (only used in 'docked' palette) */
USTRUCT()
struct QUEST_SYSTEM_EDITOR_API FAssetSchemaAction_QuestSystemGraph : public FEdGraphSchemaAction
{
	GENERATED_USTRUCT_BODY()

	// Simple type info
	static FName StaticGetTypeId() { static FName Type("FAssetSchemaAction_QuestSystemGraph"); return Type; }
	virtual FName GetTypeId() const override { return StaticGetTypeId(); }

	/** Name of function or class */
	FName FuncName;


	/** The associated editor graph for this schema */
	UEdGraph* EdGraph;

	FAssetSchemaAction_QuestSystemGraph()
		: FEdGraphSchemaAction()
	{}

	FAssetSchemaAction_QuestSystemGraph(FText InNodeCategory, FText InMenuDesc, FText InToolTip, const int32 InGrouping, const int32 InSectionID = 0)
		: FEdGraphSchemaAction(MoveTemp(InNodeCategory), MoveTemp(InMenuDesc), MoveTemp(InToolTip), InGrouping, FText(), InSectionID)
		, EdGraph(nullptr)
	{}

	// FEdGraphSchemaAction interface
	//virtual bool IsParentable() const override { return true; }
	//virtual void MovePersistentItemToCategory(const FText& NewCategoryName) override;
	//virtual int32 GetReorderIndexInContainer() const override;
	//virtual bool ReorderToBeforeAction(TSharedRef<FEdGraphSchemaAction> OtherAction) override;
	//virtual FEdGraphSchemaActionDefiningObject GetPersistentItemDefiningObject() const override;
	// End of FEdGraphSchemaAction interface

	//UFunction* GetFunction() const;
	//UBlueprint* GetSourceBlueprint() const;
};


UCLASS(MinimalAPI)
class UAssetGraphSchema_QuestSystem : public UEdGraphSchema
{
	GENERATED_BODY()
	public:

	void GetBreakLinkToSubMenuActions(class UToolMenu* Menu, class UEdGraphPin* InGraphPin);

	//~ Begin EdGraphSchema Interface
	virtual void CreateDefaultNodesForGraph(UEdGraph& Graph) const override;
	virtual EGraphType GetGraphType(const UEdGraph* TestEdGraph) const override;

 	virtual void GetGraphContextActions(FGraphContextMenuBuilder& ContextMenuBuilder) const override;

	virtual void GetContextMenuActions(class UToolMenu* Menu, class UGraphNodeContextMenuContext* Context) const override;

 	virtual const FPinConnectionResponse CanCreateConnection(const UEdGraphPin* A, const UEdGraphPin* B) const override;

	virtual bool TryCreateConnection(UEdGraphPin* A, UEdGraphPin* B) const override;
	virtual bool CreateAutomaticConversionNodeAndConnections(UEdGraphPin* A, UEdGraphPin* B) const override;

	virtual class FConnectionDrawingPolicy* CreateConnectionDrawingPolicy(int32 InBackLayerID, int32 InFrontLayerID, float InZoomFactor, const FSlateRect& InClippingRect, class FSlateWindowElementList& InDrawElements, class UEdGraph* InGraphObj) const override;

 	virtual FLinearColor GetPinTypeColor(const FEdGraphPinType& PinType) const override;

 	virtual void BreakNodeLinks(UEdGraphNode& TargetNode) const override;

 	virtual void BreakPinLinks(UEdGraphPin& TargetPin, bool bSendsNodeNotifcation) const override;

	virtual void BreakSinglePinLink(UEdGraphPin* SourcePin, UEdGraphPin* TargetPin) const override;

	virtual UEdGraphPin* DropPinOnNode(UEdGraphNode* InTargetNode, const FName& InSourcePinName, const FEdGraphPinType& InSourcePinType, EEdGraphPinDirection InSourcePinDirection) const override;

	virtual bool SupportsDropPinOnNode(UEdGraphNode* InTargetNode, const FEdGraphPinType& InSourcePinType, EEdGraphPinDirection InSourcePinDirection, FText& OutErrorMessage) const override;

	virtual bool IsCacheVisualizationOutOfDate(int32 InVisualizationCacheID) const override;

	virtual int32 GetCurrentVisualizationCacheID() const override;

	virtual void ForceVisualizationCacheClear() const override;

protected:
	static TSharedPtr<FAssetSchemaAction_QuestSystem_NewNode> AddNewNodeAction(FGraphActionListBuilderBase& ContextMenuBuilder, const FText& Category, const FText& MenuDesc, const FText& Tooltip);
private:
	static int32 CurrentCacheRefreshID;
};
