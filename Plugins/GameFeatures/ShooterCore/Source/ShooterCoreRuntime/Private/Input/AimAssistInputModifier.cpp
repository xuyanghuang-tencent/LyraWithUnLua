// Copyright Epic Games, Inc. All Rights Reserved.

#include "Input/AimAssistInputModifier.h"
#include "EnhancedPlayerInput.h"
#include "GameFramework/GameStateBase.h"
#include "Input/AimAssistTargetManagerComponent.h"
#include "Input/LyraAimSensitivityData.h"
#include "Player/LyraLocalPlayer.h"
#include "Player/LyraPlayerState.h"
#include "Settings/LyraSettingsShared.h"

#if ENABLE_DRAW_DEBUG
#include "Engine/Canvas.h"
#include "Debug/DebugDrawService.h"
#endif	// ENABLE_DRAW_DEBUG

DEFINE_LOG_CATEGORY(LogAimAssist);

namespace LyraConsoleVariables
{
	static bool bEnableAimAssist = true;
	static FAutoConsoleVariableRef CVarEnableAimAssist(
		TEXT("lyra.Weapon.EnableAimAssist"),
		bEnableAimAssist,
		TEXT("Should we enable aim assist while shooting?"),
		ECVF_Cheat);

	static bool bDrawAimAssistDebug = false;
	static FAutoConsoleVariableRef CVarDrawAimAssistDebug(
		TEXT("lyra.Weapon.DrawAimAssistDebug"),
		bDrawAimAssistDebug,
		TEXT("Should we draw some debug stats about aim assist?"),
		ECVF_Cheat);
}

///////////////////////////////////////////////////////////////////
// FLyraAimAssistTarget

void FLyraAimAssistTarget::ResetTarget()
{
	TargetShapeComponent = nullptr;

	Location = FVector::ZeroVector;
	DeltaMovement = FVector::ZeroVector;
	ScreenBounds.Init();

	ViewDistance = 0.0f;
	SortScore = 0.0f;

	AssistTime = 0.0f;
	AssistWeight = 0.0f;

	VisibilityTraceHandle = FTraceHandle();

	bIsVisible = false;
	bUnderAssistInnerReticle = false;
	bUnderAssistOuterReticle = false;	
}

FRotator FLyraAimAssistTarget::GetRotationFromMovement(const FAimAssistOwnerViewData& OwnerInfo) const
{
	ensure(OwnerInfo.IsDataValid());

	// Convert everything into player space.
	// Account for player movement in new target location.
	const FVector OldLocation = OwnerInfo.PlayerInverseTransform.TransformPositionNoScale(Location - DeltaMovement);
	const FVector NewLocation = OwnerInfo.PlayerInverseTransform.TransformPositionNoScale(Location - OwnerInfo.DeltaMovement);

	FRotator RotationToTarget;
	RotationToTarget.Yaw = CalculateRotationToTarget2D(NewLocation.X, NewLocation.Y, OldLocation.Y);
	RotationToTarget.Pitch = CalculateRotationToTarget2D(NewLocation.X, NewLocation.Z, OldLocation.Z);
	RotationToTarget.Roll = 0.0f;

	return RotationToTarget;
}

float FLyraAimAssistTarget::CalculateRotationToTarget2D(float TargetX, float TargetY, float OffsetY) const
{
	if (TargetX <= 0.0f)
	{
		return 0.0f;
	}

	const float AngleA = FMath::RadiansToDegrees(FMath::Atan2(TargetY, TargetX));

	if (FMath::IsNearlyZero(OffsetY))
	{
		return AngleA;
	}

	const float Distance = FMath::Sqrt((TargetX * TargetX) + (TargetY * TargetY));
	ensure(Distance > 0.0f);

	const float AngleB = FMath::RadiansToDegrees(FMath::Asin(OffsetY / Distance));

	return FRotator::NormalizeAxis(AngleA - AngleB);
}

///////////////////////////////////////////////////////////////////
// FAimAssistSettings

FAimAssistSettings::FAimAssistSettings()
{
	AssistInnerReticleWidth.SetValue(20.0f);
	AssistInnerReticleHeight.SetValue(20.0f);
	AssistOuterReticleWidth.SetValue(80.0f);
	AssistOuterReticleHeight.SetValue(80.0f);

	TargetingReticleWidth.SetValue(1200.0f);
	TargetingReticleHeight.SetValue(675.0f);
	TargetRange.SetValue(10000.0f);
	TargetWeightCurve = nullptr;

	PullInnerStrengthHip.SetValue(0.6f);
	PullOuterStrengthHip.SetValue(0.5f);
	PullInnerStrengthAds.SetValue(0.7f);
	PullOuterStrengthAds.SetValue(0.4f);
	PullLerpInRate.SetValue(60.0f);
	PullLerpOutRate.SetValue(4.0f);
	PullMaxRotationRate.SetValue(0.0f);

	SlowInnerStrengthHip.SetValue(0.6f);
	SlowOuterStrengthHip.SetValue(0.5f);
	SlowInnerStrengthAds.SetValue(0.7f);
	SlowOuterStrengthAds.SetValue(0.4f);
	SlowLerpInRate.SetValue(60.0f);
	SlowLerpOutRate.SetValue(4.0f);
	SlowMinRotationRate.SetValue(0.0f);

	bEnableAsyncVisibilityTrace = true;
	bRequireInput = true;
	bApplyPull = true;
	bApplySlowing = true;
	bApplyStrafePullScale = true;
	bUseDynamicSlow = true;
	bUseRadialLookRates = true;
}

float FAimAssistSettings::GetTargetWeightForTime(float Time) const
{
	if (!ensure(TargetWeightCurve != nullptr))
	{
		return 0.0f;
	}

	return FMath::Clamp(TargetWeightCurve->GetFloatValue(Time), 0.0f, 1.0f);
}

float FAimAssistSettings::GetTargetWeightMaxTime() const
{
	if (!ensure(TargetWeightCurve != nullptr))
	{
		return 0.0f;
	}

	float MinTime = 0.0f;
	float MaxTime = 0.0f;

	TargetWeightCurve->FloatCurve.GetTimeRange(MinTime, MaxTime);

	return MaxTime;
}

///////////////////////////////////////////////////////////////////
// FAimAssistOwnerViewData

void FAimAssistOwnerViewData::UpdateViewData(const APlayerController* PC)
{
	FSceneViewProjectionData ProjectionData;
	PlayerController = PC;
	LocalPlayer = PlayerController ? PlayerController->GetLocalPlayer() : nullptr;

	if (!IsDataValid() || !PlayerController || !LocalPlayer)
	{
		ResetViewData();
		return;
	}
	
	const APawn* Pawn = Cast<APawn>(PlayerController->GetPawn());
	
	if (!Pawn || !LocalPlayer || !LocalPlayer->ViewportClient || !LocalPlayer->GetProjectionData(LocalPlayer->ViewportClient->Viewport, ProjectionData))
	{
		ResetViewData();
		return;
	}

	FVector ViewLocation;
	FRotator ViewRotation;
	PC->GetPlayerViewPoint(ViewLocation, ViewRotation);

	ProjectionMatrix = ProjectionData.ProjectionMatrix;
	ViewProjectionMatrix = ProjectionData.ComputeViewProjectionMatrix();
	ViewRect = ProjectionData.GetConstrainedViewRect();
	ViewTransform = FTransform(ViewRotation, ViewLocation);
	ViewForward = ViewTransform.GetUnitAxis(EAxis::X);

	const FVector OldLocation = PlayerTransform.GetTranslation();
	const FVector NewLocation = Pawn->GetActorLocation();
	const FRotator NewRotation = PC->GetControlRotation();

	PlayerTransform = FTransform(NewRotation, NewLocation);
	PlayerInverseTransform = PlayerTransform.Inverse();

	DeltaMovement = (NewLocation - OldLocation);

	// Set the Team ID
	if (ALyraPlayerState* LyraPS = PlayerController->GetPlayerState<ALyraPlayerState>())
	{
		TeamID = LyraPS->GetTeamId();
	}
	else
	{
		TeamID = INDEX_NONE;
	}
}

void FAimAssistOwnerViewData::ResetViewData()
{
	PlayerController = nullptr;
	LocalPlayer = nullptr;
	
	ProjectionMatrix = FMatrix::Identity;
	ViewProjectionMatrix = FMatrix::Identity;
	ViewRect = FIntRect(0, 0, 0, 0);
	ViewTransform = FTransform::Identity;

	PlayerTransform = FTransform::Identity;
	PlayerInverseTransform = FTransform::Identity;
	ViewForward = FVector::ZeroVector;

	DeltaMovement = FVector::ZeroVector;
	TeamID = INDEX_NONE;
}

FBox2D FAimAssistOwnerViewData::ProjectReticleToScreen(float ReticleWidth, float ReticleHeight, float ReticleDepth) const
{
	FBox2D ReticleBounds(ForceInitToZero);

	const FVector ReticleExtents((ReticleWidth * 0.5f), -(ReticleHeight * 0.5f), ReticleDepth);

	if (FSceneView::ProjectWorldToScreen(ReticleExtents, ViewRect, ProjectionMatrix, ReticleBounds.Max))
	{
		ReticleBounds.Min.X = ViewRect.Min.X + (ViewRect.Max.X - ReticleBounds.Max.X);
		ReticleBounds.Min.Y = ViewRect.Min.Y + (ViewRect.Max.Y - ReticleBounds.Max.Y);

		ReticleBounds.bIsValid = true;
	}

	return ReticleBounds;
}

FBox2D FAimAssistOwnerViewData::ProjectBoundsToScreen(const FBox& Bounds) const
{
	FBox2D Box2D(ForceInitToZero);

	if (Bounds.IsValid)
	{
		const FVector Vertices[] =
		{
			FVector(Bounds.Min),
			FVector(Bounds.Min.X, Bounds.Min.Y, Bounds.Max.Z),
			FVector(Bounds.Min.X, Bounds.Max.Y, Bounds.Min.Z),
			FVector(Bounds.Max.X, Bounds.Min.Y, Bounds.Min.Z),
			FVector(Bounds.Max.X, Bounds.Max.Y, Bounds.Min.Z),
			FVector(Bounds.Max.X, Bounds.Min.Y, Bounds.Max.Z),
			FVector(Bounds.Min.X, Bounds.Max.Y, Bounds.Max.Z),
			FVector(Bounds.Max)
		};

		for (int32 VerticeIndex = 0; VerticeIndex < UE_ARRAY_COUNT(Vertices); ++VerticeIndex)
		{
			FVector2D ScreenPoint;
			if (FSceneView::ProjectWorldToScreen(Vertices[VerticeIndex], ViewRect, ViewProjectionMatrix, ScreenPoint))
			{
				Box2D += ScreenPoint;
			}
		}
	}

	return Box2D;
}

FBox2D FAimAssistOwnerViewData::ProjectShapeToScreen(const FCollisionShape& Shape, const FVector& ShapeOrigin, const FTransform& WorldTransform) const
{
	FBox2D Box2D(ForceInitToZero);

	switch (Shape.ShapeType)
	{
	case ECollisionShape::Box:
		Box2D = ProjectBoxToScreen(Shape, ShapeOrigin, WorldTransform);
		break;
	case ECollisionShape::Sphere:
		Box2D = ProjectSphereToScreen(Shape, ShapeOrigin, WorldTransform);
		break;
	case ECollisionShape::Capsule:
		Box2D = ProjectCapsuleToScreen(Shape, ShapeOrigin, WorldTransform);
		break;
	default:
		UE_LOG(LogAimAssist, Warning, TEXT("FAimAssistOwnerViewData::ProjectShapeToScreen() - Invalid shape type!"));
		break;
	}

	return Box2D;
}

FBox2D FAimAssistOwnerViewData::ProjectBoxToScreen(const FCollisionShape& Shape, const FVector& ShapeOrigin, const FTransform& WorldTransform) const
{
	check(Shape.IsBox());
	check(!Shape.IsNearlyZero());

	const FVector BoxExtents = Shape.GetBox();

	const FVector Vertices[] =
	{
		FVector(-BoxExtents.X, -BoxExtents.Y, -BoxExtents.Z),
		FVector(-BoxExtents.X, -BoxExtents.Y,  BoxExtents.Z),
		FVector(-BoxExtents.X,  BoxExtents.Y, -BoxExtents.Z),
		FVector(-BoxExtents.X,  BoxExtents.Y,  BoxExtents.Z),
		FVector( BoxExtents.X, -BoxExtents.Y, -BoxExtents.Z),
		FVector( BoxExtents.X, -BoxExtents.Y,  BoxExtents.Z),
		FVector( BoxExtents.X,  BoxExtents.Y, -BoxExtents.Z),
		FVector( BoxExtents.X,  BoxExtents.Y,  BoxExtents.Z)
	};

	FBox2D Box2D(ForceInitToZero);

	for (int32 VerticeIndex = 0; VerticeIndex < UE_ARRAY_COUNT(Vertices); ++VerticeIndex)
	{
		const FVector Vertex = WorldTransform.TransformPositionNoScale(Vertices[VerticeIndex] + ShapeOrigin);

		FVector2D ScreenPoint;
		if (FSceneView::ProjectWorldToScreen(Vertex, ViewRect, ViewProjectionMatrix, ScreenPoint))
		{
			Box2D += ScreenPoint;
		}
	}

	return Box2D;
}

FBox2D FAimAssistOwnerViewData::ProjectSphereToScreen(const FCollisionShape& Shape, const FVector& ShapeOrigin, const FTransform& WorldTransform) const
{
	check(Shape.IsSphere());
	check(!Shape.IsNearlyZero());

	const FVector ViewAxisY = ViewTransform.GetUnitAxis(EAxis::Y);
	const FVector ViewAxisZ = ViewTransform.GetUnitAxis(EAxis::Z);

	const float SphereRadius = Shape.GetSphereRadius();
	const FVector SphereLocation = WorldTransform.TransformPositionNoScale(ShapeOrigin);
	const FVector SphereExtent = (ViewAxisY * SphereRadius) + (ViewAxisZ * SphereRadius);

	const FVector Vertices[] =
	{
		FVector(SphereLocation + SphereExtent),
		FVector(SphereLocation - SphereExtent),
	};

	FBox2D Box2D(ForceInitToZero);

	for (int32 VerticeIndex = 0; VerticeIndex < UE_ARRAY_COUNT(Vertices); ++VerticeIndex)
	{
		FVector2D ScreenPoint;
		if (FSceneView::ProjectWorldToScreen(Vertices[VerticeIndex], ViewRect, ViewProjectionMatrix, ScreenPoint))
		{
			Box2D += ScreenPoint;
		}
	}

	return Box2D;
}

FBox2D FAimAssistOwnerViewData::ProjectCapsuleToScreen(const FCollisionShape& Shape, const FVector& ShapeOrigin, const FTransform& WorldTransform) const
{
	check(Shape.IsCapsule());
	check(!Shape.IsNearlyZero());

	const FVector ViewAxisY = ViewTransform.GetUnitAxis(EAxis::Y);
	const FVector ViewAxisZ = ViewTransform.GetUnitAxis(EAxis::Z);

	const float CapsuleAxisHalfLength = Shape.GetCapsuleAxisHalfLength();
	const float CapsuleRadius = Shape.GetCapsuleRadius();

	const FVector TopSphereLocation = WorldTransform.TransformPositionNoScale(FVector(0.0f, 0.0f, CapsuleAxisHalfLength) + ShapeOrigin);
	const FVector BottomSphereLocation = WorldTransform.TransformPositionNoScale(FVector(0.0f, 0.0f, -CapsuleAxisHalfLength) + ShapeOrigin);
	const FVector SphereExtent = (ViewAxisY * CapsuleRadius) + (ViewAxisZ * CapsuleRadius);

	const FVector Vertices[] =
	{
		FVector(TopSphereLocation + SphereExtent),
		FVector(TopSphereLocation - SphereExtent),
		FVector(BottomSphereLocation + SphereExtent),
		FVector(BottomSphereLocation - SphereExtent),
	};

	FBox2D Box2D(ForceInitToZero);

	for (int32 VerticeIndex = 0; VerticeIndex < UE_ARRAY_COUNT(Vertices); ++VerticeIndex)
	{
		FVector2D ScreenPoint;
		if (FSceneView::ProjectWorldToScreen(Vertices[VerticeIndex], ViewRect, ViewProjectionMatrix, ScreenPoint))
		{
			Box2D += ScreenPoint;
		}
	}

	return Box2D;
}

///////////////////////////////////////////////////////////////////
// UAimAssistInputModifier

static const float GamepadUserOptions_YawLookRateBase = 900.0f;
static const float GamepadUserOptions_PitchLookRateBase = (GamepadUserOptions_YawLookRateBase * 0.6f);

// TODO Make this a constexpr instead of a define
#define YawLookSpeedToRotationRate(_Speed)		((_Speed) / 100.0f * GamepadUserOptions_YawLookRateBase)
#define PitchLookSpeedToRotationRate(_Speed)	((_Speed) / 100.0f * GamepadUserOptions_PitchLookRateBase)

FRotator UAimAssistInputModifier::GetLookRates(const FVector& LookInput)
{
	FRotator LookRates;

	const float SensitivityHipLevel = 50.0f;
	{
		LookRates.Yaw = YawLookSpeedToRotationRate(SensitivityHipLevel);
		LookRates.Pitch = PitchLookSpeedToRotationRate(SensitivityHipLevel);
		LookRates.Roll = 0.0f;
	}

	LookRates.Yaw = FMath::Clamp(LookRates.Yaw, 0.0f, GamepadUserOptions_YawLookRateBase);
	LookRates.Pitch = FMath::Clamp(LookRates.Pitch, 0.0f, GamepadUserOptions_PitchLookRateBase);

	if (Settings.bUseRadialLookRates)
	{
		// Blend between yaw and pitch based on stick deflection.  This keeps diagonals accurate.
		const float RadialLerp = FMath::Atan2(FMath::Abs(LookInput.Y), FMath::Abs(LookInput.X)) / HALF_PI;
		const float RadialLookRate = FMath::Lerp(LookRates.Yaw, LookRates.Pitch, RadialLerp);

		LookRates.Yaw = RadialLookRate;
		LookRates.Pitch = RadialLookRate;	
	}
	
	return LookRates;
}

FInputActionValue UAimAssistInputModifier::ModifyRaw_Implementation(const UEnhancedPlayerInput* PlayerInput, FInputActionValue CurrentValue, float DeltaTime)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UAimAssistInputModifier::ModifyRaw_Implementation);

#if ENABLE_DRAW_DEBUG
	if (LyraConsoleVariables::bDrawAimAssistDebug)
	{
		if (!DebugDrawHandle.IsValid())
		{
			DebugDrawHandle = UDebugDrawService::Register(TEXT("Game"), FDebugDrawDelegate::CreateUObject(this, &UAimAssistInputModifier::AimAssistDebugDraw));
		}
		else
		{
			UDebugDrawService::Unregister(DebugDrawHandle);
			DebugDrawHandle.Reset();
		}
		bRegisteredDebug = true;
	}
#endif

#if !UE_BUILD_SHIPPING
	if (!LyraConsoleVariables::bEnableAimAssist)
	{
		return CurrentValue;
	}
#endif //UE_BUILD_SHIPPING

	APlayerController* PC = PlayerInput ? Cast<APlayerController>(PlayerInput->GetOuter()) : nullptr;
	if (!PC)
	{
		return CurrentValue;
	}

	// Update the "owner" information based on our current player controller. This calculates and stores things like the view matrix
	// and current rotation that is used to determine what targets are visible
	OwnerViewData.UpdateViewData(PC);

	if (!OwnerViewData.IsDataValid())
	{
		return CurrentValue;
	}
	
	// Swaps the target cache's and determines what targets are currently visible. Updates the score of each target to determine
	// how much pull/slow effect should be applied to each
	UpdateTargetData(DeltaTime);

	FVector BaselineInput = CurrentValue.Get<FVector>();
	
	FVector OutAssistedInput = BaselineInput;
	FVector CurrentMoveInput = MoveInputAction ? PlayerInput->GetActionValue(MoveInputAction).Get<FVector>() : FVector::ZeroVector;	

	// Something about the look rates is incorrect
	FRotator LookRates = GetLookRates(BaselineInput);
	
	const FRotator RotationalVelocity = UpdateRotationalVelocity(PC, DeltaTime, BaselineInput, CurrentMoveInput);
	
	if (LookRates.Yaw > 0.0f)
	{
		OutAssistedInput.X = (RotationalVelocity.Yaw / LookRates.Yaw);
		OutAssistedInput.X = FMath::Clamp(OutAssistedInput.X, -1.0f, 1.0f);
	}
	
	if (LookRates.Pitch > 0.0f)
	{
		OutAssistedInput.Y = (RotationalVelocity.Pitch / LookRates.Pitch);
		OutAssistedInput.Y = FMath::Clamp(OutAssistedInput.Y, -1.0f, 1.0f);
	}

#if ENABLE_DRAW_DEBUG
	LastBaselineValue = BaselineInput;
	LastLookRatePitch = LookRates.Pitch;
	LastLookRateYaw = LookRates.Yaw;
	LastOutValue = OutAssistedInput;
#endif
	return OutAssistedInput;
}

void UAimAssistInputModifier::UpdateTargetData(float DeltaTime)
{
	if(!ensure(OwnerViewData.PlayerController))
	{
		UE_LOG(LogAimAssist, Error, TEXT("[UAimAssistInputModifier::UpdateTargetData] Invalid player controller in owner view data!"));
		return;
	}
	
	UAimAssistTargetManagerComponent* TargetManager = nullptr;

	if (UWorld* World = OwnerViewData.PlayerController->GetWorld())
	{
		if (AGameStateBase* GameState = World->GetGameState())
		{
			TargetManager = GameState->FindComponentByClass<UAimAssistTargetManagerComponent>();	
		}
	}
	
	if (!TargetManager)
	{
		return;
	}

	// Update the targets based on what is visible
	SwapTargetCaches();
	const TArray<FLyraAimAssistTarget>& OldTargetCache = GetPreviousTargetCache();
	TArray<FLyraAimAssistTarget>& NewTargetCache = GetCurrentTargetCache();
	
	TargetManager->GetVisibleTargets(Filter, Settings, OwnerViewData, OldTargetCache, NewTargetCache);

	//
	// Update target weights.
	//
	float TotalAssistWeight = 0.0f;

	for (FLyraAimAssistTarget& Target : NewTargetCache)
	{
		if (Target.bUnderAssistOuterReticle && Target.bIsVisible)
		{
			const float MaxAssistTime = Settings.GetTargetWeightMaxTime();
			Target.AssistTime = FMath::Min((Target.AssistTime + DeltaTime), MaxAssistTime);
		}
		else
		{
			Target.AssistTime = FMath::Max((Target.AssistTime - DeltaTime), 0.0f);
		}

		// Look up assist weight based on how long the target has been under the assist reticle.
		Target.AssistWeight = Settings.GetTargetWeightForTime(Target.AssistTime);

		TotalAssistWeight += Target.AssistWeight;
	}

	// Normalize the weights.
	if (TotalAssistWeight > 0.0f)
	{
		for (FLyraAimAssistTarget& Target : NewTargetCache)
		{
			Target.AssistWeight = (Target.AssistWeight / TotalAssistWeight);
		}
	}
}

const float UAimAssistInputModifier::GetSensitivtyScalar(const ULyraSettingsShared* SharedSettings) const
{
	if (SharedSettings && SensitivityLevelTable)
	{
		const ELyraGamepadSensitivity Sens = TargetingType == ELyraTargetingType::Normal ? SharedSettings->GetGamepadLookSensitivityPreset() : SharedSettings->GetGamepadTargetingSensitivityPreset();
		return SensitivityLevelTable->SensitivtyEnumToFloat(Sens);
	}
	
	UE_LOG(LogAimAssist, Warning, TEXT("SensitivityLevelTable is null, using default value!"));
	return (TargetingType == ELyraTargetingType::Normal) ? 1.0f : 0.5f;	
}

FRotator UAimAssistInputModifier::UpdateRotationalVelocity(APlayerController* PC, float DeltaTime, FVector CurrentLookInputValue, FVector CurrentMoveInputValue)
{
	FRotator RotationalVelocity(ForceInitToZero);
	FRotator RotationNeeded(ForceInitToZero);
	
	float PullStrength = 0.0f;
	float SlowStrength = 0.0f;
	
	const TArray<FLyraAimAssistTarget>& TargetCache = GetCurrentTargetCache();

	float LookStickDeadzone = 0.25f;
	float MoveStickDeadzone = 0.25f;
	float SettingStrengthScalar = (TargetingType == ELyraTargetingType::Normal) ? 1.0f : 0.5f;

	if (ULyraLocalPlayer* LP = Cast<ULyraLocalPlayer>(PC->GetLocalPlayer()))
	{
		ULyraSettingsShared* SharedSettings = LP->GetSharedSettings();
		LookStickDeadzone = SharedSettings->GetGamepadLookStickDeadZone();
		MoveStickDeadzone = SharedSettings->GetGamepadMoveStickDeadZone();
		SettingStrengthScalar = GetSensitivtyScalar(SharedSettings);
	}
	
	for (const FLyraAimAssistTarget& Target : TargetCache)
	{
		if (Target.bUnderAssistOuterReticle && Target.bIsVisible)
		{
			// Add up total rotation needed to follow weighted targets based on target and player movement.
			RotationNeeded += (Target.GetRotationFromMovement(OwnerViewData) * Target.AssistWeight);

			float TargetPullStrength = 0.0f;
			float TargetSlowStrength = 0.0f;
			CalculateTargetStrengths(Target, TargetPullStrength, TargetSlowStrength);

			// Add up total amount of weighted pull and slow from the targets.
			PullStrength += TargetPullStrength;
			SlowStrength += TargetSlowStrength;
		}
	}

	// You could also apply some scalars based on the current weapon that is equipped, the player's movement state,
	// or any other factors you want here
	PullStrength *= Settings.StrengthScale * SettingStrengthScalar;
	SlowStrength *= Settings.StrengthScale * SettingStrengthScalar;

	const float PullLerpRate = (PullStrength > LastPullStrength) ? Settings.PullLerpInRate.GetValue() : Settings.PullLerpOutRate.GetValue();
	if (PullLerpRate > 0.0f)
	{
		PullStrength = FMath::FInterpConstantTo(LastPullStrength, PullStrength, DeltaTime, PullLerpRate);
	}

	const float SlowLerpRate = (SlowStrength > LastSlowStrength) ? Settings.SlowLerpInRate.GetValue() : Settings.SlowLerpOutRate.GetValue();
	if (SlowLerpRate > 0.0f)
	{
		SlowStrength = FMath::FInterpConstantTo(LastSlowStrength, SlowStrength, DeltaTime, SlowLerpRate);
	}

	LastPullStrength = PullStrength;
	LastSlowStrength = SlowStrength;

	const bool bIsLookInputActive =  (CurrentLookInputValue.SizeSquared() > FMath::Square(LookStickDeadzone));
	const bool bIsMoveInputActive = (CurrentMoveInputValue.SizeSquared() > FMath::Square(MoveStickDeadzone));
	
	const bool bIsApplyingLookInput = (bIsLookInputActive || !Settings.bRequireInput);
	const bool bIsApplyingMoveInput = (bIsMoveInputActive || !Settings.bRequireInput);
	const bool bIsApplyingAnyInput = (bIsApplyingLookInput || bIsApplyingMoveInput);

	// Apply pulling towards the target
	if (Settings.bApplyPull && bIsApplyingAnyInput && !FMath::IsNearlyZero(PullStrength))
	{
		// The amount of pull is a percentage of the rotation needed to stay on target.
		FRotator PullRotation = (RotationNeeded * PullStrength);

		if (!bIsApplyingLookInput && Settings.bApplyStrafePullScale)
		{
			// Scale pull strength by amount of player strafe if the player isn't actively looking around.
			// This helps prevent view yanks when running forward past targets.
			float StrafePullScale = FMath::Abs(CurrentMoveInputValue.Y);
		
			PullRotation.Yaw *= StrafePullScale;
			PullRotation.Pitch *= StrafePullScale;
		}

		// Clamp the maximum amount of pull rotation to prevent it from yanking the player's view too much.
		// The clamped rate is scaled so it feels the same regardless of field of view.
		const float FOVScale = UAimAssistTargetManagerComponent::GetFOVScale(PC, ECommonInputType::Gamepad);
		const float PullMaxRotationRate = (Settings.PullMaxRotationRate.GetValue() * FOVScale);
		if (PullMaxRotationRate > 0.0f)
		{
			const float PullMaxRotation = (PullMaxRotationRate * DeltaTime);

			PullRotation.Yaw = FMath::Clamp(PullRotation.Yaw, -PullMaxRotation, PullMaxRotation);
			PullRotation.Pitch = FMath::Clamp(PullRotation.Pitch, -PullMaxRotation, PullMaxRotation);
		}

		RotationNeeded -= PullRotation;
		RotationalVelocity += (PullRotation * (1.0f / DeltaTime));
	}

	FRotator LookRates = GetLookRates(CurrentLookInputValue);

	// Apply slowing
	if (Settings.bApplySlowing && bIsApplyingLookInput && !FMath::IsNearlyZero(SlowStrength))
	{
		// The slowed rotation rate is a percentage of the normal look rotation rates.
		FRotator SlowRates = (LookRates * (1.0f - SlowStrength));

		const bool bUseDynamicSlow = true;

		if (Settings.bUseDynamicSlow)
		{
			const FRotator BoostRotation = (RotationNeeded * (1.0f / DeltaTime));

			const float YawDynamicBoost = (BoostRotation.Yaw * FMath::Sign(CurrentLookInputValue.X));
			if (YawDynamicBoost > 0.0f)
			{
				SlowRates.Yaw += YawDynamicBoost;
			}

			const float PitchDynamicBoost = (BoostRotation.Pitch * FMath::Sign(CurrentLookInputValue.Y));
			if (PitchDynamicBoost > 0.0f)
			{
				SlowRates.Pitch += PitchDynamicBoost;
			}
		}

		// Clamp the minimum amount of slow to prevent it from feeling sluggish on low sensitivity settings.
		// The clamped rate is scaled so it feels the same regardless of field of view.
		const float FOVScale = UAimAssistTargetManagerComponent::GetFOVScale(PC, ECommonInputType::Gamepad);
		const float SlowMinRotationRate = (Settings.SlowMinRotationRate.GetValue() * FOVScale);
		if (SlowMinRotationRate > 0.0f)
		{
			SlowRates.Yaw = FMath::Max(SlowRates.Yaw, SlowMinRotationRate);
			SlowRates.Pitch = FMath::Max(SlowRates.Pitch, SlowMinRotationRate);
		}

		// Make sure the slow rate isn't faster then our default.
		SlowRates.Yaw = FMath::Min(SlowRates.Yaw, LookRates.Yaw);
		SlowRates.Pitch = FMath::Min(SlowRates.Pitch, LookRates.Pitch);

		RotationalVelocity.Yaw += (CurrentLookInputValue.X * SlowRates.Yaw);
		RotationalVelocity.Pitch += (CurrentLookInputValue.Y * SlowRates.Pitch);
		RotationalVelocity.Roll = 0.0f;
	}
	else
	{
		RotationalVelocity.Yaw += (CurrentLookInputValue.X * LookRates.Yaw);
		RotationalVelocity.Pitch += (CurrentLookInputValue.Y * LookRates.Pitch);
		RotationalVelocity.Roll = 0.0f;
	}

	return RotationalVelocity;
}

void UAimAssistInputModifier::CalculateTargetStrengths(const FLyraAimAssistTarget& Target, float& OutPullStrength, float& OutSlowStrength) const
{
	const bool bIsADS = (TargetingType == ELyraTargetingType::ADS);
	
	if (Target.bUnderAssistInnerReticle)
	{
		if (bIsADS)
		{
			OutPullStrength = Settings.PullInnerStrengthAds.GetValue();
			OutSlowStrength = Settings.SlowInnerStrengthAds.GetValue();
		}
		else
		{
			OutPullStrength = Settings.PullInnerStrengthHip.GetValue();
			OutSlowStrength = Settings.SlowInnerStrengthHip.GetValue();
		}
	}
	else if (Target.bUnderAssistOuterReticle)
	{
		if (bIsADS)
		{
			OutPullStrength = Settings.PullOuterStrengthAds.GetValue();
			OutSlowStrength = Settings.SlowOuterStrengthAds.GetValue();
		}
		else
		{
			OutPullStrength = Settings.PullOuterStrengthHip.GetValue();
			OutSlowStrength = Settings.SlowOuterStrengthHip.GetValue();
		}
	}
	else
	{
		OutPullStrength = 0.0f;
		OutSlowStrength = 0.0f;
	}

	OutPullStrength *= Target.AssistWeight;
	OutSlowStrength *= Target.AssistWeight;
}

#if ENABLE_DRAW_DEBUG
void UAimAssistInputModifier::AimAssistDebugDraw(UCanvas* Canvas, APlayerController* PC)
{
	if (!Canvas || !OwnerViewData.IsDataValid() || !LyraConsoleVariables::bDrawAimAssistDebug)
	{
		return;
	}

	const bool bIsADS = (TargetingType == ELyraTargetingType::ADS);
	
	FDisplayDebugManager& DisplayDebugManager = Canvas->DisplayDebugManager;
	DisplayDebugManager.Initialize(Canvas, GEngine->GetSmallFont(), FVector2D((bIsADS ? 4.0f : 170.0f), 150.0f));
	DisplayDebugManager.SetDrawColor(FColor::Yellow);

	DisplayDebugManager.DrawString(FString(TEXT("------------------------------")));
	DisplayDebugManager.DrawString(FString(TEXT("Aim Assist Debug Draw")));
	DisplayDebugManager.DrawString(FString(TEXT("------------------------------")));
	DisplayDebugManager.DrawString(FString::Printf(TEXT("Strength Scale: (%.4f)"), Settings.StrengthScale));
	DisplayDebugManager.DrawString(FString::Printf(TEXT("Pull Strength: (%.4f)"), LastPullStrength));
	DisplayDebugManager.DrawString(FString::Printf(TEXT("Slow Strength: (%.4f)"), LastSlowStrength));
	DisplayDebugManager.DrawString(FString::Printf(TEXT("Look Rate Yaw: (%.4f)"), LastLookRateYaw));
	DisplayDebugManager.DrawString(FString::Printf(TEXT("Look Rate Pitch: (%.4f)"), LastLookRatePitch));
	DisplayDebugManager.DrawString(FString::Printf(TEXT("Baseline Value: (%.4f, %.4f, %.4f)"), LastBaselineValue.X, LastBaselineValue.Y, LastBaselineValue.Z));
	DisplayDebugManager.DrawString(FString::Printf(TEXT("Assisted Value: (%.4f, %.4f, %.4f)"), LastOutValue.X, LastOutValue.Y, LastOutValue.Z));

	
	UWorld* World = OwnerViewData.PlayerController->GetWorld();
	check(World);

	const FBox2D AssistInnerReticleBounds = OwnerViewData.ProjectReticleToScreen(Settings.AssistInnerReticleWidth.GetValue(), Settings.AssistInnerReticleHeight.GetValue(), Settings.ReticleDepth);
	const FBox2D AssistOuterReticleBounds = OwnerViewData.ProjectReticleToScreen(Settings.AssistOuterReticleWidth.GetValue(), Settings.AssistOuterReticleHeight.GetValue(), Settings.ReticleDepth);
	const FBox2D TargetingReticleBounds = OwnerViewData.ProjectReticleToScreen(Settings.TargetingReticleWidth.GetValue(), Settings.TargetingReticleHeight.GetValue(), Settings.ReticleDepth);

	if (TargetingReticleBounds.bIsValid)
	{
		FLinearColor ReticleColor(0.25f, 0.25f, 0.25f, 1.0f);
		DrawDebugCanvas2DBox(Canvas, TargetingReticleBounds, ReticleColor, 1.0f);	
	}

	if (AssistInnerReticleBounds.bIsValid)
	{
		FLinearColor ReticleColor(0.0f, 0.0f, 1.0f, 0.2f);

		FCanvasTileItem ReticleTileItem(AssistInnerReticleBounds.Min, AssistInnerReticleBounds.GetSize(), ReticleColor);
		ReticleTileItem.BlendMode = SE_BLEND_Translucent;
		Canvas->DrawItem(ReticleTileItem);

		ReticleColor.A = 1.0f;
		DrawDebugCanvas2DBox(Canvas, AssistInnerReticleBounds, ReticleColor, 1.0f);
	}

	if (AssistOuterReticleBounds.bIsValid)
	{
		FLinearColor ReticleColor(0.25f, 0.25f, 1.0f, 0.2f);

		FCanvasTileItem ReticleTileItem(AssistOuterReticleBounds.Min, AssistOuterReticleBounds.GetSize(), ReticleColor);
		ReticleTileItem.BlendMode = SE_BLEND_Translucent;
		Canvas->DrawItem(ReticleTileItem);

		ReticleColor.A = 1.0f;
		DrawDebugCanvas2DBox(Canvas, AssistOuterReticleBounds, ReticleColor, 1.0f);
	}

	const TArray<FLyraAimAssistTarget>& TargetCache = GetCurrentTargetCache();
	for (const FLyraAimAssistTarget& Target : TargetCache)
	{
		if (Target.ScreenBounds.bIsValid)
		{
			FLinearColor TargetColor = ((Target.AssistWeight > 0.0f) ? FLinearColor::LerpUsingHSV(FLinearColor::Yellow, FLinearColor::Green, Target.AssistWeight) : FLinearColor::Black);
			TargetColor.A = 0.2f;

			FCanvasTileItem TargetTileItem(Target.ScreenBounds.Min, Target.ScreenBounds.GetSize(), TargetColor);
			TargetTileItem.BlendMode = SE_BLEND_Translucent;
			Canvas->DrawItem(TargetTileItem);

			if (Target.bIsVisible)
			{
				TargetColor.A = 1.0f;
				DrawDebugCanvas2DBox(Canvas, Target.ScreenBounds, TargetColor, 1.0f);
			}

			FCanvasTextItem TargetTextItem(FVector2D::ZeroVector, FText::FromString(FString::Printf(TEXT("Weight: %.2f\nDist: %.2f\nScore: %.2f\nTime: %.2f"), Target.AssistWeight, Target.ViewDistance, Target.SortScore, Target.AssistTime)), GEngine->GetSmallFont(), FLinearColor::White);
			TargetTextItem.EnableShadow(FLinearColor::Black);
			Canvas->DrawItem(TargetTextItem, FVector2D(FMath::CeilToFloat(Target.ScreenBounds.Min.X), FMath::CeilToFloat(Target.ScreenBounds.Min.Y)));
		}
	}
}
#endif	// ENABLE_DRAW_DEBUG