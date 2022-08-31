// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraCameraMode_TopDownArenaCamera.h"
#include "Curves/CurveVector.h"
#include "GameFramework/Character.h"
#include "EngineUtils.h"

ULyraCameraMode_TopDownArenaCamera::ULyraCameraMode_TopDownArenaCamera()
{
	ArenaWidth = 1000.0f;
	ArenaHeight = 1000.0f;
}

void ULyraCameraMode_TopDownArenaCamera::UpdateView(float DeltaTime)
{
	FBox ArenaBounds(FVector(-ArenaWidth, -ArenaHeight, 0.0f), FVector(ArenaWidth, ArenaHeight, 100.0f));

	const double BoundsMaxComponent = ArenaBounds.GetSize().GetMax();

	const double CameraLoftDistance = BoundsSizeToDistance.GetRichCurveConst()->Eval(BoundsMaxComponent);
	
	FVector PivotLocation = ArenaBounds.GetCenter() - DefaultPivotRotation.Vector() * CameraLoftDistance;
	
	FRotator PivotRotation = DefaultPivotRotation;

	View.Location = PivotLocation;
	View.Rotation = PivotRotation;
	View.ControlRotation = View.Rotation;
	View.FieldOfView = FieldOfView;
}
