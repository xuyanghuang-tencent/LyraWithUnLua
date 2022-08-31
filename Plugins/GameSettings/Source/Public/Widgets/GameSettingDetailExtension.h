// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameSettingFilterState.h"
#include "Blueprint/UserWidget.h"
#include "GameSettingDetailExtension.generated.h"

class UGameSetting;

/**
 * 
 */
UCLASS(Abstract, meta = (Category = "Settings", DisableNativeTick))
class GAMESETTINGS_API UGameSettingDetailExtension : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetSetting(UGameSetting* InSetting);
	
protected:
	virtual void NativeSettingAssigned(UGameSetting* InSetting);
	virtual void NativeSettingValueChanged(UGameSetting* InSetting, EGameSettingChangeReason Reason);

	UFUNCTION(BlueprintImplementableEvent)
	void OnSettingAssigned(UGameSetting* InSetting);

	UFUNCTION(BlueprintImplementableEvent)
	void OnSettingValueChanged(UGameSetting* InSetting);

protected:
	UPROPERTY(Transient)
	UGameSetting* Setting;
};
