// Copyright 2024 Ivan Chandra. All Rights Reserved.


#include "SMyQuest.h"
#include "Framework/Commands/GenericCommands.h"
#include "GraphEditorActions.h"
#include "SGraphActionMenu.h"
#include "SGraphPalette.h"
#include "QuestSystemEditorUtils.h"
#include "Quest_System_EditorCommands.h"
#include "Widgets/Input/SSearchBox.h"
#include "Quest_System_EditorCommands.h"
#include "SlateOptMacros.h"
#include "SQuestPalette.h"


#define LOCTEXT_NAMESPACE "MyQuest"

void FMyQuestCommands::RegisterCommands()
{
	UI_COMMAND(OpenGraph, "Open Graph", "Opens up this function, macro, or event graph's graph panel up.", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(OpenGraphInNewTab, "Open in New Tab", "Opens up this function, macro, or event graph's graph panel up in a new tab. Hold down Ctrl and double click for shortcut.", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(OpenExternalGraph, "Open External Graph", "Opens up this external graph's graph panel in its own asset editor", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(FocusNode, "Focus", "Focuses on the associated node", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(FocusNodeInNewTab, "Focus in New Tab", "Focuses on the associated node in a new tab", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(DeleteEntry, "Delete", "Deletes this function or variable from this Quest Editor.", EUserInterfaceActionType::Button, FInputChord(EKeys::Platform_Delete));
}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SMyQuest::Construct(const FArguments& InArgs, TWeakPtr<FAssetEditor_QuestSystem> InQuestEditor, const UQuestSystemGraph* InQuestSystemGraph)
{
	bNeedsRefresh = false;
	bShowReplicatedVariablesOnly = false;

	QuestEditorPtr = InQuestEditor;
	EdGraph = nullptr;

	TSharedPtr<SWidget> ToolbarBuilderWidget = TSharedPtr<SWidget>();

	if (InQuestEditor.IsValid())
	{
		QuestSystemGraph = QuestEditorPtr.Pin()->GetQuestSystemObj();

		CommandList = MakeShareable(new FUICommandList);

		CommandList->Append(InQuestEditor.Pin()->GetToolkitCommands());

		CommandList->MapAction(FMyQuestCommands::Get().OpenGraph,
			FExecuteAction::CreateSP(this, &SMyQuest::OnOpenGraph),
			FCanExecuteAction(), FIsActionChecked(),
			FIsActionButtonVisible::CreateSP(this, &SMyQuest::CanOpenGraph));

		CommandList->MapAction(FMyQuestCommands::Get().OpenGraphInNewTab,
			FExecuteAction::CreateSP(this, &SMyQuest::OnOpenGraphInNewTab),
			FCanExecuteAction(), FIsActionChecked(),
			FIsActionButtonVisible::CreateSP(this, &SMyQuest::CanOpenGraph));

		CommandList->MapAction(FMyQuestCommands::Get().OpenExternalGraph,
			FExecuteAction::CreateSP(this, &SMyQuest::OnOpenExternalGraph),
			FCanExecuteAction(), FIsActionChecked(),
			FIsActionButtonVisible::CreateSP(this, &SMyQuest::CanOpenExternalGraph));

		CommandList->MapAction(FMyQuestCommands::Get().FocusNode,
			FExecuteAction::CreateSP(this, &SMyQuest::OnFocusNode),
			FCanExecuteAction(), FIsActionChecked(),
			FIsActionButtonVisible::CreateSP(this, &SMyQuest::CanFocusOnNode));

		CommandList->MapAction(FMyQuestCommands::Get().FocusNodeInNewTab,
			FExecuteAction::CreateSP(this, &SMyQuest::OnFocusNodeInNewTab),
			FCanExecuteAction(), FIsActionChecked(),
			FIsActionButtonVisible::CreateSP(this, &SMyQuest::CanFocusOnNode));


		CommandList->MapAction(FMyQuestCommands::Get().DeleteEntry,
			FExecuteAction::CreateSP(this, &SMyQuest::OnDeleteEntry),
			FCanExecuteAction::CreateSP(this, &SMyQuest::CanDeleteEntry));

		CommandList->MapAction(FGenericCommands::Get().Duplicate,
			FExecuteAction::CreateSP(this, &SMyQuest::OnDuplicateAction),
			FCanExecuteAction::CreateSP(this, &SMyQuest::CanDuplicateAction),
			FIsActionChecked(),
			FIsActionButtonVisible::CreateSP(this, &SMyQuest::IsDuplicateActionVisible));

		ToolbarBuilderWidget = SNullWidget::NullWidget;

		CommandList->MapAction(FGenericCommands::Get().Rename,
			FExecuteAction::CreateSP(this, &SMyQuest::OnRequestRenameOnActionNode),
			FCanExecuteAction::CreateSP(this, &SMyQuest::CanRequestRenameOnActionNode));

		CommandList->MapAction(FGenericCommands::Get().Copy,
			FExecuteAction::CreateSP(this, &SMyQuest::OnCopy),
			FCanExecuteAction::CreateSP(this, &SMyQuest::CanCopy));

		CommandList->MapAction(FGenericCommands::Get().Cut,
			FExecuteAction::CreateSP(this, &SMyQuest::OnCut),
			FCanExecuteAction::CreateSP(this, &SMyQuest::CanCut));

		CommandList->MapAction(FGenericCommands::Get().Paste,
			FExecuteAction::CreateSP(this, &SMyQuest::OnPasteGeneric),
			FCanExecuteAction(), FIsActionChecked(),
			FIsActionButtonVisible::CreateSP(this, &SMyQuest::CanPasteGeneric));

	}
	else
	{
		// we're in read only mode when there's no quest editor:
		QuestSystemGraph = const_cast<UQuestSystemGraph*>(InQuestSystemGraph);
		check(QuestSystemGraph);
		ToolbarBuilderWidget = SNew(SBox);
	}


	SAssignNew(FilterBox, SSearchBox)
		.OnTextChanged(this, &SMyQuest::OnFilterTextChanged);

	// create the main action list piece of this widget
	SAssignNew(GraphActionMenu, SGraphActionMenu, false)
		.OnGetFilterText(this, &SMyQuest::GetFilterText)
		.OnCreateWidgetForAction(this, &SMyQuest::OnCreateWidgetForAction)
		.OnCollectAllActions(this, &SMyQuest::CollectAllActions)
		.OnCollectStaticSections(this, &SMyQuest::CollectStaticSections)
		.OnActionDragged(this, &SMyQuest::OnActionDragged)
		.OnCategoryDragged(this, &SMyQuest::OnCategoryDragged)
		.OnActionSelected(this, &SMyQuest::OnGlobalActionSelected)
		.OnActionDoubleClicked(this, &SMyQuest::OnActionDoubleClicked)
		.OnContextMenuOpening(this, &SMyQuest::OnContextMenuOpening)
		.OnCategoryTextCommitted(this, &SMyQuest::OnCategoryNameCommitted)
		.OnCanRenameSelectedAction(this, &SMyQuest::CanRequestRenameOnActionNode)
		.OnGetSectionTitle(this, &SMyQuest::OnGetSectionTitle)
		.OnGetSectionWidget(this, &SMyQuest::OnGetSectionWidget)
		.OnActionMatchesName(this, &SMyQuest::HandleActionMatchesName)
		.AlphaSortItems(false)
		.UseSectionStyling(true);

	ChildSlot
	[
		SNew(SVerticalBox)

			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SBorder)
					.Padding(4.0f)
					.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
					.AddMetaData<FTagMetaData>(FTagMetaData(TEXT("MyQuestPanel")))
					[
						SNew(SVerticalBox)

						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							ToolbarBuilderWidget.ToSharedRef()
						]

						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(SHorizontalBox)

							+ SHorizontalBox::Slot()
							.FillWidth(1.0f)
							.VAlign(VAlign_Center)
							[
								FilterBox.ToSharedRef()
							]
						]
					]
			]

			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			[
				GraphActionMenu.ToSharedRef()
			]
	];

	//ResetLastPinType();

	if (!QuestEditorPtr.IsValid())
	{
		Refresh();
	}

	TMap<int32, bool> ExpandedSections;
	ExpandedSections.Add(QuestSectionID::QUESTGRAPH, true);

	GraphActionMenu->SetSectionExpansion(ExpandedSections);

	FCoreUObjectDelegates::OnObjectPropertyChanged.AddRaw(this, &SMyQuest::OnObjectPropertyChanged);

}

SMyQuest::~SMyQuest()
{
	FCoreUObjectDelegates::OnObjectPropertyChanged.RemoveAll(this);
}
void SMyQuest::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	if (bNeedsRefresh)
	{
		Refresh();
	}
}

FReply SMyQuest::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	if (CommandList.IsValid() && CommandList->ProcessCommandBindings(InKeyEvent))
	{
		return FReply::Handled();
	}
	return FReply::Unhandled();
}

template<class SchemaActionType> SchemaActionType* SelectionAsType(const TSharedPtr< SGraphActionMenu >& GraphActionMenu)
{
	TArray<TSharedPtr<FEdGraphSchemaAction> > SelectedActions;
	GraphActionMenu->GetSelectedActions(SelectedActions);

	SchemaActionType* Selection = NULL;

	TSharedPtr<FEdGraphSchemaAction> SelectedAction(SelectedActions.Num() > 0 ? SelectedActions[0] : NULL);
	if (SelectedAction.IsValid() &&
		SelectedAction->GetTypeId() == SchemaActionType::StaticGetTypeId())
	{
		// TODO Why not? StaticCastSharedPtr<>()

		Selection = (SchemaActionType*)SelectedActions[0].Get();
	}

	return Selection;
}

FAssetSchemaAction_QuestSystemGraph* SMyQuest::SelectionAsGraph() const
{
	return SelectionAsType<FAssetSchemaAction_QuestSystemGraph>(GraphActionMenu);
}
void SMyQuest::OnRequestRenameOnActionNode()
{
	// Attempt to rename in both menus, only one of them will have anything selected
	GraphActionMenu->OnRequestRenameOnActionNode();
}
void SMyQuest::OnPasteGeneric()
{
}
bool SMyQuest::CanPasteGeneric()
{
	return false;
}
void SMyQuest::Refresh()
{
	bNeedsRefresh = false;

	// Conform to our interfaces here to ensure we catch any newly added functions
	//FBlueprintEditorUtils::ConformImplementedInterfaces(GetQuestSystemObj());

	GraphActionMenu->RefreshAllActions(/*bPreserveExpansion=*/ true);
}

bool SMyQuest::SelectionIsCategory() const
{
	return !SelectionHasContextMenu();
}

void SMyQuest::SelectItemByName(const FName& ItemName, ESelectInfo::Type SelectInfo, int32 SectionId, bool bIsCategory)
{
	// Check if the graph action menu is being told to clear
	if (ItemName == NAME_None)
	{
		ClearGraphActionMenuSelection();
	}
	else
	{
		// Attempt to select the item in the main graph action menu
		const bool bSucceededAtSelecting = GraphActionMenu->SelectItemByName(ItemName, SelectInfo, SectionId, bIsCategory);
		if (!bSucceededAtSelecting)
		{
			// We failed to select the item, maybe because it was filtered out?
			// Reset the item filter and try again (we don't do this first because someone went to the effort of typing
			// a filter and probably wants to keep it unless it is getting in the way, as it just has)
			//OnResetItemFilter();
			GraphActionMenu->SelectItemByName(ItemName, SelectInfo, SectionId, bIsCategory);
		}
	}
}

void SMyQuest::ClearGraphActionMenuSelection()
{
	GraphActionMenu->SelectItemByName(NAME_None);
}


TSharedRef<SWidget> SMyQuest::OnCreateWidgetForAction(FCreateWidgetForActionData* const InCreateData)
{
	return QuestEditorPtr.IsValid() ? SNew(SQuestPaletteItem, InCreateData, QuestEditorPtr.Pin()) : SNew(SQuestPaletteItem, InCreateData, GetQuestSystemObj());
}

void SMyQuest::CollectAllActions(FGraphActionListBuilderBase& OutAllActions)
{
	const UAssetGraphSchema_QuestSystem* QuestSystemSchema = GetDefault<UAssetGraphSchema_QuestSystem>();

	UQuestSystemGraph* QuestSystemObj = GetQuestSystemObj();
	check(QuestSystemObj);

	TSharedPtr<FAssetEditor_QuestSystem> QuestSystemEditor = QuestEditorPtr.Pin();

	EFieldIteratorFlags::SuperClassFlags FieldIteratorSuperFlag = EFieldIteratorFlags::IncludeSuper;
	/*if (ShowUserVarsOnly())
	{
		FieldIteratorSuperFlag = EFieldIteratorFlags::ExcludeSuper;
	}*/

	//bool bShowReplicatedOnly = IsShowingReplicatedVariablesOnly();

	// Initialise action sorting instance
	//FGraphActionSort SortList(QuestSystemObj->CategorySorting);

	if (QuestSystemEditor.IsValid())
	{
		// Grab ubergraph pages
		for (int32 i = 0; i < QuestSystemObj->QuestGraphPages.Num(); i++)
		{
			UEdGraph* Graph = QuestSystemObj->QuestGraphPages[i];
			check(Graph);

			FGraphDisplayInfo DisplayInfo;
			Graph->GetSchema()->GetGraphDisplayInformation(*Graph, DisplayInfo);

			TSharedPtr<FAssetSchemaAction_QuestSystemGraph> NeUbergraphAction = MakeShareable(new FAssetSchemaAction_QuestSystemGraph(FText::GetEmpty(), DisplayInfo.PlainName, DisplayInfo.Tooltip, 2, QuestSectionID::QUESTGRAPH));
			NeUbergraphAction->FuncName = Graph->GetFName();
			NeUbergraphAction->EdGraph = Graph;
			OutAllActions.AddAction(NeUbergraphAction);

			//GetChildGraphs(Graph, NeUbergraphAction->GetSectionID(), SortList);
			//GetChildEvents(Graph, NeUbergraphAction->GetSectionID(), SortList);
		}
	}
}
void SMyQuest::CollectStaticSections(TArray<int32>& StaticSectionIDs)
{
	StaticSectionIDs.Add(QuestSectionID::QUESTGRAPH);
}
FReply SMyQuest::OnActionDragged(const TArray<TSharedPtr<FEdGraphSchemaAction>>& InActions, const FPointerEvent& MouseEvent)
{
	return FReply::Handled();
}
FReply SMyQuest::OnCategoryDragged(const FText& InCategory, const FPointerEvent& MouseEvent)
{
	return FReply::Handled();
}
void SMyQuest::OnActionSelected(const TArray<TSharedPtr<FEdGraphSchemaAction>>& InActions)
{
	TSharedPtr<FEdGraphSchemaAction> InAction(InActions.Num() > 0 ? InActions[0] : NULL);
	if (InAction.IsValid())
	{
		FAssetSchemaAction_QuestSystemGraph* GraphAction = (FAssetSchemaAction_QuestSystemGraph*)InAction.Get();
		QuestEditorPtr.Pin()->FocusedEdGraph = GraphAction->EdGraph;

		if (GraphAction->EdGraph)
		{
			TSharedPtr<FAssetEditor_QuestSystem> QuestEditor = QuestEditorPtr.Pin();

			if (UEdGraph_QuestSystemGraph* QuestGraph = Cast<UEdGraph_QuestSystemGraph>(QuestEditor->FocusedEdGraph))
			{
				QuestEditor->PropertyWidget->SetObject(QuestGraph->QuestSystem);
			}
		}
	}
}

void SMyQuest::OnGlobalActionSelected(const TArray<TSharedPtr<FEdGraphSchemaAction>>& InActions, ESelectInfo::Type InSelectionType)
{
	if (InSelectionType == ESelectInfo::OnMouseClick || InSelectionType == ESelectInfo::OnKeyPress || InSelectionType == ESelectInfo::OnNavigation || InActions.Num() == 0)
	{
		OnActionSelected(InActions);

	}
}

void SMyQuest::OnActionDoubleClicked(const TArray<TSharedPtr<FEdGraphSchemaAction>>& InActions)
{
	if (!QuestEditorPtr.IsValid())
	{
		return;
	}

	TSharedPtr<FEdGraphSchemaAction> InAction(InActions.Num() > 0 ? InActions[0] : NULL);
	ExecuteAction(InAction);

}

void SMyQuest::ExecuteAction(TSharedPtr<FEdGraphSchemaAction> InAction)
{
	// Force it to open in a new document if shift is pressed
	const bool bIsShiftPressed = FSlateApplication::Get().GetModifierKeys().IsShiftDown();
	FDocumentTracker::EOpenDocumentCause OpenMode = bIsShiftPressed ? FDocumentTracker::ForceOpenNewDocument : FDocumentTracker::OpenNewDocument;

	UQuestSystemGraph* QuestSystemObj = QuestEditorPtr.Pin()->GetQuestSystemObj();
	if (InAction.IsValid())
	{
		if (InAction->GetTypeId() == FAssetSchemaAction_QuestSystemGraph::StaticGetTypeId())
		{
			FAssetSchemaAction_QuestSystemGraph* GraphAction = (FAssetSchemaAction_QuestSystemGraph*)InAction.Get();
			QuestEditorPtr.Pin()->FocusedEdGraph = GraphAction->EdGraph;

			if (GraphAction->EdGraph)
			{
				QuestEditorPtr.Pin()->JumpToHyperlink(GraphAction->EdGraph);

				TSharedPtr<FAssetEditor_QuestSystem> QuestEditor = QuestEditorPtr.Pin();

				if (UEdGraph_QuestSystemGraph* QuestGraph = Cast<UEdGraph_QuestSystemGraph>(QuestEditor->ViewportWidget->GetCurrentGraph()))
				{
					QuestEditor->PropertyWidget->SetObject(QuestGraph->QuestSystem);
				}
			}
		}
	}
}

TSharedPtr<SWidget> SMyQuest::OnContextMenuOpening()
{
	if (!QuestEditorPtr.IsValid())
	{
		return TSharedPtr<SWidget>();
	}

	const bool bShouldCloseWindowAfterMenuSelection = true;
	FMenuBuilder MenuBuilder(bShouldCloseWindowAfterMenuSelection, CommandList);

	// Check if the selected action is valid for a context menu
	if (SelectionHasContextMenu())
	{
		MenuBuilder.BeginSection("BasicOperations");
		{
			MenuBuilder.AddMenuEntry(FMyQuestCommands::Get().OpenGraph);
			MenuBuilder.AddMenuEntry(FMyQuestCommands::Get().OpenGraphInNewTab);
			MenuBuilder.AddMenuEntry(FMyQuestCommands::Get().OpenExternalGraph);
			MenuBuilder.AddMenuEntry(FMyQuestCommands::Get().FocusNode);
			MenuBuilder.AddMenuEntry(FMyQuestCommands::Get().FocusNodeInNewTab);
			MenuBuilder.AddMenuEntry(FGenericCommands::Get().Rename, NAME_None, LOCTEXT("Rename", "Rename"), LOCTEXT("Rename_Tooltip", "Renames this Quest Graph."));
			MenuBuilder.AddMenuEntry(FGenericCommands::Get().Cut);
			MenuBuilder.AddMenuEntry(FGenericCommands::Get().Copy);
			MenuBuilder.AddMenuEntry(FGenericCommands::Get().Duplicate);
			MenuBuilder.AddMenuEntry(FMyQuestCommands::Get().DeleteEntry);
		}
		MenuBuilder.EndSection();

		FAssetSchemaAction_QuestSystemGraph* Graph = SelectionAsGraph();

		//if (Var && BlueprintEditorPtr.IsValid() && FBlueprintEditorUtils::DoesSupportEventGraphs(GetBlueprintObj()))
		//{
		//	FObjectProperty* ComponentProperty = CastField<FObjectProperty>(Var->GetProperty());

		//	if (ComponentProperty && ComponentProperty->PropertyClass &&
		//		ComponentProperty->PropertyClass->IsChildOf(UActorComponent::StaticClass()))
		//	{
		//		if (FBlueprintEditorUtils::CanClassGenerateEvents(ComponentProperty->PropertyClass))
		//		{
		//			TSharedPtr<FBlueprintEditor> BlueprintEditor(BlueprintEditorPtr.Pin());

		//			// If the selected item is valid, and is a component of some sort, build a context menu
		//			// of events appropriate to the component.
		//			MenuBuilder.AddSubMenu(LOCTEXT("AddEventSubMenu", "Add Event"),
		//				LOCTEXT("AddEventSubMenu_ToolTip", "Add Event"),
		//				FNewMenuDelegate::CreateStatic(&SSubobjectBlueprintEditor::BuildMenuEventsSection,
		//					BlueprintEditor->GetBlueprintObj(), ComponentProperty->PropertyClass,
		//					FCanExecuteAction::CreateRaw(this, &SMyBlueprint::IsEditingMode),
		//					FGetSelectedObjectsDelegate::CreateSP(this, &SMyBlueprint::GetSelectedItemsForContextMenu)));
		//		}
		//	}
		//}
		
	}
	else
	{
		BuildAddNewMenu(MenuBuilder);
	}

	return MenuBuilder.MakeWidget();
}
void SMyQuest::BuildAddNewMenu(FMenuBuilder& MenuBuilder)
{
	MenuBuilder.BeginSection("AddNewItem", LOCTEXT("AddOperations", "Add New"));

	if (UQuestSystemGraph* CurrentBlueprint = GetQuestSystemObj())
	{
		MenuBuilder.AddMenuEntry(FQuest_System_EditorCommands::Get().AddNewQuestGraph);
		
	}
	MenuBuilder.EndSection();
}
TSharedRef<SWidget> SMyQuest::CreateAddToSectionButton(int32 InSectionID, TWeakPtr<SWidget> WeakRowWidget, FText AddNewText, FName MetaDataTag)
{
	return
		SNew(SButton)
		.ButtonStyle(FAppStyle::Get(), "SimpleButton")
		.OnClicked(this, &SMyQuest::OnAddButtonClickedOnSection, InSectionID)
		.IsEnabled(this, &SMyQuest::CanAddNewElementToSection, InSectionID)
		.ContentPadding(FMargin(1, 0))
		.AddMetaData<FTagMetaData>(FTagMetaData(MetaDataTag))
		.ToolTipText(AddNewText)
		[
			SNew(SImage)
				.Image(FAppStyle::Get().GetBrush("Icons.PlusCircle"))
				.ColorAndOpacity(FSlateColor::UseForeground())
		];
}
void SMyQuest::OnCategoryNameCommitted(const FText& InNewText, ETextCommit::Type InTextCommit, TWeakPtr<struct FGraphActionNode> InAction)
{
}
bool SMyQuest::CanRequestRenameOnActionNode(TWeakPtr<struct FGraphActionNode> InSelectedNode) const
{
	return true;
}

FText SMyQuest::OnGetSectionTitle(int32 InSectionID)
{
	FText SeperatorTitle;
	/* Setup an appropriate name for the section for this node */
	switch (InSectionID)
	{
	case QuestSectionID::QUESTGRAPH:
		SeperatorTitle = NSLOCTEXT("GraphActionNode", "QuestGraphs", "QuestGraphs");
		break; 
	default:
	case QuestSectionID::NONE:
		SeperatorTitle = FText::GetEmpty();
		break;
	}
	
	return SeperatorTitle;
}
TSharedRef<SWidget> SMyQuest::OnGetSectionWidget(TSharedRef<SWidget> RowWidget, int32 InSectionID)
{
	TWeakPtr<SWidget> WeakRowWidget = RowWidget;

	FText AddNewText;
	FName MetaDataTag;

	switch (InSectionID)
	{
	case QuestSectionID::QUESTGRAPH:
		AddNewText = LOCTEXT("AddNewQuestGraph", "New Quest Graph");
		MetaDataTag = TEXT("AddNewGraph");
		break;
	default:
		return SNullWidget::NullWidget;
	}

	return CreateAddToSectionButton(InSectionID, WeakRowWidget, AddNewText, MetaDataTag);
}
FReply SMyQuest::OnAddButtonClickedOnSection(int32 InSectionID)
{
	TSharedPtr<FAssetEditor_QuestSystem> QuestEditor = QuestEditorPtr.Pin();

	switch (InSectionID)
	{
	case QuestSectionID::QUESTGRAPH:
		CommandList->ExecuteAction(FQuest_System_EditorCommands::Get().AddNewQuestGraph.ToSharedRef());
		break;
	}

	return FReply::Handled();
}
bool SMyQuest::CanAddNewElementToSection(int32 InSectionID) const
{
	return true;
}
bool SMyQuest::HandleActionMatchesName(FEdGraphSchemaAction* InAction, const FName& InName) const
{
	return false;
}
void SMyQuest::OnOpenGraph()
{
	OpenGraph(FDocumentTracker::OpenNewDocument);
}
void SMyQuest::OnOpenGraphInNewTab()
{
	OpenGraph(FDocumentTracker::ForceOpenNewDocument);
}
void SMyQuest::OnOpenExternalGraph()
{
}
bool SMyQuest::CanOpenGraph() const
{
	return true;
}
bool SMyQuest::CanOpenExternalGraph() const
{
	return false;
}
bool SMyQuest::CanFocusOnNode() const
{
	return false;
}
void SMyQuest::OnFocusNode()
{
}
void SMyQuest::OnFocusNodeInNewTab()
{
}
void SMyQuest::OnDeleteEntry()
{
	if (FAssetSchemaAction_QuestSystemGraph* GraphAction = SelectionAsGraph())
	{
		OnDeleteGraph(GraphAction->EdGraph);
	}

	Refresh();
	//BlueprintEditorPtr.Pin()->GetInspector()->ShowDetailsForObjects(TArray<UObject*>());

}

bool SMyQuest::CanDeleteEntry() const
{
	// Cannot delete entries while not in editing mode
	/*if (!IsEditingMode())
	{
		return false;
	}*/

	if (FAssetSchemaAction_QuestSystemGraph* GraphAction = SelectionAsGraph())
	{
		return (GraphAction->EdGraph ? GraphAction->EdGraph->bAllowDeletion : false);
	}
	return false;
}

bool SMyQuest::CanRequestRenameOnActionNode() const
{
	TArray<TSharedPtr<FEdGraphSchemaAction> > SelectedActions;
	GraphActionMenu->GetSelectedActions(SelectedActions);

	// If there is anything selected in the GraphActionMenu, check the item for if it can be renamed.
	if (SelectedActions.Num() || SelectionIsCategory())
	{
		return GraphActionMenu->CanRequestRenameOnActionNode();
	}
	return false;
}

bool SMyQuest::IsDuplicateActionVisible() const
{
	return false;
}
bool SMyQuest::CanDuplicateAction() const
{
	return false;
}
void SMyQuest::OnDuplicateAction()
{
}



void SMyQuest::OnCopy()
{
	
}
bool SMyQuest::CanCopy() const
{
	return false;
}
void SMyQuest::OnCut()
{
}
bool SMyQuest::CanCut() const
{
	return false;
}

void SMyQuest::OnDeleteGraph(UEdGraph* InGraph)
{
	if (InGraph && InGraph->bAllowDeletion)
	{
		if (const UEdGraphSchema* Schema = InGraph->GetSchema())
		{
			if (Schema->TryDeleteGraph(InGraph))
			{
				return;
			}
		}

		const FScopedTransaction Transaction(LOCTEXT("RemoveGraph", "Remove Graph"));
		GetQuestSystemObj()->Modify();

		InGraph->Modify();

		
		// Remove any  nodes bound to this graph
		TArray<UEdNode_QuestSystemNode*> AllNodes;
		FQuestSystemEditorUtils::GetAllNodesOfClass<UEdNode_QuestSystemNode>(GetQuestSystemObj(), AllNodes);

		const bool bDontRecompile = true;
		for (UEdNode_QuestSystemNode* CompNode : AllNodes)
		{
			if (CompNode->GetQuestSystemGraphEdGraph() == InGraph)
			{
				FQuestSystemEditorUtils::RemoveNode(GetQuestSystemObj(), CompNode, bDontRecompile);
			}
		}
		

		FQuestSystemEditorUtils::RemoveGraph(GetQuestSystemObj(), InGraph);
		QuestEditorPtr.Pin()->CloseDocumentTab(InGraph);


		InGraph = NULL;
	}
}

void SMyQuest::OpenGraph(FDocumentTracker::EOpenDocumentCause InCause, bool bOpenExternalGraphInNewEditor)
{
	UEdGraph* GraphToOpen = nullptr;

	if (FAssetSchemaAction_QuestSystemGraph* GraphAction = SelectionAsGraph())
	{
		GraphToOpen = GraphAction->EdGraph;
		// If we have no graph then this is an interface event, so focus on the event graph
		/*if (!GraphToOpen)
		{
			GraphToOpen = FQuestSystemEditorUtils::FindQuestGraph(GetQuestSystemObj());
		}*/
	}

	if (GraphToOpen)
	{
		if (bOpenExternalGraphInNewEditor )
		{
			QuestEditorPtr.Pin()->JumpToHyperlink(GraphToOpen, false);
		}
		else
		{
			QuestEditorPtr.Pin()->OpenDocument(GraphToOpen, InCause);
		}
	}
}

bool SMyQuest::SelectionHasContextMenu() const
{
	TArray<TSharedPtr<FEdGraphSchemaAction> > SelectedActions;
	GraphActionMenu->GetSelectedActions(SelectedActions);
	return SelectedActions.Num() > 0;
}

void SMyQuest::OnObjectPropertyChanged(UObject* InObject, FPropertyChangedEvent& InPropertyChangedEvent)
{
	if (InObject == QuestSystemGraph && (InPropertyChangedEvent.ChangeType != EPropertyChangeType::ValueSet && InPropertyChangedEvent.ChangeType != EPropertyChangeType::ArrayClear))
	{
		bNeedsRefresh = true;
	}
}

void SMyQuest::OnFilterTextChanged(const FText& InFilterText)
{
}
FText SMyQuest::GetFilterText() const
{
	return FText();
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION



#undef LOCTEXT_NAMESPACE