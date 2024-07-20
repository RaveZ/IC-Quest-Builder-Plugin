// Copyright 2024 Ivan Chandra. All Rights Reserved.

#include "Quest_System_EditorStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Framework/Application/SlateApplication.h"
#include "Slate/SlateGameResources.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleMacros.h"

#define RootToContentDir Style->RootToContentDir

TSharedPtr<FSlateStyleSet> FQuest_System_EditorStyle::StyleInstance = nullptr;

void FQuest_System_EditorStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FQuest_System_EditorStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

FName FQuest_System_EditorStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("Quest_System_EditorStyle"));
	return StyleSetName;
}

const FVector2D Icon16x16(16.0f, 16.0f);
const FVector2D Icon20x20(20.0f, 20.0f);
const FVector2D Icon40x40(40.0f, 40.0f);
const FVector2D Icon64x64(64.0f, 64.0f);

TSharedRef< FSlateStyleSet > FQuest_System_EditorStyle::Create()
{
	TSharedRef< FSlateStyleSet > Style = MakeShareable(new FSlateStyleSet("Quest_System_EditorStyle"));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("Quest_System_Editor")->GetBaseDir() / TEXT("Resources"));

	Style->Set("Quest_System_Editor.OpenPluginWindow", new IMAGE_BRUSH_SVG(TEXT("PlaceholderButtonIcon"), Icon20x20));
	Style->Set("Quest_System_Editor.questicon", new IMAGE_BRUSH_SVG(TEXT("questicon"), Icon40x40));
	Style->Set("Quest_System_Editor.questicon.small", new IMAGE_BRUSH_SVG(TEXT("questicon"), Icon20x20));
	return Style;
}

void FQuest_System_EditorStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

const ISlateStyle& FQuest_System_EditorStyle::Get()
{
	return *StyleInstance;
}
