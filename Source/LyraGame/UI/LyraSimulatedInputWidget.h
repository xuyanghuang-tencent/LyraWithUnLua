// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "LyraSimulatedInputWidget.generated.h"

class UEnhancedInputLocalPlayerSubsystem;
class UInputAction;
class UCommonHardwareVisibilityBorder;
class UEnhancedPlayerInput;

/**
 *  A UMG widget with base functionality to inject input (keys or input actions)
 *  to the enhanced input subsystem.
 */
UCLASS(meta=( DisplayName="Lyra Simulated Input Widget" ))
class LYRAGAME_API ULyraSimulatedInputWidget : public UCommonUserWidget
{
	GENERATED_BODY()
	
public:
	
	ULyraSimulatedInputWidget(const FObjectInitializer& ObjectInitializer);
	
	//~ Begin UWidget
#if WITH_EDITOR
	virtual const FText GetPaletteCategory() override;
#endif
	//~ End UWidget interface

	//~ Begin UUserWidget
	virtual void NativeConstruct() override;
	virtual FReply NativeOnTouchEnded(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent) override;
	//~ End UUserWidget interface
	
	/** Get the enhanced input subsystem based on the owning local player of this widget. Will return null if there is no owning player */
	UFUNCTION(BlueprintCallable)
	UEnhancedInputLocalPlayerSubsystem* GetEnhancedInputSubsystem() const;

	/** Get the current player input from the current input subsystem */
	UEnhancedPlayerInput* GetPlayerInput() const;

	/**  */
	UFUNCTION(BlueprintCallable)
	const UInputAction* GetAssociatedAction() const { return AssociatedAction; }

	/** Returns the current key that will be used to input any values. */
	UFUNCTION(BlueprintCallable)
	FKey GetSimulatedKey() const { return KeyToSimulate; }

	/**
	 * Injects the given vector as an input to the current simulated key.
	 * This calls "InputKey" on the current player.
	 */
	UFUNCTION(BlueprintCallable)
	void InputKeyValue(const FVector& Value);

	/**
	 * Injects the given vector as an input to the current simulated key.
	 * This calls "InputKey" on the current player.
	 */
	UFUNCTION(BlueprintCallable)
	void InputKeyValue2D(const FVector2D& Value);

	UFUNCTION(BlueprintCallable)
	void FlushSimulatedInput();
	
protected:

	/** Set the KeyToSimulate based on a query from enhanced input about what keys are mapped to the associated action */
	void QueryKeyToSimulate();

	/** The common visiblity border will allow you to specifiy UI for only specific platorms if desired */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UCommonHardwareVisibilityBorder* CommonVisibilityBorder = nullptr;
	
	/** The associated input action that we should simulate input for */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	const UInputAction* AssociatedAction = nullptr;

	/** The Key to simulate input for in the case where none are currently bound to the associated action */
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	FKey FallbackBindingKey = EKeys::Gamepad_Right2D;

	/** The key that should be input via InputKey on the player input */
	FKey KeyToSimulate;
};
