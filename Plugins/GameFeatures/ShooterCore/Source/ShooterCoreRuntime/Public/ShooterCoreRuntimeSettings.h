// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Engine/EngineTypes.h"
#include "Engine/DeveloperSettings.h"
#include "ShooterCoreRuntimeSettings.generated.h"

/** Runtime settings specific to the ShooterCoreRuntime plugin */
UCLASS(config = Game, defaultconfig)
class UShooterCoreRuntimeSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UShooterCoreRuntimeSettings(const FObjectInitializer& Initializer);

	ECollisionChannel GetAimAssistCollisionChannel() const { return AimAssistCollisionChannel; }

private:

	/**
	 * What trace channel should be used to find available targets for Aim Assist.
	 * @see UAimAssistTargetManagerComponent::GetVisibleTargets
	 */
	UPROPERTY(config, EditAnywhere, Category = "Aim Assist")
	TEnumAsByte<ECollisionChannel> AimAssistCollisionChannel = ECollisionChannel::ECC_EngineTraceChannel5;
};