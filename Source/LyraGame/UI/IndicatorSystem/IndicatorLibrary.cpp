// Copyright Epic Games, Inc. All Rights Reserved.

#include "IndicatorLibrary.h"
#include "LyraIndicatorManagerComponent.h"

UIndicatorLibrary::UIndicatorLibrary()
{
}

ULyraIndicatorManagerComponent* UIndicatorLibrary::GetIndicatorManagerComponent(AController* Controller)
{
	return ULyraIndicatorManagerComponent::GetComponent(Controller);
}
