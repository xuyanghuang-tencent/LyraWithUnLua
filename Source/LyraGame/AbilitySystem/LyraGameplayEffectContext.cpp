// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraGameplayEffectContext.h"
#include "Components/PrimitiveComponent.h"
#include "AbilitySystem/LyraAbilitySourceInterface.h"

FLyraGameplayEffectContext* FLyraGameplayEffectContext::ExtractEffectContext(struct FGameplayEffectContextHandle Handle)
{
	FGameplayEffectContext* BaseEffectContext = Handle.Get();
	if ((BaseEffectContext != nullptr) && BaseEffectContext->GetScriptStruct()->IsChildOf(FLyraGameplayEffectContext::StaticStruct()))
	{
		return (FLyraGameplayEffectContext*)BaseEffectContext;
	}

	return nullptr;
}

bool FLyraGameplayEffectContext::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
	FGameplayEffectContext::NetSerialize(Ar, Map, bOutSuccess);

	// Not serialized for post-activation use:
	// CartridgeID

	return true;
}

void FLyraGameplayEffectContext::SetAbilitySource(const ILyraAbilitySourceInterface* InObject, float InSourceLevel)
{
	AbilitySourceObject = MakeWeakObjectPtr(Cast<const UObject>(InObject));
	//SourceLevel = InSourceLevel;
}

const ILyraAbilitySourceInterface* FLyraGameplayEffectContext::GetAbilitySource() const
{
	return Cast<ILyraAbilitySourceInterface>(AbilitySourceObject.Get());
}

const UPhysicalMaterial* FLyraGameplayEffectContext::GetPhysicalMaterial() const
{
	if (const FHitResult* HitResultPtr = GetHitResult())
	{
		return HitResultPtr->PhysMaterial.Get();
	}
	return nullptr;
}
