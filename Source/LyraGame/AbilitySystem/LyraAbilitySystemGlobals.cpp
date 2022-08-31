// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraAbilitySystemGlobals.h"
#include "LyraGameplayEffectContext.h"

ULyraAbilitySystemGlobals::ULyraAbilitySystemGlobals(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

FGameplayEffectContext* ULyraAbilitySystemGlobals::AllocGameplayEffectContext() const
{
	return new FLyraGameplayEffectContext();
}
