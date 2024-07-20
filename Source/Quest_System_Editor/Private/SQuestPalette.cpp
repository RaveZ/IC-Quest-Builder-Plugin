// Copyright 2024 Ivan Chandra. All Rights Reserved.


#include "SQuestPalette.h"
#include "Quest_System_EditorStyle.h"
#include "SlateOptMacros.h"	
#include "TutorialMetaData.h"
#include "SMyQuest.h"
#include "Widgets/Text/SInlineEditableTextBlock.h"
#include "AssetGraphSchema_QuestSystem.h"
#include "EdGraph_QuestSystemGraph.h"
#include "QuestSystemGraph.h"
#include "Condition/QuestCondition.h"
#include "Event/QuestEvent.h"


#define LOCTEXT_NAMESPACE "QuestPaletteItem"


BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SQuestPaletteItem::Construct(const FArguments& InArgs, FCreateWidgetForActionData* const InCreateData, TWeakPtr<FAssetEditor_QuestSystem> InQuestEditor)
{
	Construct(InArgs, InCreateData, InQuestEditor.Pin()->GetQuestSystemObj(), InQuestEditor);

}

void SQuestPaletteItem::Construct(const FArguments& InArgs, FCreateWidgetForActionData* const InCreateData, UQuestSystemGraph* InQuestSystemGraph)
{
	Construct(InArgs, InCreateData, InQuestSystemGraph, TWeakPtr<FAssetEditor_QuestSystem>());
}

void SQuestPaletteItem::Construct(const FArguments& InArgs, FCreateWidgetForActionData* const InCreateData, UQuestSystemGraph* InQuestSystemGraph, TWeakPtr<FAssetEditor_QuestSystem> InQuestEditor)
{
	check(InCreateData->Action.IsValid());
	check(InQuestSystemGraph);

	QuestSystemGraph = InQuestSystemGraph;

	bShowClassInTooltip = InArgs._ShowClassInTooltip;

	TSharedPtr<FEdGraphSchemaAction> GraphAction = InCreateData->Action;
	ActionPtr = InCreateData->Action;
	QuestEditorPtr = InQuestEditor;

	// construct the icon widget
	FSlateBrush const* IconBrush = FAppStyle::GetBrush(TEXT("GraphEditor.EventGraph_16x"));
	FSlateBrush const* SecondaryBrush = FAppStyle::GetBrush(TEXT("NoBrush"));
	FSlateColor        IconColor = FSlateColor::UseForeground();
	FSlateColor        SecondaryIconColor = FSlateColor::UseForeground();
	FText			   IconToolTip = GraphAction->GetTooltipDescription();
	FString			   IconDocLink, IconDocExcerpt;
	TSharedRef<SWidget> IconWidget = CreateIconWidget(IconToolTip, IconBrush, IconColor);
	
	// Setup a meta tag for this node
	FTutorialMetaData TagMeta("PaletteItem");
	if (ActionPtr.IsValid())
	{
		TagMeta.Tag = *FString::Printf(TEXT("PaletteItem,%s,%d"), *GraphAction->GetMenuDescription().ToString(), GraphAction->GetSectionID());
		TagMeta.FriendlyName = GraphAction->GetMenuDescription().ToString();
	}

	// construct the text widget
	bool bIsReadOnly = false;
	TSharedRef<SWidget> NameSlotWidget = CreateTextSlotWidget(InCreateData, bIsReadOnly);

	// Create the widget with an icon
	TSharedRef<SHorizontalBox> ActionBox = SNew(SHorizontalBox)
		.AddMetaData<FTutorialMetaData>(TagMeta);


	ActionBox.Get().AddSlot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			IconWidget
		];


	ActionBox.Get().AddSlot()
		.FillWidth(1.f)
		.VAlign(VAlign_Center)
		.Padding(/* horizontal */ 3.0f, /* vertical */ 3.0f)
		[
			NameSlotWidget
		];

	// Now, create the actual widget
	ChildSlot
		[
			ActionBox
		];

}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION



void SQuestPaletteItem::OnDragEnter(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent)
{
	if (QuestEditorPtr.IsValid())
	{
		SGraphPaletteItem::OnDragEnter(MyGeometry, DragDropEvent);
	}
}

TSharedRef<SWidget> SQuestPaletteItem::CreateTextSlotWidget(FCreateWidgetForActionData* const InCreateData, TAttribute<bool> bIsReadOnly)
{
	FName const ActionTypeId = InCreateData->Action->GetTypeId();

	FOnVerifyTextChanged OnVerifyTextChanged;
	FOnTextCommitted     OnTextCommitted;

	// default to our own rename methods
	OnVerifyTextChanged.BindSP(this, &SQuestPaletteItem::OnNameTextVerifyChanged);
	OnTextCommitted.BindSP(this, &SQuestPaletteItem::OnNameTextCommitted);

	// Copy the mouse delegate binding if we want it
	if (InCreateData->bHandleMouseButtonDown)
	{
		MouseButtonDownDelegate = InCreateData->MouseButtonDownDelegate;
	}

	TSharedPtr<SToolTip> ToolTipWidget = ConstructToolTipWidget();

	TSharedPtr<SOverlay> DisplayWidget;
	TSharedPtr<SInlineEditableTextBlock> EditableTextElement;
	SAssignNew(DisplayWidget, SOverlay)
		+ SOverlay::Slot()
		[
			SAssignNew(EditableTextElement, SInlineEditableTextBlock)
				.Text(this, &SQuestPaletteItem::GetDisplayText)
				.HighlightText(InCreateData->HighlightText)
				.ToolTip(ToolTipWidget)
				.OnVerifyTextChanged(OnVerifyTextChanged)
				.OnTextCommitted(OnTextCommitted)
				.IsSelected(InCreateData->IsRowSelectedDelegate)
				.IsReadOnly(false)
		];
	InlineRenameWidget = EditableTextElement.ToSharedRef();

	InCreateData->OnRenameRequest->BindSP(InlineRenameWidget.Get(), &SInlineEditableTextBlock::EnterEditingMode);

	return DisplayWidget.ToSharedRef();
}

FText SQuestPaletteItem::GetDisplayText() const
{
	const UAssetGraphSchema_QuestSystem* QuestSchema = GetDefault<UAssetGraphSchema_QuestSystem>();
	if (MenuDescriptionCache.IsOutOfDate(QuestSchema))
	{
		TSharedPtr< FEdGraphSchemaAction > GraphAction = ActionPtr.Pin();
		FAssetSchemaAction_QuestSystemGraph* QuestGraphAction = (FAssetSchemaAction_QuestSystemGraph*)GraphAction.Get();
		if (UEdGraph_QuestSystemGraph* QuestEdGraph = Cast<UEdGraph_QuestSystemGraph>(QuestGraphAction->EdGraph))
		{
			MenuDescriptionCache.SetCachedText(FText::FromName(QuestEdGraph->QuestSystem->QuestID), QuestSchema);
		}
	}

	return MenuDescriptionCache;
}

bool SQuestPaletteItem::OnNameTextVerifyChanged(const FText& InNewText, FText& OutErrorMessage)
{
	FString TextAsString = InNewText.ToString();

	FName OriginalName;

	FAssetSchemaAction_QuestSystemGraph* QuestGraphAction = (FAssetSchemaAction_QuestSystemGraph*)ActionPtr.Pin().Get();
	if (UEdGraph_QuestSystemGraph* QuestEdGraph = Cast<UEdGraph_QuestSystemGraph>(QuestGraphAction->EdGraph))
	{
		OriginalName = QuestEdGraph->QuestSystem->QuestID;
	}

	for (TObjectIterator<UQuestSystem> It; It; ++It)
	{
		if (UQuestSystem* QuestSystem = Cast<UQuestSystem>(*It))
		{
			if (FName(InNewText.ToString()) == OriginalName)
			{
				return true;
			}
			if (FName(InNewText.ToString()).IsNone())
			{
				OutErrorMessage = LOCTEXT("RenameFailed_NotValid", "Empty ID is Not Valid.");
				return false;
			}
			if (QuestSystem->QuestID == FName(InNewText.ToString()))
			{
				OutErrorMessage = LOCTEXT("RenameFailed_NotValid", "This ID is already used in another Quest.");
				return false;
			}
		}
	}

	UStruct* ValidationScope = nullptr;

	const UEdGraphSchema* Schema = nullptr;

	return true;
}

void SQuestPaletteItem::OnNameTextCommitted(const FText& NewText, ETextCommit::Type InTextCommit)
{

	const FString NewNameString = NewText.ToString();
	const FName NewName = *NewNameString;
	FName OriginalName;

	FAssetSchemaAction_QuestSystemGraph* QuestGraphAction = (FAssetSchemaAction_QuestSystemGraph*)ActionPtr.Pin().Get();
	if (UEdGraph_QuestSystemGraph* QuestEdGraph = Cast<UEdGraph_QuestSystemGraph>(QuestGraphAction->EdGraph))
	{
		OriginalName = QuestEdGraph->QuestSystem->QuestID;
		QuestEdGraph->QuestSystem->QuestID = NewName;
		QuestEdGraph->Rename(*(NewName.ToString()), QuestSystemGraph, REN_DoNotDirty | REN_ForceNoResetLoaders);
		for (TObjectIterator<UQuestCondition> It; It; ++It)
		{
			if (UQuestCondition* QuestCondition = Cast<UQuestCondition>(*It))
			{
				if (QuestCondition->QuestID == OriginalName)
				{
					QuestCondition->QuestID = NewName;
				}
			}
		}

		for (TObjectIterator<UQuestEvent> It; It; ++It)
		{
			if (UQuestEvent* QuestEvent = Cast<UQuestEvent>(*It))
			{
				if (QuestEvent->QuestID == OriginalName)
				{
					QuestEvent->QuestID = NewName;
				}
			}
		}
	}
	QuestEditorPtr.Pin()->GetMyQuestWidget()->Refresh();

	QuestEditorPtr.Pin()->GetQuestSystemObj()->Modify();
}

TSharedPtr<SToolTip> SQuestPaletteItem::ConstructToolTipWidget() const
{

	return TSharedPtr<SToolTip>();
}

FText SQuestPaletteItem::GetToolTipText() const
{
	return FText::FromString(TEXT(""));
}

void SQuestPalette::Construct(const FArguments& InArgs, TWeakPtr<FAssetEditor_QuestSystem> InQuestEditor)
{
}

void SQuestPalette::OnSplitterResized() const
{
}

#undef LOCTEXT_NAMESPACE