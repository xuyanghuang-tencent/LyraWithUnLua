// Copyright Epic Games, Inc. All Rights Reserved.

#include "TopDownArenaMovementComponent.h"
#include "GameplayAbilities/Public/AbilitySystemGlobals.h"
// #include "GameFramework/Character.h"
// #include "CollisionQueryParams.h"
// #include "Components/CapsuleComponent.h"
#include "TopDownArenaAttributeSet.h"

UTopDownArenaMovementComponent::UTopDownArenaMovementComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

float UTopDownArenaMovementComponent::GetMaxSpeed() const
{
	if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwner()))
	{
		if (MovementMode == MOVE_Walking)
		{
			if (ASC->HasMatchingGameplayTag(TAG_Gameplay_MovementStopped))
			{
				return 0;
			}

			const float MaxSpeedFromAttribute = ASC->GetNumericAttribute(UTopDownArenaAttributeSet::GetMovementSpeedAttribute());
			if (MaxSpeedFromAttribute > 0.0f)
			{
				return MaxSpeedFromAttribute;
			}
		}
	}

	return Super::GetMaxSpeed();
}