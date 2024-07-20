// Copyright 2024 Ivan Chandra. All Rights Reserved.


#include "AssetEditor_QuestSystem.h"
#include "QuestSystemGraphNode_Root.h"
#include "Condition/QuestCondition.h"
#include "EngineGlobals.h"
#include "Editor/EditorEngine.h"
#include "Editor.h"
#include "UnrealEdGlobals.h"
#include "Event/QuestEvent.h"
#include "BlueprintEditor.h"
#include "UObject/ObjectSaveContext.h"
#include "GraphEditorActions.h"
#include "Framework/Commands/GenericCommands.h"
#include "AssetEditorToolbar_QuestSystem.h"
#include "Quest_System_EditorCommands.h"
#include "EdGraph_QuestSystemGraph.h"
#include "EdNode_QuestSystemNode.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "K2Node.h"
#include "EdNode_QuestSystemEdge.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "HAL/PlatformApplicationMisc.h"
#include "AssetGraphSchema_QuestSystem.h"
#include "QuestSystemEditorUtils.h"
#include "QuestEditorFactory.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "SMyQuest.h"
#include "Task/QuestTask.h"
#include "Quest_System_Editor.h"
#include <ContentBrowserModule.h>
#include <ContentBrowserFrontEndFilterExtension.h>
#include "Kismet2/KismetDebugUtilities.h"
#include "WorkflowOrientedApp/WorkflowUObjectDocuments.h"
#include "WorkflowOrientedApp/WorkflowCentricApplication.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"
#include <Kismet2/KismetEditorUtilities.h>

#define LOCTEXT_NAMESPACE "AssetEditor_QuestGraph"



//////////////////////////////////////////////////////////////////////////

const FName FQuestSystemAssetEditorTabs::MyQuestDetailID(TEXT("MyQuestDetail"));
const FName FQuestSystemAssetEditorTabs::GenericGraphPropertyID(TEXT("GenericGraphProperty"));
const FName FQuestSystemAssetEditorTabs::ViewportID(TEXT("Viewport"));
const FName FQuestSystemAssetEditorTabs::GenericGraphEditorSettingsID(TEXT("GenericGraphEditorSettings"));

//////////////////////////////////////////////////////////////////////////

const FName GenericGraphEditorAppName = FName(TEXT("GenericGraphEditorApp"));

FAssetEditor_QuestSystem::FAssetEditor_QuestSystem()
{
	EditingQuestGraph = nullptr;
	QuestGraphEditorSettings = NewObject<UQuestEditorSettings>(UQuestEditorSettings::StaticClass());
	DocumentManager = MakeShareable(new FDocumentTracker);

#if ENGINE_MAJOR_VERSION < 5
	OnPackageSavedDelegateHandle = UPackage::PackageSavedEvent.AddRaw(this, &FAssetEditor_QuestSystem::OnPackageSaved);
#else // #if ENGINE_MAJOR_VERSION < 5
	OnPackageSavedDelegateHandle = UPackage::PackageSavedWithContextEvent.AddRaw(this, &FAssetEditor_QuestSystem::OnPackageSavedWithContext);
#endif // #else // #if ENGINE_MAJOR_VERSION < 5
}

FAssetEditor_QuestSystem::~FAssetEditor_QuestSystem()
{
#if ENGINE_MAJOR_VERSION < 5
	UPackage::PackageSavedEvent.Remove(OnPackageSavedDelegateHandle);
#else // #if ENGINE_MAJOR_VERSION < 5
	UPackage::PackageSavedWithContextEvent.Remove(OnPackageSavedDelegateHandle);
#endif // #else // #if ENGINE_MAJOR_VERSION < 5
}

void FAssetEditor_QuestSystem::InitQuestSystemGraphAssetEditor(const EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost>& InitToolkitHost, UQuestSystemGraph* Graph)
{
	EditingQuestGraph = Graph;
	FGenericCommands::Register();
	FGraphEditorCommands::Register();
	FMyQuestCommands::Register();
	FQuest_System_EditorCommands::Register();

	


	if (!ToolbarBuilder.IsValid())
	{
		ToolbarBuilder = MakeShareable(new FAssetEditorToolbar_QuestSystem(SharedThis(this)));
	}

	BindCommands();

	CreateInternalWidgets();

	TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);

	ToolbarBuilder->AddQuestSystemToolbar(ToolbarExtender);
	
	// Layout
	const TSharedRef<FTabManager::FLayout> StandaloneDefaultLayout = FTabManager::NewLayout("Standalone_GenericGraphEditor_Layout_v1")
		->AddArea
		(
			FTabManager::NewPrimaryArea()->SetOrientation(Orient_Vertical)
#if ENGINE_MAJOR_VERSION < 5
			->Split
			(
				FTabManager::NewStack()
				->SetSizeCoefficient(0.1f)
				->AddTab(GetToolbarTabId(), ETabState::OpenedTab)->SetHideTabWell(true)
			)
#endif // #if ENGINE_MAJOR_VERSION < 5
			->Split
			(
				FTabManager::NewSplitter()->SetOrientation(Orient_Horizontal)->SetSizeCoefficient(0.9f)
				->Split
				(
					FTabManager::NewStack()
					->SetSizeCoefficient(0.2f)
					->AddTab(FQuestSystemAssetEditorTabs::MyQuestDetailID, ETabState::OpenedTab)
				)
				->Split
				(
					FTabManager::NewStack()
					->SetSizeCoefficient(0.55f)
					->AddTab(/*FQuestSystemAssetEditorTabs::ViewportID*/"Document", ETabState::ClosedTab)
				)
				->Split
				(
					FTabManager::NewSplitter()->SetOrientation(Orient_Vertical)->SetSizeCoefficient(0.25f)
					->Split
					(
						FTabManager::NewStack()
						->SetSizeCoefficient(0.55f)
						->AddTab(FQuestSystemAssetEditorTabs::GenericGraphPropertyID, ETabState::OpenedTab)
					)
					/*->Split
					(
						FTabManager::NewStack()
						->SetSizeCoefficient(0.45f)
						->AddTab(FQuestSystemAssetEditorTabs::GenericGraphEditorSettingsID, ETabState::OpenedTab)
					)*/
				)
			)
		);

	const bool bCreateDefaultStandaloneMenu = true;
	const bool bCreateDefaultToolbar = true;
	FAssetEditorToolkit::InitAssetEditor(Mode, InitToolkitHost, GenericGraphEditorAppName, StandaloneDefaultLayout, bCreateDefaultStandaloneMenu, bCreateDefaultToolbar, EditingQuestGraph, false);
	CommonInitialization(Graph, true);
	RegenerateMenusAndToolbars();
	//InitializeDocumentTab();

	if (Graph->bIsNewlyCreated)
	{
		NewDocument_OnClicked(CGT_NewQuestGraph);
		Graph->bIsNewlyCreated = false;
		Graph->Modify();
	}
	else
	{
		if (GetQuestSystemObj()->QuestGraphPages.Num() > 0)
		{
			OpenDocument(GetQuestSystemObj()->QuestGraphPages[0], FDocumentTracker::OpenNewDocument);
		}
	}
	
}

void FAssetEditor_QuestSystem::CommonInitialization(const UQuestSystemGraph* Graph, bool bShouldOpenInDefaultsMode)
{
	TSharedPtr<FAssetEditor_QuestSystem> ThisPtr(SharedThis(this));

	// @todo TabManagement
	DocumentManager->Initialize(ThisPtr);

	// Register the document factories
	{
		//DocumentManager->RegisterDocumentFactory(MakeShareable(new FTimelineEditorSummoner(ThisPtr)));

		TSharedRef<FDocumentTabFactory> GraphEditorFactory = MakeShareable(new FQuestGraphEditorSummoner(ThisPtr,
			FQuestGraphEditorSummoner::FOnCreateGraphEditorWidget::CreateSP(this, &FAssetEditor_QuestSystem::CreateGraphEditorWidget)
		));

		// Also store off a reference to the grapheditor factory so we can find all the tabs spawned by it later.
		QuestEditorTabFactoryPtr = GraphEditorFactory;
		DocumentManager->RegisterDocumentFactory(GraphEditorFactory);
	}

	CreateDefaultTabContents(Graph);
}

void FAssetEditor_QuestSystem::RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{

	DocumentManager->SetTabManager(InTabManager);

	//FWorkflowCentricApplication::RegisterTabSpawners(InTabManager);

	WorkspaceMenuCategory = InTabManager->AddLocalWorkspaceMenuCategory(LOCTEXT("WorkspaceMenu_GenericGraphEditor", "Generic Graph Editor"));
	auto WorkspaceMenuCategoryRef = WorkspaceMenuCategory.ToSharedRef();

	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

	InTabManager->RegisterTabSpawner(FQuestSystemAssetEditorTabs::MyQuestDetailID, FOnSpawnTab::CreateSP(this, &FAssetEditor_QuestSystem::SpawnTab_MyQuest))
		.SetDisplayName(LOCTEXT("Quest Details", "My Quest"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Details"));

	InTabManager->RegisterTabSpawner(FQuestSystemAssetEditorTabs::ViewportID, FOnSpawnTab::CreateSP(this, &FAssetEditor_QuestSystem::SpawnTab_Viewport))
		.SetDisplayName(LOCTEXT("GraphCanvasTab", "Viewport"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "GraphEditor.EventGraph_16x"));

	InTabManager->RegisterTabSpawner(FQuestSystemAssetEditorTabs::GenericGraphPropertyID, FOnSpawnTab::CreateSP(this, &FAssetEditor_QuestSystem::SpawnTab_Details))
		.SetDisplayName(LOCTEXT("DetailsTab", "Property"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Details"));
}

void FAssetEditor_QuestSystem::UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);

	InTabManager->UnregisterTabSpawner(FQuestSystemAssetEditorTabs::MyQuestDetailID);
	InTabManager->UnregisterTabSpawner(FQuestSystemAssetEditorTabs::ViewportID);
	InTabManager->UnregisterTabSpawner(FQuestSystemAssetEditorTabs::GenericGraphPropertyID);
	InTabManager->UnregisterTabSpawner(FQuestSystemAssetEditorTabs::GenericGraphEditorSettingsID);
}





void FAssetEditor_QuestSystem::BindCommands()
{
	ToolkitCommands->MapAction(FQuest_System_EditorCommands::Get().NewTask,
		FExecuteAction::CreateSP(this, &FAssetEditor_QuestSystem::CreateNewTask),
		FCanExecuteAction::CreateSP(this, &FAssetEditor_QuestSystem::CanCreateNewTask),
		FIsActionChecked(),
		FIsActionButtonVisible::CreateSP(this, &FAssetEditor_QuestSystem::IsNewTaskButtonVisible)
	);

	ToolkitCommands->MapAction(FQuest_System_EditorCommands::Get().NewQuestCondition,
		FExecuteAction::CreateSP(this, &FAssetEditor_QuestSystem::CreateNewQuestCondition),
		FCanExecuteAction::CreateSP(this, &FAssetEditor_QuestSystem::CanCreateQuestCondition)
	);

	ToolkitCommands->MapAction(FQuest_System_EditorCommands::Get().NewQuestEvent,
		FExecuteAction::CreateSP(this, &FAssetEditor_QuestSystem::CreateNewQuestEvent),
		FCanExecuteAction::CreateSP(this, &FAssetEditor_QuestSystem::CanCreateQuestEvent)
	); 
	
	ToolkitCommands->MapAction(FQuest_System_EditorCommands::Get().QuestSetting,
		FExecuteAction::CreateSP(this, &FAssetEditor_QuestSystem::OpenQuestSetting),
		FCanExecuteAction::CreateSP(this, &FAssetEditor_QuestSystem::CanOpenQuestSetting)
	);
	
	ToolkitCommands->MapAction(FQuest_System_EditorCommands::Get().AddNewQuestGraph,
		FExecuteAction::CreateSP(this, &FAssetEditor_QuestSystem::NewDocument_OnClicked, CGT_NewQuestGraph),
		FCanExecuteAction::CreateSP(this, &FAssetEditor_QuestSystem::InEditingMode),
		FIsActionChecked(),
		FIsActionButtonVisible::CreateSP(this, &FAssetEditor_QuestSystem::NewDocument_IsVisibleForType, CGT_NewQuestGraph)
	);

}

void FAssetEditor_QuestSystem::CreateCommandList()
{
	if (GraphEditorCommands.IsValid()) {
		return;
	}

	GraphEditorCommands = MakeShareable(new FUICommandList);
	// Can't use CreateSP here because derived editor are already implementing TSharedFromThis<FAssetEditorToolkit>
	// however it should be safe, since commands are being used only within this editor
	// if it ever crashes, this function will have to go away and be reimplemented in each derived class

	GraphEditorCommands->MapAction(FQuest_System_EditorCommands::Get().AddNewQuestGraph,
		FExecuteAction::CreateSP(this, &FAssetEditor_QuestSystem::NewDocument_OnClicked, CGT_NewQuestGraph),
		FCanExecuteAction::CreateSP(this, &FAssetEditor_QuestSystem::InEditingMode),
		FIsActionChecked(),
		FIsActionButtonVisible::CreateSP(this, &FAssetEditor_QuestSystem::NewDocument_IsVisibleForType, CGT_NewQuestGraph)
	);

	GraphEditorCommands->MapAction(FQuest_System_EditorCommands::Get().NewTask,
		FExecuteAction::CreateRaw(this, &FAssetEditor_QuestSystem::CreateNewTask),
		FCanExecuteAction::CreateRaw(this, &FAssetEditor_QuestSystem::CanCreateNewTask));

	GraphEditorCommands->MapAction(FQuest_System_EditorCommands::Get().NewQuestCondition,
		FExecuteAction::CreateRaw(this, &FAssetEditor_QuestSystem::CreateNewQuestEvent),
		FCanExecuteAction::CreateRaw(this, &FAssetEditor_QuestSystem::CanCreateQuestEvent));
	
	GraphEditorCommands->MapAction(FQuest_System_EditorCommands::Get().NewQuestEvent,
		FExecuteAction::CreateRaw(this, &FAssetEditor_QuestSystem::CreateNewQuestCondition),
		FCanExecuteAction::CreateRaw(this, &FAssetEditor_QuestSystem::CanCreateQuestCondition));

	GraphEditorCommands->MapAction(FQuest_System_EditorCommands::Get().QuestSetting,
		FExecuteAction::CreateRaw(this, &FAssetEditor_QuestSystem::OpenQuestSetting),
		FCanExecuteAction::CreateRaw(this, &FAssetEditor_QuestSystem::CanOpenQuestSetting));


	GraphEditorCommands->MapAction(FGenericCommands::Get().SelectAll,
		FExecuteAction::CreateRaw(this, &FAssetEditor_QuestSystem::SelectAllNodes),
		FCanExecuteAction::CreateRaw(this, &FAssetEditor_QuestSystem::CanSelectAllNodes)
	);

	GraphEditorCommands->MapAction(FGenericCommands::Get().Delete,
		FExecuteAction::CreateRaw(this, &FAssetEditor_QuestSystem::DeleteSelectedNodes),
		FCanExecuteAction::CreateRaw(this, &FAssetEditor_QuestSystem::CanDeleteNodes)
	);

	GraphEditorCommands->MapAction(FGenericCommands::Get().Copy,
		FExecuteAction::CreateRaw(this, &FAssetEditor_QuestSystem::CopySelectedNodes),
		FCanExecuteAction::CreateRaw(this, &FAssetEditor_QuestSystem::CanCopyNodes)
	);

	GraphEditorCommands->MapAction(FGenericCommands::Get().Cut,
		FExecuteAction::CreateRaw(this, &FAssetEditor_QuestSystem::CutSelectedNodes),
		FCanExecuteAction::CreateRaw(this, &FAssetEditor_QuestSystem::CanCutNodes)
	);

	GraphEditorCommands->MapAction(FGenericCommands::Get().Paste,
		FExecuteAction::CreateRaw(this, &FAssetEditor_QuestSystem::PasteNodes),
		FCanExecuteAction::CreateRaw(this, &FAssetEditor_QuestSystem::CanPasteNodes)
	);

	GraphEditorCommands->MapAction(FGenericCommands::Get().Duplicate,
		FExecuteAction::CreateRaw(this, &FAssetEditor_QuestSystem::DuplicateNodes),
		FCanExecuteAction::CreateRaw(this, &FAssetEditor_QuestSystem::CanDuplicateNodes)
	);

	GraphEditorCommands->MapAction(FGenericCommands::Get().Rename,
		FExecuteAction::CreateSP(this, &FAssetEditor_QuestSystem::OnRenameNode),
		FCanExecuteAction::CreateSP(this, &FAssetEditor_QuestSystem::CanRenameNodes)
	);
}

TSharedPtr<SGraphEditor> FAssetEditor_QuestSystem::GetCurrGraphEditor() const
{
	return ViewportWidget;
}

FGraphPanelSelectionSet FAssetEditor_QuestSystem::GetSelectedNodes() const
{
	FGraphPanelSelectionSet CurrentSelection;
	TSharedPtr<SGraphEditor> FocusedGraphEd = GetCurrGraphEditor();
	if (FocusedGraphEd.IsValid())
	{
		CurrentSelection = FocusedGraphEd->GetSelectedNodes();
	}

	return CurrentSelection;
}

FName FAssetEditor_QuestSystem::GetToolkitFName() const
{
	return FName("FQuestGraphEditor");
}

FText FAssetEditor_QuestSystem::GetBaseToolkitName() const
{
	return LOCTEXT("QuestGraphEditorAppLabel", "Quest Graph Editor");
}

FText FAssetEditor_QuestSystem::GetToolkitName() const
{
	const bool bDirtyState = EditingQuestGraph->GetOutermost()->IsDirty();

	FFormatNamedArguments Args;
	Args.Add(TEXT("QuestGraphName"), FText::FromString(EditingQuestGraph->GetName()));
	Args.Add(TEXT("DirtyState"), bDirtyState ? FText::FromString(TEXT("*")) : FText::GetEmpty());
	return FText::Format(LOCTEXT("QuestGraphEditorToolkitName", "{QuestGraphName}{DirtyState}"), Args);
}

FText FAssetEditor_QuestSystem::GetToolkitToolTipText() const
{
	return FAssetEditorToolkit::GetToolTipTextForObject(EditingQuestGraph);
}

FLinearColor FAssetEditor_QuestSystem::GetWorldCentricTabColorScale() const
{
	return FLinearColor::Blue;
}

FString FAssetEditor_QuestSystem::GetWorldCentricTabPrefix() const
{
	return TEXT("QuestGraphEditor");
}

FString FAssetEditor_QuestSystem::GetDocumentationLink() const
{
	//make documentation from notion add this link
	return TEXT("");
}

void FAssetEditor_QuestSystem::SaveAsset_Execute()
{
	if (EditingQuestGraph != nullptr)
	{
		RebuildQuestSystemGraph();
	}

	FAssetEditorToolkit::SaveAsset_Execute();
}

void FAssetEditor_QuestSystem::RefreshEditors()
{
}

void FAssetEditor_QuestSystem::RefreshMyQuest()
{
}

void FAssetEditor_QuestSystem::RefreshInspector()
{
}

void FAssetEditor_QuestSystem::AddToSelection(UEdGraphNode* InNode)
{
}

void FAssetEditor_QuestSystem::JumpToHyperlink(const UObject* ObjectReference, bool bRequestRename)
{
	//SetCurrentMode(FBlueprintEditorApplicationModes::StandardBlueprintEditorMode);
	if (const UEdGraph* Graph = Cast<const UEdGraph>(ObjectReference))
	{
		// Navigating into things should re-use the current tab when it makes sense
		FDocumentTracker::EOpenDocumentCause OpenMode = FDocumentTracker::OpenNewDocument;
		if ((Graph->GetSchema()->GetGraphType(Graph) == GT_Ubergraph) /*|| Cast<UK2Node>(Graph->GetOuter())*/ || Cast<UEdGraph>(Graph-> GetOuter()))
		{
			// Ubergraphs directly reuse the current graph
			OpenMode = FDocumentTracker::NavigatingCurrentDocument;
		}
		else
		{
			// Walk up the outer chain to see if any tabs have a parent of this document open for edit, and if so
			// we should reuse that one and drill in deeper instead
			for (UObject* WalkPtr = const_cast<UEdGraph*>(Graph); WalkPtr != nullptr; WalkPtr = WalkPtr->GetOuter())
			{
				TArray< TSharedPtr<SDockTab> > TabResults;
				if (FindOpenTabsContainingDocument(WalkPtr, /*out*/ TabResults))
				{
					// See if the parent was active
					bool bIsActive = false;
					for (TSharedPtr<SDockTab> Tab : TabResults)
					{
						if (Tab->IsActive())
						{
							bIsActive = true;
							break;
						}
					}

					if (bIsActive)
					{
						OpenMode = FDocumentTracker::NavigatingCurrentDocument;
						break;
					}
				}
			}
		}

		// Force it to open in a new document if shift is pressed
		const bool bIsShiftPressed = FSlateApplication::Get().GetModifierKeys().IsShiftDown();
		if (bIsShiftPressed)
		{
			OpenMode = FDocumentTracker::ForceOpenNewDocument;
		}

		// Open the document
		OpenDocument(Graph, OpenMode);
	}

	else
	{
		UE_LOG(LogBlueprint, Warning, TEXT("Unknown type of hyperlinked object (%s), cannot focus it"), *GetNameSafe(ObjectReference));
	}

	//@TODO: Hacky way to ensure a message is seen when hitting an exception and doing intraframe debugging
	const FText ExceptionMessage = FKismetDebugUtilities::GetAndClearLastExceptionMessage();
	if (!ExceptionMessage.IsEmpty())
	{
		LogSimpleMessage(ExceptionMessage);
	}
}

void FAssetEditor_QuestSystem::JumpToPin(const UEdGraphPin* Pin)
{
}

void FAssetEditor_QuestSystem::SummonSearchUI(bool bSetFindWithinBlueprint, FString NewSearchTerms, bool bSelectFirstResult)
{
}

void FAssetEditor_QuestSystem::SummonFindAndReplaceUI()
{
}

TSharedPtr<SGraphEditor> FAssetEditor_QuestSystem::OpenGraphAndBringToFront(UEdGraph* Graph, bool bSetFocus)
{
	return TSharedPtr<SGraphEditor>();
}

void FAssetEditor_QuestSystem::UpdateToolbar()
{
}

void FAssetEditor_QuestSystem::RegisterToolbarTab(const TSharedRef<class FTabManager>& InTabManager)
{
	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);
}

void FAssetEditor_QuestSystem::AddReferencedObjects(FReferenceCollector& Collector)
{
	/*if (GetObjectsCurrentlyBeingEdited()->Num() > 0)
	{
		TArray<UObject*>& LocalEditingObjects = const_cast<TArray<UObject*>&>(GetEditingObjects());

		Collector.AddReferencedObjects(LocalEditingObjects);
	}

	Collector.AddReferencedObject(EditingQuestGraph);*/
}

FString FAssetEditor_QuestSystem::GetReferencerName() const
{
	return FString();
}

UQuestSystemGraph* FAssetEditor_QuestSystem::GetQuestSystemObj() const
{
	return EditingQuestGraph;
	//return GetEditingObjects().Num() == 1 ? Cast<UQuestSystemGraph>(GetEditingObjects()[0]) : nullptr;
}

bool FAssetEditor_QuestSystem::NewDocument_IsVisibleForType(ECreatedQuestDocumentType GraphType) const
{
	return true;
}

void FAssetEditor_QuestSystem::NewDocument_OnClicked(ECreatedQuestDocumentType GraphType)
{
	FText DocumentNameText;
	bool bResetMyBlueprintFilter = false;

	switch (GraphType)
	{
	case CGT_NewQuestGraph:
		DocumentNameText = LOCTEXT("NewDocQuestGraphName", "QuestGraph");
		bResetMyBlueprintFilter = true;
		break;
	
	default:
		DocumentNameText = LOCTEXT("NewDocNewName", "NewDocument");
		break;
	}

	FName DocumentName = FName(*DocumentNameText.ToString());

	

	// Make sure the new name is valid
	DocumentName = FQuestSystemEditorUtils::FindUniqueQuestName(DocumentNameText.ToString());
		
	//check(IsEditingSingleBlueprint());

	const FScopedTransaction Transaction(LOCTEXT("AddNewQuestGraph", "Add New Quest Graph"));
	GetQuestSystemObj()->Modify();

	UEdGraph* NewGraph = nullptr;

	
	if (GraphType == CGT_NewQuestGraph)
	{
		NewGraph = FQuestSystemEditorUtils::CreateNewGraph(GetQuestSystemObj(), DocumentName, UEdGraph_QuestSystemGraph::StaticClass(), UAssetGraphSchema_QuestSystem::StaticClass());
		FQuestSystemEditorUtils::AddQuestGraphPage(GetQuestSystemObj(), NewGraph);
	}
	else
	{
		ensureMsgf(false, TEXT("GraphType is invalid"));
	}

	// Now open the new graph
	if (NewGraph)
	{
		OpenDocument(NewGraph, FDocumentTracker::OpenNewDocument);

		RenameNewlyAddedAction(DocumentName);
	}
	else
	{
		LogSimpleMessage(LOCTEXT("AddDocument_Error", "Adding new document failed."));
	}
}

bool FAssetEditor_QuestSystem::InEditingMode() const
{
	return true;
}

TSharedPtr<SDockTab> FAssetEditor_QuestSystem::OpenDocument(const UObject* DocumentID, FDocumentTracker::EOpenDocumentCause Cause)
{
	TSharedRef<FTabPayload_UObject> Payload = FTabPayload_UObject::Make(DocumentID);
	return DocumentManager->OpenDocument(Payload, Cause);
}

void FAssetEditor_QuestSystem::CloseDocumentTab(const UObject* DocumentID)
{
	TSharedRef<FTabPayload_UObject> Payload = FTabPayload_UObject::Make(DocumentID);
	DocumentManager->CloseTab(Payload);
}

void FAssetEditor_QuestSystem::RenameNewlyAddedAction(FName InActionName)
{
	//TabManager->TryInvokeTab(FBlueprintEditorTabs::MyBlueprintID);
	//TryInvokingDetailsTab(/*Flash*/false);

	if (MyQuestWidget.IsValid())
	{
		// Force a refresh immediately, the item has to be present in the list for the rename requests to be successful.
		MyQuestWidget->Refresh();
		MyQuestWidget->SelectItemByName(InActionName, ESelectInfo::OnMouseClick);
		MyQuestWidget->OnRequestRenameOnActionNode();
	}
}

void FAssetEditor_QuestSystem::LogSimpleMessage(const FText& MessageText)
{
	FNotificationInfo Info(MessageText);
	Info.ExpireDuration = 3.0f;
	Info.bUseLargeFont = false;
	TSharedPtr<SNotificationItem> Notification = FSlateNotificationManager::Get().AddNotification(Info);
	if (Notification.IsValid())
	{
		Notification->SetCompletionState(SNotificationItem::CS_Fail);
	}
}

TSharedRef<SWidget> FAssetEditor_QuestSystem::CreateGraphTitleBarWidget(TSharedRef<FTabInfo> InTabInfo, UEdGraph* InGraph)
{
	// Create the title bar widget
	return SNew(SBox);
		/*SNew(SGraphTitleBar)
		.EdGraphObj(InGraph)
		.Kismet2(SharedThis(this))
		.OnDifferentGraphCrumbClicked(this, &FAssetEditor_QuestSystem::OnChangeBreadCrumbGraph)
		.HistoryNavigationWidget(InTabInfo->CreateHistoryNavigationWidget());*/
}

void FAssetEditor_QuestSystem::CreateDefaultTabContents(const UQuestSystemGraph* Graph)
{
}

const FSlateBrush* FAssetEditor_QuestSystem::GetGlyphForGraph(const UEdGraph* Graph, bool bInLargeIcon)
{
	const FSlateBrush* ReturnValue = FAppStyle::GetBrush(bInLargeIcon ? TEXT("GraphEditor.Function_24x") : TEXT("GraphEditor.Function_16x"));

	check(Graph != nullptr);
	const UEdGraphSchema* Schema = Graph->GetSchema();
	if (Schema != nullptr)
	{
		const EGraphType GraphType = Schema->GetGraphType(Graph);
		switch (GraphType)
		{
		default:
		case GT_Ubergraph:
		{
			ReturnValue = FAppStyle::GetBrush(bInLargeIcon ? TEXT("GraphEditor.EventGraph_24x") : TEXT("GraphEditor.EventGraph_16x"));
		}
		break;
		}
	}

	return ReturnValue;
}

bool FAssetEditor_QuestSystem::FindOpenTabsContainingDocument(const UObject* DocumentID, TArray<TSharedPtr<SDockTab>>& Results)
{
	int32 StartingCount = Results.Num();

	TSharedRef<FTabPayload_UObject> Payload = FTabPayload_UObject::Make(DocumentID);

	DocumentManager->FindMatchingTabs(Payload, /*inout*/ Results);

	// Did we add anything new?
	return (StartingCount != Results.Num());
}


void FAssetEditor_QuestSystem::InitializeDocumentTab()
{
	check(IsEditingSingleQuestGraph());

	UQuestSystemGraph* QuestSystemGraph = GetQuestSystemObj();
	if (QuestSystemGraph->LastEditedDocuments.Num() == 0)
	{
			QuestSystemGraph->LastEditedDocuments.Add(FQuestSystemEditorUtils::FindQuestGraph(QuestSystemGraph));
	}

	for (int32 i = 0; i < QuestSystemGraph->LastEditedDocuments.Num(); i++)
	{
		if (UObject* Obj = QuestSystemGraph->LastEditedDocuments[i].EditedObjectPath.ResolveObject())
		{
			if (UEdGraph* Graph = Cast<UEdGraph>(Obj))
			{
				struct LocalStruct
				{
					static TSharedPtr<SDockTab> OpenGraphTree(FAssetEditor_QuestSystem* InQuestSystemGraphEditor, UEdGraph* InGraph)
					{
						FDocumentTracker::EOpenDocumentCause OpenCause = FDocumentTracker::QuickNavigateCurrentDocument;

						for (UObject* OuterObject = InGraph->GetOuter(); OuterObject; OuterObject = OuterObject->GetOuter())
						{
							if (OuterObject->IsA<UQuestSystemGraph>())
							{
								// reached up to the QuestSystemGraph for the graph, we are done climbing the tree
								OpenCause = FDocumentTracker::RestorePreviousDocument;
								break;
							}
							else if (UEdGraph* OuterGraph = Cast<UEdGraph>(OuterObject))
							{
								// Found another graph, open it up
								OpenGraphTree(InQuestSystemGraphEditor, OuterGraph);
								break;
							}
						}

						return InQuestSystemGraphEditor->OpenDocument(InGraph, OpenCause);
					}
				};
				TSharedPtr<SDockTab> TabWithGraph = LocalStruct::OpenGraphTree(this, Graph);
				if (TabWithGraph.IsValid())
				{
					TSharedRef<SGraphEditor> GraphEditor = StaticCastSharedRef<SGraphEditor>(TabWithGraph->GetContent());
					GraphEditor->SetViewLocation(QuestSystemGraph->LastEditedDocuments[i].SavedViewOffset, QuestSystemGraph->LastEditedDocuments[i].SavedZoomAmount);
				}
			}
			else
			{
				TSharedPtr<SDockTab> TabWithGraph = OpenDocument(Obj, FDocumentTracker::RestorePreviousDocument);
			}
		}
	}
}



bool FAssetEditor_QuestSystem::IsEditingSingleQuestGraph() const
{
	return GetQuestSystemObj() != nullptr;
}

UEdGraph* FAssetEditor_QuestSystem::GetFocusedGraph() const
{
	if (GetCurrGraphEditor().IsValid())
	{
		if (UEdGraph* Graph = GetCurrGraphEditor()->GetCurrentGraph())
		{
			if (IsValid(Graph))
			{
				return Graph;
			}
		}
	}
	return nullptr;
}

UEdGraphNode* FAssetEditor_QuestSystem::GetSingleSelectedNode() const
{
	const FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();
	return (SelectedNodes.Num() == 1) ? Cast<UEdGraphNode>(*SelectedNodes.CreateConstIterator()) : nullptr;
}

void FAssetEditor_QuestSystem::OnGraphEditorFocused(const TSharedRef<class SGraphEditor>& InGraphEditor)
{
	// Update the graph editor that is currently focused
	ViewportWidget = InGraphEditor;
	InGraphEditor->SetPinVisibility(SGraphEditor::EPinVisibility::Pin_Show);

	// Update the inspector as well, to show selection from the focused graph editor
	FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();
	//FocusInspectorOnGraphSelection(SelectedNodes, /*bForceRefresh=*/ true);

	// During undo, garbage graphs can be temporarily brought into focus, ensure that before a refresh of the MyBlueprint window that the graph is owned by a Blueprint
	if (ViewportWidget.IsValid() && MyQuestWidget.IsValid())
	{
		// The focused graph can be garbage as well
		TWeakObjectPtr< UEdGraph > FocusedGraphPtr = ViewportWidget->GetCurrentGraph();
		UEdGraph* FocusedGraph = FocusedGraphPtr.Get();

		if (FocusedGraph != nullptr)
		{
			MyQuestWidget->Refresh();
		}
	}

	
}

void FAssetEditor_QuestSystem::OnGraphEditorBackgrounded(const TSharedRef<SGraphEditor>& InGraphEditor)
{
}

bool FAssetEditor_QuestSystem::IsGraphInCurrentQuestGraph(const UEdGraph* InGraph) const
{
	bool bEditable = true;

	UQuestSystemGraph* EditingBP = GetQuestSystemObj();
	if (EditingBP)
	{
		TArray<UEdGraph*> Graphs;
		EditingBP->GetAllGraphs(Graphs);
		bEditable &= Graphs.Contains(InGraph);
	}

	return bEditable;
}

UQuestEditorSettings* FAssetEditor_QuestSystem::GetSettings() const
{
	return QuestGraphEditorSettings;
}

FGraphAppearanceInfo FAssetEditor_QuestSystem::GetGraphAppearance() const
{
	FGraphAppearanceInfo AppearanceInfo;
	AppearanceInfo.CornerText = LOCTEXT("AppearanceCornerText", "Quest Editor");

	if (FAssetEditor_QuestSystem::IsPIESimulating())
	{
		if (GetQuestSystemObj()->QuestComponent)
		{
			AppearanceInfo.PIENotifyText = LOCTEXT("ActiveLabel", "ACTIVE");
		}
		else
		{
			AppearanceInfo.PIENotifyText = LOCTEXT("InactiveLabel", "INACTIVE");
		}
	}
	
	
	return AppearanceInfo;
}

bool FAssetEditor_QuestSystem::InEditingMode(bool bGraphIsEditable) const
{
	return bGraphIsEditable && FAssetEditor_QuestSystem::IsPIENotSimulating();
}


bool FAssetEditor_QuestSystem::IsPIESimulating()
{
	return GEditor->bIsSimulatingInEditor || GEditor->PlayWorld;
}

bool FAssetEditor_QuestSystem::IsPIENotSimulating()
{
	return !GEditor->bIsSimulatingInEditor && (GEditor->PlayWorld == NULL);
}

void FAssetEditor_QuestSystem::OnChangeBreadCrumbGraph(UEdGraph* InGraph)
{
}

TSharedRef<SDockTab> FAssetEditor_QuestSystem::SpawnTab_Viewport(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == FQuestSystemAssetEditorTabs::ViewportID);

	TSharedRef<SDockTab> SpawnedTab = SNew(SDockTab)
		.Label(LOCTEXT("ViewportTab_Title", "Viewport"));

	if (ViewportWidget.IsValid())
	{
		SpawnedTab->SetContent(ViewportWidget.ToSharedRef());
	}

	return SpawnedTab;
}

TSharedRef<SDockTab> FAssetEditor_QuestSystem::SpawnTab_Details(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == FQuestSystemAssetEditorTabs::GenericGraphPropertyID);

	return SNew(SDockTab)
#if ENGINE_MAJOR_VERSION < 5
		.Icon(FAppStyle::GetBrush("LevelEditor.Tabs.Details"))
#endif // #if ENGINE_MAJOR_VERSION < 5
		.Label(LOCTEXT("Details_Title", "Property"))
		[
			PropertyWidget.ToSharedRef()
		];
}

TSharedRef<SDockTab> FAssetEditor_QuestSystem::SpawnTab_EditorSettings(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == FQuestSystemAssetEditorTabs::GenericGraphEditorSettingsID);

	return SNew(SDockTab)
#if ENGINE_MAJOR_VERSION < 5
		.Icon(FAppStyle::GetBrush("LevelEditor.Tabs.Details"))
#endif // #if ENGINE_MAJOR_VERSION < 5
		.Label(LOCTEXT("EditorSettings_Title", "Generic Graph Editor Setttings"))
		[
			EditorSettingsWidget.ToSharedRef()
		];
}

TSharedRef<SDockTab> FAssetEditor_QuestSystem::SpawnTab_MyQuest(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == FQuestSystemAssetEditorTabs::MyQuestDetailID);

	return SNew(SDockTab)
#if ENGINE_MAJOR_VERSION < 5
		.Icon(FAppStyle::GetBrush("LevelEditor.Tabs.Details"))
#endif // #if ENGINE_MAJOR_VERSION < 5
		.Label(LOCTEXT("MyQuest_Title", "My Quest"))
		[
			MyQuestWidget.ToSharedRef()
		];
}

void FAssetEditor_QuestSystem::CreateInternalWidgets()
{
	//ViewportWidget = CreateViewportWidget();

	FDetailsViewArgs Args;
	Args.bHideSelectionTip = true;
	Args.NotifyHook = this;

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyWidget = PropertyModule.CreateDetailView(Args);
	PropertyWidget->SetObject(nullptr);
	PropertyWidget->OnFinishedChangingProperties().AddSP(this, &FAssetEditor_QuestSystem::OnFinishedChangingProperties);

	EditorSettingsWidget = PropertyModule.CreateDetailView(Args);
	EditorSettingsWidget->SetObject(QuestGraphEditorSettings);

	this->MyQuestWidget = SNew(SMyQuest, SharedThis(this));
}

TSharedRef<SGraphEditor> FAssetEditor_QuestSystem::CreateGraphEditorWidget(TSharedRef<class FTabInfo> InTabInfo, UEdGraph* InGraph)
{

	// Create the title bar widget
	TSharedPtr<SWidget> TitleBarWidget = CreateGraphTitleBarWidget(InTabInfo, InGraph);

	CreateCommandList();

	SGraphEditor::FGraphEditorEvents InEvents;
	InEvents.OnSelectionChanged = SGraphEditor::FOnSelectionChanged::CreateSP(this, &FAssetEditor_QuestSystem::OnSelectedNodesChanged);
	InEvents.OnNodeDoubleClicked = FSingleNodeEvent::CreateSP(this, &FAssetEditor_QuestSystem::OnNodeDoubleClicked);

	// Make full graph editor
	const bool bGraphIsEditable = InGraph->bEditable;
	return SNew(SGraphEditor)
		.AdditionalCommands(GraphEditorCommands)
		.IsEditable(this, &FAssetEditor_QuestSystem::InEditingMode, bGraphIsEditable)
		.TitleBar(TitleBarWidget)
		.Appearance(this, &FAssetEditor_QuestSystem::GetGraphAppearance)
		.GraphToEdit(InGraph)
		.GraphEvents(InEvents)
		.AutoExpandActionMenu(true);
}


void FAssetEditor_QuestSystem::RebuildQuestSystemGraph()
{
	if (EditingQuestGraph == nullptr)
	{
		//LOG_WARNING(TEXT("FGenericGraphAssetEditor::RebuildGenericGraph EditingQuestGraph is nullptr"));
		return;
	}

	for (UEdGraph* EdGraph : EditingQuestGraph->QuestGraphPages)
	{
		UEdGraph_QuestSystemGraph* QuestGraph = Cast<UEdGraph_QuestSystemGraph>(EdGraph);
		check(EdGraph != nullptr);
		QuestGraph->RebuildQuestSystemGraph();
	}
}
void FAssetEditor_QuestSystem::SelectAllNodes()
{
	TSharedPtr<SGraphEditor> CurrentGraphEditor = GetCurrGraphEditor();
	if (CurrentGraphEditor.IsValid())
	{
		CurrentGraphEditor->SelectAllNodes();
	}
}
bool FAssetEditor_QuestSystem::CanSelectAllNodes()
{
	return true;
}
void FAssetEditor_QuestSystem::DeleteSelectedNodes()
{
	TSharedPtr<SGraphEditor> CurrentGraphEditor = GetCurrGraphEditor();
	if (!CurrentGraphEditor.IsValid())
	{
		return;
	}

	const FScopedTransaction Transaction(FGenericCommands::Get().Delete->GetDescription());

	CurrentGraphEditor->GetCurrentGraph()->Modify();

	const FGraphPanelSelectionSet SelectedNodes = CurrentGraphEditor->GetSelectedNodes();
	CurrentGraphEditor->ClearSelectionSet();

	for (FGraphPanelSelectionSet::TConstIterator NodeIt(SelectedNodes); NodeIt; ++NodeIt)
	{
		UEdGraphNode* EdNode = Cast<UEdGraphNode>(*NodeIt);
		if (EdNode == nullptr || !EdNode->CanUserDeleteNode())
			continue;;

		if (UEdNode_QuestSystemNode* EdNode_Node = Cast<UEdNode_QuestSystemNode>(EdNode))
		{
			EdNode_Node->Modify();

			const UEdGraphSchema* Schema = EdNode_Node->GetSchema();
			if (Schema != nullptr)
			{
				Schema->BreakNodeLinks(*EdNode_Node);
			}

			EdNode_Node->DestroyNode();
		}
		else
		{
			EdNode->Modify();
			EdNode->DestroyNode();
		}
	}
}
bool FAssetEditor_QuestSystem::CanDeleteNodes()
{
	if (IsPIESimulating())
		return false;

	// If any of the nodes can be deleted then we should allow deleting
	const FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();
	for (FGraphPanelSelectionSet::TConstIterator SelectedIter(SelectedNodes); SelectedIter; ++SelectedIter)
	{
		UEdGraphNode* Node = Cast<UEdGraphNode>(*SelectedIter);
		if (Node != nullptr && Node->CanUserDeleteNode())
		{
			return true;
		}
	}
	return false;
}
void FAssetEditor_QuestSystem::DeleteSelectedDuplicatableNodes()
{
	TSharedPtr<SGraphEditor> CurrentGraphEditor = GetCurrGraphEditor();
	if (!CurrentGraphEditor.IsValid())
	{
		return;
	}

	const FGraphPanelSelectionSet OldSelectedNodes = CurrentGraphEditor->GetSelectedNodes();
	CurrentGraphEditor->ClearSelectionSet();

	for (FGraphPanelSelectionSet::TConstIterator SelectedIter(OldSelectedNodes); SelectedIter; ++SelectedIter)
	{
		UEdGraphNode* Node = Cast<UEdGraphNode>(*SelectedIter);
		if (Node && Node->CanDuplicateNode())
		{
			CurrentGraphEditor->SetNodeSelection(Node, true);
		}
	}

	// Delete the duplicatable nodes
	DeleteSelectedNodes();

	CurrentGraphEditor->ClearSelectionSet();

	for (FGraphPanelSelectionSet::TConstIterator SelectedIter(OldSelectedNodes); SelectedIter; ++SelectedIter)
	{
		if (UEdGraphNode* Node = Cast<UEdGraphNode>(*SelectedIter))
		{
			CurrentGraphEditor->SetNodeSelection(Node, true);
		}
	}
}
void FAssetEditor_QuestSystem::CutSelectedNodes()
{
	CopySelectedNodes();
	DeleteSelectedDuplicatableNodes();
	ShouldGetNewID = false;
}
bool FAssetEditor_QuestSystem::CanCutNodes()
{
	if (IsPIESimulating())
		return false;
	return CanCopyNodes() && CanDeleteNodes();
}

void FAssetEditor_QuestSystem::CopySelectedNodes()
{
	// Export the selected nodes and place the text on the clipboard
	FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();

	FString ExportedText;

	ShouldGetNewID = true;

	for (FGraphPanelSelectionSet::TIterator SelectedIter(SelectedNodes); SelectedIter; ++SelectedIter)
	{
		UEdGraphNode* Node = Cast<UEdGraphNode>(*SelectedIter);
		if (Node == nullptr)
		{
			SelectedIter.RemoveCurrent();
			continue;
		}

		if (UEdNode_QuestSystemEdge* EdNode_Edge = Cast<UEdNode_QuestSystemEdge>(*SelectedIter))
		{
			UEdNode_QuestSystemNode* StartNode = EdNode_Edge->GetStartNode();
			UEdNode_QuestSystemNode* EndNode = EdNode_Edge->GetEndNode();

			if (!SelectedNodes.Contains(StartNode) || !SelectedNodes.Contains(EndNode))
			{
				SelectedIter.RemoveCurrent();
				continue;
			}
		}

		Node->PrepareForCopying();
	}

	FEdGraphUtilities::ExportNodesToText(SelectedNodes, ExportedText);
	FPlatformApplicationMisc::ClipboardCopy(*ExportedText);
}
bool FAssetEditor_QuestSystem::CanCopyNodes()
{
	// If any of the nodes can be duplicated then we should allow copying
	const FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();
	for (FGraphPanelSelectionSet::TConstIterator SelectedIter(SelectedNodes); SelectedIter; ++SelectedIter)
	{
		UEdGraphNode* Node = Cast<UEdGraphNode>(*SelectedIter);
		if (Node && Node->CanDuplicateNode())
		{
			return true;
		}
	}

	return false;
}
void FAssetEditor_QuestSystem::PasteNodes()
{
	TSharedPtr<SGraphEditor> CurrentGraphEditor = GetCurrGraphEditor();
	if (CurrentGraphEditor.IsValid())
	{
		PasteNodesHere(CurrentGraphEditor->GetCurrentGraph(), CurrentGraphEditor->GetPasteLocation());
	}
}
void FAssetEditor_QuestSystem::PasteNodesHere(UEdGraph* DestinationGraph, const FVector2D& Location)
{
	// Find the graph editor with focus
	TSharedPtr<SGraphEditor> CurrentGraphEditor = GetCurrGraphEditor();
	if (!CurrentGraphEditor.IsValid())
	{
		return;
	}
	// Select the newly pasted stuff
	UEdGraph* EdGraph = CurrentGraphEditor->GetCurrentGraph();

	{
		const FScopedTransaction Transaction(FGenericCommands::Get().Paste->GetDescription());
		EdGraph->Modify();

		// Clear the selection set (newly pasted stuff will be selected)
		CurrentGraphEditor->ClearSelectionSet();

		// Grab the text to paste from the clipboard.
		FString TextToImport;
		FPlatformApplicationMisc::ClipboardPaste(TextToImport);

		// Import the nodes
		TSet<UEdGraphNode*> PastedNodes;
		FEdGraphUtilities::ImportNodesFromText(EdGraph, TextToImport, PastedNodes);

		//Average position of nodes so we can move them while still maintaining relative distances to each other
		FVector2D AvgNodePosition(0.0f, 0.0f);

		for (TSet<UEdGraphNode*>::TIterator It(PastedNodes); It; ++It)
		{
			UEdGraphNode* Node = *It;
			AvgNodePosition.X += Node->NodePosX;
			AvgNodePosition.Y += Node->NodePosY;

			if (ShouldGetNewID)
			{
				if (UEdNode_QuestSystemNode* EdNode = Cast<UEdNode_QuestSystemNode>(*It))
				{
					EdNode->QuestSystemGraphNode->ID = FQuestSystemEditorUtils::FindUniqueNodeName(EdNode->QuestSystemGraphNode->ID.ToString());
				}
			}
			else
			{
				ShouldGetNewID = true;
			}
		}

		float InvNumNodes = 1.0f / float(PastedNodes.Num());
		AvgNodePosition.X *= InvNumNodes;
		AvgNodePosition.Y *= InvNumNodes;

		for (TSet<UEdGraphNode*>::TIterator It(PastedNodes); It; ++It)
		{
			UEdGraphNode* Node = *It;
			CurrentGraphEditor->SetNodeSelection(Node, true);

			Node->NodePosX = (Node->NodePosX - AvgNodePosition.X) + Location.X;
			Node->NodePosY = (Node->NodePosY - AvgNodePosition.Y) + Location.Y;

			Node->SnapToGrid(16);

			// Give new node a different Guid from the old one
			Node->CreateNewGuid();
		}
	}

	// Update UI
	CurrentGraphEditor->NotifyGraphChanged();

	UObject* GraphOwner = EdGraph->GetOuter();
	if (GraphOwner)
	{
		GraphOwner->PostEditChange();
		GraphOwner->MarkPackageDirty();
	}
}

bool FAssetEditor_QuestSystem::CanPasteNodes() const
{
	TSharedPtr<SGraphEditor> CurrentGraphEditor = GetCurrGraphEditor();
	if (!CurrentGraphEditor.IsValid())
	{
		return false;
	}

	FString ClipboardContent;
	FPlatformApplicationMisc::ClipboardPaste(ClipboardContent);

	return FEdGraphUtilities::CanImportNodesFromText(CurrentGraphEditor->GetCurrentGraph(), ClipboardContent);
}
void FAssetEditor_QuestSystem::DuplicateNodes()
{
	ShouldGetNewID = true;
	CopySelectedNodes();
	PasteNodes();
}
bool FAssetEditor_QuestSystem::CanDuplicateNodes()
{
	return CanCopyNodes();
}
void FAssetEditor_QuestSystem::CreateNewTask()
{
	HandleNewNodeClassPicked(UQuestTask::StaticClass());
}

void FAssetEditor_QuestSystem::HandleNewNodeClassPicked(UClass* InClass) const
{
	/*UE_CLOG(InClass == nullptr, LogBehaviorTreeEditor, Error, TEXT("Trying to handle new node of NULL class for Behavior Treee %s ")
		, *GetNameSafe(BehaviorTree));*/

	if (EditingQuestGraph != nullptr && InClass != nullptr && EditingQuestGraph->GetOutermost())
	{
		const FString ClassName = FBlueprintEditorUtils::GetClassNameWithoutSuffix(InClass);

		FString PathName = EditingQuestGraph->GetOutermost()->GetPathName();
		PathName = FPaths::GetPath(PathName);

		// Now that we've generated some reasonable default locations/names for the package, allow the user to have the final say
		// before we create the package and initialize the blueprint inside of it.
		FSaveAssetDialogConfig SaveAssetDialogConfig;
		SaveAssetDialogConfig.DialogTitleOverride = LOCTEXT("SaveAssetDialogTitle", "Save Asset As");
		SaveAssetDialogConfig.DefaultPath = PathName;
		SaveAssetDialogConfig.DefaultAssetName = ClassName + TEXT("_New");
		SaveAssetDialogConfig.ExistingAssetPolicy = ESaveAssetDialogExistingAssetPolicy::Disallow;

		const FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
		const FString SaveObjectPath = ContentBrowserModule.Get().CreateModalSaveAssetDialog(SaveAssetDialogConfig);
		if (!SaveObjectPath.IsEmpty())
		{
			const FString SavePackageName = FPackageName::ObjectPathToPackageName(SaveObjectPath);
			const FString SavePackagePath = FPaths::GetPath(SavePackageName);
			const FString SaveAssetName = FPaths::GetBaseFilename(SavePackageName);

			UPackage* Package = CreatePackage(*SavePackageName);
			if (ensure(Package))
			{
				// Create and init a new Blueprint
				if (UBlueprint* NewBP = FKismetEditorUtilities::CreateBlueprint(InClass, Package, FName(*SaveAssetName), BPTYPE_Normal, UBlueprint::StaticClass(), UBlueprintGeneratedClass::StaticClass()))
				{
					GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(NewBP);

					// Notify the asset registry
					FAssetRegistryModule::AssetCreated(NewBP);

					// Mark the package dirty...
					Package->MarkPackageDirty();
				}
			}
		}
	}

	FSlateApplication::Get().DismissAllMenus();
}

bool FAssetEditor_QuestSystem::CanCreateNewTask() const
{
	return true;
}
bool FAssetEditor_QuestSystem::IsNewTaskButtonVisible() const
{
	return true;
}

void FAssetEditor_QuestSystem::CreateNewQuestCondition()
{
	HandleNewNodeClassPicked(UQuestCondition::StaticClass());
}
bool FAssetEditor_QuestSystem::CanCreateQuestCondition() const
{
	return true;
}
void FAssetEditor_QuestSystem::CreateNewQuestEvent()
{
	HandleNewNodeClassPicked(UQuestEvent::StaticClass());
}
bool FAssetEditor_QuestSystem::CanCreateQuestEvent() const
{
	return true;
}
void FAssetEditor_QuestSystem::OpenQuestSetting()
{
	PropertyWidget->SetObject(GetQuestSystemObj());
}
bool FAssetEditor_QuestSystem::CanOpenQuestSetting() const
{
	return true;
}
void FAssetEditor_QuestSystem::OnRenameNode()
{
	TSharedPtr<SGraphEditor> CurrentGraphEditor = GetCurrGraphEditor();
	if (CurrentGraphEditor.IsValid())
	{
		const FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();
		for (FGraphPanelSelectionSet::TConstIterator NodeIt(SelectedNodes); NodeIt; ++NodeIt)
		{
			UEdGraphNode* SelectedNode = Cast<UEdGraphNode>(*NodeIt);
			if (SelectedNode != NULL && SelectedNode->bCanRenameNode)
			{
				CurrentGraphEditor->IsNodeTitleVisible(SelectedNode, true);
				break;
			}
		}
	}
}
bool FAssetEditor_QuestSystem::CanRenameNodes() const
{
	if (GetFocusedGraph())
	{
		if (const UEdGraphNode* SelectedNode = GetSingleSelectedNode())
		{
			return SelectedNode->GetCanRenameNode();
		}
	}
	return false;

}
void FAssetEditor_QuestSystem::OnSelectedNodesChanged(const TSet<class UObject*>& NewSelection)
{
	TArray<UObject*> Selection;

	for (UObject* SelectionEntry : NewSelection)
	{
		Selection.Add(SelectionEntry);
	}

	if (Selection.Num() == 0)
	{
		if (UEdGraph_QuestSystemGraph* QuestGraph = Cast<UEdGraph_QuestSystemGraph>(ViewportWidget->GetCurrentGraph()))
		{
			PropertyWidget->SetObject(QuestGraph->QuestSystem);
		}
	}
	else if (Selection.Num() == 1)
	{
		if (UQuestSystemGraphNode_Root* RootNode = Cast<UQuestSystemGraphNode_Root>(Selection[0]))
		{
			if (UEdGraph_QuestSystemGraph* QuestGraph = Cast<UEdGraph_QuestSystemGraph>(ViewportWidget->GetCurrentGraph()))
			{
				PropertyWidget->SetObject(QuestGraph->QuestSystem);
			}
		}
		else
		{
			PropertyWidget->SetObjects(Selection);
		}
	}
	else
	{
		PropertyWidget->SetObjects(Selection);
	}
}

void FAssetEditor_QuestSystem::OnNodeDoubleClicked(UEdGraphNode* Node)
{
	UEdNode_QuestSystemNode* MyNode = Cast<UEdNode_QuestSystemNode>(Node);
	if (MyNode && MyNode->QuestSystemGraphNode &&
		MyNode->QuestSystemGraphNode->GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint))
	{
		UClass* NodeClass = MyNode->QuestSystemGraphNode->GetClass();
		UPackage* Pkg = NodeClass->GetOuterUPackage();
		FString ClassName = NodeClass->GetName().LeftChop(2);
		UBlueprint* BlueprintOb = FindObject<UBlueprint>(Pkg, *ClassName);
		if (BlueprintOb)
		{
			GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(BlueprintOb);
		}
	}
}

void FAssetEditor_QuestSystem::OnFinishedChangingProperties(const FPropertyChangedEvent& PropertyChangedEvent)
{
	if (EditingQuestGraph == nullptr)
		return;

	for (UEdGraph* EdGraph : EditingQuestGraph->QuestGraphPages)
	{
		EdGraph->GetSchema()->ForceVisualizationCacheClear();
	}
	DocumentManager->RefreshAllTabs();

}
//Called when saving our file graph
#if ENGINE_MAJOR_VERSION < 5

void FAssetEditor_QuestSystem::OnPackageSaved(const FString& PackageFileName, UObject* Outer)
{
	RebuildQuestSystemGraph();
}
#else // #if ENGINE_MAJOR_VERSION < 5
void FAssetEditor_QuestSystem::OnPackageSavedWithContext(const FString& PackageFileName, UPackage* Package, FObjectPostSaveContext ObjectSaveContext)
{
	RebuildQuestSystemGraph();
	if (QuestGraphEditorSettings == nullptr) {
		UE_LOG(LogTemp, Log, TEXT("Null"))
	}
}

#endif // #else // #if ENGINE_MAJOR_VERSION < 5

#undef LOCTEXT_NAMESPACE
