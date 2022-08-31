// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraControllerDisconnectedScreen.h"

#include "Components/HorizontalBox.h"
#include "HAL/PlatformMisc.h"

void ULyraControllerDisconnectedScreen::NativeOnActivated()
{
	Super::NativeOnActivated();

	HBox_SwitchUser->SetVisibility(ESlateVisibility::Collapsed);

//#if PLATFORM_HAS_STRICT_CONTROLLER_PAIRING
//	if (FPlatformMisc::GetControllerIdForUnpairedControllers() != INDEX_NONE)
//	{
//		HBox_SwitchUser->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
//	}
//#endif
}

