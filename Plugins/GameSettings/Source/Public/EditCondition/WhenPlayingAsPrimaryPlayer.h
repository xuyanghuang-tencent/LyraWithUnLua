// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "GameSetting.h"

class ULocalPlayer;
class FGameSettingEditableState;


class GAMESETTINGS_API FWhenPlayingAsPrimaryPlayer : public FGameSettingEditCondition
{
public:
	static TSharedRef<FWhenPlayingAsPrimaryPlayer> Get();

	virtual void GatherEditState(const ULocalPlayer* InLocalPlayer, FGameSettingEditableState& InOutEditState) const override;
};
