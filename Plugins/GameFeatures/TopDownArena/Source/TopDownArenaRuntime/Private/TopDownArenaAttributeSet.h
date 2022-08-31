// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Attributes/LyraAttributeSet.h"
#include "NativeGameplayTags.h"
#include "TopDownArenaAttributeSet.generated.h"

/**
 * UTopDownArenaAttributeSet
 *
 *	Class that defines attributes specific to the top-down arena gameplay mode.
 */
UCLASS(BlueprintType)
class UTopDownArenaAttributeSet : public ULyraAttributeSet
{
	GENERATED_BODY()

public:
	UTopDownArenaAttributeSet();

	ATTRIBUTE_ACCESSORS(ThisClass, BombsRemaining);
	ATTRIBUTE_ACCESSORS(ThisClass, BombCapacity);
	ATTRIBUTE_ACCESSORS(ThisClass, BombRange);
	ATTRIBUTE_ACCESSORS(ThisClass, MovementSpeed);

	//~UAttributeSet interface
	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	//~End of UAttributeSet interface

protected:

	UFUNCTION()
	void OnRep_BombsRemaining(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_BombCapacity(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_BombRange(const FGameplayAttributeData& OldValue);
	
	UFUNCTION()
	void OnRep_MovementSpeed(const FGameplayAttributeData& OldValue);

	void ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const;

private:
	// The number of bombs remaining
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_BombsRemaining, Category="TopDownArenaGame", Meta=(AllowPrivateAccess=true))
	FGameplayAttributeData BombsRemaining;

	// The maximum number of bombs that can be placed at once
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_BombCapacity, Category="TopDownArenaGame", Meta=(AllowPrivateAccess=true))
	FGameplayAttributeData BombCapacity;

	// The range/radius of bomb blasts
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_BombRange, Category="TopDownArenaGame", Meta=(AllowPrivateAccess=true))
	FGameplayAttributeData BombRange;

	// The range/radius of bomb blasts
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_MovementSpeed, Category="TopDownArenaGame", Meta=(AllowPrivateAccess=true))
	FGameplayAttributeData MovementSpeed;
};
