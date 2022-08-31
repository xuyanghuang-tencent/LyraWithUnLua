// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "GameSettingValueDiscrete.h"
#include "Performance/LyraPerformanceStatTypes.h"

#include "LyraSettingValueDiscrete_PerfStat.generated.h"

struct FScreenOverallQualityRHI;

UCLASS()
class ULyraSettingValueDiscrete_PerfStat : public UGameSettingValueDiscrete
{
	GENERATED_BODY()
	
public:

	ULyraSettingValueDiscrete_PerfStat();

	void SetStat(ELyraDisplayablePerformanceStat InStat);

	/** UGameSettingValue */
	virtual void StoreInitial() override;
	virtual void ResetToDefault() override;
	virtual void RestoreToInitial() override;

	/** UGameSettingValueDiscrete */
	virtual void SetDiscreteOptionByIndex(int32 Index) override;
	virtual int32 GetDiscreteOptionIndex() const override;
	virtual TArray<FText> GetDiscreteOptions() const override;

protected:
	/** UGameSettingValue */
	virtual void OnInitialized() override;
	
	void AddMode(FText&& Label, ELyraStatDisplayMode Mode);
protected:
	TArray<FText> Options;
	TArray<ELyraStatDisplayMode> DisplayModes;

	ELyraDisplayablePerformanceStat StatToDisplay;
	ELyraStatDisplayMode InitialMode;
};