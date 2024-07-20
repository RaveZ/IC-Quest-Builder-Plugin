// Copyright 2024 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Setting_QuestEditor.h"
#include "SSubobjectEditor.h"
#include "Quest_System_Editor.h"
#include "WorkflowOrientedApp/WorkflowTabManager.h"
#include "QuestSystemGraph.h"

class SMyQuest;

namespace QuestSectionID
{
	enum Type
	{
		NONE = 0,
		QUESTGRAPH,					// Quest Graph
		//Add More to populate the section
	};
};

struct FQuestSystemAssetEditorTabs
{
	// Tab identifiers
	static const FName MyQuestDetailID;
	static const FName GenericGraphPropertyID;
	static const FName ViewportID;
	static const FName GenericGraphEditorSettingsID;
};



class QUEST_SYSTEM_EDITOR_API FAssetEditor_QuestSystem : public IQuestEditor, public FNotifyHook, public FGCObject
{
public:
	FAssetEditor_QuestSystem();
	virtual	~FAssetEditor_QuestSystem();

	void InitQuestSystemGraphAssetEditor(const EToolkitMode::Type Mode, const TSharedPtr< IToolkitHost >& InitToolkitHost, UQuestSystemGraph* Graph);

	// Should be called when initializing any editor built off this foundation
	void CommonInitialization(const UQuestSystemGraph* Graph, bool bShouldOpenInDefaultsMode);

	// IToolkit interface
	virtual void RegisterTabSpawners(const TSharedRef<FTabManager>& TabManager) override;
	virtual void UnregisterTabSpawners(const TSharedRef<FTabManager>& TabManager) override;
	// End of IToolkit interface

	// FAssetEditorToolkit
	virtual FName GetToolkitFName() const override;
	virtual FText GetBaseToolkitName() const override;
	virtual FText GetToolkitName() const override;
	virtual FText GetToolkitToolTipText() const override;
	virtual FLinearColor GetWorldCentricTabColorScale() const override;
	virtual FString GetWorldCentricTabPrefix() const override;
	virtual FString GetDocumentationLink() const override;
	virtual void SaveAsset_Execute() override;
	// End of FAssetEditorToolkit
	
	//IQuestEditor
	virtual void RefreshEditors() override;
	virtual void RefreshMyQuest();
	virtual void RefreshInspector();
	virtual void AddToSelection(UEdGraphNode* InNode) override;
	virtual void JumpToHyperlink(const UObject* ObjectReference, bool bRequestRename = false) override;
	virtual void JumpToPin(const class UEdGraphPin* Pin) override;
	virtual void SummonSearchUI(bool bSetFindWithinBlueprint, FString NewSearchTerms = FString(), bool bSelectFirstResult = false) override;
	virtual void SummonFindAndReplaceUI() override;
	virtual TSharedPtr<SGraphEditor> OpenGraphAndBringToFront(UEdGraph* Graph, bool bSetFocus = true) override;



	//Toolbar
	void UpdateToolbar();
	TSharedPtr<class FAssetEditorToolbar_QuestSystem> GetToolbarBuilder() { return ToolbarBuilder; }
	void RegisterToolbarTab(const TSharedRef<class FTabManager>& TabManager);

	
	// FSerializableObject interface
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	virtual FString GetReferencerName() const override;
	// End of FSerializableObject interface

	/** Returns a pointer to the Quest graph object we are currently editing, as long as we are editing exactly one */
	virtual UQuestSystemGraph* GetQuestSystemObj() const;

	// Type of new document/graph being created by a menu item
	enum ECreatedQuestDocumentType
	{
		CGT_None,
		CGT_NewQuestGraph,
	};

	// Called to see if the new document menu items is visible for this type
	virtual bool IsSectionVisible(QuestSectionID::Type InSectionID) const { return true; }
	virtual bool NewDocument_IsVisibleForType(ECreatedQuestDocumentType GraphType) const;
	EVisibility NewDocument_GetVisibilityForType(ECreatedQuestDocumentType GraphType) const
	{
		return NewDocument_IsVisibleForType(GraphType) ? EVisibility::Visible : EVisibility::Collapsed;
	}

	/** Called when New Function button is clicked */
	virtual void NewDocument_OnClicked(ECreatedQuestDocumentType GraphType);
	FReply NewDocument_OnClick(ECreatedQuestDocumentType GraphType) { NewDocument_OnClicked(GraphType); return FReply::Handled(); }

	/** Returns true if in editing mode */
	bool InEditingMode() const;

	TSharedPtr<SDockTab> OpenDocument(const UObject* DocumentID, FDocumentTracker::EOpenDocumentCause Cause);
	
	/** Finds the tab associated with the specified asset, and closes if it is open */
	void CloseDocumentTab(const UObject* DocumentID);

	/** Utility function to handle all steps required to rename a newly added action */
	void RenameNewlyAddedAction(FName InActionName);

	/** Throw a simple message into the log */
	void LogSimpleMessage(const FText& MessageText);

	/** Create a graph title bar widget */
	TSharedRef<SWidget> CreateGraphTitleBarWidget(TSharedRef<FTabInfo> InTabInfo, UEdGraph* InGraph);

	/** Creates the widgets that go into the tabs (note: does not create the tabs themselves) **/
	virtual void CreateDefaultTabContents(const UQuestSystemGraph* Graph);

	/**
	 * Util for finding a glyph for a graph
	 *
	 * @param Graph - The graph to evaluate
	 * @param bInLargeIcon - if true the icon returned is 22x22 pixels, else it is 16x16
	 * @return An appropriate brush to use to represent the graph, if the graph is an unknown type the function will return the default "function" glyph
	 */
	static const FSlateBrush* GetGlyphForGraph(const UEdGraph* Graph, bool bInLargeIcon = false);

	// Finds any open tabs containing the specified document and adds them to the specified array; returns true if at least one is found
	bool FindOpenTabsContainingDocument(const UObject* DocumentID, /*inout*/ TArray< TSharedPtr<SDockTab> >& Results);

	/** Create new tab for each element of LastEditedObjects array */
	void InitializeDocumentTab();

	/**	Returns whether the editor is currently editing a single Quest object */
	bool IsEditingSingleQuestGraph() const;

	/**
	 * Returns the currently focused graph in the Quest editor
	 */
	UEdGraph* GetFocusedGraph() const;

	/** Returns the currently selected node if there is a single node selected (if there are multiple nodes selected or none selected, it will return nullptr) */
	UEdGraphNode* GetSingleSelectedNode() const;

	/** Called when graph editor focus is changed */
	virtual void OnGraphEditorFocused(const TSharedRef<class SGraphEditor>& InGraphEditor);

	/** Called when the graph editor tab is backgrounded */
	virtual void OnGraphEditorBackgrounded(const TSharedRef<SGraphEditor>& InGraphEditor);

	/** Checks to see if the provided graph is contained within the current QuestSystemGraph */
	bool IsGraphInCurrentQuestGraph(const UEdGraph* InGraph) const;

	UQuestEditorSettings* GetSettings() const;

	//Getter
	TSharedPtr<SMyQuest> GetMyQuestWidget() const { return MyQuestWidget; }


	FGraphAppearanceInfo GetGraphAppearance() const;
	bool InEditingMode(bool bGraphIsEditable) const;

	static bool IsPIESimulating();
	static bool IsPIENotSimulating();

private:
	/** Helper to move focused graph when clicking on graph breadcrumb */
	void OnChangeBreadCrumbGraph(class UEdGraph* InGraph);


protected:
	TSharedRef<SDockTab> SpawnTab_Viewport(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTab_Details(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTab_EditorSettings(const FSpawnTabArgs& Args); 
	TSharedRef<SDockTab> SpawnTab_MyQuest(const FSpawnTabArgs& Args);
	
	void CreateInternalWidgets();

	// Create new graph editor widget for the supplied document container
	virtual TSharedRef<SGraphEditor> CreateGraphEditorWidget(TSharedRef<class FTabInfo> InTabInfo, class UEdGraph* InGraph);

	
	void BindCommands();


	void CreateCommandList();
	
	
	TSharedPtr<SGraphEditor> GetCurrGraphEditor() const;

	FGraphPanelSelectionSet GetSelectedNodes() const;

	void RebuildQuestSystemGraph();

	// Delegates for graph editor commands
	void SelectAllNodes();
	bool CanSelectAllNodes();
	void DeleteSelectedNodes();
	bool CanDeleteNodes();
	void DeleteSelectedDuplicatableNodes();
	void CutSelectedNodes();
	bool CanCutNodes();
	void CopySelectedNodes();
	bool CanCopyNodes();
	void PasteNodes();
	virtual void PasteNodesHere(class UEdGraph* DestinationGraph, const FVector2D& Location) override;

	virtual bool CanPasteNodes() const override;
	void DuplicateNodes();
	bool CanDuplicateNodes();

	//New Task Functions from toolbar
	void CreateNewTask();
	void HandleNewNodeClassPicked(UClass* InClass) const;
	bool CanCreateNewTask() const;
	bool IsNewTaskButtonVisible() const;

	void CreateNewQuestCondition();
	bool CanCreateQuestCondition() const;

	void CreateNewQuestEvent();
	bool CanCreateQuestEvent() const;
	
	void OpenQuestSetting();
	bool CanOpenQuestSetting() const;

	void OnRenameNode();
	bool CanRenameNodes() const;

	//////////////////////////////////////////////////////////////////////////
	// graph editor event
	void OnSelectedNodesChanged(const TSet<class UObject*>& NewSelection);

	void OnNodeDoubleClicked(UEdGraphNode* Node);
	
	void OnFinishedChangingProperties(const FPropertyChangedEvent& PropertyChangedEvent);


#if ENGINE_MAJOR_VERSION < 5
	void OnPackageSaved(const FString& PackageFileName, UObject* Outer);
#else // #if ENGINE_MAJOR_VERSION < 5
	void OnPackageSavedWithContext(const FString& PackageFileName, UPackage* Package, FObjectPostSaveContext ObjectSaveContext);
#endif // #else // #if ENGINE_MAJOR_VERSION < 5

public:

	TSharedPtr<FDocumentTracker> DocumentManager;
	TSharedPtr<class IDetailsView> PropertyWidget;

	/** Currently focused Editor Graph*/
	UEdGraph* FocusedEdGraph;

	/** Currently focused graph editor */
	TSharedPtr<SGraphEditor> ViewportWidget;

	/** determine if node should be checked for new unique ID*/
	bool ShouldGetNewID = false;

protected:

	/** Factory that spawns graph editors; used to look up all tabs spawned by it. */
	TWeakPtr<FDocumentTabFactory> QuestEditorTabFactoryPtr;

	UQuestEditorSettings* QuestGraphEditorSettings;

	UQuestSystemGraph* EditingQuestGraph;


	//Toolbar
	TSharedPtr<class FAssetEditorToolbar_QuestSystem> ToolbarBuilder;

	/** Handle to the registered OnPackageSave delegate */
	FDelegateHandle OnPackageSavedDelegateHandle;

	TSharedPtr<class IDetailsView> EditorSettingsWidget;
	TSharedPtr<class SMyQuest> MyQuestWidget;

	/** The command list for this editor */
	TSharedPtr<FUICommandList> GraphEditorCommands;



};
