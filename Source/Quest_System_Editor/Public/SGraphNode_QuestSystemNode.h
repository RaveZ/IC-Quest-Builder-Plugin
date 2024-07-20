// Copyright 2024 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SGraphNode.h"
#include "Widgets/SCompoundWidget.h"
#include "EdNode_QuestSystemNode.h"

class UEdNode_QuestSystemNode;
class QUEST_SYSTEM_EDITOR_API SGraphNode_QuestSystemNode : public SGraphNode
{
public:
	SLATE_BEGIN_ARGS(SGraphNode_QuestSystemNode)
	{}
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs, UEdNode_QuestSystemNode* InNode);
	virtual void UpdateGraphNode() override;
	virtual void CreatePinWidgets() override;
	virtual void AddPin(const TSharedRef<SGraphPin>& PinToAdd) override;
	virtual bool IsNameReadOnly() const override;
	virtual FText GetDescription() const;
	virtual EVisibility GetDescriptionVisibility() const;

	
	/** Called when text is being committed to check for validity */
	bool OnVerifyNameTextChanged ( const FText& InText, FText& OutErrorMessage );

	/* Called when text is committed on the node */
	void OnNameTextCommited(const FText& InText, ETextCommit::Type CommitInfo);

	FText GetPinTooltip(UEdGraphPin* GraphPinObj) const;

	virtual FSlateColor GetBorderBackgroundColor() const;
	virtual FSlateColor GetHighlightColor() const;
	virtual FSlateColor GetBackgroundColor() const;

	virtual EVisibility GetDragOverMarkerVisibility() const;

	/** shows red marker when search failed*/
	EVisibility GetDebuggerSearchFailedMarkerVisibility() const;

	virtual const FSlateBrush* GetNameIcon() const;

protected:
	TSharedPtr<SHorizontalBox> OutputPinBox;


	virtual EVisibility GetBlueprintIconVisibility() const;
};
