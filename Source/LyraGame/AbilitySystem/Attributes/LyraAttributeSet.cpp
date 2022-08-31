// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraAttributeSet.h"
#include "AbilitySystem/LyraAbilitySystemComponent.h"


ULyraAttributeSet::ULyraAttributeSet()
{
}

UWorld* ULyraAttributeSet::GetWorld() const
{
	const UObject* Outer = GetOuter();
	check(Outer);

	return Outer->GetWorld();
}

ULyraAbilitySystemComponent* ULyraAttributeSet::GetLyraAbilitySystemComponent() const
{
	return Cast<ULyraAbilitySystemComponent>(GetOwningAbilitySystemComponent());
}
