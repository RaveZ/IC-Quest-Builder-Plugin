// Copyright 2024 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AssetTypeActions_Base.h"
//#include "AssetTypeActions_QuestEditor.generated.h"

class QUEST_SYSTEM_EDITOR_API FAssetTypeActions_QuestSystem : public FAssetTypeActions_Base
{
public:
	//GENERATED_BODY()
	FAssetTypeActions_QuestSystem(EAssetTypeCategories::Type InAssetCategory);

	virtual FText GetName() const override;
	virtual FColor GetTypeColor() const override;
	virtual UClass* GetSupportedClass() const override;
	virtual void OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor = TSharedPtr<IToolkitHost>()) override;
	virtual uint32 GetCategories() override;

private:
	EAssetTypeCategories::Type MyAssetCategory;
	
};
