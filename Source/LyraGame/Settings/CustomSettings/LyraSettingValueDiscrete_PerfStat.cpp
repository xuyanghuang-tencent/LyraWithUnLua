// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraSettingValueDiscrete_PerfStat.h"
#include "Settings/LyraSettingsLocal.h"
#include "GameSettingFilterState.h"
#include "Performance/LyraPerformanceSettings.h"
#include "CommonUIVisibilitySubsystem.h"

#define LOCTEXT_NAMESPACE "LyraSettings"

//////////////////////////////////////////////////////////////////////

class FGameSettingEditCondition_PerfStatAllowed : public FGameSettingEditCondition
{
public:
	static TSharedRef<FGameSettingEditCondition_PerfStatAllowed> Get(ELyraDisplayablePerformanceStat Stat)
	{
		return MakeShared<FGameSettingEditCondition_PerfStatAllowed>(Stat);
	}

	FGameSettingEditCondition_PerfStatAllowed(ELyraDisplayablePerformanceStat Stat)
		: AssociatedStat(Stat)
	{
	}

	//~FGameSettingEditCondition interface
	virtual void GatherEditState(const ULocalPlayer* InLocalPlayer, FGameSettingEditableState& InOutEditState) const override
	{
		const FGameplayTagContainer& VisibilityTags = UCommonUIVisibilitySubsystem::GetChecked(InLocalPlayer)->GetVisibilityTags();

		bool bCanShowStat = false;
		for (const FLyraPerformanceStatGroup& Group : GetDefault<ULyraPerformanceSettings>()->UserFacingPerformanceStats) //@TODO: Move this stuff to per-platform instead of doing vis queries too?
		{
			if (Group.AllowedStats.Contains(AssociatedStat))
			{
				const bool bShowGroup = (Group.VisibilityQuery.IsEmpty() || Group.VisibilityQuery.Matches(VisibilityTags));
				if (bShowGroup)
				{
					bCanShowStat = true;
					break;
				}
			}
		}

		if (!bCanShowStat)
		{
			InOutEditState.Hide(TEXT("Stat is not listed in ULyraPerformanceSettings or is suppressed by current platform traits"));
		}
	}
	//~End of FGameSettingEditCondition interface

private:
	ELyraDisplayablePerformanceStat AssociatedStat;
};

//////////////////////////////////////////////////////////////////////

ULyraSettingValueDiscrete_PerfStat::ULyraSettingValueDiscrete_PerfStat()
{
}

void ULyraSettingValueDiscrete_PerfStat::SetStat(ELyraDisplayablePerformanceStat InStat)
{
	StatToDisplay = InStat;
	SetDevName(FName(*FString::Printf(TEXT("PerfStat_%d"), (int32)StatToDisplay)));
	AddEditCondition(FGameSettingEditCondition_PerfStatAllowed::Get(StatToDisplay));
}

void ULyraSettingValueDiscrete_PerfStat::AddMode(FText&& Label, ELyraStatDisplayMode Mode)
{
	Options.Emplace(MoveTemp(Label));
	DisplayModes.Add(Mode);
}

void ULyraSettingValueDiscrete_PerfStat::OnInitialized()
{
	Super::OnInitialized();

	AddMode(LOCTEXT("PerfStatDisplayMode_None", "None"), ELyraStatDisplayMode::Hidden);
	AddMode(LOCTEXT("PerfStatDisplayMode_TextOnly", "Text Only"), ELyraStatDisplayMode::TextOnly);
	AddMode(LOCTEXT("PerfStatDisplayMode_GraphOnly", "Graph Only"), ELyraStatDisplayMode::GraphOnly);
	AddMode(LOCTEXT("PerfStatDisplayMode_TextAndGraph", "Text and Graph"), ELyraStatDisplayMode::TextAndGraph);
}

void ULyraSettingValueDiscrete_PerfStat::StoreInitial()
{
	const ULyraSettingsLocal* Settings = ULyraSettingsLocal::Get();
	InitialMode = Settings->GetPerfStatDisplayState(StatToDisplay);
}

void ULyraSettingValueDiscrete_PerfStat::ResetToDefault()
{
	ULyraSettingsLocal* Settings = ULyraSettingsLocal::Get();
	Settings->SetPerfStatDisplayState(StatToDisplay, ELyraStatDisplayMode::Hidden);
	NotifySettingChanged(EGameSettingChangeReason::ResetToDefault);
}

void ULyraSettingValueDiscrete_PerfStat::RestoreToInitial()
{
	ULyraSettingsLocal* Settings = ULyraSettingsLocal::Get();
	Settings->SetPerfStatDisplayState(StatToDisplay, InitialMode);
	NotifySettingChanged(EGameSettingChangeReason::RestoreToInitial);
}

void ULyraSettingValueDiscrete_PerfStat::SetDiscreteOptionByIndex(int32 Index)
{
	if (DisplayModes.IsValidIndex(Index))
	{
		const ELyraStatDisplayMode DisplayMode = DisplayModes[Index];

		ULyraSettingsLocal* Settings = ULyraSettingsLocal::Get();
		Settings->SetPerfStatDisplayState(StatToDisplay, DisplayMode);
	}
	NotifySettingChanged(EGameSettingChangeReason::Change);
}

int32 ULyraSettingValueDiscrete_PerfStat::GetDiscreteOptionIndex() const
{
	const ULyraSettingsLocal* Settings = ULyraSettingsLocal::Get();
	return DisplayModes.Find(Settings->GetPerfStatDisplayState(StatToDisplay));
}

TArray<FText> ULyraSettingValueDiscrete_PerfStat::GetDiscreteOptions() const
{
	return Options;
}

#undef LOCTEXT_NAMESPACE