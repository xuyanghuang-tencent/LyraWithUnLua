// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PocketLevelInstance.generated.h"

class UPocketLevel;
class ULocalPlayer;
class ULevelStreamingDynamic;
class UPocketLevelInstance;

DECLARE_MULTICAST_DELEGATE_OneParam(FPocketLevelInstanceEvent, UPocketLevelInstance*);

/**
 *
 */
UCLASS(Within = PocketLevelSubsystem, BlueprintType)
class POCKETWORLDS_API UPocketLevelInstance : public UObject
{
	GENERATED_BODY()

public:
	UPocketLevelInstance();

	virtual void BeginDestroy() override;

	void StreamIn();
	void StreamOut();

	FDelegateHandle AddReadyCallback(FPocketLevelInstanceEvent::FDelegate Callback);
	void RemoveReadyCallback(FDelegateHandle CallbackToRemove);

	virtual class UWorld* GetWorld() const override { return World; }

private:
	bool Initialize(ULocalPlayer* LocalPlayer, UPocketLevel* PocketLevel, FVector SpawnPoint);

	UFUNCTION()
	void HandlePocketLevelLoaded();

	UFUNCTION()
	void HandlePocketLevelShown();

private:
	UPROPERTY()
	ULocalPlayer* LocalPlayer;

	UPROPERTY()
	UPocketLevel* PocketLevel;

	UPROPERTY()
	UWorld* World;

	UPROPERTY()
	ULevelStreamingDynamic* StreamingPocketLevel;

	FPocketLevelInstanceEvent OnReadyEvent;

	FBoxSphereBounds Bounds;

	friend class UPocketLevelSubsystem;
};
