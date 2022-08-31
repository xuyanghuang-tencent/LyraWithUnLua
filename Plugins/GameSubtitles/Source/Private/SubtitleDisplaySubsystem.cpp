// Copyright Epic Games, Inc. All Rights Reserved.

#include "SubtitleDisplaySubsystem.h"
#include "Subsystems/SubsystemCollection.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"

USubtitleDisplaySubsystem* USubtitleDisplaySubsystem::Get(const ULocalPlayer* LocalPlayer)
{
	return LocalPlayer ? LocalPlayer->GetGameInstance()->GetSubsystem<USubtitleDisplaySubsystem>() : nullptr;
}

USubtitleDisplaySubsystem::USubtitleDisplaySubsystem()
{
}

void USubtitleDisplaySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{

}

void USubtitleDisplaySubsystem::Deinitialize()
{

}

const FSubtitleFormat& USubtitleDisplaySubsystem::GetSubtitleDisplayOptions() const
{
	return SubtitleFormat;
}

void USubtitleDisplaySubsystem::SetSubtitleDisplayOptions(const FSubtitleFormat& InOptions)
{
	SubtitleFormat = InOptions;
	DisplayFormatChangedEvent.Broadcast(SubtitleFormat);
}
