// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CommonActivatableWidget.h"

#include "LyraControllerDisconnectedScreen.generated.h"

class UHorizontalBox;

UCLASS(Abstract, BlueprintType, Blueprintable)
class ULyraControllerDisconnectedScreen : public UCommonActivatableWidget
{
	GENERATED_BODY()

protected:
	void NativeOnActivated() override;

private:
	UPROPERTY(meta = (BindWidget))
	UHorizontalBox* HBox_SwitchUser;
};