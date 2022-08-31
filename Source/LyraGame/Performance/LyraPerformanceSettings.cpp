// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraPerformanceSettings.h"
#include "Engine/PlatformSettings.h"

//////////////////////////////////////////////////////////////////////

ULyraPlatformSpecificRenderingSettings::ULyraPlatformSpecificRenderingSettings()
{
	MobileFrameRateLimits.Append({ 20, 30, 45, 60, 90, 120 });
}

const ULyraPlatformSpecificRenderingSettings* ULyraPlatformSpecificRenderingSettings::Get()
{
	ULyraPlatformSpecificRenderingSettings* Result = UPlatformSettingsManager::Get().GetSettingsForPlatform<ThisClass>();
	check(Result);
	return Result;
}

//////////////////////////////////////////////////////////////////////

ULyraPerformanceSettings::ULyraPerformanceSettings()
{
	PerPlatformSettings.Initialize(ULyraPlatformSpecificRenderingSettings::StaticClass());

	CategoryName = TEXT("Game");

	DesktopFrameRateLimits.Append({ 30, 60, 120, 144, 160, 165, 180, 200, 240, 360 });

	// Default to all stats are allowed
	FLyraPerformanceStatGroup& StatGroup = UserFacingPerformanceStats.AddDefaulted_GetRef();
	for (ELyraDisplayablePerformanceStat PerfStat : TEnumRange<ELyraDisplayablePerformanceStat>())
	{
		StatGroup.AllowedStats.Add(PerfStat);
	}
}
