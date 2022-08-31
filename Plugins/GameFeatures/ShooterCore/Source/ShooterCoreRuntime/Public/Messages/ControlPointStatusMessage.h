// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "ControlPointStatusMessage.generated.h"

// Message indicating the state of a control point is changing
USTRUCT(BlueprintType)
struct FLyraControlPointStatusMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category=Gameplay)
	AActor* ControlPoint = nullptr;

	UPROPERTY(BlueprintReadWrite, Category=Gameplay)
	int32 OwnerTeamID = 0;
};
