// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/PawnComponent.h"
#include "LyraPawnComponent.generated.h"


UINTERFACE(BlueprintType)
class LYRAGAME_API ULyraReadyInterface : public UInterface
{
	GENERATED_BODY()
};

class ILyraReadyInterface
{
	GENERATED_BODY()

public:
	virtual bool IsPawnComponentReadyToInitialize() const = 0;
};




/**
 * ULyraPawnComponent
 *
 *	An actor component that can be used for adding custom behavior to pawns.
 */
UCLASS(Blueprintable, Meta = (BlueprintSpawnableComponent))
class LYRAGAME_API ULyraPawnComponent : public UPawnComponent, public ILyraReadyInterface
{
	GENERATED_BODY()

public:

	ULyraPawnComponent(const FObjectInitializer& ObjectInitializer);

	virtual bool IsPawnComponentReadyToInitialize() const override { return true; }
};
