// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFeatureAction.h"
#include "Input/LyraMappableConfigPair.h"
#include "GameFeatureAction_AddInputConfig.generated.h"

/**
 * Registers a Player Mappable Input config to the Game User Settings
 * 
 * Expects that local players are set up to use the EnhancedInput system.
 */
UCLASS(meta = (DisplayName = "Add Input Config"))
class LYRAGAME_API UGameFeatureAction_AddInputConfig : public UGameFeatureAction
{
	GENERATED_BODY()
	
public:
	//~UObject UGameFeatureAction
	virtual void OnGameFeatureRegistering() override;
	virtual void OnGameFeatureActivating(FGameFeatureActivatingContext& Context) override;
	virtual void OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context) override;
	virtual void OnGameFeatureUnregistering() override;
	//~End of UGameFeatureAction interface

	//~UObject interface
#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(TArray<FText>& ValidationErrors) override;
#endif
	//~End of UObject interface

	/** The player mappable configs to register for user with this config */
	UPROPERTY(EditAnywhere)
	TArray<FMappableConfigPair> InputConfigs;
};