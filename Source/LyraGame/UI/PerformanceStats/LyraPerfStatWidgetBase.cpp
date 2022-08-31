// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraPerfStatWidgetBase.h"
#include "Performance/LyraPerformanceStatSubsystem.h"

//////////////////////////////////////////////////////////////////////
// ULyraPerfStatWidgetBase

ULyraPerfStatWidgetBase::ULyraPerfStatWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

double ULyraPerfStatWidgetBase::FetchStatValue()
{
	if (CachedStatSubsystem == nullptr)
	{
		if (UWorld* World = GetWorld())
		{
			if (UGameInstance* GameInstance = World->GetGameInstance())
			{
				CachedStatSubsystem = GameInstance->GetSubsystem<ULyraPerformanceStatSubsystem>();
			}
		}
	}

	if (CachedStatSubsystem)
	{
		return CachedStatSubsystem->GetCachedStat(StatToDisplay);
	}
	else
	{
		return 0.0;
	}
}
