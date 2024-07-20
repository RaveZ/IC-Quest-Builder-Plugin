// Copyright 2024 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Setting_QuestEditor.generated.h"

UENUM(BlueprintType)
enum class EAutoLayoutType : uint8
{
	Tree,
	ForceDirected,
};

UCLASS()
class QUEST_SYSTEM_EDITOR_API UQuestEditorSettings : public UObject
{
	GENERATED_BODY()

	UQuestEditorSettings();
	virtual ~UQuestEditorSettings();

	UPROPERTY(EditDefaultsOnly, Category = "AutoArrange")
		float OptimalDistance;

	UPROPERTY(EditDefaultsOnly, AdvancedDisplay, Category = "AutoArrange")
		EAutoLayoutType AutoLayoutStrategy;

	UPROPERTY(EditDefaultsOnly, AdvancedDisplay, Category = "AutoArrange")
		int32 MaxIteration;

	UPROPERTY(EditDefaultsOnly, AdvancedDisplay, Category = "AutoArrange")
		bool bFirstPassOnly;

	UPROPERTY(EditDefaultsOnly, AdvancedDisplay, Category = "AutoArrange")
		bool bRandomInit;

	UPROPERTY(EditDefaultsOnly, AdvancedDisplay, Category = "AutoArrange")
		float InitTemperature;

	UPROPERTY(EditDefaultsOnly, AdvancedDisplay, Category = "AutoArrange")
		float CoolDownRate;
};
