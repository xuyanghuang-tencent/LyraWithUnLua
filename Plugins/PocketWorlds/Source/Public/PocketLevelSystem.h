// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Subsystems/LocalPlayerSubsystem.h"
#include "Subsystems/WorldSubsystem.h"

#include "PocketLevelSystem.generated.h"

class UPocketLevel;
class ULocalPlayer;
class ULevelStreamingDynamic;
class UPocketLevelInstance;

/**
 *
 */
UCLASS()
class POCKETWORLDS_API UPocketLevelSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/**
	 * 
	 */
	UPocketLevelInstance* GetOrCreatePocketLevelFor(ULocalPlayer* LocalPlayer, UPocketLevel* PocketLevel, FVector DesiredSpawnPoint);

private:
	UPROPERTY()
	TArray<UPocketLevelInstance*> PocketInstances;
};
