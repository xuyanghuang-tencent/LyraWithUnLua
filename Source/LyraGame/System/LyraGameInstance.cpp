// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraGameInstance.h"
#include "Player/LyraPlayerController.h"

ULyraGameInstance::ULyraGameInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void ULyraGameInstance::Init()
{
	Super::Init();
}

void ULyraGameInstance::Shutdown()
{
	Super::Shutdown();
}

ALyraPlayerController* ULyraGameInstance::GetPrimaryPlayerController() const
{
	return Cast<ALyraPlayerController>(Super::GetPrimaryPlayerController(false));
}