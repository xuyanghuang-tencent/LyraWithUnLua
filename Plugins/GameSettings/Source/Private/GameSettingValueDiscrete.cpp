// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameSettingValueDiscrete.h"

#define LOCTEXT_NAMESPACE "GameSetting"

//--------------------------------------
// UGameSettingValueDiscrete
//--------------------------------------

UGameSettingValueDiscrete::UGameSettingValueDiscrete()
{

}

FString UGameSettingValueDiscrete::GetAnalyticsValue() const
{
	const TArray<FText> Options = GetDiscreteOptions();
	const int32 CurrentOptionIndex = GetDiscreteOptionIndex();
	if (Options.IsValidIndex(CurrentOptionIndex))
	{
		const FString* SourceString = FTextInspector::GetSourceString(Options[CurrentOptionIndex]);
		if (SourceString)
		{
			return *SourceString;
		}
	}

	return TEXT("<Unknown Index>");
}

#undef LOCTEXT_NAMESPACE
