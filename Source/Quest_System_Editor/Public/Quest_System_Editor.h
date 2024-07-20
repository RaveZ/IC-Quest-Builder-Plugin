// Copyright 2024 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIGraphTypes.h"
#include "Modules/ModuleManager.h"
#include <EdGraphUtilities.h>
#include "SSubobjectEditor.h"
#include <IAssetTools.h>
#include <QuestSystemNodeFactory.h>

class FToolBarBuilder;
class FMenuBuilder;
class UQuestSystem;

/**
 * Quest editor public interface
 */
class QUEST_SYSTEM_EDITOR_API IQuestEditor : public FWorkflowCentricApplication
{
public:
	virtual void JumpToHyperlink(const UObject* ObjectReference, bool bRequestRename) = 0;
	virtual void JumpToPin(const UEdGraphPin* PinToFocusOn) = 0;

	/** Invokes the search UI and sets the mode and search terms optionally */
	virtual void SummonSearchUI(bool bSetFindWithinQuestSystem, FString NewSearchTerms = FString(), bool bSelectFirstResult = false) = 0;

	/** Invokes the Find and Replace UI */
	virtual void SummonFindAndReplaceUI() = 0;

	/** Tries to open the specified graph and bring it's document to the front (note: this can return NULL) */
	virtual TSharedPtr<class SGraphEditor> OpenGraphAndBringToFront(class UEdGraph* Graph, bool bSetFocus = true) = 0;

	virtual void RefreshEditors() = 0;

	virtual void RefreshMyQuest() = 0;

	virtual void RefreshInspector() = 0;

	virtual void AddToSelection(UEdGraphNode* InNode) = 0;

	virtual bool CanPasteNodes() const = 0;

	virtual void PasteNodesHere(class UEdGraph* Graph, const FVector2D& Location) = 0;

	/** Return the class viewer filter associated with the current set of imported namespaces within this editor context. Default is NULL (no filter). */
	virtual TSharedPtr<class IClassViewerFilter> GetImportedClassViewerFilter() const { return nullptr; }

	/** Return the pin type selector filter associated with the current set of imported namespaces within this editor context. Default is NULL (no filter). */
	UE_DEPRECATED(5.1, "Please use GetPinTypeSelectorFilters")
		virtual TSharedPtr<class IPinTypeSelectorFilter> GetImportedPinTypeSelectorFilter() const { return nullptr; }

	/** Get all the the pin type selector filters within this editor context. */
	virtual void GetPinTypeSelectorFilters(TArray<TSharedPtr<class IPinTypeSelectorFilter>>& OutFilters) const {}

	/** Return whether the given object falls outside the scope of the current set of imported namespaces within this editor context. Default is FALSE (imported). */
	virtual bool IsNonImportedObject(const UObject* InObject) const { return false; }

	/** Return whether the given object (referenced by path) falls outside the scope of the current set of imported namespaces within this editor context. Default is FALSE (imported). */
	virtual bool IsNonImportedObject(const FSoftObjectPath& InObject) const { return false; }

};


class FQuest_System_EditorModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
	/** This function will be bound to Command (by default it will bring up plugin window) */
	void PluginButtonClicked();

	TSharedPtr<struct FGraphNodeClassHelper> GetClassCache() { return ClassCache; }
	
	void CheckClassCache();


private:

	void RegisterMenus();

	void AddToolbarExtension(FToolBarBuilder& Builder);
	void AddMenuExtension(FMenuBuilder& Builder);

	void RegisterAssetTypeAction(IAssetTools& AssetTools, TSharedRef<IAssetTypeActions> Action);

	TSharedRef<class SDockTab> OnSpawnPluginTab(const class FSpawnTabArgs& SpawnTabArgs);

private:
	TSharedPtr<class FUICommandList> PluginCommands;
	TSharedPtr<FQuestSystemNodeFactory> GraphPanelNodeFactory_QuestEditor;
	TSharedPtr<class IDetailsView> PropertyWidget;

	EAssetTypeCategories::Type QuestEditorAssetCategoryBit;
	TArray< TSharedPtr<IAssetTypeActions> > CreatedAssetTypeActions;

	TSharedPtr<struct FGraphNodeClassHelper> ClassCache;
};
