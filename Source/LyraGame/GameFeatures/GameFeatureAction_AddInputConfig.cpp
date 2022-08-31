// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameFeatures/GameFeatureAction_AddInputConfig.h"

#define LOCTEXT_NAMESPACE "GameFeatures_AddInputConfig"

void UGameFeatureAction_AddInputConfig::OnGameFeatureRegistering()
{
	Super::OnGameFeatureRegistering();	

	for (const FMappableConfigPair& Pair : InputConfigs)
	{
		FMappableConfigPair::RegisterPair(Pair);
	}
}

void UGameFeatureAction_AddInputConfig::OnGameFeatureActivating(FGameFeatureActivatingContext& Context)
{
	Super::OnGameFeatureActivating(Context);

	for (const FMappableConfigPair& Pair : InputConfigs)
	{
		if (Pair.bShouldActivateAutomatically)
		{
			FMappableConfigPair::ActivatePair(Pair);	
		}		
	}
}

void UGameFeatureAction_AddInputConfig::OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context)
{
	Super::OnGameFeatureDeactivating(Context);

	for (const FMappableConfigPair& Pair : InputConfigs)
	{
		FMappableConfigPair::DeactivatePair(Pair);
	}
}

void UGameFeatureAction_AddInputConfig::OnGameFeatureUnregistering()
{
	Super::OnGameFeatureUnregistering();

	for (const FMappableConfigPair& Pair : InputConfigs)
	{
		FMappableConfigPair::UnregisterPair(Pair);
	}
}

#if WITH_EDITOR
EDataValidationResult UGameFeatureAction_AddInputConfig::IsDataValid(TArray<FText>& ValidationErrors)
{
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(ValidationErrors), EDataValidationResult::Valid);

	int32 EntryIndex = 0;
	for (const FMappableConfigPair& Pair : InputConfigs)
	{
		if (Pair.Config.IsNull())
		{
			Result = EDataValidationResult::Invalid;
			ValidationErrors.Add(FText::Format(LOCTEXT("NullConfigPointer", "Null Config pointer at index {0} in Pair list"), FText::AsNumber(EntryIndex)));
		}

		++EntryIndex;
	}
	
	return Result;
}
#endif	// WITH_EDITOR

#undef LOCTEXT_NAMESPACE