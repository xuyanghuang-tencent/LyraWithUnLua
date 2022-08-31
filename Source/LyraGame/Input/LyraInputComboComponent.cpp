// Copyright Epic Games, Inc. All Rights Reserved.


#include "Input/LyraInputComboComponent.h"
#include "EnhancedInputComponent.h"
#include "LyraInputComponent.h"
#include "GameFramework/PlayerController.h"

ULyraInputComboComponent::ULyraInputComboComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void ULyraInputComboComponent::OnRegister()
{
	Super::OnRegister();
	
	BindInputCallbacks();
}

void ULyraInputComboComponent::OnUnregister()
{
	Super::OnUnregister();
	
	RemoveInputCallbacks();
}

void ULyraInputComboComponent::BindInputCallbacks()
{
	if (ULyraInputComponent* Input = GetInputComponent())
	{
		for (const FComboStep& Step : ComboSteps)
		{
			if (Step.Action)
			{
				Input->BindAction(Step.Action, ETriggerEvent::Triggered, this, &ULyraInputComboComponent::OnComboStepCompleted);
				// TODO: store handles
				BindHandles.Add(8);
			}
		}
	}
}

void ULyraInputComboComponent::RemoveInputCallbacks()
{
	if (ULyraInputComponent* Input = GetInputComponent())
	{
		Input->RemoveBinds(BindHandles);
	}
}

void ULyraInputComboComponent::OnComboStepCompleted(const FInputActionInstance& Instance)
{
	// It shouldn't be possible to have no combo steps
	ensure(!ComboSteps.IsEmpty());
	
	int32 CurrentComboStep = ComboStack.Num();
	const float TriggeredWorldTime = Instance.GetLastTriggeredWorldTime();	
	const float PreviousStepActivationTime = ComboStack[CurrentComboStep].Value;
	
	// If this is the first combo then we can just add it to the stack
	if (Instance.GetSourceAction() == ComboSteps[CurrentComboStep].Action)
	{
		if (TriggeredWorldTime - PreviousStepActivationTime <= ComboSteps[CurrentComboStep].CooldownTime)
		{
			// Check if the time is valid
			TTuple<const UInputAction*, float> NewlyCompletedStep;
			NewlyCompletedStep.Key = Instance.GetSourceAction();
			NewlyCompletedStep.Value = TriggeredWorldTime;
			ComboStack.Emplace(NewlyCompletedStep);	
		}
	}
	
	// Check if the combo was completed or not
	if (ComboStack.Num() == ComboSteps.Num())
	{
		NativeOnComboCompleted();
	}
}

void ULyraInputComboComponent::NativeOnComboCompleted()
{
	ComboStack.Empty();
	ScriptOnComboCompleted();
}

void ULyraInputComboComponent::NativeOnComboCanceled()
{
	ComboStack.Empty();
	ScriptOnComboCanceled();
}

ULyraInputComponent* ULyraInputComboComponent::GetInputComponent() const
{
	APlayerController* Owner = Cast<APlayerController>(GetOwner());
	return Owner ? Cast<ULyraInputComponent>(Owner->PlayerInput) : nullptr;
}

