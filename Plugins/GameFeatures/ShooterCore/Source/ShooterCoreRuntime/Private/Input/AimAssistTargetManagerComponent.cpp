// Copyright Epic Games, Inc. All Rights Reserved.

#include "Input/AimAssistTargetManagerComponent.h"
#include "Input/AimAssistTargetComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/InputSettings.h"
#include "GameFramework/Character.h"
#include "GameFramework/InputSettings.h"
#include "Components/ShapeComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Character/LyraHealthComponent.h"
#include "Player/LyraPlayerState.h"
#include "Character/LyraHealthComponent.h"
#include "ShooterCoreRuntimeSettings.h"
#include "DrawDebugHelpers.h"

namespace LyraConsoleVariables
{
	static bool bDrawDebugViewfinder = false;
	static FAutoConsoleVariableRef CVarDrawDebugViewfinder(
		TEXT("lyra.Weapon.AimAssist.DrawDebugViewfinder"),
		bDrawDebugViewfinder,
		TEXT("Should we draw a debug box for the aim assist target viewfinder?"),
		ECVF_Cheat);
}

const FLyraAimAssistTarget* FindTarget(const TArray<FLyraAimAssistTarget>& Targets, const UShapeComponent* TargetComponent)
{
	const FLyraAimAssistTarget* FoundTarget = Targets.FindByPredicate(
	[&TargetComponent](const FLyraAimAssistTarget& Target)
	{
		return (Target.TargetShapeComponent == TargetComponent);
	});

	return FoundTarget;
}

static bool GatherTargetInfo(const AActor* Actor, const UShapeComponent* ShapeComponent, FTransform& OutTransform, FCollisionShape& OutShape, FVector& OutShapeOrigin)
{
	check(Actor);
	check(ShapeComponent);

	const FCollisionShape TargetShape = ShapeComponent->GetCollisionShape();
	const bool bIsValidShape = (TargetShape.IsBox() || TargetShape.IsSphere() || TargetShape.IsCapsule());

	if (!bIsValidShape || TargetShape.IsNearlyZero())
	{
		return false;
	}

	FTransform TargetTransform;
	FVector TargetShapeOrigin(ForceInitToZero);

	if (const ACharacter* TargetCharacter = Cast<ACharacter>(Actor))
	{
		if (ShapeComponent == TargetCharacter->GetCapsuleComponent())
		{
			// Character capsules don't move smoothly for remote players.  Use the mesh location since it's smoothed out.
			const USkeletalMeshComponent* TargetMesh = TargetCharacter->GetMesh();
			check(TargetMesh);

			TargetTransform = TargetMesh->GetComponentTransform();
			TargetShapeOrigin = -TargetCharacter->GetBaseTranslationOffset();
		}
		else
		{
			TargetTransform = ShapeComponent->GetComponentTransform();
		}
	}
	else
	{
		TargetTransform = ShapeComponent->GetComponentTransform();
	}

	OutTransform = TargetTransform;
	OutShape = TargetShape;
	OutShapeOrigin = TargetShapeOrigin;

	return true;
}


void UAimAssistTargetManagerComponent::GetVisibleTargets(const FAimAssistFilter& Filter, const FAimAssistSettings& Settings, const FAimAssistOwnerViewData& OwnerData, const TArray<FLyraAimAssistTarget>& OldTargets, OUT TArray<FLyraAimAssistTarget>& OutNewTargets)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UAimAssistTargetManagerComponent::GetVisibleTargets);
	OutNewTargets.Reset();
	const APlayerController* PC = OwnerData.PlayerController;
	
	if (!PC)
	{
		UE_LOG(LogAimAssist, Error, TEXT("Invalid player controller passed to GetVisibleTargets!"));
		return;
	}

	const APawn* OwnerPawn = PC->GetPawn();

	if (!OwnerPawn)
	{
		UE_LOG(LogAimAssist, Error, TEXT("Could not find a valid pawn for aim assist!"));
		return;	
	}
	
	const FVector ViewLocation = OwnerData.ViewTransform.GetTranslation();
	const FVector ViewForward = OwnerData.ViewTransform.GetUnitAxis(EAxis::X);

	const float FOVScale = GetFOVScale(PC, ECommonInputType::Gamepad);
	const float InvFieldOfViewScale = (FOVScale > 0.0f) ? (1.0f / FOVScale) : 1.0f;
	const float TargetRange = (Settings.TargetRange.GetValue() * InvFieldOfViewScale);

	// Use the field of view to scale the reticle projection.  This maintains the same reticle size regardless of field of view.
	const float ReticleDepth = (Settings.ReticleDepth * InvFieldOfViewScale);

	// Calculate the bounds of this reticle in screen space
	const FBox2D AssistInnerReticleBounds = OwnerData.ProjectReticleToScreen(Settings.AssistInnerReticleWidth.GetValue(), Settings.AssistInnerReticleHeight.GetValue(), ReticleDepth);
	const FBox2D AssistOuterReticleBounds = OwnerData.ProjectReticleToScreen(Settings.AssistOuterReticleWidth.GetValue(), Settings.AssistOuterReticleHeight.GetValue(), ReticleDepth);
	const FBox2D TargetingReticleBounds = OwnerData.ProjectReticleToScreen(Settings.TargetingReticleWidth.GetValue(), Settings.TargetingReticleHeight.GetValue(), ReticleDepth);

	static TArray<FOverlapResult> OverlapResults;
	// Do a world trace on the Aim Assist channel to get any visible targets
	{
		UWorld* World = GetWorld();
		
		OverlapResults.Reset();

		const FVector PawnLocation = OwnerPawn->GetActorLocation();
		ECollisionChannel AimAssistChannel = GetAimAssistChannel();
		FCollisionQueryParams Params(SCENE_QUERY_STAT(AimAssist_QueryTargetsInRange), true);
		Params.AddIgnoredActor(OwnerPawn);

		// Need to multiply these by 0.5 because MakeBox takes in half extents
		FCollisionShape BoxShape = FCollisionShape::MakeBox(FVector3f(ReticleDepth * 0.5f, Settings.AssistOuterReticleWidth.GetValue() * 0.5f, Settings.AssistOuterReticleHeight.GetValue() * 0.5f));						
		World->OverlapMultiByChannel(OUT OverlapResults, PawnLocation, OwnerData.PlayerTransform.GetRotation(), AimAssistChannel, BoxShape, Params);

#if ENABLE_DRAW_DEBUG && !UE_BUILD_SHIPPING
		if(LyraConsoleVariables::bDrawDebugViewfinder)
		{
			DrawDebugBox(World, PawnLocation, BoxShape.GetBox(), OwnerData.PlayerTransform.GetRotation(), FColor::Red);	
		}
#endif
	}

	// Gather target options from any visibile hit results that implement the IAimAssistTarget interface
	TArray<FAimAssistTargetOptions> NewTargetData;
	{
		for (const FOverlapResult& Overlap : OverlapResults)
		{
			TScriptInterface<IAimAssistTaget> TargetActor(Overlap.GetActor());
			if (TargetActor)
			{
				FAimAssistTargetOptions TargetData;
				TargetActor->GatherTargetOptions(TargetData);
				NewTargetData.Add(TargetData);
			}
			
			TScriptInterface<IAimAssistTaget> TargetComponent(Overlap.GetComponent());
			if (TargetComponent)
			{
				FAimAssistTargetOptions TargetData;
				TargetComponent->GatherTargetOptions(TargetData);
				NewTargetData.Add(TargetData);
			}			
		}
	}
	
	// Gather targets that are in front of the player
	{
		const FVector PawnLocation = OwnerPawn->GetActorLocation();		
		
		for (FAimAssistTargetOptions& AimAssistTarget : NewTargetData)
		{
			if (!DoesTargetPassFilter(OwnerData, Filter, AimAssistTarget, TargetRange))
			{
				continue;
			}
			
			AActor* OwningActor = AimAssistTarget.TargetShapeComponent->GetOwner();

			FTransform TargetTransform;
			FCollisionShape TargetShape;
			FVector TargetShapeOrigin;

			if (!GatherTargetInfo(OwningActor, AimAssistTarget.TargetShapeComponent.Get(), TargetTransform, TargetShape, TargetShapeOrigin))
			{
				continue;
			}
			
			const FVector TargetViewLocation = TargetTransform.TransformPositionNoScale(TargetShapeOrigin);
			const FVector TargetViewVector = (TargetViewLocation - ViewLocation);

			FVector TargetViewDirection;
			float TargetViewDistance;
			TargetViewVector.ToDirectionAndLength(TargetViewDirection, TargetViewDistance);
			const float TargetViewDot = FVector::DotProduct(TargetViewDirection, ViewForward);
			if (TargetViewDot <= 0.0f)
			{
				continue;
			}
			
			const FLyraAimAssistTarget* OldTarget = FindTarget(OldTargets, AimAssistTarget.TargetShapeComponent.Get());

			// Calculate the screen bounds for this target
			FBox2D TargetScreenBounds(ForceInitToZero);
			const bool bUpdateTargetProjections = true;
			if (bUpdateTargetProjections)
			{
				TargetScreenBounds = OwnerData.ProjectShapeToScreen(TargetShape, TargetShapeOrigin, TargetTransform);
			}
			else
			{
				// Target projections are not being updated so use the values from the previous frame if the target existed.
				if (OldTarget)
				{
					TargetScreenBounds = OldTarget->ScreenBounds;
				}
			}

			if (!TargetScreenBounds.bIsValid)
			{
				continue;
			}

			if (!TargetingReticleBounds.Intersect(TargetScreenBounds))
			{
				continue;
			}

			FLyraAimAssistTarget NewTarget;

			NewTarget.TargetShapeComponent = AimAssistTarget.TargetShapeComponent;
			NewTarget.Location = TargetTransform.GetTranslation();
			NewTarget.ScreenBounds = TargetScreenBounds;
			NewTarget.ViewDistance = TargetViewDistance;
			NewTarget.bUnderAssistInnerReticle = AssistInnerReticleBounds.Intersect(TargetScreenBounds);
			NewTarget.bUnderAssistOuterReticle = AssistOuterReticleBounds.Intersect(TargetScreenBounds);
			
			// Transfer target data from last frame.
			if (OldTarget)
			{
				NewTarget.DeltaMovement = (NewTarget.Location - OldTarget->Location);
				NewTarget.AssistTime = OldTarget->AssistTime;
				NewTarget.AssistWeight = OldTarget->AssistWeight;
				NewTarget.VisibilityTraceHandle = OldTarget->VisibilityTraceHandle;
			}

			// Calculate a score used for sorting based on previous weight, distance from target, and distance from reticle.
			const float AssistWeightScore = (NewTarget.AssistWeight * Settings.TargetScore_AssistWeight);
			const float ViewDotScore = ((TargetViewDot * Settings.TargetScore_ViewDot) - Settings.TargetScore_ViewDotOffset);
			const float ViewDistanceScore = ((1.0f - (TargetViewDistance / TargetRange)) * Settings.TargetScore_ViewDistance);

			NewTarget.SortScore = (AssistWeightScore + ViewDotScore + ViewDistanceScore);

			OutNewTargets.Add(NewTarget);
		}
	}

	// Sort the targets by their score so if there are too many so we can limit the amount of visibility traces performed.
	if (OutNewTargets.Num() > Settings.MaxNumberOfTargets)
	{
		OutNewTargets.Sort([](const FLyraAimAssistTarget& TargetA, const FLyraAimAssistTarget& TargetB)
		{
			return (TargetA.SortScore > TargetB.SortScore);
		});
		
		OutNewTargets.SetNum(Settings.MaxNumberOfTargets, false);
	}

	// Do visibliity traces on the targets
	{
		for (FLyraAimAssistTarget& Target : OutNewTargets)
		{
			DetermineTargetVisibility(Target, Settings, Filter, OwnerData);
		}
	}
}

bool UAimAssistTargetManagerComponent::DoesTargetPassFilter(const FAimAssistOwnerViewData& OwnerData, const FAimAssistFilter& Filter, const FAimAssistTargetOptions& Target, const float AcceptableRange) const
{
	const APawn* OwnerPawn = OwnerData.PlayerController ? OwnerData.PlayerController->GetPawn() : nullptr;
	
	if (!Target.bIsActive || !OwnerPawn || !Target.TargetShapeComponent.IsValid())
	{
		return false;
	}
	
	const AActor* TargetOwningActor = Target.TargetShapeComponent->GetOwner();
	check(TargetOwningActor);
	if (TargetOwningActor == OwnerPawn || TargetOwningActor == OwnerPawn->GetInstigator())
	{
		return false;
	}
	
	const FVector PawnLocation = OwnerPawn->GetActorLocation();
	
	// Do a distance check on the given actor
	const FVector TargetVector = TargetOwningActor->GetActorLocation() - PawnLocation;
	const float TargetViewDistanceCheck = FVector::DotProduct(OwnerData.ViewForward, TargetVector);

	if ((TargetViewDistanceCheck < 0.0f) || (TargetViewDistanceCheck > AcceptableRange))
	{
		return false;
	}
	
	if (const ACharacter* TargetCharacter = Cast<ACharacter>(TargetOwningActor))
	{
		// If the given target is on the same team as the owner, then exclude it from the search	
		if (!Filter.bIncludeSameFriendlyTargets)
		{
			if (const ALyraPlayerState* PS = TargetCharacter->GetPlayerState<ALyraPlayerState>())
			{
				if (PS->GetTeamId() == OwnerData.TeamID)
				{
					return false;
				}
			}
		}

		// Exclude dead or dying characters
		if (Filter.bExcludeDeadOrDying)
		{
			if (const ULyraHealthComponent* HealthComponent = ULyraHealthComponent::FindHealthComponent(TargetCharacter))
			{
				if (HealthComponent->IsDeadOrDying())
				{
					return false;
				}
			}	
		}
	}

	// If this target has any tags that the filter wants to exlclude, then ignore it
	if (Target.AssociatedTags.HasAny(Filter.ExclusionGameplayTags))
	{
		return false;
	}

	if (Filter.ExcludedClasses.Contains(TargetOwningActor->GetClass()))
	{
		return false;
	}

	return true;
}

float UAimAssistTargetManagerComponent::GetFOVScale(const APlayerController* PC, ECommonInputType InputType)
{
	float FovScale = 1.0f;
	const UInputSettings* DefaultInputSettings = GetDefault<UInputSettings>();
	check(DefaultInputSettings && PC);

	if (PC->PlayerCameraManager && DefaultInputSettings->bEnableFOVScaling)
	{
		const float FOVAngle = PC->PlayerCameraManager->GetFOVAngle();
		switch (InputType)
		{
		case ECommonInputType::Gamepad:
		case ECommonInputType::Touch:
		{
			static const float PlayerInput_BaseFOV = 80.0f;
			// This is the proper way to scale based off FOV changes.
			// Ideally mouse would use this too but changing it now will cause sensitivity to change for existing players.
			const float BaseHalfFOV = PlayerInput_BaseFOV * 0.5f;
			const float HalfFOV = FOVAngle * 0.5f;
			const float BaseTanHalfFOV = FMath::Tan(FMath::DegreesToRadians(BaseHalfFOV));
			const float TanHalfFOV = FMath::Tan(FMath::DegreesToRadians(HalfFOV));

			check(BaseTanHalfFOV > 0.0f);
			FovScale = (TanHalfFOV / BaseTanHalfFOV);
			break;
		}
		case ECommonInputType::MouseAndKeyboard:
			FovScale = (DefaultInputSettings->FOVScale * FOVAngle);
			break;
		default:
			ensure(false);
			break;
		}
	}
	return FovScale;
}

void UAimAssistTargetManagerComponent::DetermineTargetVisibility(FLyraAimAssistTarget& Target, const FAimAssistSettings& Settings, const FAimAssistFilter& Filter, const FAimAssistOwnerViewData& OwnerData)
{
	UWorld* World = GetWorld();
	check(World);

	const AActor* Actor = Target.TargetShapeComponent->GetOwner();
	if (!Actor)
	{
		ensure(false);
		return;
	}

	FVector TargetEyeLocation;
	FRotator TargetEyeRotation;
	Actor->GetActorEyesViewPoint(TargetEyeLocation, TargetEyeRotation);
	
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(AimAssist_DetermineTargetVisibility), true);
	InitTargetSelectionCollisionParams(QueryParams, *Actor, Filter);
	QueryParams.AddIgnoredActor(Actor);

	const UShooterCoreRuntimeSettings* ShooterSettings = GetDefault<UShooterCoreRuntimeSettings>();
	const ECollisionChannel AimAssistChannel = ShooterSettings->GetAimAssistCollisionChannel();
		
	FCollisionResponseParams ResponseParams;
	ResponseParams.CollisionResponse.SetResponse(ECC_Pawn, ECR_Ignore);	
	ResponseParams.CollisionResponse.SetResponse(AimAssistChannel, ECR_Ignore);

	if (Target.bIsVisible && Settings.bEnableAsyncVisibilityTrace)
	{
		// Query for previous asynchronous trace result.
		if (Target.VisibilityTraceHandle.IsValid())
		{
			FTraceDatum TraceDatum;
			if (World->QueryTraceData(Target.VisibilityTraceHandle, TraceDatum))
			{
				Target.bIsVisible = (FHitResult::GetFirstBlockingHit(TraceDatum.OutHits) == nullptr);
			}
			else
			{
				UE_LOG(LogAimAssist, Warning, TEXT("UAimAssistTargetManagerComponent::DetermineTargetVisibility() - Failed to find async visibility trace data!"));
				Target.bIsVisible = false;
			}

			// Invalidate the async trace handle.
			Target.VisibilityTraceHandle = FTraceHandle();
		}

		// Only start a new asynchronous trace for next frame if the target is still visible.
		if (Target.bIsVisible)
		{
			Target.VisibilityTraceHandle = World->AsyncLineTraceByChannel(EAsyncTraceType::Test, OwnerData.ViewTransform.GetTranslation(), TargetEyeLocation, ECC_Visibility, QueryParams, ResponseParams);
		}
	}
	else
	{
		Target.bIsVisible = !World->LineTraceTestByChannel(OwnerData.ViewTransform.GetTranslation(), TargetEyeLocation, ECC_Visibility, QueryParams, ResponseParams);

		// Invalidate the async trace handle.
		Target.VisibilityTraceHandle = FTraceHandle();		
	}
}

void UAimAssistTargetManagerComponent::InitTargetSelectionCollisionParams(FCollisionQueryParams& OutParams, const AActor& RequestedBy, const FAimAssistFilter& Filter) const
{
	// Exclude Requester
	if (Filter.bExcludeRequester)
	{
		OutParams.AddIgnoredActor(&RequestedBy);
	}

	// Exclude attached to Requester
	if (Filter.bExcludeAllAttachedToRequester)
	{
		TArray<AActor*> ActorsAttachedToRequester;
		RequestedBy.GetAttachedActors(ActorsAttachedToRequester);

		OutParams.AddIgnoredActors(ActorsAttachedToRequester);
	}

	if (Filter.bExcludeInstigator)
	{
		OutParams.AddIgnoredActor(RequestedBy.GetInstigator());
	}

	// Exclude attached to Instigator
	if (Filter.bExcludeAllAttachedToInstigator && RequestedBy.GetInstigator())
	{
		TArray<AActor*> ActorsAttachedToInstigator;
		RequestedBy.GetInstigator()->GetAttachedActors(ActorsAttachedToInstigator);

		OutParams.AddIgnoredActors(ActorsAttachedToInstigator);
	}

	OutParams.bTraceComplex = Filter.bTraceComplexCollision;
}

ECollisionChannel UAimAssistTargetManagerComponent::GetAimAssistChannel() const
{
	const UShooterCoreRuntimeSettings* ShooterSettings = GetDefault<UShooterCoreRuntimeSettings>();
	const ECollisionChannel AimAssistChannel = ShooterSettings->GetAimAssistCollisionChannel();

	ensureMsgf(AimAssistChannel != ECollisionChannel::ECC_MAX, TEXT("The aim assist collision channel has not been set! Do this in the ShooterCoreRuntime plugin settings"));
	
	return AimAssistChannel;
}