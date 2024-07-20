// Copyright 2024 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SGraphActionMenu.h"
#include "AssetGraphSchema_QuestSystem.h"
#include "Widgets/SCompoundWidget.h"
#include "AssetEditor_QuestSystem.h"



class SKismetInspector;
class SSearchBox;
struct FGraphActionSort;

class FMyQuestCommands : public TCommands<FMyQuestCommands>
{
public:
	/** Constructor */
	FMyQuestCommands()
		: TCommands<FMyQuestCommands>(TEXT("MyQuest"), NSLOCTEXT("Contexts", "My Quest", "My Quest"), NAME_None, FAppStyle::GetAppStyleSetName())
	{
	}

	// Basic operations
	TSharedPtr<FUICommandInfo> OpenGraph;
	TSharedPtr<FUICommandInfo> OpenGraphInNewTab;
	TSharedPtr<FUICommandInfo> OpenExternalGraph;
	TSharedPtr<FUICommandInfo> FocusNode;
	TSharedPtr<FUICommandInfo> FocusNodeInNewTab;
	TSharedPtr<FUICommandInfo> DeleteEntry;
	// Add New Item
	/** Initialize commands */
	virtual void RegisterCommands() override;
};


class QUEST_SYSTEM_EDITOR_API SMyQuest : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SMyQuest)
	{}
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs, TWeakPtr<FAssetEditor_QuestSystem> InQuestEditor, const UQuestSystemGraph* InQuestSystemGraph = nullptr);
	~SMyQuest();

	/* SWidget interface */
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime);
	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent);

	/** Accessor for getting the current selection as a Quest graph */
	FAssetSchemaAction_QuestSystemGraph* SelectionAsGraph() const;

	/** Accessor the blueprint object from the main editor */
	UQuestSystemGraph* GetQuestSystemObj() const { return QuestSystemGraph; }

	/** Initiates a rename on the selected action node, if possible */
	void OnRequestRenameOnActionNode();

	/** Callbacks for Paste Commands */
	void OnPasteGeneric();
	bool CanPasteGeneric();

	/** Refreshes the graph action menu */
	void Refresh();
	void SetFocusedGraph(UEdGraph* InEdGraph) { EdGraph = InEdGraph; }

	/** Accessor for determining if the current selection is a category*/
	bool SelectionIsCategory() const;

	/** Selects an item by name in either the main graph action menu or the local one */
	void SelectItemByName(const FName& ItemName, ESelectInfo::Type SelectInfo = ESelectInfo::Direct, int32 SectionId = INDEX_NONE, bool bIsCategory = false);

	/** Clears the selection in the graph action menus */
	void ClearGraphActionMenuSelection();

private:
	/** Creates widgets for the graph schema actions */
	TSharedRef<SWidget> OnCreateWidgetForAction(struct FCreateWidgetForActionData* const InCreateData);

	/** Callback used to populate all actions list in SGraphActionMenu */
	void CollectAllActions(FGraphActionListBuilderBase& OutAllActions);
	void CollectStaticSections(TArray<int32>& StaticSectionIDs);
	//void GetChildGraphs(UEdGraph* EdGraph, int32 const SectionId, FGraphActionSort& SortList, const FText& ParentCategory = FText::GetEmpty()) const;
	//void GetChildEvents(UEdGraph const* EdGraph, int32 const SectionId, FGraphActionSort& SortList, const FText& ParentCategory = FText::GetEmpty(), bool bInAddChildGraphs = true) const;
	
	/** Callbacks for the graph action menu */
	FReply OnActionDragged(const TArray< TSharedPtr<FEdGraphSchemaAction> >& InActions, const FPointerEvent& MouseEvent);
	FReply OnCategoryDragged(const FText& InCategory, const FPointerEvent& MouseEvent);
	void OnActionSelected(const TArray< TSharedPtr<FEdGraphSchemaAction> >& InActions);
	//void OnActionSelectedHelper(TSharedPtr<FEdGraphSchemaAction> InAction, TWeakPtr<FAssetEditor_QuestSystem> InQuestEditor, UQuestSystemGraph* InQuestSystemGraph, TSharedRef<SKismetInspector> Inspector);
	void OnGlobalActionSelected(const TArray< TSharedPtr<FEdGraphSchemaAction> >& InActions, ESelectInfo::Type InSelectionType);
	void OnActionDoubleClicked(const TArray< TSharedPtr<FEdGraphSchemaAction> >& InActions);
	void ExecuteAction(TSharedPtr<FEdGraphSchemaAction> InAction);
	TSharedPtr<SWidget> OnContextMenuOpening();

	//TSharedRef<SWidget> CreateAddNewMenuWidget();
	void BuildAddNewMenu(FMenuBuilder& MenuBuilder);
	TSharedRef<SWidget> CreateAddToSectionButton(int32 InSectionID, TWeakPtr<SWidget> WeakRowWidget, FText AddNewText, FName MetaDataTag);

	void OnCategoryNameCommitted(const FText& InNewText, ETextCommit::Type InTextCommit, TWeakPtr< struct FGraphActionNode > InAction);
	bool CanRequestRenameOnActionNode(TWeakPtr<struct FGraphActionNode> InSelectedNode) const;
	FText OnGetSectionTitle(int32 InSectionID);
	TSharedRef<SWidget> OnGetSectionWidget(TSharedRef<SWidget> RowWidget, int32 InSectionID);
	//EVisibility OnGetSectionTextVisibility(TWeakPtr<SWidget> RowWidget, int32 InSectionID) const;
	//TSharedRef<SWidget> OnGetFunctionListMenu();
	//void BuildOverridableFunctionsMenu(FMenuBuilder& MenuBuilder);
	FReply OnAddButtonClickedOnSection(int32 InSectionID);
	bool CanAddNewElementToSection(int32 InSectionID) const;

	bool HandleActionMatchesName(FEdGraphSchemaAction* InAction, const FName& InName) const;

	/** Callbacks for commands */
	void OnOpenGraph();
	void OnOpenGraphInNewTab();
	void OnOpenExternalGraph();
	bool CanOpenGraph() const;
	bool CanOpenExternalGraph() const;
	bool CanFocusOnNode() const;
	void OnFocusNode();
	void OnFocusNodeInNewTab();
	void OnDeleteEntry();
	bool CanDeleteEntry() const;
	bool CanRequestRenameOnActionNode() const;
	bool IsDuplicateActionVisible() const;
	bool CanDuplicateAction() const;
	void OnDuplicateAction();
	void OnCopy();
	bool CanCopy() const;
	void OnCut();
	bool CanCut() const;

	/** Helper function to delete a graph in the MyBlueprint window */
	void OnDeleteGraph(UEdGraph* InGraph);

	/** Helper function to open the selected graph */
	void OpenGraph(FDocumentTracker::EOpenDocumentCause InCause, bool bOpenExternalGraphInNewEditor = false);

	/** Checks if the selected action has context menu */
	bool SelectionHasContextMenu() const;

	/** Delegate to hook us into non-structural Blueprint object post-change events */
	void OnObjectPropertyChanged(UObject* InObject, FPropertyChangedEvent& InPropertyChangedEvent);

	/** Callback when the filter is changed, forces the action tree(s) to filter */
	void OnFilterTextChanged(const FText& InFilterText);

	/** Callback for the action trees to get the filter text */
	FText GetFilterText() const;

	/** The filter box that handles filtering for both graph action menus. */
	TSharedPtr< SSearchBox > FilterBox;

	/** List of UI Commands for this scope */
	TSharedPtr<FUICommandList> CommandList;

	/** Graph Action Menu for displaying all our variables and functions */
	TSharedPtr<class SGraphActionMenu> GraphActionMenu;

	/** Pointer back to the Quest editor that owns us */
	TWeakPtr<FAssetEditor_QuestSystem> QuestEditorPtr;

	/** The Quest graph being displayed: */
	UQuestSystemGraph* QuestSystemGraph;

	/** The Ed Graph being displayed: */
	UEdGraph* EdGraph;

	/** Flag to indicate whether or not we need to refresh the panel */
	bool bNeedsRefresh;

	/** If set we'll show only replicated variables (local to a particular blueprint view). */
	bool bShowReplicatedVariablesOnly;
};
