// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "LyraVerbMessage.h"
#include "GameplayEffectTypes.h"

#include "LyraVerbMessageHelpers.generated.h"

class APlayerState;
class APlayerController;
struct FGameplayCueParameters;


UCLASS()
class LYRAGAME_API ULyraVerbMessageHelpers : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Lyra")
	static APlayerState* GetPlayerStateFromObject(UObject* Object);

	UFUNCTION(BlueprintCallable, Category = "Lyra")
	static APlayerController* GetPlayerControllerFromObject(UObject* Object);

	UFUNCTION(BlueprintCallable, Category = "Lyra")
	static FGameplayCueParameters VerbMessageToCueParameters(const FLyraVerbMessage& Message);

	UFUNCTION(BlueprintCallable, Category = "Lyra")
	static FLyraVerbMessage CueParametersToVerbMessage(const FGameplayCueParameters& Params);
};
