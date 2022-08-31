// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Camera/LyraCameraMode.h"
#include "Curves/CurveFloat.h"

#include "LyraCameraMode_TopDownArenaCamera.generated.h"


class UCurveVector;


/**
 * ULyraCameraMode_TopDownArenaCamera
 *
 *	A basic third person camera mode that looks down at a fixed arena.
 */
UCLASS(Abstract, Blueprintable)
class ULyraCameraMode_TopDownArenaCamera : public ULyraCameraMode
{
	GENERATED_BODY()

public:

	ULyraCameraMode_TopDownArenaCamera();

protected:

	//~ULyraCameraMode interface
	virtual void UpdateView(float DeltaTime) override;
	//~End of ULyraCameraMode interface

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Third Person")
	float ArenaWidth;

	UPROPERTY(EditDefaultsOnly, Category = "Third Person")
	float ArenaHeight;

	UPROPERTY(EditDefaultsOnly, Category = "Third Person")
	FRotator DefaultPivotRotation;

	UPROPERTY(EditDefaultsOnly, Category = "Third Person")
	FRuntimeFloatCurve BoundsSizeToDistance;
};
