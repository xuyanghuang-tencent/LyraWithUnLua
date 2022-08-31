// Copyright Epic Games, Inc. All Rights Reserved.

#include "CoreMinimal.h"
#include "InputTriggers.h"
#include "InputActionValue.h"

#include "InputTriggerCombo.generated.h"

class UEnhancedPlayerInput;
class UInputAction;

/**
 * Represents one part of a larger input combo. UIsed 
 */
USTRUCT(BlueprintType)
struct FComboTriggerStep
{
	GENERATED_BODY()

public:

	/** The Input Action that this step */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	const UInputAction* DependentAction = nullptr;

	/** The amount of time that is acceptable between this combo step and the next. */
	UPROPERTY(EditAnywhere, Config, BlueprintReadWrite, meta = (ClampMin = "0"))
	float ActivationThreshold = 0.5f;
};

/**
 * UInputTriggerComboAction
 *
 * This will be triggered when all dependent actions have been triggered in order.
*/
UCLASS(NotBlueprintable, MinimalAPI, meta = (DisplayName = "Combo Action", NotInputConfigurable = "true"))
class UInputTriggerComboAction final : public UInputTrigger
{
	GENERATED_BODY()

protected:
	// Implicit, so it will only trigger if any other triggers on this action are also being triggered.
	virtual ETriggerType GetTriggerType_Implementation() const override { return ETriggerType::Implicit; }	
	virtual ETriggerState UpdateState_Implementation(const UEnhancedPlayerInput* PlayerInput, FInputActionValue ModifiedValue, float DeltaTime) override;
	virtual FString GetDebugState() const override;

public:

	/** The step that must be completed before this input trigger can happen */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Trigger Settings")
	FComboTriggerStep ComboStep;

	/** This is the amount of time that must pass before this can be triggered again. */
	UPROPERTY(EditAnywhere, Config, BlueprintReadWrite, meta = (ClampMin = "0"))
	float ActivationCooldown = 0.5f;

	/** The time in RealWorldSeconds that this trigger was last Triggered */
	float LastActivationTime = 0.0f;
};