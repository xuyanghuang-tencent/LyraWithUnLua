// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraEditor.h"
#include "Modules/ModuleManager.h"
#include "Modules/ModuleInterface.h"
#include "GameplayAbilitiesModule.h"
#include "GameplayAbilitiesEditorModule.h"
#include "AbilitySystemGlobals.h"
#include "GameplayCueInterface.h"
#include "GameplayCueNotify_Burst.h"
#include "GameplayCueNotify_BurstLatent.h"
#include "GameplayCueNotify_Looping.h"
#include "Editor.h"
#include "UnrealEdGlobals.h"
#include "ToolMenus.h"
#include "Editor/UnrealEdEngine.h"
#include "Private/AssetTypeActions_LyraContextEffectsLibrary.h"
#include "Validation/EditorValidator.h"
#include "GameEditorStyle.h"
#include "GameModes/LyraExperienceManager.h"

#define LOCTEXT_NAMESPACE "LyraEditor"

DEFINE_LOG_CATEGORY(LogLyraEditor);

// This function tells the GameplayCue editor what classes to expose when creating new notifies.
static void GetGameplayCueDefaultClasses(TArray<UClass*>& Classes)
{
	Classes.Empty();
	Classes.Add(UGameplayCueNotify_Burst::StaticClass());
	Classes.Add(AGameplayCueNotify_BurstLatent::StaticClass());
	Classes.Add(AGameplayCueNotify_Looping::StaticClass());
}

// This function tells the GameplayCue editor what classes to search for GameplayCue events.
static void GetGameplayCueInterfaceClasses(TArray<UClass*>& Classes)
{
	Classes.Empty();

	for (UClass* Class : TObjectRange<UClass>())
	{
		if (Class->IsChildOf<AActor>() && Class->ImplementsInterface(UGameplayCueInterface::StaticClass()))
		{
			Classes.Add(Class);
		}
	}
}

// This function tells the GameplayCue editor where to create the GameplayCue notifies based on their tag.
static FString GetGameplayCuePath(FString GameplayCueTag)
{
	FString Path = FString(TEXT("/Game"));

	//@TODO: Try plugins (e.g., GameplayCue.ShooterGame.Foo should be in ShooterCore or something)

	// Default path to the first entry in the UAbilitySystemGlobals::GameplayCueNotifyPaths.
	if (IGameplayAbilitiesModule::IsAvailable())
	{
		IGameplayAbilitiesModule& GameplayAbilitiesModule = IGameplayAbilitiesModule::Get();

		if (GameplayAbilitiesModule.IsAbilitySystemGlobalsAvailable())
		{
			UAbilitySystemGlobals* AbilitySystemGlobals = GameplayAbilitiesModule.GetAbilitySystemGlobals();
			check(AbilitySystemGlobals);

			TArray<FString> GetGameplayCueNotifyPaths = AbilitySystemGlobals->GetGameplayCueNotifyPaths();

			if (GetGameplayCueNotifyPaths.Num() > 0)
			{
				Path = GetGameplayCueNotifyPaths[0];
			}
		}
	}

	GameplayCueTag.RemoveFromStart(TEXT("GameplayCue."));

	FString NewDefaultPathName = FString::Printf(TEXT("%s/GCN_%s"), *Path, *GameplayCueTag);

	return NewDefaultPathName;
}

static bool HasPlayWorld()
{
	return GEditor->PlayWorld != nullptr;
}

static bool HasNoPlayWorld()
{
	return !HasPlayWorld();
}

static bool HasPlayWorldAndRunning()
{
	return HasPlayWorld() && !GUnrealEd->PlayWorld->bDebugPauseExecution;
}

static void AddPlayer_Clicked()
{
	if (ensure(GEditor->PlayWorld))
	{
		if (UGameInstance* GameInstance = GEditor->PlayWorld->GetGameInstance())
		{
			if (GameInstance->GetNumLocalPlayers() == 1)
			{
				GameInstance->DebugCreatePlayer(1);
			}
			else
			{
				GameInstance->DebugRemovePlayer(1);
			}
		}
	}
}

static TSharedRef<SWidget> AddLocalPlayer()
{
	FMenuBuilder MenuBuilder(true, nullptr);

	MenuBuilder.AddMenuEntry(
		LOCTEXT("SplitscreenButton", "Splitscreen"),
		LOCTEXT("SplitscreenDescription", "Adds/Removes a Splitscreen Player to the current PIE session"),
		FSlateIcon(),
		FUIAction(
			FExecuteAction::CreateStatic(&AddPlayer_Clicked),
			FCanExecuteAction::CreateStatic(&HasPlayWorld),
			FIsActionChecked(),
			FIsActionButtonVisible::CreateStatic(&HasPlayWorld)
		)
	);

	return MenuBuilder.MakeWidget();
}

static void CheckGameContent_Clicked()
{
	UEditorValidator::ValidateCheckedOutContent(/*bInteractive=*/true, EDataValidationUsecase::Manual);
}

static void RegisterGameEditorMenus()
{
	UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar");
	FToolMenuSection& Section = Menu->AddSection("PlayGameExtensions", TAttribute<FText>(), FToolMenuInsert("Play", EToolMenuInsertType::After));
	
	FToolMenuEntry BlueprintEntry = FToolMenuEntry::InitComboButton(
		"OpenGameMenu",
		FUIAction(
			FExecuteAction(),
			FCanExecuteAction::CreateStatic(&HasPlayWorld),
			FIsActionChecked(),
			FIsActionButtonVisible::CreateStatic(&HasPlayWorld)),
		FOnGetContent::CreateStatic(&AddLocalPlayer),
		LOCTEXT("GameOptions_Label", "Game Options"),
		LOCTEXT("GameOptions_ToolTip", "Game Options"),
		FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.OpenLevelBlueprint")
	);
	BlueprintEntry.StyleNameOverride = "CalloutToolbar";
	Section.AddEntry(BlueprintEntry);

	FToolMenuEntry CheckContentEntry = FToolMenuEntry::InitToolBarButton(
		"CheckContent",
		FUIAction(
			FExecuteAction::CreateStatic(&CheckGameContent_Clicked),
			FCanExecuteAction::CreateStatic(&HasNoPlayWorld),
			FIsActionChecked(),
			FIsActionButtonVisible::CreateStatic(&HasNoPlayWorld)),
		LOCTEXT( "CheckContentButton", "Check Content" ),
		LOCTEXT( "CheckContentDescription", "Runs the Content Validation job on all checked out assets to look for warnings and errors" ),
		FSlateIcon(FGameEditorStyle::GetStyleSetName(), "GameEditor.CheckContent")
	);
	CheckContentEntry.StyleNameOverride = "CalloutToolbar";
	Section.AddEntry(CheckContentEntry);
}

/**
 * FLyraEditorModule
 */
class FLyraEditorModule : public FDefaultGameModuleImpl
{
	typedef FLyraEditorModule ThisClass;

	virtual void StartupModule() override
	{
		FGameEditorStyle::Initialize();

		if (!IsRunningGame())
		{
			FModuleManager::Get().OnModulesChanged().AddRaw(this, &FLyraEditorModule::ModulesChangedCallback);

			BindGameplayAbilitiesEditorDelegates();

			if (FSlateApplication::IsInitialized())
			{
				UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateStatic(&RegisterGameEditorMenus));
			}

			FEditorDelegates::BeginPIE.AddRaw(this, &ThisClass::OnBeginPIE);
			FEditorDelegates::EndPIE.AddRaw(this, &ThisClass::OnEndPIE);
		}

		// Register the Context Effects Library asset type actions.
		IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

		AssetTools.RegisterAssetTypeActions(MakeShareable(new FAssetTypeActions_LyraContextEffectsLibrary));
	}

	void OnBeginPIE(bool bIsSimulating)
	{
		ULyraExperienceManager* ExperienceManager = GEngine->GetEngineSubsystem<ULyraExperienceManager>();
		check(ExperienceManager);
		ExperienceManager->OnPlayInEditorBegun();
	}

	void OnEndPIE(bool bIsSimulating)
	{
	}

	virtual void ShutdownModule() override
	{
		FModuleManager::Get().OnModulesChanged().RemoveAll(this);
	}

protected:

	static void BindGameplayAbilitiesEditorDelegates()
	{
		IGameplayAbilitiesEditorModule& GameplayAbilitiesEditorModule = IGameplayAbilitiesEditorModule::Get();

		GameplayAbilitiesEditorModule.GetGameplayCueNotifyClassesDelegate().BindStatic(&GetGameplayCueDefaultClasses);
		GameplayAbilitiesEditorModule.GetGameplayCueInterfaceClassesDelegate().BindStatic(&GetGameplayCueInterfaceClasses);
		GameplayAbilitiesEditorModule.GetGameplayCueNotifyPathDelegate().BindStatic(&GetGameplayCuePath);
	}

	void ModulesChangedCallback(FName ModuleThatChanged, EModuleChangeReason ReasonForChange)
	{
		if ((ReasonForChange == EModuleChangeReason::ModuleLoaded) && (ModuleThatChanged.ToString() == TEXT("GameplayAbilitiesEditor")))
		{
			BindGameplayAbilitiesEditorDelegates();
		}
	}
};

IMPLEMENT_MODULE(FLyraEditorModule, LyraEditor);

#undef LOCTEXT_NAMESPACE