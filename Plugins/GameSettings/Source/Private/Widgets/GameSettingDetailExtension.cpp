// Copyright Epic Games, Inc. All Rights Reserved.

#include "Widgets/GameSettingDetailExtension.h"
#include "GameSetting.h"

void UGameSettingDetailExtension::SetSetting(UGameSetting* InSetting)
{
	NativeSettingAssigned(InSetting);
}

void UGameSettingDetailExtension::NativeSettingAssigned(UGameSetting* InSetting)
{
	if (Setting)
	{
		Setting->OnSettingChangedEvent.RemoveAll(this);
	}

	Setting = InSetting;
	Setting->OnSettingChangedEvent.AddUObject(this, &ThisClass::NativeSettingValueChanged);

	OnSettingAssigned(InSetting);
}

void UGameSettingDetailExtension::NativeSettingValueChanged(UGameSetting* InSetting, EGameSettingChangeReason Reason)
{
	OnSettingValueChanged(InSetting);
}