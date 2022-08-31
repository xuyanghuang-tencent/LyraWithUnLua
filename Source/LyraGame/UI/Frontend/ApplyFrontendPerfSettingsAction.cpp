// Copyright Epic Games, Inc. All Rights Reserved.

#include "ApplyFrontendPerfSettingsAction.h"
#include "Settings/LyraSettingsLocal.h"

//////////////////////////////////////////////////////////////////////
// UApplyFrontendPerfSettingsAction

// Game user settings (and engine performance/scalability settings they drive)
// are global, so there's no point in tracking this per world for multi-player PIE:
// we just apply it if any PIE world is in the menu.
//
// However, by default we won't apply front-end performance stuff in the editor
// unless the developer setting ApplyFrontEndPerformanceOptionsInPIE is enabled
int32 UApplyFrontendPerfSettingsAction::ApplicationCounter = 0;

void UApplyFrontendPerfSettingsAction::OnGameFeatureActivating(FGameFeatureActivatingContext& Context)
{
	ApplicationCounter++;
	if (ApplicationCounter == 1)
	{
		ULyraSettingsLocal::Get()->SetShouldUseFrontendPerformanceSettings(true);
	}
}

void UApplyFrontendPerfSettingsAction::OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context)
{
	ApplicationCounter--;
	check(ApplicationCounter >= 0);

	if (ApplicationCounter == 0)
	{
		ULyraSettingsLocal::Get()->SetShouldUseFrontendPerformanceSettings(false);
	}
}
