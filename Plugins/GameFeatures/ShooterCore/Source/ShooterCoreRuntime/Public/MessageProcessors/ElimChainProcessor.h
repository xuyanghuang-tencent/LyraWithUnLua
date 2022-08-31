// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Messages/GameplayMessageProcessor.h"
#include "GameplayTagContainer.h"
#include "ElimChainProcessor.generated.h"

struct FLyraVerbMessage;
class APlayerState;

USTRUCT()
struct FPlayerElimChainInfo
{
	GENERATED_BODY()

	double LastEliminationTime = 0.0;

	int32 ChainCounter = 1;
};

// Tracks a chain of eliminations (X eliminations without more than Y seconds passing between each one)
UCLASS(Abstract)
class UElimChainProcessor : public UGameplayMessageProcessor
{
	GENERATED_BODY()

public:
	virtual void StartListening() override;

protected:
	UPROPERTY(EditDefaultsOnly)
	float ChainTimeLimit = 4.5f;

	// The event to rebroadcast when a user gets a chain of a certain length
	UPROPERTY(EditDefaultsOnly)
	TMap<int32, FGameplayTag> ElimChainTags;

private:
	void OnEliminationMessage(FGameplayTag Channel, const FLyraVerbMessage& Payload);

private:
	UPROPERTY(Transient)
	TMap<TObjectPtr<APlayerState>, FPlayerElimChainInfo> PlayerChainHistory;
};
