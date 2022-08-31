// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/CapsuleComponent.h"
#include "IAimAssistTargetInterface.h"
#include "AimAssistTargetComponent.generated.h"

/**
 * This component can be added to any actor to have it register with the Aim Assist Target Manager.
 */
UCLASS(BlueprintType, meta=(BlueprintSpawnableComponent))
class SHOOTERCORERUNTIME_API UAimAssistTargetComponent : public UCapsuleComponent, public IAimAssistTaget
{
	GENERATED_BODY()

public:
	
	//~ Begin IAimAssistTaget interface
	virtual void GatherTargetOptions(OUT FAimAssistTargetOptions& TargetData) override;
	//~ End IAimAssistTaget interface
	
protected:
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FAimAssistTargetOptions TargetData {};
};