// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "GameSettingRegistry.h"
#include "DataSource/GameSettingDataSourceDynamic.h"

#include "LyraGameSettingRegistry.generated.h"

//--------------------------------------
// ULyraGameSettingRegistry
//--------------------------------------

class ULyraLocalPlayer;
class ULyraClientSettingsRecord;
class ULyraGameUserSettings;
class UGameSettingCollection;
class UGameSetting;
struct FGameSettingsFilterState;
class ULyraContentControlSettingRegistry;

DECLARE_LOG_CATEGORY_EXTERN(LogLyraGameSettingRegistry, Log, Log);

#define GET_SHARED_SETTINGS_FUNCTION_PATH(FunctionOrPropertyName)							\
	MakeShared<FGameSettingDataSourceDynamic>(TArray<FString>({								\
		GET_FUNCTION_NAME_STRING_CHECKED(ULyraLocalPlayer, GetSharedSettings),				\
		GET_FUNCTION_NAME_STRING_CHECKED(ULyraSettingsShared, FunctionOrPropertyName)		\
	}))

#define GET_LOCAL_SETTINGS_FUNCTION_PATH(FunctionOrPropertyName)							\
	MakeShared<FGameSettingDataSourceDynamic>(TArray<FString>({								\
		GET_FUNCTION_NAME_STRING_CHECKED(ULyraLocalPlayer, GetLocalSettings),				\
		GET_FUNCTION_NAME_STRING_CHECKED(ULyraSettingsLocal, FunctionOrPropertyName)		\
	}))

/**
 * 
 */
UCLASS()
class ULyraGameSettingRegistry : public UGameSettingRegistry
{
	GENERATED_BODY()

public:
	ULyraGameSettingRegistry();

	static ULyraGameSettingRegistry* Get(ULyraLocalPlayer* InLocalPlayer);
	
	virtual void SaveChanges() override;

protected:
	virtual void OnInitialize(ULocalPlayer* InLocalPlayer) override;
	virtual bool IsFinishedInitializing() const override;

	UGameSettingCollection* InitializeVideoSettings(ULyraLocalPlayer* InLocalPlayer);
	void InitializeVideoSettings_FrameRates(UGameSettingCollection* Screen, ULyraLocalPlayer* InLocalPlayer);
	void AddPerformanceStatPage(UGameSettingCollection* Screen, ULyraLocalPlayer* InLocalPlayer);

	UGameSettingCollection* InitializeAudioSettings(ULyraLocalPlayer* InLocalPlayer);
	UGameSettingCollection* InitializeGameplaySettings(ULyraLocalPlayer* InLocalPlayer);

	UGameSettingCollection* InitializeMouseAndKeyboardSettings(ULyraLocalPlayer* InLocalPlayer);
	UGameSettingCollection* InitializeGamepadSettings(ULyraLocalPlayer* InLocalPlayer);

	UPROPERTY()
	UGameSettingCollection* VideoSettings;

	UPROPERTY()
	UGameSettingCollection* AudioSettings;

	UPROPERTY()
	UGameSettingCollection* GameplaySettings;

	UPROPERTY()
	UGameSettingCollection* MouseAndKeyboardSettings;

	UPROPERTY()
	UGameSettingCollection* GamepadSettings;
};
