// Copyright Epic Games, Inc. All Rights Reserved.

#include "Input/InputTriggerCombo.h"
#include "EnhancedPlayerInput.h"
#include "Engine/World.h"

ETriggerState UInputTriggerComboAction::UpdateState_Implementation(const UEnhancedPlayerInput* PlayerInput, FInputActionValue ModifiedValue, float DeltaTime)
{
	const UWorld* World = PlayerInput->GetWorld();
	if (!World || !ComboStep.DependentAction)
	{
		return ETriggerState::None;
	}

	const float CurrentTime = World->GetRealTimeSeconds();
	const float TimeSinceLastTrigger = CurrentTime - LastActivationTime;

	// If this trigger is still in a "cooldown" phase then return none 
	if (TimeSinceLastTrigger < ActivationCooldown)
	{
		return ETriggerState::None;
	}
	
	const FInputActionInstance* EventData = PlayerInput->FindActionInstanceData(ComboStep.DependentAction);
	const float LastTriggeredTime = EventData ? EventData->GetLastTriggeredWorldTime() : 0.0f;
	const float TimeBetweenSteps = CurrentTime - LastTriggeredTime;
	
	// If this action was triggered within an acceptable threshold of the previous one 
	if ((LastTriggeredTime != 0.0f) && TimeBetweenSteps <= ComboStep.ActivationThreshold)
	{
		LastActivationTime = CurrentTime;
		return ETriggerState::Triggered;
	}
	
	return ETriggerState::None;
}

FString UInputTriggerComboAction::GetDebugState() const
{
	return FString::Printf(TEXT("Last Activation:%.2f"), LastActivationTime);
}