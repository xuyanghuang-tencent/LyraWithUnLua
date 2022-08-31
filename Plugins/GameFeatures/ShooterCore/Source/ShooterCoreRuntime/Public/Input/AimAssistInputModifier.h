// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InputModifiers.h"
#include "GameplayTagContainer.h"
#include "ScalableFloat.h"
#include "WorldCollision.h"
#include "Input/LyraInputModifiers.h"
#include "DrawDebugHelpers.h"
#include "AimAssistInputModifier.generated.h"

class APlayerController;
class UInputAction;
class ULocalPlayer;
class UShapeComponent;
class ULyraAimSensitivityData;
class ULyraSettingsShared;

DECLARE_LOG_CATEGORY_EXTERN(LogAimAssist, Log, All);

/** A container for some commonly used viewport data based on the current pawn */
struct FAimAssistOwnerViewData
{
	FAimAssistOwnerViewData() { ResetViewData(); }

	/**
	 * Update the "owner" information based on our current player controller. This calculates and stores things like the view matrix
	 * and current rotation that is used to determine what targets are visible
	 */
	void UpdateViewData(const APlayerController* PC);

	/** Reset all the properties on this set of data to their defaults */
	void ResetViewData();

	/** Returns true if this owner struct has a valid player controller */
	bool IsDataValid() const { return PlayerController != nullptr && LocalPlayer != nullptr; }

	FBox2D ProjectReticleToScreen(float ReticleWidth, float ReticleHeight, float ReticleDepth) const;
	FBox2D ProjectBoundsToScreen(const FBox& Bounds) const;
	FBox2D ProjectShapeToScreen(const FCollisionShape& Shape, const FVector& ShapeOrigin, const FTransform& WorldTransform) const;
	FBox2D ProjectBoxToScreen(const FCollisionShape& Shape, const FVector& ShapeOrigin, const FTransform& WorldTransform) const;
	FBox2D ProjectSphereToScreen(const FCollisionShape& Shape, const FVector& ShapeOrigin, const FTransform& WorldTransform) const;
	FBox2D ProjectCapsuleToScreen(const FCollisionShape& Shape, const FVector& ShapeOrigin, const FTransform& WorldTransform) const;

	/** Pointer to the player controller that can be used to calculate the data we need to check for visible targets */
	const APlayerController* PlayerController = nullptr;

	const ULocalPlayer* LocalPlayer = nullptr;
	
	FMatrix ProjectionMatrix = FMatrix::Identity;
	
	FMatrix ViewProjectionMatrix = FMatrix::Identity;
	
	FIntRect ViewRect = FIntRect(0, 0, 0, 0);
	
	FTransform ViewTransform = FTransform::Identity;
	
	FVector ViewForward = FVector::ZeroVector;
	
	// Player transform is the actor's location and the controller's rotation.
	FTransform PlayerTransform = FTransform::Identity;
	
	FTransform PlayerInverseTransform = FTransform::Identity;

	/** The movement delta between the current frame and the last */
	FVector DeltaMovement = FVector::ZeroVector;

	/** The ID of the team that this owner is from. It is populated from the ALyraPlayerState. If the owner does not have a player state, then it will be INDEX_NONE */
	int32 TeamID = INDEX_NONE;
};

/** A container for keeping the state of targets between frames that can be cached */
USTRUCT(BlueprintType)
struct FLyraAimAssistTarget
{
	GENERATED_BODY()

	FLyraAimAssistTarget() { ResetTarget(); }

	bool IsTargetValid() const { return TargetShapeComponent.IsValid(); }

	void ResetTarget();

	FRotator GetRotationFromMovement(const FAimAssistOwnerViewData& OwnerInfo) const;
	
	TWeakObjectPtr<UShapeComponent> TargetShapeComponent;
	
	FVector Location = FVector::ZeroVector;
	FVector DeltaMovement = FVector::ZeroVector;
	FBox2D ScreenBounds;

	float ViewDistance = 0.0f;
	float SortScore = 0.0f;

	float AssistTime = 0.0f;
	float AssistWeight = 0.0f;

	FTraceHandle VisibilityTraceHandle;
	
	uint8 bIsVisible : 1;
	
	uint8 bUnderAssistInnerReticle : 1;
	
	uint8 bUnderAssistOuterReticle : 1;
	
protected:

	float CalculateRotationToTarget2D(float TargetX, float TargetY, float OffsetY) const;
};

/** Options for filtering out certain aim assist targets */
USTRUCT(BlueprintType)
struct FAimAssistFilter
{
	GENERATED_BODY()

	FAimAssistFilter()
		: bIncludeSameFriendlyTargets(false)
		, bExcludeInstigator(true)
		, bExcludeAllAttachedToInstigator(false)
		, bExcludeRequester(true)
		, bExcludeAllAttachedToRequester(false)
		, bTraceComplexCollision(false)
		, bExcludeDeadOrDying(true)
	{}

	/** If true, then we should include any targets even if they are on our team */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	uint8 bIncludeSameFriendlyTargets : 1;
	
	/** Exclude 'RequestedBy->Instigator' Actor */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = TargetSelection)
	uint8 bExcludeInstigator : 1;
	
	/** Exclude all actors attached to 'RequestedBy->Instigator' Actor */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = TargetSelection)
	uint32 bExcludeAllAttachedToInstigator : 1;

	/** Exclude 'RequestedBy Actor */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = TargetSelection)
	uint8 bExcludeRequester : 1;
	
	/** Exclude all actors attached to 'RequestedBy Actor */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = TargetSelection)
	uint8 bExcludeAllAttachedToRequester : 1;
	
	/** Trace against complex collision. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = TargetSelection)
	uint8 bTraceComplexCollision : 1;
	
	/** Exclude all dead or dying targets */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = TargetSelection)
	uint8 bExcludeDeadOrDying : 1;

	/** Any target whose owning actor is of this type will be excluded. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSet<UClass*> ExcludedClasses;

	/** Targets with any of these tags will be excluded. */
	FGameplayTagContainer ExclusionGameplayTags;

	/** Any target outside of this range will be excluded */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	double TargetRange = 10000.0;
};

/** Settings for how aim assist should behave when there are active targets */
USTRUCT(BlueprintType)
struct FAimAssistSettings
{
	GENERATED_BODY()

	FAimAssistSettings();

	float GetTargetWeightForTime(float Time) const;
	float GetTargetWeightMaxTime() const;
	
	// Width of aim assist inner reticle in world space.
	UPROPERTY(EditAnywhere)
	FScalableFloat AssistInnerReticleWidth;

	// Height of aim assist inner reticle in world space.
	UPROPERTY(EditAnywhere)
	FScalableFloat AssistInnerReticleHeight;

	// Width of aim assist outer reticle in world space.
	UPROPERTY(EditAnywhere)
	FScalableFloat AssistOuterReticleWidth;

	// Height of aim assist outer reticle in world space.
	UPROPERTY(EditAnywhere)
	FScalableFloat AssistOuterReticleHeight;

	// Width of targeting reticle in world space.
	UPROPERTY(EditAnywhere)
	FScalableFloat TargetingReticleWidth;

	// Height of targeting reticle in world space.
	UPROPERTY(EditAnywhere)
	FScalableFloat TargetingReticleHeight;

	// Range from player's camera used to gather potential targets.
	// Note: This is scaled using the field of view in order to limit targets by their screen size.
	UPROPERTY(EditAnywhere)
	FScalableFloat TargetRange;

	// How much weight the target has based on the time it has been targeted.  (0 = None, 1 = Max)
	UPROPERTY(EditAnywhere)
	const UCurveFloat* TargetWeightCurve = nullptr;

	// How much target and player movement contributes to the aim assist pull when target is under the inner reticle. (0 = None, 1 = Max)
	UPROPERTY(EditAnywhere)
	FScalableFloat PullInnerStrengthHip;

	// How much target and player movement contributes to the aim assist pull when target is under the outer reticle. (0 = None, 1 = Max)
	UPROPERTY(EditAnywhere)
	FScalableFloat PullOuterStrengthHip;

	// How much target and player movement contributes to the aim assist pull when target is under the inner reticle. (0 = None, 1 = Max)
	UPROPERTY(EditAnywhere)
	FScalableFloat PullInnerStrengthAds;

	// How much target and player movement contributes to the aim assist pull when target is under the outer reticle. (0 = None, 1 = Max)
	UPROPERTY(EditAnywhere)
	FScalableFloat PullOuterStrengthAds;

	// Exponential interpolation rate used to ramp up the pull strength.  Set to '0' to disable.
	UPROPERTY(EditAnywhere)
	FScalableFloat PullLerpInRate;

	// Exponential interpolation rate used to ramp down the pull strength.  Set to '0' to disable.
	UPROPERTY(EditAnywhere)
	FScalableFloat PullLerpOutRate;

	// Rotation rate maximum cap on amount of aim assist pull.  Set to '0' to disable.
	// Note: This is scaled based on the field of view so it feels the same regardless of zoom.
	UPROPERTY(EditAnywhere)
	FScalableFloat PullMaxRotationRate;

	// Amount of aim assist slow applied to desired turn rate when target is under the inner reticle. (0 = None, 1 = Max)
	UPROPERTY(EditAnywhere)
	FScalableFloat SlowInnerStrengthHip;

	// Amount of aim assist slow applied to desired turn rate when target is under the outer reticle. (0 = None, 1 = Max)
	UPROPERTY(EditAnywhere)
	FScalableFloat SlowOuterStrengthHip;

	// Amount of aim assist slow applied to desired turn rate when target is under the inner reticle. (0 = None, 1 = Max)
	UPROPERTY(EditAnywhere)
	FScalableFloat SlowInnerStrengthAds;

	// Amount of aim assist slow applied to desired turn rate when target is under the outer reticle. (0 = None, 1 = Max)
	UPROPERTY(EditAnywhere)
	FScalableFloat SlowOuterStrengthAds;

	// Exponential interpolation rate used to ramp up the slow strength.  Set to '0' to disable.
	UPROPERTY(EditAnywhere)
	FScalableFloat SlowLerpInRate;

	// Exponential interpolation rate used to ramp down the slow strength.  Set to '0' to disable.
	UPROPERTY(EditAnywhere)
	FScalableFloat SlowLerpOutRate;

	// Rotation rate minimum cap on amount to aim assist slow.  Set to '0' to disable.
	// Note: This is scaled based on the field of view so it feels the same regardless of zoom.
	UPROPERTY(EditAnywhere)
	FScalableFloat SlowMinRotationRate;
	
	/** The maximum number of targets that can be considered during a given frame. */
	UPROPERTY(EditAnywhere)
	int32 MaxNumberOfTargets = 6;

	/**  */
	UPROPERTY(EditAnywhere)
	float ReticleDepth = 3000.0f;

	UPROPERTY(EditAnywhere)
	float TargetScore_AssistWeight = 10.0f;

	UPROPERTY(EditAnywhere)
	float TargetScore_ViewDot = 50.0f;

	UPROPERTY(EditAnywhere)
	float TargetScore_ViewDotOffset = 40.0f;

	UPROPERTY(EditAnywhere)
	float TargetScore_ViewDistance = 0.25f;

	UPROPERTY(EditAnywhere)
	float StrengthScale = 1.0f;

	/** Enabled/Disable asynchronous visibility traces. */
	UPROPERTY(EditAnywhere)
	uint8 bEnableAsyncVisibilityTrace : 1;

	/** Whether or not we require input for aim assist to be applied */
	UPROPERTY(EditAnywhere)
	uint8 bRequireInput : 1;

	/** Whether or not pull should be applied to aim assist */
	UPROPERTY(EditAnywhere)
	uint8 bApplyPull : 1;

	/** Whether or not to apply a strafe pull based off of movement input */
	UPROPERTY(EditAnywhere)
	uint8 bApplyStrafePullScale : 1;
	
	/** Whether or not to apply a slowing effect during aim assist */
	UPROPERTY(EditAnywhere)
	uint8 bApplySlowing : 1;

	/** Whether or not to apply a dynamic slow effect based off of look input */
	UPROPERTY(EditAnywhere)
	uint8 bUseDynamicSlow : 1;

	/** Whether or not look rates should blend between yaw and pitch based on stick deflection using radial look rates */
	UPROPERTY(EditAnywhere)
	uint8 bUseRadialLookRates : 1;
};

/**
 * An input modifier to help gamepad players have better targeting.
 */
UCLASS()
class UAimAssistInputModifier : public UInputModifier
{
	GENERATED_BODY()
	
public:
		
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category=Settings, Config)
	FAimAssistSettings Settings {};

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category=Settings, Config)
	FAimAssistFilter Filter {};

	/** The input action that represents the actual movement of the player */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category=Settings)
	const UInputAction* MoveInputAction = nullptr;
	
	/** The type of targeting to use for this Sensitivity */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category=Settings, Config)
	ELyraTargetingType TargetingType = ELyraTargetingType::Normal;

	/** Asset that gives us access to the float scalar value being used for sensitivty */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(AssetBundles="Client,Server"))
	TObjectPtr<const ULyraAimSensitivityData> SensitivityLevelTable = nullptr;
	
protected:
	
	virtual FInputActionValue ModifyRaw_Implementation(const UEnhancedPlayerInput* PlayerInput, FInputActionValue CurrentValue, float DeltaTime) override;

	/**
	* Swaps the target cache's and determines what targets are currently visible.
	* Updates the score of each target to determine
	* how much pull/slow effect should be applied to each
	*/
	void UpdateTargetData(float DeltaTime);

	FRotator UpdateRotationalVelocity(APlayerController* PC, float DeltaTime, FVector CurrentLookInputValue, FVector CurrentMoveInputValue);

	/** Calcualte the pull and slow strengh of a given target */
	void CalculateTargetStrengths(const FLyraAimAssistTarget& Target, float& OutPullStrength, float& OutSlowStrength) const;

	FRotator GetLookRates(const FVector& LookInput);
	
	void SwapTargetCaches() { TargetCacheIndex ^= 1; }
	const TArray<FLyraAimAssistTarget>& GetPreviousTargetCache() const	{ return ((TargetCacheIndex == 0) ? TargetCache1 : TargetCache0); }
	TArray<FLyraAimAssistTarget>& GetPreviousTargetCache()				{ return ((TargetCacheIndex == 0) ? TargetCache1 : TargetCache0); }

	const TArray<FLyraAimAssistTarget>& GetCurrentTargetCache() const	{ return ((TargetCacheIndex == 0) ? TargetCache0 : TargetCache1); }
	TArray<FLyraAimAssistTarget>& GetCurrentTargetCache()				{ return ((TargetCacheIndex == 0) ? TargetCache0 : TargetCache1); }

	bool HasAnyCurrentTargets() const { return !GetCurrentTargetCache().IsEmpty(); }

	const float GetSensitivtyScalar(const ULyraSettingsShared* SharedSettings) const;
	
	// Tracking of the current and previous frame's targets
	UPROPERTY()
	TArray<FLyraAimAssistTarget> TargetCache0;

	UPROPERTY()
	TArray<FLyraAimAssistTarget> TargetCache1;

	/** The current in use target cache */
	uint32 TargetCacheIndex;

	FAimAssistOwnerViewData OwnerViewData;

	float LastPullStrength = 0.0f;
	float LastSlowStrength = 0.0f;
	
#if ENABLE_DRAW_DEBUG
	float LastLookRateYaw;
	float LastLookRatePitch;

	FVector LastOutValue;
	FVector LastBaselineValue;

	// TODO: Remove this variable and move debug visualization out of this 
	bool bRegisteredDebug = false;

	void AimAssistDebugDraw(class UCanvas* Canvas, APlayerController* PC);
	FDelegateHandle	DebugDrawHandle;
#endif
};
