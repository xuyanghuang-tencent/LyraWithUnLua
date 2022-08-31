// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraPawnComponent.h"


ULyraPawnComponent::ULyraPawnComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bCanEverTick = false;
}
