// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Components/ControllerComponent.h"
#include "Delegates/DelegateCombinations.h"

#include "LyraIndicatorManagerComponent.generated.h"

class UIndicatorDescriptor;

/**
 * @class ULyraIndicatorManagerComponent
 */
UCLASS(BlueprintType, Blueprintable)
class LYRAGAME_API ULyraIndicatorManagerComponent : public UControllerComponent
{
	GENERATED_BODY()

public:
	ULyraIndicatorManagerComponent(const FObjectInitializer& ObjectInitializer);

	static ULyraIndicatorManagerComponent* GetComponent(AController* Controller);

	UFUNCTION(BlueprintCallable, Category = Indicator)
	void AddIndicator(UIndicatorDescriptor* IndicatorDescriptor);
	
	UFUNCTION(BlueprintCallable, Category = Indicator)
	void RemoveIndicator(UIndicatorDescriptor* IndicatorDescriptor);

	DECLARE_EVENT_OneParam(ULyraIndicatorManagerComponent, FIndicatorEvent, UIndicatorDescriptor* Descriptor)
	FIndicatorEvent OnIndicatorAdded;
	FIndicatorEvent OnIndicatorRemoved;

	const TArray<UIndicatorDescriptor*>& GetIndicators() const { return Indicators; }

private:
	UPROPERTY()
	TArray<UIndicatorDescriptor*> Indicators;
};
